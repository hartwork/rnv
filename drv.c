/* $Id$ */

#include <string.h> /*strcmp*/
#include <stdlib.h> /*calloc,free*/
#include <stdio.h> /*debugging*/
#include "util.h" /*tokncmp,xml_white_space*/
#include "ht.h"
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
#define LEN_EXP 16
#define M_SIZE 6
#define LEN_M 1024

#define M_STO 0
#define M_STC 1
#define M_END 2
#define M_TYP(m) memo[m][0]
#define M_SET(p) memo[i_m][5]=p
#define M_RET(m) memo[m][5]
#define M_NEW(x) memset(memo[i_m],0,sizeof(int[M_SIZE])); memo[i_m][0]=M_##x

static struct dtl *dtl;
static int len_dtl,n_dtl;
static int (*memo)[M_SIZE];
static int i_m,len_m;
static struct hashtable ht_m;

static int equal_m(int m1,int m2) {
  int *me1=memo[m1],*me2=memo[m2];
  return (me1[0]==me2[0])&&(me1[1]==me2[1])&&(me1[2]==me2[2])&&(me1[3]==me2[3])&&(me1[4]==me2[4]);
}
static int hash_m(int m) {
  int *me=memo[m];
  return (me[0]&0x2)|((me[1]^me[2]^me[3])<<2);
}

static int accept_m();
static int newStartTagOpen(int p,int uri,int name,int recover) { 
  int *me=memo[i_m];
  M_NEW(STO);
  me[1]=p; me[2]=uri; me[3]=name; me[4]=recover;
  return ht_get(&ht_m,i_m);
}

static int newStartTagClose(int p,int recover) {
  int *me=memo[i_m];
  M_NEW(STC);
  me[1]=p; me[4]=recover;
  return ht_get(&ht_m,i_m);
}

static int newEndTag(int p,int recover) {
  int *me=memo[i_m];
  M_NEW(END);
  me[1]=p; me[4]=recover;
  return ht_get(&ht_m,i_m);
}

static int accept_m() {
  int j;
  if((j=ht_get(&ht_m,i_m))==-1) {
    ht_put(&ht_m,j=i_m++);
    if(i_m==len_m) {
      int (*newmemo)[M_SIZE]=(int (*)[])calloc(len_m*=2,sizeof(int[M_SIZE]));
      memcpy(newmemo,memo,(i_m)*sizeof(int[M_SIZE]));
      free(memo); memo=newmemo;
    } 
  }
  return j;
}

static int fallback_equal(char *typ,char *val,char *s,int n) {return 1;}
static int fallback_allows(char *typ,char *ps,char *s,int n) {return 1;}

static int builtin_equal(char *typ,char *val,char *s,int n) {
  int dt=newDatatype(0,typ-rn_string);
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
    memo=(int (*)[])calloc(len_m=LEN_M,sizeof(int[M_SIZE]));
    dtl=(struct dtl*)calloc(len_dtl=LEN_DTL,sizeof(struct dtl));
    ht_init(&ht_m,len_m,&hash_m,&equal_m);
    windup();
  }
}

static void windup() {
  i_m=0; n_dtl=0;
  drv_add_dtl(rn_string+0,&fallback_equal,&fallback_allows); /* guard at 0 */
  drv_add_dtl(rn_string+0,&builtin_equal,&builtin_allows);
  drv_add_dtl(rn_string+rn_xsd_uri,&xsd_equal,&xsd_allows);
}

void drv_clear() {
  ht_clear(&ht_m);
  windup();
}

void drv_add_dtl(char *suri,int (*equal)(char *typ,char *val,char *s,int n),int (*allows)(char *typ,char *ps,char *s,int n)) {
  if(n_dtl==len_dtl) {
    struct dtl *newdtl=(struct dtl*)calloc(len_dtl*=2,sizeof(struct dtl)); 
    memcpy(newdtl,dtl,n_dtl*sizeof(struct dtl)); free(dtl);
    dtl=newdtl;
  }
  dtl[n_dtl].uri=newString(suri);
  dtl[n_dtl].equal=equal;
  dtl[n_dtl].allows=allows;
  ++n_dtl;
}

static struct dtl *getdtl(int uri) {
  int i;
  dtl[0].uri=uri; i=n_dtl;
  while(dtl[--i].uri!=uri);
  if(i==0) er_handler(ER_NODTL,rn_string+uri);
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

static int start_tag_open(int p,int uri,int name,int recover) {
  int nc,p1,p2,m,ret=0;
  m=newStartTagOpen(p,uri,name,recover);
  if(m!=-1) return M_RET(m);
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
  newStartTagOpen(p,uri,name,recover); M_SET(ret); 
  accept_m();
  return ret;
}

int drv_start_tag_open(int p,char *suri,char *sname) {return start_tag_open(p,newString(suri),newString(sname),0);}
int drv_start_tag_open_recover(int p,char *suri,char *sname) {return start_tag_open(p,newString(suri),newString(sname),1);}

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

int drv_attribute_recover(int p,char *suri,char *sname,char *s) {
  return p;
}

static int start_tag_close(int p,int recover) {
  int p1,p2,ret=0,m;
  m=newStartTagClose(p,recover);
  if(m!=-1) return M_RET(m);
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
  newStartTagClose(p,recover); M_SET(ret); 
  accept_m();
  return ret;
}
int drv_start_tag_close(int p) {return start_tag_close(p,0);}
int drv_start_tag_close_recover(int p) {return start_tag_close(p,1);}

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
int drv_text_recover(int p,char *s,int n) {return p;}

static int end_tag(int p,int recover) {
  int p1,p2,ret=0,m;
  m=newEndTag(p,recover);
  if(m!=-1) return M_RET(m);
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
  newEndTag(p,recover); M_SET(ret);
  accept_m();
  return ret;
}
int drv_end_tag(int p) {return end_tag(p,0);}
int drv_end_tag_recover(int p) {return end_tag(p,1);}

/*
 * $Log$
 * Revision 1.11  2003/12/14 15:21:49  dvd
 * much better hash functions
 *
 * Revision 1.10  2003/12/14 14:52:24  dvd
 * efficient memoization
 *
 * Revision 1.9  2003/12/14 10:52:36  dvd
 * recovery
 *
 * Revision 1.8  2003/12/14 10:39:58  dvd
 * +rnx
 *
 * Revision 1.7  2003/12/13 22:31:54  dvd
 * a few bugfixes
 *
 * Revision 1.6  2003/12/13 22:03:30  dvd
 * rnv works
 *
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