/* $Id$ */

#include <stdarg.h>
#include <stdlib.h> /*calloc,free*/
#include <fcntl.h>  /*open,close*/
#include UNISTD_H   /*open,read,close*/
#include <stdio.h>  /*fprintf,stderr*/
#include <string.h> /*strerror,strncpy*/
#include <errno.h>
#include EXPAT_H
#include "erbit.h"
#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnv.h"
#include "rnx.h"
#include "ll.h"

extern int rx_compact,drv_compact;

#define LEN_T XCL_LEN_T
#define LIM_T XCL_LIM_T

/* maximum number of candidates to display */
#define NEXP 6

#define XCL_ER_IO 0
#define XCL_ER_XML 1

static void 
  (*rncverror0)(int erno,va_list ap),
  (*rndverror0)(int erno,va_list ap),
  (*rnvverror0)(int erno,va_list ap);

static int peipe,explain;
static char *xml;
static XML_Parser expat=NULL;
static int start,current,previous;
static int mixed=0;
static int lastline,lastcol,level;
static int errors;

/* Expat does not normalize strings on input unless the whole file is loaded into the buffer */
static char *text; static int len_t;
static int n_t;

#define err(msg) vfprintf(stderr,msg,ap);
static void verror_handler(int erno,va_list ap) {
  if(erno&ERBIT_RNC) {
    (*rncverror0)(erno&~ERBIT_RNC,ap);
  } else if(erno&ERBIT_RNC) {
    (*rncverror0)(erno&~ERBIT_RNC,ap);
  } else {
    int line=XML_GetCurrentLineNumber(expat),col=XML_GetCurrentColumnNumber(expat);
    if(line!=lastline||col!=lastcol) {
      char *s;
      fprintf(stderr,"error (%s,%i,%i): ",xml,lastline=line,lastcol=col);
      if(erno&ERBIT_RNV) {
	(*rnvverror0)(erno&~ERBIT_RNV,ap);
	if(explain) {
	  rnx_expected(previous);
	  if(rnx_n_exp!=0 && rnx_n_exp<=NEXP) {
	    int i;
	    fprintf(stderr,"expected:\n");
	    for(i=0;i!=rnx_n_exp;++i) {
	      fprintf(stderr,"\t%s\n",s=rnx_p2str(rnx_exp[i]));
	      free(s);
	    }
	  }
	}
      } else {
	switch(erno) {
	case XCL_ER_IO: err("%s"); break;
	case XCL_ER_XML: err("%s"); break;
	default: assert(0);
	}                
      }
    }
  }
}

static void verror_handler_rnc(int erno,va_list ap) {verror_handler(erno|ERBIT_RNC,ap);}
static void verror_handler_rnd(int erno,va_list ap) {verror_handler(erno|ERBIT_RND,ap);}
static void verror_handler_rnv(int erno,va_list ap) {verror_handler(erno|ERBIT_RNV,ap);}

static void windup(void);
static int initialized=0;
static void init(void) {
  if(!initialized) {initialized=1;
    rn_init();
    rnc_init(); rncverror0=rnc_verror_handler; rnc_verror_handler=&verror_handler_rnc;
    rnd_init(); rndverror0=rnd_verror_handler; rnd_verror_handler=&verror_handler_rnd;
    rnv_init(); rnvverror0=rnv_verror_handler; rnv_verror_handler=&verror_handler_rnv;
    rnx_init();
    text=(char*)calloc(len_t=LEN_T,sizeof(char));
    windup();
  }
}

static void clear(void) {
  if(len_t>LIM_T) {free(text); text=(char*)calloc(len_t=LEN_T,sizeof(char));}
  windup();
}

static void windup(void) {
  text[n_t=0]='\0';
  errors=0; level=0; lastline=lastcol=-1;
}

static int load_rnc(char *fn) {
  struct rnc_source *sp=rnc_alloc();
  if(rnc_open(sp,fn)!=-1) start=rnc_parse(sp); rnc_close(sp); 
  {int errors=rnc_errors(sp); rnc_free(sp); if(errors) return 0;}
  
  rnd_deref(start); if(rnd_errors()) return 0;
  rnd_restrictions(); if(rnd_errors()) return 0;
  rnd_traits();
  start=rnd_release(); 

  start=rn_compress_last(start);

  return 1;
}

static void error(int erno,...) {
  va_list ap; va_start(ap,erno); verror_handler(erno,ap); va_end(ap);
  ++errors;
}

