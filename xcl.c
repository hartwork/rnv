/* $Id$ */

#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>  /*open,close*/
#include <sys/types.h>
#include UNISTD_H   /*open,read,close*/
#include <string.h> /*strerror*/
#include <errno.h>
#include <assert.h>
#include EXPAT_H
#include "m.h"
#include "s.h"
#include "erbit.h"
#include "drv.h"
#include "rnl.h"
#include "rnv.h"
#include "rnx.h"
#include "ll.h"
#include "dxl.h"
#include "dsl.h"
#include "er.h"

extern int rn_notAllowed,rx_compact,drv_compact;

#define LEN_T XCL_LEN_T
#define LIM_T XCL_LIM_T

#define BUFSIZE 1024

/* maximum number of candidates to display */
#define NEXP 16

#define XCL_ER_IO 0
#define XCL_ER_XML 1
#define XCL_ER_NOXENT 2
#define XCL_ER_NOSID 3

static int peipe,verbose,nexp,rnck;
static char *xml;
static XML_Parser expat=NULL;
static int start,current,previous;
static int mixed=0;
static int lastline,lastcol,level;
static int ok;

/* Expat does not normalize strings on input */
static char *text; static int len_txt;
static int n_txt;

#define err(msg) (*er_vprintf)(msg"\n",ap);
static void verror_handler(int erno,va_list ap) {
  if(erno&ERBIT_RNL) {
    rnl_default_verror_handler(erno&~ERBIT_RNL,ap);
  } else {
    int line=XML_GetCurrentLineNumber(expat),col=XML_GetCurrentColumnNumber(expat);
    if(line!=lastline||col!=lastcol) {
      (*er_printf)("error (%s,%i,%i): ",xml,lastline=line,lastcol=col);
      if(erno&ERBIT_RNV) { int mexp=nexp;
	rnv_default_verror_handler(erno&~ERBIT_RNV,ap);
	if(mexp) { int req=2, i; char *s;
	  while(req--) {
            rnx_expected(previous,req);
	    if(rnx_n_exp==0) continue;
	    if(rnx_n_exp>mexp) break; mexp-=rnx_n_exp;
            (*er_printf)(req?"required:\n":"expected:\n");
            for(i=0;i!=rnx_n_exp;++i) {
              (*er_printf)("\t%s\n",s=rnx_p2str(rnx_exp[i]));
              m_free(s);
            }
	  }
	}
      } else {
	switch(erno) {
	case XCL_ER_IO: err("%s"); break;
	case XCL_ER_XML: err("%s"); break;
	case XCL_ER_NOXENT: err("cannot open external entity '%s': %s"); break;
	case XCL_ER_NOSID: err("cannot process external entity without systemId"); break;
	default: assert(0);
	}
      }
    }
  }
}

static void verror_handler_rnl(int erno,va_list ap) {verror_handler(erno|ERBIT_RNL,ap);}
static void verror_handler_rnv(int erno,va_list ap) {verror_handler(erno|ERBIT_RNV,ap);}

static void windup(void);
static int initialized=0;
static void init(void) {
  if(!initialized) {initialized=1;
    rnl_init(); rnl_verror_handler=&verror_handler_rnl;
    rnv_init(); rnv_verror_handler=&verror_handler_rnv;
    rnx_init();
    drv_add_dtl(DXL_URL,&dxl_equal,&dxl_allows);
    drv_add_dtl(DSL_URL,&dsl_equal,&dsl_allows);
    text=(char*)m_alloc(len_txt=LEN_T,sizeof(char));
    windup();
  }
}

static void clear(void) {
  if(len_txt>LIM_T) {m_free(text); text=(char*)m_alloc(len_txt=LEN_T,sizeof(char));}
  windup();
}

static void windup(void) {
  text[n_txt=0]='\0';
  level=0; lastline=lastcol=-1;
}

static void error_handler(int erno,...) {
  va_list ap; va_start(ap,erno); verror_handler(erno,ap); va_end(ap);
}

static void flush_text(void) {
  ok=rnv_text(&current,&previous,text,n_txt,mixed)&&ok;
  text[n_txt=0]='\0';
}

static void start_element(void *userData,const char *name,const char **attrs) {
  if(current!=rn_notAllowed) {
    mixed=1;
    flush_text();
    ok=rnv_start_tag(&current,&previous,(char*)name,(char**)attrs)&&ok;
    mixed=0;
  } else {
    ++level;
  }
}

static void end_element(void *userData,const char *name) {
  if(current!=rn_notAllowed) {
    flush_text();
    ok=rnv_end_tag(&current,&previous,(char*)name)&&ok;
    mixed=1;
  } else {
    if(level==0) current=previous; else --level;
  }
}

static void characters(void *userData,const char *s,int len) {
  if(current!=rn_notAllowed) {
    int newlen_txt=n_txt+len+1;
    if(newlen_txt<=LIM_T&&LIM_T<len_txt) newlen_txt=LIM_T;
    else if(newlen_txt<len_txt) newlen_txt=len_txt;
    if(len_txt!=newlen_txt) text=(char*)m_stretch(text,len_txt=newlen_txt,n_txt,sizeof(char));
    memcpy(text+n_txt,s,len); n_txt+=len; text[n_txt]='\0'; /* '\0' guarantees that the text is bounded, and strto[ld] work for data */
  }
}

