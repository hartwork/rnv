/* $Id$ */

#include <stdio.h>
#include "xmlc.h" /*xmlc_white_space*/
#include "memops.h"
#include "strops.h" /*tokcmpn*/
#include "ht.h"
#include "rn.h"
#include "xsd.h"
#include "ll.h"
#include "erbit.h"
#include "drv.h"

struct dtl {
  int uri;
  int (*equal)(char *typ,char *val,char *s,int n);
  int (*allows)(char *typ,char *ps,char *s,int n);
};

#define LEN_DTL 4
#define LEN_M DRV_LEN_M
#define PRIME_M DRV_PRIME_M
#define LIM_M DRV_LIM_M

#define M_SIZE 5

#define M_STO 0
#define M_STC 1
#define M_ATT 2
#define M_TXT 3
#define M_END 4
#define M_SET(p) memo[i_m][M_SIZE-1]=p
#define M_RET(m) memo[m][M_SIZE-1]

int drv_compact=0;

static struct dtl *dtl;
static int len_dtl,n_dtl;
static int (*memo)[M_SIZE];
static int i_m,len_m;
static struct hashtable ht_m;

static void (*xsdverror0)(int erno,va_list ap);

#define err(msg) vfprintf(stderr,msg"\n",ap);
static void default_verror_handler(int erno,va_list ap) {
  if(erno&ERBIT_XSD) {
    (*xsdverror0)(erno&~ERBIT_XSD,ap);
  } else {
    switch(erno) {
    case DRV_ER_NODTL: err("no datatype library for URI '%s'"); break;
    default: assert(0);
    }                
  }
}

void (*drv_verror_handler)(int erno,va_list ap)=&default_verror_handler;

static void error_handler(int erno,...) {
  va_list ap; va_start(ap,erno); (*drv_verror_handler)(erno,ap); va_end(ap);
}

static void verror_handler_xsd(int erno,va_list ap) {(*drv_verror_handler)(erno|ERBIT_XSD,ap);}

static void new_memo(int typ) {
  if(drv_compact && ht_get(&ht_m,i_m)==i_m) ht_del(&ht_m,i_m); 
  memo[i_m][0]=typ;
}

static int equal_m(int m1,int m2) {
  int *me1=memo[m1],*me2=memo[m2];
  return (me1[0]==me2[0])&&(me1[1]==me2[1])&&(me1[2]==me2[2])&&(me1[3]==me2[3]);
}
static int hash_m(int m) {
  int *me=memo[m];
  return ((me[0]&0x7)|((me[1]^me[2]^me[3])<<3))*PRIME_M;
}

static int newStartTagOpen(int p,int uri,int name) { 
  int *me=memo[i_m];
  new_memo(M_STO);
  me[1]=p; me[2]=uri; me[3]=name;
  return ht_get(&ht_m,i_m);
}

static int newAttributeOpen(int p,int uri,int name) { 
  int *me=memo[i_m];
  new_memo(M_ATT);
  me[1]=p; me[2]=uri; me[3]=name;
  return ht_get(&ht_m,i_m);
}

static int newStartTagClose(int p) {
  int *me=memo[i_m];
  new_memo(M_STC);
  me[1]=p; me[2]=me[3]=0;
  return ht_get(&ht_m,i_m);
}

static int newMixedText(int p) {
  int *me=memo[i_m];
  new_memo(M_TXT);
  me[1]=p; me[2]=me[3]=0;
  return ht_get(&ht_m,i_m);
}

static int newEndTag(int p) {
  int *me=memo[i_m];
  new_memo(M_END);
  me[1]=p; me[2]=me[3]=0;
  return ht_get(&ht_m,i_m);
}

static void accept_m(void) {
  if(ht_get(&ht_m,i_m)!=-1) {
    if(drv_compact) ht_del(&ht_m,i_m); else return;
  }
  ht_put(&ht_m,i_m++);
  if(drv_compact&&i_m==LIM_M) i_m=0;
  if(i_m==len_m) memo=(int(*)[M_SIZE])memstretch(memo,len_m=2*i_m,i_m,sizeof(int[M_SIZE]));
}

static int fallback_equal(char *typ,char *val,char *s,int n) {return 1;}
static int fallback_allows(char *typ,char *ps,char *s,int n) {return 1;}

