/* $Id$ */

#include <string.h> /*strcmp*/
#include <stdlib.h> /*calloc,free*/
#include "util.h" /*tokncmp*/
#include "rn.h"
#include "xsd.h"
#include "drv.h"

struct dtl {
  int uri;
  int (*equal)(char *typ,char *val,char *s,int n);
  int (*allows)(char *typ,char **ps,char *s,int n);
};

#define LEN_DTL 4

static struct dtl *dtls;
static int len_dtl,n_dtl;

static int builtin_equal(char *typ,char *val,char *s,int n) {
  int dt=typ-rn_string;
  if(dt==rn_dt_string) return strncmp(val,s,n)==0;
  else if(dt==rn_dt_token) return tokncmp(val,s,n)==0;
  else assert(0);
  return 0;
}

static int builtin_allows(char *typ,char **ps,char *s,int n) {return 1;}

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
  drv_add_dtl(rn_string+0,&builtin_equal,&builtin_allows);
  drv_add_dtl(rn_string+rn_xsd_uri,&xsd_equal,&xsd_allows);
}

void drv_clear() {
  windup();
}

void drv_add_dtl(char *suri,int (*equal)(char *typ,char *val,char *s,int n),int (*allows)(char *typ,char **ps,char *s,int n)) {
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

static int start_tag_open(int p,int uri,int name) {
  int nc,p1,p2;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: 
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE:
    return rn_notAllowed;
  case P_CHOICE: Choice(p,p1,p2);
    return rn_choice(start_tag_open(p1,uri,name),start_tag_open(p2,uri,name));
  case P_ELEMENT: Element(p,nc,p1); 
    return ncof(nc,uri,name)?rn_after(p1,rn_empty):rn_notAllowed; 
  case P_INTERLEAVE: Interleave(p,p1,p2); 
    return rn_choice(
      apply_after(&rn_ileave,start_tag_open(p1,uri,name),p2),
      apply_after(&rn_ileave,start_tag_open(p2,uri,name),p1));
  case P_GROUP: Group(p,p1,p2); 
    { int p11=apply_after(&rn_group,start_tag_open(p1,uri,name),p2);
      return nullable(p1)?rn_choice(p11,start_tag_open(p2,uri,name)):p11;
    }
  case P_ONE_OR_MORE: OneOrMore(p,p1);
    return apply_after(&rn_group,start_tag_open(p1,uri,name),rn_choice(p,rn_empty));
  case P_AFTER: After(p,p1,p2);
    return apply_after(&rn_after,start_tag_open(p1,uri,name),p2);
  default: assert(0);
  }
  return 0;
}

int drv_start_tag_open(int p,char *suri,char *sname) {return start_tag_open(p,newString(suri),newString(sname));}


static int attribute(int p,int uri,int name,char *s) {
  return 0;
}

int drv_attribute(int p,char *suri,char *sname,char *s) {return attribute(p,newString(suri),newString(sname),s);}

int drv_start_tag_close(int p) {
  return 0;
}

int drv_text(int p,char *s) {
  return 0;
}

int drv_end_tag(int p) {
  return 0;
}
#if 0
  case P_EMPTY:
  case P_NOT_ALLOWED:
  case P_TEXT:
  case P_CHOICE:
  case P_INTERLEAVE:
  case P_GROUP:
  case P_ONE_OR_MORE:
  case P_LIST:
  case P_DATA:
  case P_DATA_EXCEPT:
  case P_VALUE:
  case P_ATTRIBUTE:
  case P_ELEMENT:
  case P_AFTER:
#endif
/*
 * $Log$
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
