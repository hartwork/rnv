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

int rn_i_p, rn_i_nc, rn_i_s;
int (*rn_pattern)[P_SIZE];
int (*rn_nameclass)[NC_SIZE];
char *rn_string;
int *rn_first, *rn_first_a, *rn_first_c, *rn_first_to;
int *rn_firsts;

static struct hashtable ht_p, ht_nc, ht_s;

static int len_p, len_nc, len_s, len_f;
static int len_s, len_f;

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
    rn_i_p=0; rn_i_nc=0; rn_i_s=0;

    rn_pattern=(int (*)[])calloc(len_p,sizeof(int[P_SIZE]));
    rn_nameclass=(int (*)[])calloc(len_nc,sizeof(int[NC_SIZE]));
    rn_string=(char*)calloc(len_s,sizeof(char));

    ht_init(&ht_p,len_p,&hash_p,&equal_p);
    ht_init(&ht_nc,len_nc,&hash_nc,&equal_nc);
    ht_init(&ht_s,len_s,&hash_s,&equal_s);

    memset(rn_pattern[0],0,sizeof(int[P_SIZE]));
    memset(rn_nameclass[0],0,sizeof(int[NC_SIZE]));
    rn_pattern[0][0]=P_ERROR;  rn_accept_p();
    rn_nameclass[0][0]=NC_ERROR; rn_accept_nc();
    rn_accept_s("");

    rn_first=(int*)calloc(len_p,sizeof(int));
    rn_first_a=(int*)calloc(len_p,sizeof(int));
    rn_first_c=(int*)calloc(len_p,sizeof(int));
    rn_first_to=(int*)calloc(len_p,sizeof(int));

    rn_firsts=(int*)calloc(len_f,sizeof(int));

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

#define accept(name,n,N)  \
int rn_accept_##n() { \
  int j; \
  if((j=ht_get(&ht_##n,rn_i_##n))==-1) { \
    ht_put(&ht_##n,j=rn_i_##n); \
    ++rn_i_##n; \
    if(rn_i_##n==len_##n) { \
      int (*name)[N##_SIZE]=(int (*)[])calloc(len_##n*=2,sizeof(int[N##_SIZE])); \
      memcpy(name,rn_##name,rn_i_##n*sizeof(int[N##_SIZE])); \
      free(rn_##name); rn_##name=name; \
    } \
  } \
  memset(rn_##name[rn_i_##n],0,sizeof(int[N##_SIZE])); \
  return j; \
}

accept(pattern,p,P)
accept(nameclass,nc,NC)

int rn_accept_s(char *s) {
  int len=strlen(s)+1, j;
  if(rn_i_s+len>len_s) {
    char *string=(char*)calloc(len_s=(rn_i_s+len)*2,sizeof(char));
    memcpy(string,rn_string,rn_i_s); free(rn_string); rn_string=string;
  }
  strcpy(rn_string+rn_i_s,s);
  if((j=ht_get(&ht_s,rn_i_s))==-1) {
    ht_put(&ht_s,j=rn_i_s);
    rn_i_s+=len;
  }
  return j;
}

/* 
 * $Log$
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