static int builtin_equal(char *typ,char *val,char *s,int n) {
  int dt=newDatatype(0,typ-rn_string);
  if(dt==rn_dt_string) return strcmpn(val,s,n)==0;
  else if(dt==rn_dt_token) return tokcmpn(val,s,n)==0;
  else assert(0);
  return 0;
}

static int builtin_allows(char *typ,char *ps,char *s,int n) {return 1;}

static void windup(void);

static int initialized=0;
void drv_init(void) {
  if(!initialized) { initialized=1;
    rn_init(); 
    xsd_init(); xsdverror0=xsd_verror_handler; xsd_verror_handler=&verror_handler_xsd;
    memo=(int (*)[M_SIZE])memalloc(len_m=LEN_M,sizeof(int[M_SIZE]));
    dtl=(struct dtl*)memalloc(len_dtl=LEN_DTL,sizeof(struct dtl));
    ht_init(&ht_m,LEN_M,&hash_m,&equal_m);
    windup();
  }
}

static void windup(void) {
  i_m=0; n_dtl=0;
  drv_add_dtl(rn_string+0,&fallback_equal,&fallback_allows); /* guard at 0 */
  drv_add_dtl(rn_string+0,&builtin_equal,&builtin_allows);
  drv_add_dtl(rn_string+rn_xsd_uri,&xsd_equal,&xsd_allows);
}

void drv_clear(void) {
  ht_clear(&ht_m);
  windup();
}

void drv_add_dtl(char *suri,int (*equal)(char *typ,char *val,char *s,int n),int (*allows)(char *typ,char *ps,char *s,int n)) {
  if(n_dtl==len_dtl) dtl=(struct dtl *)memstretch(dtl,len_dtl=n_dtl*2,n_dtl,sizeof(struct dtl));
  dtl[n_dtl].uri=newString(suri);
  dtl[n_dtl].equal=equal;
  dtl[n_dtl].allows=allows;
  ++n_dtl;
}

static struct dtl *getdtl(int uri) {
  int i;
  dtl[0].uri=uri; i=n_dtl;
  while(dtl[--i].uri!=uri);
  if(i==0) error_handler(DRV_ER_NODTL,rn_string+uri);
  return dtl+i;
}

static int ncof(int nc,int uri,int name) {
  int uri2,name2,nc1,nc2;
  switch(NC_TYP(nc)) {
  case NC_QNAME: QName(nc,uri2,name2); return uri2==uri&&name2==name;
  case NC_NSNAME: NsName(nc,uri2); return uri2==uri;
  case NC_ANY_NAME: return 1;
  case NC_EXCEPT: NameClassExcept(nc,nc1,nc2); return ncof(nc1,uri,name)&&!ncof(nc2,uri,name);
  case NC_CHOICE: NameClassChoice(nc,nc1,nc2); return ncof(nc1,uri,name)||ncof(nc2,uri,name);
  default: assert(0);
  }
  return 0;
}

static int apply_after(int (*f)(int q1,int q2),int p1,int p0) {
  int p11,p12;
  switch(P_TYP(p1)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_INTERLEAVE: case P_GROUP: case P_ONE_OR_MORE:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE: case P_ELEMENT:
    return rn_notAllowed;
  case P_CHOICE: Choice(p1,p11,p12); return rn_choice(apply_after(f,p11,p0),apply_after(f,p12,p0));
  case P_AFTER: After(p1,p11,p12); return rn_after(p11,(*f)(p12,p0));
  default: assert(0);
  }
  return 0;
}

static int start_tag_open(int p,int uri,int name,int recover) {
  int nc,p1,p2,m,ret=0;
  if(!recover) {
    m=newStartTagOpen(p,uri,name);
    if(m!=-1) return M_RET(m);
  }
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: 
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE:
    ret=rn_notAllowed;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(start_tag_open(p1,uri,name,recover),start_tag_open(p2,uri,name,recover));
    break;
  case P_ELEMENT: Element(p,nc,p1); 
    ret=ncof(nc,uri,name)?rn_after(p1,rn_empty):rn_notAllowed; 
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2); 
    ret=rn_choice(
      apply_after(&rn_ileave,start_tag_open(p1,uri,name,recover),p2),
      apply_after(&rn_ileave,start_tag_open(p2,uri,name,recover),p1));
    break;
  case P_GROUP: Group(p,p1,p2); 
    { int p11=apply_after(&rn_group,start_tag_open(p1,uri,name,recover),p2);
      ret=(nullable(p1)||recover)?rn_choice(p11,start_tag_open(p2,uri,name,recover)):p11;
    } break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=apply_after(&rn_group,start_tag_open(p1,uri,name,recover),rn_choice(p,rn_empty));
    break;
  case P_AFTER: After(p,p1,p2);
    ret=apply_after(&rn_after,start_tag_open(p1,uri,name,recover),p2);
    break;
  default: assert(0);
  }
  if(!recover) {
    newStartTagOpen(p,uri,name); M_SET(ret); 
    accept_m();
  }
  return ret;
}

