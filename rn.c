/* $Id$ */

#include <stdlib.h> /* calloc */
#include <string.h> /* strcmp,memcmp,strlen,strcpy,memcpy,memset */

#include "util.h"
#include "ht.h"
#include "rn.h"

#define LEN_P 1024
#define LEN_NC 1024
#define LEN_S 16384
#define LEN_F 4096

int (*rn_pattern)[P_SIZE];
int (*rn_nameclass)[NC_SIZE];
char *rn_string;
int *rn_first, *rn_first_a, *rn_first_c, *rn_first_to;
int *rn_firsts;
int rn_empty,rn_text,rn_notAllowed;

static struct hashtable ht_p, ht_nc, ht_s;

int i_p, i_nc, i_s;
static int len_p, len_nc, len_s, len_f;
static int len_s, len_f;

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
#define setNullable(x) if(x) rn_pattern[i_p][0]|=P_FLG_NUL
#define setCdata(x) if(x) rn_pattern[i_p][0]|=P_FLG_TXT
#define setContentType(x) rn_pattern[i_[][0]|=x

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
  setNullable(1);
  return accept_p();
}

int newNotAllowed() { P_NEW(NOT_ALLOWED);
  return accept_p();
}

int newText() { P_NEW(TEXT);
  setNullable(1); setCdata(1);
  return accept_p();
}

int newChoice(int p1,int p2) { P_NEW(CHOICE);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setNullable(nullable(p1)||nullable(p2));
  setCdata(cdata(p1)||cdata(p2));
  return accept_p();
}

int newInterleave(int p1,int p2) { P_NEW(INTERLEAVE);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setNullable(nullable(p1)&&nullable(p2));
  setCdata(cdata(p1)||cdata(p2));
  return accept_p();
}

int newGroup(int p1,int p2) { P_NEW(GROUP);
  rn_pattern[i_p][1]=p1; rn_pattern[i_p][2]=p2;
  setNullable(nullable(p1)&&nullable(p2));
  setCdata(cdata(p1)||cdata(p2));
  return accept_p();
}

int newOneOrMore(int p1) { P_NEW(ONE_OR_MORE);
  rn_pattern[i_p][1]=p1;
  setNullable(nullable(p1)); setCdata(cdata(p1));
  return accept_p();
}

int newList(int p1) { P_NEW(LIST);
  rn_pattern[i_p][1]=p1;
  setCdata(1);
  return accept_p();
}

int newData(int dt,int ps) { P_NEW(DATA);
  rn_pattern[i_p][1]=dt;
  setCdata(1);
  return accept_p();
}

int newDataExcept(int dt,int p1) { P_NEW(DATA_EXCEPT);
  rn_pattern[i_p][1]=dt; rn_pattern[i_p][2]=p1;
  setCdata(1);
  return accept_p();
}

int newValue(int dt,int s) { P_NEW(VALUE);
  rn_pattern[i_p][1]=dt; rn_pattern[i_p][2]=s;
  setCdata(1);
  return accept_p();
}

int newAttribute(int nc,int p1) { P_NEW(ATTRIBUTE);
  rn_pattern[i_p][1]=nc; rn_pattern[i_p][2]=p1;
  return accept_p();
}

int newElement(int nc,int p1) { P_NEW(ELEMENT);
  rn_pattern[i_p][1]=nc; rn_pattern[i_p][2]=p1;
  return accept_p();
}

int newAfter(int qn,int p1,int p2) { P_NEW(AFTER);
  rn_pattern[i_p][1]=qn; rn_pattern[i_p][2]=p1; rn_pattern[i_p][3]=p2;
  setCdata(cdata(p1));
  return accept_p();
}

#define NC_NEW(x) rn_nameclass[i_nc][0]=NC_##x

int newAnyName() { NC_NEW(ANY_NAME);
  return accept_nc();
}

