/* $Id$ */

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h> /*strerror,strncpy,strrchr*/
#include <errno.h>
#include <xmlparse.h>
#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnx.h"
#include "drv.h"

#define RNV_VERSION "1.0.0"

static char *xml;
static XML_Parser expat;
static int start,current,previous;
static int errors;

static void init() {
  rn_init();
  rnc_init();
  rnd_init();
  drv_init();
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

static void error() {
  char *s;
  ++errors;
  rnx_expected(previous);
  if(rnx_n_exp!=0) {
    int i;
    fprintf(stderr,"invalid document (%s,%i,%i)\nexpected:\n",xml,XML_GetCurrentLineNumber(expat),XML_GetCurrentColumnNumber(expat));
    for(i=0;i!=rnx_n_exp;++i) {
      fprintf(stderr,"\t%s\n",s=p2str(rnx_exp[i]));
      free(s);
    }
  }
}

static char *suri=NULL,*sname;
static int len_suri=-1;

static void qname(char *name) {
  char *sep,len;
  sep=strrchr(name,':'); 
  if(sep) {
    sname=sep+1;
  } else {
    sep=sname=name;
  }
  len=sep-name+1;
  if(len>len_suri) { len_suri=len;
    free(suri); suri=(char*)calloc(len_suri,sizeof(char));
  }
  strncpy(suri,name,sep-name);
  suri[sep-name]='\0';
}

static void start_element(void *userData,const char *name,const char **attrs) {
  if(current!=rn_notAllowed) { 
    qname((char*)name);
    current=drv_start_tag_open(previous=current,suri,sname);
    if(current==rn_notAllowed) {
      error();
      current=drv_start_tag_open_recover(previous,suri,sname);
    }
    while(current) {
      if(!(*attrs)) break;
      qname((char*)*attrs);
      ++attrs;
      current=drv_attribute(previous=current,suri,sname,(char*)*attrs);
      if(current==rn_notAllowed) {
	error();
	current=drv_attribute_recover(previous,suri,sname,(char*)*attrs);
      }
      ++attrs;
    }
    if(current) {
      current=drv_start_tag_close(previous=current);
      if(current==rn_notAllowed) {
	error();
	current=drv_start_tag_close_recover(previous);
      }
    }
  }
}

static void end_element(void *userData,const char *name) {
  if(current!=rn_notAllowed) {
    current=drv_end_tag(previous=current);
    if(current==rn_notAllowed) {
      error();
      current=drv_end_tag_recover(previous);
    }
  }
}

static void characters(void *userData,const char *s,int len) {
  if(current!=rn_notAllowed) {
    current=drv_text(previous=current,(char*)s,len);
    if(current==rn_notAllowed) {
      error();
      current=drv_text_recover(previous,(char*)s,len);
    }
  }
}

static int validate(int fd) {
  char *buf; int len;
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
    if(len!=BUFSIZ) {
      if(!XML_ParseBuffer(expat,len,1)) goto PARSE_ERROR;
      break;
    } else {
      if(!XML_ParseBuffer(expat,BUFSIZ,0)) goto PARSE_ERROR;
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

int main(int argc,char **argv) {
  init();

  if(argc<2) {
    fprintf(stderr,"rnv version  %s\nusage: rnv schema.rnc {document.xml}\n",RNV_VERSION);
    goto ERRORS;
  }

  if(load_rnc(*(++argv))) {
    int ok=1;
    if(*(++argv)) {
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

    return !ok;
  }

ERRORS:
  fprintf(stderr,"exiting on errors\n");
  return 1;
}


/*
 * $Log$
 * Revision 1.12  2003/12/14 10:39:58  dvd
 * +rnx
 *
 * Revision 1.11  2003/12/13 22:55:04  dvd
 * cleanups
 *
 * Revision 1.10  2003/12/13 22:03:31  dvd
 * rnv works
 *
 * Revision 1.9  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 */