int drv_start_tag_open(int p,char *suri,char *sname) {return start_tag_open(p,newString(suri),newString(sname),0);}
int drv_start_tag_open_recover(int p,char *suri,char *sname) {return start_tag_open(p,newString(suri),newString(sname),1);}

static int puorg_rn(int p2,int p1) {return rn_group(p1,p2);}

static int attribute_open(int p,int uri,int name) {
  int nc,p1,p2,m,ret=0;
  m=newAttributeOpen(p,uri,name);
  if(m!=-1) return M_RET(m);
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: 
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ELEMENT:
    ret=rn_notAllowed;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(attribute_open(p1,uri,name),attribute_open(p2,uri,name));
    break;
  case P_ATTRIBUTE: Attribute(p,nc,p1);
    ret=ncof(nc,uri,name)?rn_after(p1,rn_empty):rn_notAllowed; 
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2); 
    ret=rn_choice(
      apply_after(&rn_ileave,attribute_open(p1,uri,name),p2),
      apply_after(&rn_ileave,attribute_open(p2,uri,name),p1));
    break;
  case P_GROUP: Group(p,p1,p2); 
    ret=rn_choice(
      apply_after(&rn_group,attribute_open(p1,uri,name),p2),
      apply_after(&puorg_rn,attribute_open(p2,uri,name),p1));
    break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=apply_after(&rn_group,attribute_open(p1,uri,name),rn_choice(p,rn_empty));
    break;
  case P_AFTER: After(p,p1,p2);
    ret=apply_after(&rn_after,attribute_open(p1,uri,name),p2);
    break;
  default: assert(0);
  }
  newAttributeOpen(p,uri,name); M_SET(ret); 
  accept_m();
  return ret;
}

int drv_attribute_open(int p,char *suri,char *sname) {return attribute_open(p,newString(suri),newString(sname));}
int drv_attribute_open_recover(int p,char *suri,char *sname) {return p;}

extern int drv_attribute_close(int p) {return drv_end_tag(p);}
extern int drv_attribute_close_recover(int p) {return drv_end_tag_recover(p);}

static int start_tag_close(int p,int recover) {
  int p1,p2,ret=0,m;
  if(!recover) {
    m=newStartTagClose(p);
    if(m!=-1) return M_RET(m);
  }
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ELEMENT:
    ret=p;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(start_tag_close(p1,recover),start_tag_close(p2,recover));
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2);
    ret=rn_ileave(start_tag_close(p1,recover),start_tag_close(p2,recover));
    break;
  case P_GROUP: Group(p,p1,p2);
    ret=rn_group(start_tag_close(p1,recover),start_tag_close(p2,recover));
    break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=rn_one_or_more(start_tag_close(p1,recover));
    break;
  case P_ATTRIBUTE: 
    ret=recover?rn_empty:rn_notAllowed;
    break;
  case P_AFTER: After(p,p1,p2);
    ret=rn_after(start_tag_close(p1,recover),p2);
    break;
  default: assert(0);
  }
  if(!recover) {
    newStartTagClose(p); M_SET(ret); 
    accept_m();
  }
  return ret;
}
int drv_start_tag_close(int p) {return start_tag_close(p,0);}
int drv_start_tag_close_recover(int p) {return start_tag_close(p,1);}

static int text(int p,char *s,int n);
static int list(int p,char *s,int n) {
  char *end=s+n,*sp;
  for(;;) {
    while(s!=end&&xmlc_white_space(*s)) ++s;
    sp=s;
    while(sp!=end&&!xmlc_white_space(*sp)) ++sp;
    if(s==end) break;
    p=text(p,s,sp-s);
    s=sp;
  }
  return p;
}