static void flush_text(void) {
  current=rnv_text(previous=current,text,n_t,mixed);
  text[n_t=0]='\0';
}

static void start_element(void *userData,const char *name,const char **attrs) {
  if(current!=rn_notAllowed) { 
    mixed=1;
    flush_text();
    current=rnv_start_tag(previous=current,(char*)name,(char**)attrs);
    mixed=0;
  } else {
    ++level;
  }
}

static void end_element(void *userData,const char *name) {
  if(current!=rn_notAllowed) {
    flush_text(); 
    current=rnv_end_tag(previous=current,(char*)name);
    mixed=1;
  } else {
    if(level==0) current=previous; else --level;
  }
}

static void characters(void *userData,const char *s,int len) {
  if(current!=rn_notAllowed) {
    int newlen_t=n_t+len+1;
    if(newlen_t<=LIM_T&&LIM_T<len_t) newlen_t=LIM_T; 
    else if(newlen_t<len_t) newlen_t=len_t;
    if(len_t!=newlen_t) {
      char *newtext=(char*)calloc(len_t=newlen_t,sizeof(char)); 
      memcpy(newtext,text,n_t*sizeof(char)); free(text); 
      text=newtext;
    }
    memcpy(text+n_t,s,len); n_t+=len; text[n_t]='\0'; /* '\0' guarantees that the text is bounded, and strto[ld] work for data */
  }
}

static int pipeout(void *buf,int len) {
  int ofs=0,iw,lenw=len;
  for(;;) {
    if((iw=write(1,(char*)buf+ofs,lenw))==-1) {error(XCL_ER_IO,strerror(errno)); return 0;}
    ofs+=iw; lenw-=iw; if(lenw==0) return 1;
  }
}

static int validate(int fd) {
  void *buf; int len;
  previous=current=start;

  expat=XML_ParserCreateNS(NULL,':');
  XML_SetElementHandler(expat,&start_element,&end_element);
  XML_SetCharacterDataHandler(expat,&characters);
  for(;;) {
    buf=XML_GetBuffer(expat,BUFSIZ);
    len=read(fd,buf,BUFSIZ);
    if(len<0) {
      error(XCL_ER_IO,strerror(errno));
      goto ERROR;
    }
    if(peipe) peipe=peipe&&pipeout(buf,len);
    if(!XML_ParseBuffer(expat,len,len==0)) goto PARSE_ERROR;
    if(len==0) break;
  }
  XML_ParserFree(expat);
  return !errors;

PARSE_ERROR:
  error(XCL_ER_XML,XML_ErrorString(XML_GetErrorCode(expat)));
  while(peipe&&(len=read(fd,buf,BUFSIZ))!=0) peipe=peipe&&pipeout(buf,len);
ERROR:
  return 0;
}

static void version(void) {fprintf(stderr,"rnv version %s\n",RNV_VERSION);}
static void usage(void) {fprintf(stderr,"usage: rnv {-[qpsvh?]} schema.rnc {document.xml}\n");}

int main(int argc,char **argv) {
  int ok;

  init();

  peipe=0; explain=1;
  while(*(++argv)&&**argv=='-') {
    int i=1;
    for(;;) {
      switch(*(*argv+i)) {
      case '\0': goto END_OF_OPTIONS;
      case 'h': case '?': usage(); return 1;
      case 'v': version(); break;
      case 's': drv_compact=1; rx_compact=1; break;
      case 'p': peipe=1; break;
      case 'q': explain=0; break;
      default: fprintf(stderr,"unknown option '-%c'\n",*(*argv+i)); break;
      }
      ++i;
    }
    END_OF_OPTIONS:;
  }

  if(!*(argv)) {
    usage();
    return 1;
  }

  if((ok=load_rnc(*(argv++)))) {
    if(*argv) {
      do {
	int fd; xml=*argv;
	if((fd=open(xml,O_RDONLY))==-1) {
	  fprintf(stderr,"I/O error (%s): %s\n",xml,strerror(errno));
	  ok=0;
	  continue;
	}
	if(explain) fprintf(stderr,"%s\n",xml);
	ok=validate(fd)&&ok;
	close(fd);
	clear();
      } while(*(++argv));
    } else { /* stdin */
      xml="stdin";
      ok=validate(0)&&ok;
      clear();
    }
  }

  return !ok;
}
