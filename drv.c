/* $Id$ */

#include <string.h> /*strcmp*/
#include <stdlib.h> /*calloc,free*/
#include "util.h" /*tokncmp,xml_white_space*/
#include "rn.h"
#include "er.h"
#include "xsd.h"
#include "drv.h"

struct dtl {
  int uri;
  int (*equal)(char *typ,char *val,char *s,int n);
  int (*allows)(char *typ,char *ps,char *s,int n);
};

#define LEN_DTL 4

static struct dtl *dtls;
static int len_dtl,n_dtl;

static int fallback_equal(char *typ,char *val,char *s,int n) {return 1;}
static int fallback_allows(char *typ,char *ps,char *s,int n) {return 1;}

static int builtin_equal(char *typ,char *val,char *s,int n) {
  int dt=typ-rn_string;
  if(dt==rn_dt_string) return strncmp(val,s,n)==0;
  else if(dt==rn_dt_token) return tokncmp(val,s,n)==0;
  else assert(0);
  return 0;
}

static int builtin_allows(char *typ,char *ps,char *s,int n) {return 1;}

static void windup();

static int initialized=0;
void drv_init() {
  if(!initialized) { initialized=1;
    dtls=(struct dtl*)calloc(len_dtl=LEN_DTL,sizeof(struct dtl));
    windup();
  }
}

static void windup() {
  n_dtl=0;
  drv_add_dtl(rn_string+0,&fallback_equal,&fallback_allows); /* guard at 0 */
  drv_add_dtl(rn_string+0,&builtin_equal,&builtin_allows);
  drv_add_dtl(rn_string+rn_xsd_uri,&xsd_equal,&xsd_allows);
}

void drv_clear() {
  windup();
}

void drv_add_dtl(char *suri,int (*equal)(char *typ,char *val,char *s,int n),int (*allows)(char *typ,char *ps,char *s,int n)) {
  if(n_dtl==len_dtl) {
    struct dtl *newdtls=(struct dtl*)calloc(len_dtl*=2,sizeof(struct dtl)); 
    memcpy(newdtls,dtls,n_dtl*sizeof(struct dtl)); free(dtls);
    dtls=newdtls;
  }
  dtls[n_dtl].uri=newString(suri);
  dtls[n_dtl].equal=equal;
  dtls[n_dtl].allows=allows;
  ++n_dtl;
}

static struct dtl *getdtl(int uri) {
  int i;
  dtls[0].uri=uri; i=n_dtl;
  while(dtls[--i].uri!=uri);
  if(i==0) er_handler(ER_NODTL,rn_string+uri);
  return dtls+i;
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

int apply_after(int (*f)(int q1,int q2),int p1,int p0) {
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

/* ret is for experiments with memoization of results */

static int start_tag_open(int p,int uri,int name) {
  int nc,p1,p2,ret=0;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: 
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE:
    ret=rn_notAllowed;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(start_tag_open(p1,uri,name),start_tag_open(p2,uri,name));
    break;
  case P_ELEMENT: Element(p,nc,p1); 
    ret=ncof(nc,uri,name)?rn_after(p1,rn_empty):rn_notAllowed; 
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2); 
    ret=rn_choice(
      apply_after(&rn_ileave,start_tag_open(p1,uri,name),p2),
      apply_after(&rn_ileave,start_tag_open(p2,uri,name),p1));
    break;
  case P_GROUP: Group(p,p1,p2); 
    { int p11=apply_after(&rn_group,start_tag_open(p1,uri,name),p2);
      ret=nullable(p1)?rn_choice(p11,start_tag_open(p2,uri,name)):p11;
    } break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=apply_after(&rn_group,start_tag_open(p1,uri,name),rn_choice(p,rn_empty));
    break;
  case P_AFTER: After(p,p1,p2);
    ret=apply_after(&rn_after,start_tag_open(p1,uri,name),p2);
    break;
  default: assert(0);
  }
  return ret;
}

