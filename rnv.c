/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <fcntl.h>  /*open,close*/
#include UNISTD_H   /*open,read,close*/
#include <stdio.h>  /*fprintf,stderr*/
#include <string.h> /*strerror,strncpy,strrchr*/
#include <errno.h>
#include EXPAT_H
#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnx.h"
#include "drv.h"

#define LEN_T 1024

static char *xml;
static XML_Parser expat;
static int start,current,previous;
static int errors;
static int explain=1;

/* Expat does not normalize strings on input unless the whole file is loaded into the buffer */
static char *text; static int len_t;
static int n_t;

static void init(void) {
  rn_init();
  rnc_init();
  rnd_init();
  rnx_init();
  drv_init();
  text=(char*)calloc(len_t=LEN_T,sizeof(char));
  n_t=0;
}

static int load_rnc(char *fn) {
  struct rnc_source *sp=rnc_alloc();
  if(rnc_open(sp,fn)!=-1) start=rnc_parse(sp); rnc_close(sp); 
  rnc_free(sp); if(rnc_errors(sp)) return 0;
  
  rnd_deref(start); if(rnd_errors()) return 0;
  rnd_restrictions(); if(rnd_errors()) return 0;
  rnd_traits();

  start=rnd_release(); 
  return 1;
}

static void error(char *msg) {
  char *s;
  ++errors;
  fprintf(stderr,"invalid document (%s,%i,%i): %s",xml,XML_GetCurrentLineNumber(expat),XML_GetCurrentColumnNumber(expat),msg);
  if(explain) {
    rnx_expected(previous);
    if(rnx_n_exp!=0) {
      int i;
      fprintf(stderr,"\nexpected:");
      for(i=0;i!=rnx_n_exp;++i) {
	fprintf(stderr,"\n\t%s",s=p2str(rnx_exp[i]));
	free(s);
      }
    }
  }
  fprintf(stderr,"\n");
}

static char *suri=NULL,*sname;
static int len_suri=-1;

static void qname(char *name) {
  char *sep; int len;
  if((sep=strrchr(name,':'))) sname=sep+1; else sep=sname=name;
  len=sep-name+1;
  if(len>len_suri) {len_suri=len; free(suri); suri=(char*)calloc(len_suri,sizeof(char));}
  strncpy(suri,name,len-1);
  suri[len-1]='\0';
}

static char *msg=NULL;
static int len_msg=-1;

static void not_allowed(char *what,char *suri,char *sname) {
  int len=strlen(what)+strlen(suri)+strlen(sname);
  if(len>len_msg) {len_msg=len; free(msg); msg=(char*)calloc(len_msg,sizeof(char));}
  msg[sprintf(msg,what,suri,sname)]='\0';
  error(msg);
}

#define ELEMENT_NOT_ALLOWED "element '%s^%s' not allowed"
#define ATTRIBUTE_NOT_ALLOWED "attribute '%s' not allowed"
#define ELEMENTS_MISSING "required elements missing"
#define ATTRIBUTES_MISSING "required attributes missing"
#define TEXT_NOT_ALLOWED "text not allowed"

static void flush_text(void) {
  if(n_t!=0) {
    if(current!=rn_notAllowed) {
      current=drv_text(previous=current,text,n_t);
      if(current==rn_notAllowed) {
	error(TEXT_NOT_ALLOWED);
	current=drv_text_recover(previous,text,n_t);
      }
    }
    n_t=0;
  }
}

static void start_element(void *userData,const char *name,const char **attrs) {
  flush_text();
  if(current!=rn_notAllowed) { 
    qname((char*)name);
    current=drv_start_tag_open(previous=current,suri,sname);
    if(current==rn_notAllowed) {
      not_allowed(ELEMENT_NOT_ALLOWED,suri,sname);
      current=drv_start_tag_open_recover(previous,suri,sname);
    }
    while(current!=rn_notAllowed) {
      if(!(*attrs)) break;
      qname((char*)*attrs);
      ++attrs;
      current=drv_attribute(previous=current,suri,sname,(char*)*attrs);
      if(current==rn_notAllowed) {
	not_allowed(ATTRIBUTE_NOT_ALLOWED,suri,sname);
	current=drv_attribute_recover(previous,suri,sname,(char*)*attrs);
      }
      ++attrs;
    }
    if(current!=rn_notAllowed) {
      current=drv_start_tag_close(previous=current);
      if(current==rn_notAllowed) {
	error(ATTRIBUTES_MISSING);
	current=drv_start_tag_close_recover(previous);
      }
    }
  }
}

static void end_element(void *userData,const char *name) {
  flush_text();
  if(current!=rn_notAllowed) {
    current=drv_end_tag(previous=current);
    if(current==rn_notAllowed) {
      error(ELEMENTS_MISSING);
      current=drv_end_tag_recover(previous);
    }
  }
}

static void characters(void *userData,const char *s,int len) {
  if(current!=rn_notAllowed) {
    if(n_t+len>len_t) {
      char *newtext=(char*)calloc(n_t+len,sizeof(char)); 
      memcpy(newtext,text,n_t*sizeof(char)); free(text); 
      text=newtext;
    }
    strncpy(text+n_t,s,len); n_t+=len;
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
static void usage(void) {fprintf(stderr,"usage: rnv {-q|-v} schema.rnc {document.xml}\n");}

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
      case 'q': explain=0; break;
      default: fprintf(stderr,"unknown option '-%c'\n",*(*argv+i)); break;
      }
      ++i;
    }
    END_OF_OPTIONS:
  }

  if(!*(argv)) {
    usage();
    return 1;
  }

  ok=0;
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
