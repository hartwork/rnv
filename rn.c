/* $Id$ */

#include <stdlib.h> /* calloc */
#include <string.h> /* strcmp,memcmp,strlen,strcpy,memcpy,memset */
#include <stdio.h> /* debugging*/

#include "util.h"
#include "ht.h"
#include "rn.h"

#define LEN_P 1024
#define LEN_NC 1024
#define LEN_S 16384

int (*rn_pattern)[P_SIZE];
int (*rn_nameclass)[NC_SIZE];
char *rn_string,*rn_params;
int rn_empty,rn_text,rn_notAllowed,rn_dt_string,rn_dt_token,rn_xsd_uri;

static struct hashtable ht_p, ht_nc, ht_s;

static int i_p, i_nc, i_s,base_p,i_ref;
static int len_p, len_nc, len_s;

void rn_new_schema() {base_p=i_p; i_ref=0;}

void rn_del_p(int i) {if(ht_get(&ht_p,i)==i) ht_del(&ht_p,i);}
void rn_add_p(int i) {if(ht_get(&ht_p,i)==-1) ht_put(&ht_p,i);}

void setNullable(int i,int x) {if(x) rn_pattern[i][0]|=P_FLG_NUL;}
void setCdata(int i,int x) {if(x) rn_pattern[i][0]|=P_FLG_TXT;}
void setContentType(int i,int t1,int t2) {rn_pattern[i][0]|=(t1>t2?t1:t2);}

int newString(char *s) {
  int len=strlen(s)+1, j;
  if(i_s+len>len_s) {
    char *string=(char*)calloc(len_s=(i_s+len)*2,sizeof(char));
    memcpy(string,rn_string,i_s); free(rn_string); rn_string=string;
  }
  strcpy(rn_string+i_s,s);
  if((j=ht_get(&ht_s,i_s))==-1) {
    ht_put(&ht_s,j=i_s);
    i_s+=len;
  }
  return j;
}

#define P_NEW(x) rn_pattern[i_p][0]=P_##x

#define P_LABEL "pattern"
#define NC_LABEL "nameclass"