static int pipeout(void *buf,int len) {
  int ofs=0,iw,lenw=len;
  for(;;) {
    if((iw=write(1,(char*)buf+ofs,lenw))==-1) {error_handler(XCL_ER_IO,strerror(errno)); return 0;}
    ofs+=iw; lenw-=iw; if(lenw==0) return 1;
  }
}

static int process(int fd) {
  void *buf; int len;
  for(;;) {
    buf=XML_GetBuffer(expat,BUFSIZE);
    len=read(fd,buf,BUFSIZE);
    if(len<0) {
      error_handler(XCL_ER_IO,xml,strerror(errno));
      goto ERROR;
    }
    if(peipe) peipe=peipe&&pipeout(buf,len);
    if(!XML_ParseBuffer(expat,len,len==0)) goto PARSE_ERROR;
    if(len==0) break;
  }
  return 1;

PARSE_ERROR:
  error_handler(XCL_ER_XML,XML_ErrorString(XML_GetErrorCode(expat)));
  while(peipe&&(len=read(fd,buf,BUFSIZE))!=0) peipe=peipe&&pipeout(buf,len);
ERROR:
  return 0;
}

static int externalEntityRef(XML_Parser p,const char *context, const char *base,const char *systemId,const char *publicId) {
  ok=0;
  if(systemId) {    
    int fd; char *entity;
    assert(base);
    entity=(char*)m_alloc(strlen(base)+strlen(systemId)+2,1);
    strcpy(entity,systemId); s_abspath(entity,(char*)base);
    if((fd=open(entity,O_RDONLY))==-1) {
      error_handler(XCL_ER_NOXENT,entity,strerror(errno));
    } else { 
      XML_Parser expat0=expat; char *xml0=xml; xml=entity;
      expat=XML_ExternalEntityParserCreate(expat0,context,NULL);
      ok=process(fd);
      XML_ParserFree(expat); 
      xml=xml0; expat=expat0;
      close(fd);
    }
    m_free(entity);
  } else {
    error_handler(XCL_ER_NOSID);
  }
  return 1;
}

static void validate(int fd) {
  previous=current=start;
  expat=XML_ParserCreateNS(NULL,':');
  XML_SetElementHandler(expat,&start_element,&end_element);
  XML_SetCharacterDataHandler(expat,&characters);
  XML_SetExternalEntityRefHandler(expat,&externalEntityRef);
  XML_SetBase(expat,xml);
  ok=process(fd);
  XML_ParserFree(expat);
}

static void version(void) {(*er_printf)("rnv version %s\n",RNV_VERSION);}
static void usage(void) {(*er_printf)("usage: rnv {-[qnpsc"
#if DXL_EXC
"d"
#endif
#if DSL_SCM
"e"
#endif
"vh?]} schema.rnc {document.xml}\n");}

int main(int argc,char **argv) {
  init();

  peipe=0; verbose=1; nexp=NEXP; rnck=0;
  while(*(++argv)&&**argv=='-') {
    int i=1;
    for(;;) {
      switch(*(*argv+i)) {
      case '\0': goto END_OF_OPTIONS;
      case 'h': case '?': usage(); return 1;
      case 'v': version(); break;
      case 's': drv_compact=1; rx_compact=1; break;
      case 'p': peipe=1; break;
      case 'c': rnck=1; break;
#if DXL_EXC
      case 'd': dxl_cmd=*(argv+1); if(*(argv+1)) ++argv; goto END_OF_OPTIONS;
#endif
#if DSL_SCM
      case 'e': dsl_ld(*(argv+1)); if(*(argv+1)) ++argv; goto END_OF_OPTIONS;
#endif
      case 'n': if(*(argv+1)) nexp=atoi(*(++argv)); goto END_OF_OPTIONS;
      case 'q': verbose=0; nexp=0; break;
      default: (*er_printf)("unknown option '-%c'\n",*(*argv+i)); break;
      }
      ++i;
    }
    END_OF_OPTIONS:;
  }

  if(!*(argv)) {usage(); return 1;}

  if((ok=start=rnl_fn(*(argv++)))) {
    if(*argv) {
      do {
	int fd; xml=*argv;
	if((fd=open(xml,O_RDONLY))==-1) {
	  (*er_printf)("I/O error (%s): %s\n",xml,strerror(errno));
	  ok=0;
	  continue;
	}
	if(verbose) (*er_printf)("%s\n",xml);
	validate(fd);
	close(fd);
	clear();
      } while(*(++argv));
      if(!ok&&verbose) (*er_printf)("error: some documents are invalid\n");
    } else {
      if(!rnck) {
	xml="stdin";
	validate(0);
	clear();
	if(!ok&&verbose) (*er_printf)("error: invalid input\n");
      }
    }
  }

  return ok?EXIT_SUCCESS:EXIT_FAILURE;
}
