/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <string.h> /*strerror,strncpy,strrchr*/
#include <stdio.h>
#include "xmlc.h" /*xmlc_white_space*/
#include "erbit.h"
#include "rn.h"
#include "drv.h"
#include "rnv.h"

static void (*drvverror0)(int erno,va_list ap);

#define err(msg) vfprintf(stderr,msg"\n",ap);
static void default_verror_handler(int erno,va_list ap) {
  if(erno&ERBIT_DRV) {
    (*drvverror0)(erno&~ERBIT_DRV,ap);
  } else {
    switch(erno) {
    case RNV_ER_ELEM: err("element %s^%s not allowed"); break;
    case RNV_ER_AKEY: err("attribute %s^%s not allowed"); break;
    case RNV_ER_AVAL: err("attribute %s^%s with invalid value \"%s\""); break;
    case RNV_ER_EMIS: err("incomplete content"); break;
    case RNV_ER_AMIS: err("missing attributes of %s^%s"); break;
    case RNV_ER_UFIN: err("unfinished content of element %s^%s"); break;
    case RNV_ER_TEXT: err("invalid text or data"); break;
    case RNV_ER_NOTX: err("text not allowed"); break;
    default: assert(0);
    }                
  }
}

void (*rnv_verror_handler)(int erno,va_list ap)=&default_verror_handler;

static void error_handler(int erno,...) {
  va_list ap; va_start(ap,erno); (*rnv_verror_handler)(erno,ap); va_end(ap);
}

static void verror_handler_drv(int erno,va_list ap) {(*rnv_verror_handler)(erno|ERBIT_DRV,ap);}

static void windup(void);
static int initialized=0;
void rnv_init(void) {
  if(!initialized) {initialized=1;
    rn_init();
    drv_init(); drvverror0=drv_verror_handler; drv_verror_handler=&verror_handler_drv;
    windup();
  }
}

void rnv_clear(void) {
  windup();
}

static void windup(void) {
}

static char *suri=NULL,*sname; static int len_suri=-1; /* qname() splits, handlers use */

static void qname(char *name) {
  char *sep; int len;
  if((sep=strrchr(name,':'))) sname=sep+1; else sep=sname=name;
  len=sep-name+1;
  if(len>len_suri) {len_suri=len; free(suri); suri=(char*)calloc(len_suri,sizeof(char));}
  strncpy(suri,name,len-1);
  suri[len-1]='\0';
}

static int whitespace(char *text,int n_t) {
  char *s=text,*end=text+n_t;
  for(;;) {
    if(s==end) return 1;
    if(!xmlc_white_space(*(s++))) return 0;
  }
}

void rnv_text(int *curp,int *prevp,char *text,int n_t,int mixed) {
  if(mixed) {
    if(!whitespace(text,n_t)) {
      *curp=drv_mixed_text(*prevp=*curp);
      if(*curp==rn_notAllowed) {
	*curp=drv_mixed_text_recover(*prevp);
	error_handler(RNV_ER_NOTX);
      }
    }
  } else {
    *curp=drv_text(*prevp=*curp,text,n_t);
    if(*curp==rn_notAllowed) {
      *curp=drv_text_recover(*prevp,text,n_t);
      error_handler(cdata(*prevp)?RNV_ER_TEXT:RNV_ER_NOTX);
    }
  }
}

void rnv_start_tag(int *curp,int *prevp,char *name,char **attrs) {
  qname((char*)name);
  *curp=drv_start_tag_open(*prevp=*curp,suri,sname);
  if(*curp==rn_notAllowed) {
    *curp=drv_start_tag_open_recover(*prevp,suri,sname);
    error_handler(*curp==rn_notAllowed?RNV_ER_ELEM:RNV_ER_EMIS,suri,sname); 
  }
  while(*curp!=rn_notAllowed) {
    if(!(*attrs)) break;
    qname((char*)*attrs);
    *curp=drv_attribute_open(*prevp=*curp,suri,sname);
    ++attrs;
    if(*curp==rn_notAllowed) {
      *curp=drv_attribute_open_recover(*prevp,suri,sname);
      error_handler(RNV_ER_AKEY,suri,sname);
    } else {
      *curp=drv_text(*prevp=*curp,(char*)*attrs,strlen(*attrs));
      if(*curp==rn_notAllowed || (*curp=drv_attribute_close(*prevp=*curp))==rn_notAllowed) {
	*curp=drv_attribute_close_recover(*prevp);
	error_handler(RNV_ER_AVAL,suri,sname,*attrs);
      }
    }
    ++attrs;
  }
  if(*curp!=rn_notAllowed) {
    *curp=drv_start_tag_close(*prevp=*curp);
    if(*curp==rn_notAllowed) {
      *curp=drv_start_tag_close_recover(*prevp);
      qname((char*)name);
      error_handler(RNV_ER_AMIS,suri,sname);
    }
  }
}

void rnv_end_tag(int *curp,int *prevp,char *name) {
  *curp=drv_end_tag(*prevp=*curp);
  if(*curp==rn_notAllowed) {
    qname(name);
    error_handler(RNV_ER_UFIN,suri,sname);
    *curp=drv_end_tag_recover(*prevp);
  }
}