#define accept(name,n,N)  \
static int accept_##n() { \
  int j; \
  if((j=ht_get(&ht_##n,i_##n))==-1) { \
    ht_put(&ht_##n,j=i_##n); \
    ++i_##n; \
    if(i_##n==len_##n) { \
      int (*name)[N##_SIZE]=(int (*)[])calloc(len_##n*=2,sizeof(int[N##_SIZE])); \
      memcpy(name,rn_##name,i_##n*sizeof(int[N##_SIZE])); \
      free(rn_##name); rn_##name=name; \
    } \
  } \
  memset(rn_##name[i_##n],0,sizeof(int[N##_SIZE])); \
  return j; \
}

accept(pattern,p,P)
accept(nameclass,nc,NC)

int newEmpty() { P_NEW(EMPTY);
  setNullable(i_p,1);
  return accept_p();
}

int newNotAllowed() { P_NEW(NOT_ALLOWED);
  return accept_p();
}

int newText() { P_NEW(TEXT);
  setNullable(i_p,1); 
  setCdata(i_p,1);
  return accept_p();
}

int newChoice(int p1,int p2) { P_NEW(CHOICE);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setNullable(i_p,nullable(p1)||nullable(p2)); 
  setCdata(i_p,cdata(p1)||cdata(p2)); 
  return accept_p();
}

int newInterleave(int p1,int p2) { P_NEW(INTERLEAVE);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setNullable(i_p,nullable(p1)&&nullable(p2));
  setCdata(i_p,cdata(p1)||cdata(p2)); 
  return accept_p();
}

int newGroup(int p1,int p2) { P_NEW(GROUP);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setNullable(i_p,nullable(p1)&&nullable(p2));
  setCdata(i_p,cdata(p1)||cdata(p2)); 
  return accept_p();
}

int newOneOrMore(int p1) { P_NEW(ONE_OR_MORE);
  rn_pattern[i_p][1]=p1;
  setNullable(i_p,nullable(p1));
  setCdata(i_p,cdata(p1));
  return accept_p();
}

int newList(int p1) { P_NEW(LIST);
  rn_pattern[i_p][1]=p1;
  setCdata(i_p,1);
  return accept_p();
}

int newData(int dt,int ps) { P_NEW(DATA);
  rn_pattern[i_p][1]=dt;
  rn_pattern[i_p][2]=ps;
  setCdata(i_p,1);
  return accept_p();
}

int newDataExcept(int p1,int p2) { P_NEW(DATA_EXCEPT);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setCdata(i_p,1);
  return accept_p();
}

int newValue(int dt,int s) { P_NEW(VALUE);
  rn_pattern[i_p][1]=dt; rn_pattern[i_p][2]=s;
  setCdata(i_p,1);
  return accept_p();
}

int newAttribute(int nc,int p1) { P_NEW(ATTRIBUTE);
  rn_pattern[i_p][2]=nc; rn_pattern[i_p][1]=p1;
  return accept_p();
}

int newElement(int nc,int p1) { P_NEW(ELEMENT);
  rn_pattern[i_p][2]=nc; rn_pattern[i_p][1]=p1; 
  return accept_p();
}

int newAfter(int p1,int p2) { P_NEW(AFTER);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setCdata(i_p,cdata(p1));
  return accept_p();
}

int newRef() { P_NEW(REF);
  rn_pattern[i_p][2]=i_ref++;
  return accept_p();
}

int rn_groupable(int p1,int p2) {
  int ct1=contentType(p1),ct2=contentType(p2);
  return ((ct1&ct2&P_FLG_CTC)||((ct1|ct2)&P_FLG_CTE));
}

int rn_one_or_more(int p) {
  if(P_IS(p,EMPTY)) return p;
  if(P_IS(p,NOT_ALLOWED)) return p;
  if(P_IS(p,TEXT)) return p;
  return newOneOrMore(p);
}

int rn_group(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  if(P_IS(p1,EMPTY)) return p2;
  if(P_IS(p2,EMPTY)) return p1;
  return newGroup(p1,p2);
}

static int samechoice(p1,p2) {
  if(P_IS(p1,CHOICE)) {
    int p11,p12; Choice(p1,p11,p12);
    return p12==p2||samechoice(p11,p2);
  } else return p1==p2;
}

int rn_choice(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p2;
  if(P_IS(p2,NOT_ALLOWED)) return p1;
  if(P_IS(p2,CHOICE)) {
    int p21,p22; Choice(p2,p21,p22);
    p1=newChoice(p1,p21); return rn_choice(p1,p22);
  }
  if(samechoice(p1,p2)) return p1;
  if(nullable(p1) && (P_IS(p2,EMPTY))) return p1;
  if(nullable(p2) && (P_IS(p1,EMPTY))) return p2;
  return newChoice(p1,p2);
}

int rn_ileave(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  if(P_IS(p1,EMPTY)) return p2;
  if(P_IS(p2,EMPTY)) return p1;
  return newInterleave(p1,p2);
}

int rn_after(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  return newAfter(p1,p2);
}

#define NC_NEW(x) rn_nameclass[i_nc][0]=NC_##x

int newQName(int uri,int name) { NC_NEW(QNAME);
  rn_nameclass[i_nc][1]=uri; rn_nameclass[i_nc][2]=name;
  return accept_nc();
}

int newNsName(int uri) { NC_NEW(NSNAME);
  rn_nameclass[i_nc][1]=uri;
  return accept_nc();
}

int newAnyName() { NC_NEW(ANY_NAME);
  return accept_nc();
}

int newNameClassExcept(int nc1,int nc2) { NC_NEW(EXCEPT);
  rn_nameclass[i_nc][1]=nc1; rn_nameclass[i_nc][2]=nc2;
  return accept_nc();
}

int newNameClassChoice(int nc1,int nc2) { NC_NEW(CHOICE);
  rn_nameclass[i_nc][1]=nc1; rn_nameclass[i_nc][2]=nc2;
  return accept_nc();
}

int newDatatype(int lib,int typ) { NC_NEW(DATATYPE);
  rn_nameclass[i_nc][1]=lib; rn_nameclass[i_nc][2]=typ;
  return accept_nc();
}

char *nc2str(int nc) {
  char *s,*s1,*s2;
  int nc1,nc2,uri,name;
  switch(NC_TYP(nc)) {
  case NC_ERROR: return strdup("?");
  case NC_NSNAME:
    NsName(nc,uri);
    s=calloc(strlen(rn_string+uri)+3,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,":*");
    return s;

  case NC_QNAME:
    QName(nc,uri,name); 
    s=calloc(strlen(rn_string+uri)+strlen(rn_string+name)+2,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,"^"); strcat(s,rn_string+name);
    return s;

  case NC_ANY_NAME: return strdup("*");

  case NC_EXCEPT:
    NameClassExcept(nc,nc1,nc2);
    s1=nc2str(nc1); s2=nc2str(nc2);
    s=calloc(strlen(s1)+strlen(s2)+2,sizeof(char));
    strcpy(s,s1); strcat(s,"-"); strcat(s,s2);
    free(s1); free(s2);
    return s;
    
  case NC_CHOICE:
    NameClassChoice(nc,nc1,nc2);
    s1=nc2str(nc1); s2=nc2str(nc2);
    s=calloc(strlen(s1)+strlen(s2)+2,sizeof(char));
    strcpy(s,s1); strcat(s,"|"); strcat(s,s2);
    free(s1); free(s2);
    return s;
    
  case NC_DATATYPE:
    Datatype(nc,uri,name); 
    s=calloc(strlen(rn_string+uri)+strlen(rn_string+name)+2,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,"^"); strcat(s,rn_string+name);
    return s;
  default: 
    assert(0);
  }
  return NULL;
}

static int hash_p(int i);
static int hash_nc(int i);
static int hash_s(int i);

static int equal_p(int p1,int p2);
static int equal_nc(int nc1,int nc2); 
static int equal_s(int s1,int s2);

static void windup();

static int initialized=0;
void rn_init() {
  if(!initialized) { initialized=1;
    len_p=LEN_P; len_nc=LEN_NC; len_s=LEN_S;

    rn_pattern=(int (*)[])calloc(len_p,sizeof(int[P_SIZE]));
    rn_nameclass=(int (*)[])calloc(len_nc,sizeof(int[NC_SIZE]));
    rn_string=(char*)calloc(len_s,sizeof(char));
    rn_params="\0"; /* parameters are not supported */

    ht_init(&ht_p,len_p,&hash_p,&equal_p);
    ht_init(&ht_nc,len_nc,&hash_nc,&equal_nc);
    ht_init(&ht_s,len_s,&hash_s,&equal_s);

    windup();
  }
}

void rn_clear() {
  ht_clear(&ht_p); ht_clear(&ht_nc); ht_clear(&ht_s);
  windup();
}

static void windup() {
  i_p=i_nc=i_s=0;
  memset(rn_pattern[0],0,sizeof(int[P_SIZE]));
  memset(rn_nameclass[0],0,sizeof(int[NC_SIZE]));
  rn_pattern[0][0]=P_ERROR;  accept_p(); 
  rn_nameclass[0][0]=NC_ERROR; accept_nc();
  newString("");
  rn_empty=newEmpty(); rn_notAllowed=newNotAllowed(); rn_text=newText();
  rn_dt_string=newDatatype(0,newString("string")); rn_dt_token=newDatatype(0,newString("token"));
  rn_xsd_uri=newString("http://www.w3.org/2001/XMLSchema-datatypes");
}

static int hash_p(int p) {
  int *pp=rn_pattern[p];
  return (pp[0]<<24)+((pp[1]&0xfff)<<12)+(pp[2]&0xfff);
}
static int hash_nc(int nc) {
  int *ncp=rn_nameclass[nc];
  return (ncp[0]<<24)+((ncp[1]&0xfff)<<12)+(ncp[2]&0xfff);
}
static int hash_s(int i) {return strhash(rn_string+i);}

static int equal_p(int p1,int p2) {
  int *pp1=rn_pattern[p1],*pp2=rn_pattern[p2];
  return (pp1[0]&0xff)==(pp2[0]&0xff) && pp1[1] == pp2[1] && pp1[2] == pp2[2];
}
static int equal_nc(int nc1,int nc2) {
  int *ncp1=rn_nameclass[nc1],*ncp2=rn_nameclass[nc2];
  return (ncp1[0]&0xff)==(ncp2[0]&0xff) && ncp1[1] == ncp2[1] && ncp1[2] == ncp2[2];
}
static int equal_s(int s1,int s2) {return strcmp(rn_string+s1,rn_string+s2)==0;}

/* 
 * $Log$
 * Revision 1.20  2003/12/12 22:21:06  dvd
 * drv written, compiled, not yet debugged
 *
 * Revision 1.19  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 * Revision 1.18  2003/12/10 21:41:26  dvd
 * +xsd
 *
 * Revision 1.17  2003/12/10 09:51:34  dvd
 * rn_end_schema is removed again, but the result is good -- bugs debugged
 *
 * Revision 1.15  2003/12/10 01:08:04  dvd
 * compressing schema, work in progress
 *
 * Revision 1.14  2003/12/09 19:38:44  dvd
 * failed to compress grammar
 *
 * Revision 1.13  2003/12/08 18:54:51  dvd
 * content-type checks
 *
 * Revision 1.12  2003/12/07 20:41:42  dvd
 * bugfixes, loops, traits
 *
 * Revision 1.11  2003/12/07 16:50:55  dvd
 * stage D, dereferencing and checking for loops
 *
 * Revision 1.10  2003/12/07 09:06:16  dvd
 * +rnd
 *
 * Revision 1.9  2003/12/05 14:28:39  dvd
 * separate stacks for references
 *
 * Revision 1.8  2003/12/04 22:02:20  dvd
 * refactoring
 *
 * Revision 1.7  2003/12/04 00:37:03  dvd
 * refactoring
 *
 * Revision 1.6  2003/12/01 14:44:53  dvd
 * patterns in progress
 *
 * Revision 1.5  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.4  2003/11/29 17:47:48  dvd
 * decl
 *
 * Revision 1.3  2003/11/27 21:00:23  dvd
 * abspath,strhash
 *
 * Revision 1.2  2003/11/26 00:37:47  dvd
 * parser in progress, documentation handling removed
 *
 * Revision 1.1  2003/11/17 21:33:28  dvd
 * +cimpl
 */
