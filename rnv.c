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
#include "drv.h"

#define RNV_VERSION "1.0.0"

static int start,current,previous;
static XML_Parser expat;
char *xml;

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

static void explain_error() {
  char *s;
  drv_expected(previous);
  if(drv_n_exp!=0) {
    int i;
    fprintf(stderr,"invalid document (%s,%i,%i)\nexpected:\n",xml,XML_GetCurrentLineNumber(expat),XML_GetCurrentColumnNumber(expat));
    for(i=0;i!=drv_n_exp;++i) {
      fprintf(stderr,"\t%s\n",s=p2str(drv_exp[i]));
      free(s);
    }
  }
}

static char *suri,*sname;

static void qname(char *name) {
  char *sep;
  sep=strrchr(name,':'); 
  if(sep) {
    sname=sep+1;
  } else {
    sep=sname=name;
  }
  suri=(char*)calloc(sep-name+1,sizeof(char));
  strncpy(suri,name,sep-name);
  suri[sep-name]='\0';
}

static void start_element(void *userData,const char *name,const char **attrs) {
  if(current!=rn_notAllowed) { 
    qname((char*)name);
    current=drv_start_tag_open(previous=current,suri,sname);
    free(suri);
    for(;;) {
      if(current==rn_notAllowed) {
	explain_error();
	break;
      }
      if(!(*attrs)) break;
      qname((char*)*attrs);
      ++attrs;
      current=drv_attribute(previous=current,suri,sname,(char*)*attrs);
      free(suri);
      ++attrs;
    }
    current=drv_start_tag_close(previous=current);
    if(current==rn_notAllowed) explain_error();
  }
}

static void end_element(void *userData,const char *name) {
  if(current!=rn_notAllowed) {
    current=drv_end_tag(previous=current);
    if(current==rn_notAllowed) explain_error();
  }
}

static void characters(void *userData,const char *s,int len) {
  if(current!=rn_notAllowed) {
    current=drv_text(previous=current,(char*)s,len);
    if(current==rn_notAllowed) explain_error();
  }
}

static int validate(int fd) {
  static char buf[BUFSIZ]; 
  int len;
  previous=current=start;

  expat=XML_ParserCreateNS(NULL,':');
  XML_SetElementHandler(expat,&start_element,&end_element);
  XML_SetCharacterDataHandler(expat,&characters);
  for(;;) {
    len=read(fd,buf,BUFSIZ);
    if(len<0) {
      fprintf(stderr,"I/O error (%s): %s\n",xml,strerror(errno));
      goto ERROR;
    }
    if(len==0) {
      if(!XML_Parse(expat,buf,len,1)) goto PARSE_ERROR;
      break;
    } else {
      if(!XML_Parse(expat,buf,len,0)) goto PARSE_ERROR;
      if(current==rn_notAllowed) break;
    }
  }
  XML_ParserFree(expat);
  return current!=rn_notAllowed;

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
	ok=ok&&validate(fd);
	close(fd);
      } while(*(++argv));
    } else { /* stdin */
      xml="stdin";
      ok=ok&&validate(0);
    }

    return !ok;
  }

ERRORS:
  fprintf(stderr,"exiting on errors\n");
  return 1;
}


/*
 * $Log$
 * Revision 1.10  2003/12/13 22:03:31  dvd
 * rnv works
 *
 * Revision 1.9  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 */