int newAnyNameExcept(int nc) { NC_NEW(ANY_NAME_EXCEPT);
  rn_nameclass[i_nc][1]=nc;
  return accept_nc();
}

int newNsName(int uri) { NC_NEW(NSNAME);
  rn_nameclass[i_nc][1]=uri;
  return accept_nc();
}

int newQName(int uri,int localname) { NC_NEW(NAME);
  rn_nameclass[i_nc][1]=uri; rn_nameclass[i_nc][2]=localname;
  return accept_nc();
}

int newNsNameExcept(int uri,int nc) { NC_NEW(NSNAME_EXCEPT);
  rn_nameclass[i_nc][1]=uri; rn_nameclass[i_nc][2]=nc;
  return accept_nc();
}

int newNameClassChoice(int nc1,int nc2) { NC_NEW(CHOICE);
  rn_nameclass[i_nc][1]=nc1; rn_nameclass[i_nc][2]=nc2;
  return accept_nc();
}

int newDatatype(int lib,int dt) { NC_NEW(DATATYPE);
  rn_nameclass[i_nc][1]=lib; rn_nameclass[i_nc][2]=dt;
  return accept_nc();
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

static int hash_p(int i);
static int hash_nc(int i);
static int hash_s(int i);

static int equal_p(int p1,int p2);
static int equal_nc(int nc1,int nc2); 
static int equal_s(int s1,int s2);

static int initialized=0;
void rn_init() {
  if(!initialized) {
    len_p=LEN_P; len_nc=LEN_NC; len_s=LEN_S; len_f=LEN_F;
    i_p=0; i_nc=0; i_s=0;

    rn_pattern=(int (*)[])calloc(len_p,sizeof(int[P_SIZE]));
    rn_nameclass=(int (*)[])calloc(len_nc,sizeof(int[NC_SIZE]));
    rn_string=(char*)calloc(len_s,sizeof(char));

    ht_init(&ht_p,len_p,&hash_p,&equal_p);
    ht_init(&ht_nc,len_nc,&hash_nc,&equal_nc);
    ht_init(&ht_s,len_s,&hash_s,&equal_s);

    memset(rn_pattern[0],0,sizeof(int[P_SIZE]));
    memset(rn_nameclass[0],0,sizeof(int[NC_SIZE]));
    rn_pattern[0][0]=P_ERROR;  accept_p();
    rn_nameclass[0][0]=NC_ERROR; accept_nc();
    newString(""); /* empty string is always at 0 */

    rn_first=(int*)calloc(len_p,sizeof(int));
    rn_first_a=(int*)calloc(len_p,sizeof(int));
    rn_first_c=(int*)calloc(len_p,sizeof(int));
    rn_first_to=(int*)calloc(len_p,sizeof(int));

    rn_firsts=(int*)calloc(len_f,sizeof(int));

    rn_empty=newEmpty();
    rn_notAllowed=newNotAllowed();
    rn_text=newText();

    initialized=1;
  }
}

/* hash function for int[] */
static int hash_ary(int i,int *ary,int size) {
  int j;
  int *a=ary+i*size;
  int h=0,s=sizeof(int)*8/size;
  for(j=0;;++j) {
    h=h+a[j];
    if(j==P_SIZE) break;
    h<<=s;
  }
  return h;
}

static int hash_p(int i) {return hash_ary(i,(int*)rn_pattern,P_SIZE);}
static int hash_nc(int i) {return hash_ary(i,(int*)rn_nameclass,NC_SIZE);}
static int hash_s(int i) {return strhash(rn_string+i);}

static int equal_p(int p1,int p2) {return memcmp(rn_pattern[p1],rn_pattern[p2],P_SIZE)==0;}
static int equal_nc(int nc1,int nc2) {return memcmp(rn_nameclass[nc1],rn_pattern[nc2],NC_SIZE)==0;}
static int equal_s(int s1,int s2) {return strcmp(rn_string+s1,rn_string+s2)==0;}

/* 
 * $Log$
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