int drv_start_tag_open(int p,char *suri,char *sname) {return start_tag_open(p,newString(suri),newString(sname));}

static int text(int p,char *s,int n);
static int attribute(int p,int uri,int name,char *s) {
  int p1,p2,nc,ret=0;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ELEMENT:
    ret=rn_notAllowed;
    break;
  case P_AFTER: After(p,p1,p2);
    ret=rn_after(attribute(p1,uri,name,s),p2);
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(attribute(p1,uri,name,s),attribute(p2,uri,name,s));
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2);
    ret=rn_choice(
      rn_ileave(attribute(p1,uri,name,s),p2),
      rn_ileave(p1,attribute(p2,uri,name,s)));
    break;
  case P_GROUP: Group(p,p1,p2);
    ret=rn_choice(
      rn_group(attribute(p1,uri,name,s),p2),
      rn_group(p1,attribute(p2,uri,name,s)));
    break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=rn_group(attribute(p1,uri,name,s),rn_choice(p,rn_empty));
    break;
  case P_ATTRIBUTE: Attribute(p,nc,p1);
    ret=ncof(nc,uri,name)&&nullable(text(p1,s,strlen(s)))?rn_empty:rn_notAllowed;
    break;
  default: assert(0);
  }
  return ret;
}

int drv_attribute(int p,char *suri,char *sname,char *s) {
  return attribute(p,newString(suri),newString(sname),s);
}

static int start_tag_close(int p) {
  int p1,p2,ret=0;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ELEMENT:
    ret=p;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(start_tag_close(p1),start_tag_close(p2));
    break;
  case P_INTERLEAVE: Interleave(p,p1,p2);
    ret=rn_ileave(start_tag_close(p1),start_tag_close(p2));
    break;
  case P_GROUP: Group(p,p1,p2);
    ret=rn_group(start_tag_close(p1),start_tag_close(p2));
    break;
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    ret=rn_one_or_more(start_tag_close(p1));
    break;
  case P_ATTRIBUTE: 
    ret=rn_notAllowed;
    break;
  case P_AFTER:
  default: assert(0);
  }
  return ret;
}
int drv_start_tag_close(int p) {return start_tag_close(p);}

static int list(int p,char *s,int n) {
  char *end=s+n,*sp=s;
  for(;;) {
    while(s!=end&&xml_white_space(*s)) ++s;
    sp=s;
    while(sp!=end&&!xml_white_space(*sp)) ++sp;
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
    ret=getdtl(lib)->allows(rn_string+typ,rn_params+ps,s,n)?rn_empty:rn_notAllowed;
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
  while(s!=end) {if(!xml_white_space(*s)) {ws=0; break;} ++s;}
  return ws?rn_choice(p,p1):p1;
}
int drv_text(int p,char *s,int n) {return textws(p,s,n);}

static int end_tag(int p) {
  int p1,p2,ret=0;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_INTERLEAVE: case P_GROUP: case P_ONE_OR_MORE:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE: case P_ELEMENT:
    ret=rn_notAllowed;
    break;
  case P_CHOICE: Choice(p,p1,p2);
    ret=rn_choice(end_tag(p1),end_tag(p2));
    break;
  case P_AFTER: After(p,p1,p2);
    ret=nullable(p1)?p2:rn_notAllowed;
    break;
  default: assert(0);
  }
  return ret;
}

int drv_end_tag(int p) {return end_tag(p);}

/*
 * $Log$
 * Revision 1.5  2003/12/12 22:48:27  dvd
 * datatype parameters are supported
 *
 * Revision 1.4  2003/12/12 22:21:06  dvd
 * drv written, compiled, not yet debugged
 *
 * Revision 1.3  2003/12/11 23:35:34  dvd
 * derivative in progress
 *
 * Revision 1.2  2003/12/10 22:23:52  dvd
 * *** empty log message ***
 *
 * Revision 1.1  2003/12/08 22:39:15  dvd
 * +drv
 *
 */