static int text(int p,char *s,int n) { /* matches text, including whitespace */
  int p1,p2,dt,ps,lib,typ,val,ret=0;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED:
  case P_ATTRIBUTE: case P_ELEMENT:
    ret=rn_notAllowed;
    break;
  case P_TEXT:
    ret=p;
    break;
  case P_AFTER: After(p,p1,p2); 
    ret=rn_after(text(p1,s,n),p2);
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(text(p1,s,n),text(p2,s,n));
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2);
    ret=rn_choice(rn_ileave(text(p1,s,n),p2),rn_ileave(p1,text(p2,s,n)));
    break;
  case P_GROUP: Group(p,p1,p2);
    { int p11=rn_group(text(p1,s,n),p2);
      ret=nullable(p1)?rn_choice(p11,text(p2,s,n)):p11;
    } break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=rn_group(text(p1,s,n),rn_choice(p,rn_empty));
    break;
  case P_LIST: List(p,p1);
    ret=nullable(list(p1,s,n))?rn_empty:rn_notAllowed;
    break;
  case P_DATA: Data(p,dt,ps); Datatype(dt,lib,typ);
    ret=getdtl(lib)->allows(rn_string+typ,rn_string+ps,s,n)?rn_empty:rn_notAllowed;
    break;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2);
    ret=text(p1,s,n)==rn_empty&&!nullable(text(p2,s,n))?rn_empty:rn_notAllowed;
    break;
  case P_VALUE: Value(p,dt,val); Datatype(dt,lib,typ);
    ret=getdtl(lib)->equal(rn_string+typ,rn_string+val,s,n)?rn_empty:rn_notAllowed;
    break;
  default: assert(0);
  }
  return ret;
} 

static int textws(int p,char *s,int n) {
  int p1=text(p,s,n),ws=1;
  char *end=s+n;
  while(s!=end) {if(!xmlc_white_space(*s)) {ws=0; break;} ++s;}
  return ws?rn_choice(p,p1):p1;
}
int drv_text(int p,char *s,int n) {return textws(p,s,n);}
int drv_text_recover(int p,char *s,int n) {return p;}

static int mixed_text(int p) { /* matches text in mixed context */
  int p1,p2,ret=0,m;
  m=newMixedText(p);
  if(m!=-1) return M_RET(m);
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED:
  case P_ATTRIBUTE: case P_ELEMENT:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
    ret=rn_notAllowed;
    break;
  case P_TEXT:
    ret=p;
    break;
  case P_AFTER: After(p,p1,p2); 
    ret=rn_after(mixed_text(p1),p2);
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(mixed_text(p1),mixed_text(p2));
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2);
    ret=rn_choice(rn_ileave(mixed_text(p1),p2),rn_ileave(p1,mixed_text(p2)));
    break;
  case P_GROUP: Group(p,p1,p2);
    { int p11=rn_group(mixed_text(p1),p2);
      ret=nullable(p1)?rn_choice(p11,mixed_text(p2)):p11;
    } break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=rn_group(mixed_text(p1),rn_choice(p,rn_empty));
    break;
  default: assert(0);
  }
  newMixedText(p); M_SET(ret);
  accept_m();
  return ret;
} 
int drv_mixed_text(int p) {return mixed_text(p);}
int drv_mixed_text_recover(int p) {return p;}

static int end_tag(int p,int recover) {
  int p1,p2,ret=0,m;
  if(!recover) {
    m=newEndTag(p);
    if(m!=-1) return M_RET(m);
  }
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_INTERLEAVE: case P_GROUP: case P_ONE_OR_MORE:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE: case P_ELEMENT:
    ret=rn_notAllowed;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(end_tag(p1,recover),end_tag(p2,recover));
    break;
  case P_AFTER: After(p,p1,p2);
    ret=(nullable(p1)||recover)?p2:rn_notAllowed;
    break;
  default: assert(0);
  }
  if(!recover) {
    newEndTag(p); M_SET(ret);
    accept_m();
  }
  return ret;
}
int drv_end_tag(int p) {return end_tag(p,0);}
int drv_end_tag_recover(int p) {return end_tag(p,1);}
