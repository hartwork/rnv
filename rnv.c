/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <fcntl.h>  /*open,close*/
#include UNISTD_H   /*open,read,close*/
#include <stdio.h>  /*fprintf,stderr*/
#include <string.h> /*strerror,strncpy,strrchr*/
#include <errno.h>
#include EXPAT_H
#include "xmlc.h" /*xmlc_white_space*/
#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnx.h"
#include "drv.h"
#include "rx.h"
#include "ll.h"

#define LEN_T RNV_LEN_T
#define LIM_T RNV_LIM_T

#define RNVER_ELEM 0
#define RNVER_AKEY 1
#define RNVER_AVAL 2
#define RNVER_EMIS 3
#define RNVER_AMIS 4
#define RNVER_UFIN 5
#define RNVER_TEXT 6
#define RNVER_MIXT 7

static char *xml;
static XML_Parser expat;
static int start,current,previous;
static int mixed=0;
static int errors;
static int explain=1;

/* Expat does not normalize strings on input unless the whole file is loaded into the buffer */
static char *text; static int len_t;
static int n_t;

static char *suri=NULL,*sname; static int len_suri=-1; /* qname() splits, handlers use */

static void init(void) {
  rn_init();
  rnc_init();
  rnd_init();
  rnx_init();
  drv_init();
  text=(char*)calloc(len_t=LEN_T,sizeof(char));
  text[n_t=0]='\0';
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

#define err(msg) vfprintf(stderr,msg,ap);

static void error(int erno,...) {
  char *s;
  va_list ap;
  ++errors;
  fprintf(stderr,"invalid document (%s,%i,%i): ",xml,XML_GetCurrentLineNumber(expat),XML_GetCurrentColumnNumber(expat));
  va_start(ap,erno);
  switch(erno) {
  case RNVER_ELEM: err("element %s^%s not allowed"); break;
  case RNVER_AKEY: err("attribute %s^%s not allowed"); break;
  case RNVER_AVAL: err("attribute %s^%s with invalid value \"%s\""); break;
  case RNVER_EMIS: err("incomplete content"); break;
  case RNVER_AMIS: err("missing attributes of %s^%s"); break;
  case RNVER_UFIN: err("unfinished content"); break;
  case RNVER_TEXT: err("invalid text or data"); break;
  case RNVER_MIXT: err("text not allowed"); break;
  default: assert(0);
  }                
  va_end(ap);
  if(explain) {
    rnx_expected(previous);
    if(rnx_n_exp!=0) {
      int i;
      fprintf(stderr,"\nexpected:");
      for(i=0;i!=rnx_n_exp;++i) {
	fprintf(stderr,"\n\t%s",s=rnx_p2str(rnx_exp[i]));
	free(s);
      }
    }
  }
  fprintf(stderr,"\n");
}

static void qname(char *name) {
  char *sep; int len;
  if((sep=strrchr(name,':'))) sname=sep+1; else sep=sname=name;
  len=sep-name+1;
  if(len>len_suri) {len_suri=len; free(suri); suri=(char*)calloc(len_suri,sizeof(char));}
  strncpy(suri,name,len-1);
  suri[len-1]='\0';
}

static int whitespace(void) {
  char *s=text,*end=text+n_t;
  for(;;) {
    if(s==end) return 1;
    if(!xmlc_white_space(*(s++))) return 0;
  }
}

static void flush_text(void) {
  if(mixed) {
    if(!whitespace()) {
      current=drv_mixed_text(previous=current);
      if(current==rn_notAllowed) {
	current=drv_mixed_text_recover(previous);
	error(RNVER_MIXT);
      }
    }
  } else {
    current=drv_text(previous=current,text,n_t);
    if(current==rn_notAllowed) {
      current=drv_text_recover(previous,text,n_t);
      error(RNVER_TEXT);
    }
  }
  text[n_t=0]='\0';
}

static int level=0;

static void start_element(void *userData,const char *name,const char **attrs) {
  if(current!=rn_notAllowed) { 
    mixed=1;
    flush_text();
    qname((char*)name);
    current=drv_start_tag_open(previous=current,suri,sname);
    if(current==rn_notAllowed) {
      current=drv_start_tag_open_recover(previous,suri,sname);
      error(current==rn_notAllowed?RNVER_ELEM:RNVER_EMIS,suri,sname); 
    }
    while(current!=rn_notAllowed) {
      if(!(*attrs)) break;
      qname((char*)*attrs);
      current=drv_attribute_open(previous=current,suri,sname);
      ++attrs;
      if(current==rn_notAllowed) {
	current=drv_attribute_open_recover(previous,suri,sname);
	error(RNVER_AKEY,suri,name);
      } else {
	current=drv_text(previous=current,(char*)*attrs,strlen(*attrs));
	if(current==rn_notAllowed || (current=drv_attribute_close(previous=current))==rn_notAllowed) {
	  current=drv_attribute_close_recover(previous);
	  error(RNVER_AVAL,suri,sname,*attrs);
	}
      }
      ++attrs;
    }
    if(current!=rn_notAllowed) {
      current=drv_start_tag_close(previous=current);
      if(current==rn_notAllowed) {
	current=drv_start_tag_close_recover(previous);
	qname((char*)name);
	error(RNVER_AMIS,suri,sname);
      }
    }
    mixed=0;
  } else {
    ++level;
  }
}

static void end_element(void *userData,const char *name) {
  if(current!=rn_notAllowed) {
    flush_text(); 
    current=drv_end_tag(previous=current);
    if(current==rn_notAllowed) {
      error(RNVER_UFIN);
      current=drv_end_tag_recover(previous);
    }
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
    strncpy(text+n_t,s,len); n_t+=len; text[n_t]='\0'; /* '\0' guarantees that the text is bounded, and strto[ld] work for data */
  }
}

static int validate(int fd) {
  void *buf; int len;
  previous=current=start;
  errors=0;

  expat=XML_ParserCreateNS(NULL,':');
  XML_SetElementHandler(expat,&start_element,&end_element);
  XML_SetCharacterDataHandler(expat,&characters);
  for(;;) {
    buf=XML_GetBuffer(expat,BUFSIZ);
    len=read(fd,buf,BUFSIZ);
    if(len<0) {
      fprintf(stderr,"I/O error (%s): %s\n",xml,strerror(errno));
      goto ERROR;
    }
    if(len==0) {
      if(!XML_ParseBuffer(expat,len,1)) goto PARSE_ERROR;
      break;
    } else {
      if(!XML_ParseBuffer(expat,len,0)) goto PARSE_ERROR;
      if(current==rn_notAllowed) break;
    }
  }
  XML_ParserFree(expat);
  return !errors;

PARSE_ERROR:
  fprintf(stderr,"%s (%s,%i,%i)\n",XML_ErrorString(XML_GetErrorCode(expat)),xml,XML_GetCurrentLineNumber(expat),XML_GetCurrentColumnNumber(expat));
ERROR:
  return 0;
}

static void version(void) {fprintf(stderr,"rnv version %s\n",RNV_VERSION);}
static void usage(void) {fprintf(stderr,"usage: rnv {-[qsvh?]} schema.rnc {document.xml}\n");}

int main(int argc,char **argv) {
  int ok;

  init();

  while(*(++argv)&&**argv=='-') {
    int i=1;
    for(;;) {
      switch(*(*argv+i)) {
      case '\0': goto END_OF_OPTIONS;
      case 'h': case '?': usage(); return 1;
      case 'v': version(); break;
      case 's': drv_compact=1; rx_compact=1; break;
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
	ok=validate(fd)&&ok;
	close(fd);
      } while(*(++argv));
    } else { /* stdin */
      xml="stdin";
      ok=validate(0)&&ok;
    }
  }

  return !ok;
}
