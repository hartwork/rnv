/* $Id$ */

#include <string.h> /*strncpy,strrchr*/
#include <assert.h>
#include "m.h"
#include "xmlc.h" /*xmlc_white_space*/
#include "erbit.h"
#include "drv.h"
#include "er.h"
#include "rnv.h"

extern int rn_notAllowed;

#define err(msg) (*er_vprintf)(msg"\n",ap);
void rnv_default_verror_handler(int erno,va_list ap) {
  if(erno&ERBIT_DRV) {
    drv_default_verror_handler(erno&~ERBIT_DRV,ap);
  } else {
    switch(erno) {
    case RNV_ER_ELEM: err("element %s^%s not allowed"); break;
    case RNV_ER_AKEY: err("attribute %s^%s not allowed"); break;
    case RNV_ER_AVAL: err("attribute %s^%s with invalid value \"%s\""); break;
    case RNV_ER_EMIS: err("incomplete content"); break;
    case RNV_ER_AMIS: err("missing attributes of %s^%s"); break;
    case RNV_ER_UFIN: err("unfinished content of element %s^%s"); break;
    case RNV_ER_TEXT: err("invalid data or text not allowed"); break;
    case RNV_ER_NOTX: err("text not allowed"); break;
    default: assert(0);
    }
  }
}

void (*rnv_verror_handler)(int erno,va_list ap)=&rnv_default_verror_handler;

static void error_handler(int erno,...) {
  va_list ap; va_start(ap,erno); (*rnv_verror_handler)(erno,ap); va_end(ap);
}

static void verror_handler_drv(int erno,va_list ap) {(*rnv_verror_handler)(erno|ERBIT_DRV,ap);}

static void windup(void);
static int initialized=0;
void rnv_init(void) {
  if(!initialized) {initialized=1;
    drv_init(); drv_verror_handler=&verror_handler_drv;
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
  if(len>len_suri) {len_suri=len; m_free(suri); suri=(char*)m_alloc(len_suri,sizeof(char));}
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

int rnv_text(int *curp,int *prevp,char *text,int n_t,int mixed) {
  int ok=1;
  if(mixed) {
    if(!whitespace(text,n_t)) {
      *curp=drv_mixed_text(*prevp=*curp);
      if(*curp==rn_notAllowed) { ok=0;
	*curp=drv_mixed_text_recover(*prevp);
	error_handler(RNV_ER_NOTX);
      }
    }
  } else {
    *curp=drv_text(*prevp=*curp,text,n_t);
    if(*curp==rn_notAllowed) { ok=0;
      *curp=drv_text_recover(*prevp,text,n_t);
      error_handler(RNV_ER_TEXT);
    }
  }
  return ok;
}

int rnv_start_tag_open(int *curp,int *prevp,char *name) {
  int ok=1;
  qname((char*)name);
  *curp=drv_start_tag_open(*prevp=*curp,suri,sname);
  if(*curp==rn_notAllowed) { ok=0;
    *curp=drv_start_tag_open_recover(*prevp,suri,sname);
    error_handler(*curp==rn_notAllowed?RNV_ER_ELEM:RNV_ER_EMIS,suri,sname);
  }
  return ok;
}

int rnv_attribute(int *curp,int *prevp,char *name,char *val) {
  int ok=1;
  qname((char*)name);
  *curp=drv_attribute_open(*prevp=*curp,suri,sname);
  if(*curp==rn_notAllowed) { ok=0;
    *curp=drv_attribute_open_recover(*prevp,suri,sname);
    error_handler(RNV_ER_AKEY,suri,sname);
  } else {
    *curp=drv_text(*prevp=*curp,(char*)val,strlen(val));
    if(*curp==rn_notAllowed || (*curp=drv_attribute_close(*prevp=*curp))==rn_notAllowed) { ok=0;
      *curp=drv_attribute_close_recover(*prevp);
      error_handler(RNV_ER_AVAL,suri,sname,val);
    }
  }
  return ok;
}

int rnv_start_tag_close(int *curp,int *prevp,char *name) {
  int ok=1;
  *curp=drv_start_tag_close(*prevp=*curp);
  if(*curp==rn_notAllowed) { ok=0;
    *curp=drv_start_tag_close_recover(*prevp);
    qname((char*)name);
    error_handler(RNV_ER_AMIS,suri,sname);
  }
  return ok;
}

int rnv_start_tag(int *curp,int *prevp,char *name,char **attrs) {
  int ok=1;
  ok=rnv_start_tag_open(curp,prevp,name)&&ok;
  while(*curp!=rn_notAllowed) {
    if(!(*attrs)) break;
    ok = rnv_attribute(curp,prevp,*attrs,*(attrs+1))&&ok;
    attrs+=2;
  }
  if(*curp!=rn_notAllowed) ok=rnv_start_tag_close(curp,prevp,name)&&ok;
  return ok;
}

int rnv_end_tag(int *curp,int *prevp,char *name) {
  int ok=1;
  *curp=drv_end_tag(*prevp=*curp);
  if(*curp==rn_notAllowed) { ok=0;
    qname(name);
    error_handler(RNV_ER_UFIN,suri,sname);
    *curp=drv_end_tag_recover(*prevp);
  }
  return ok;
}
