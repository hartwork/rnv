/* $Id$ */

#include <stdlib.h> /* calloc */
#include <string.h> /* strcmp,memcmp */

#include "rn.h"

#define LEN_P 1024
#define LEN_NC 1024
#define LEN_S 16384
#define LEN_F 4096

int rn_i_p, rn_i_nc;
int (*rn_pattern)[P_SIZE];
int (*rn_nameclass)[NC_SIZE];
char *rn_string;
int *rn_first, *rn_first_a, *rn_first_b, *rn_first_to;
int *rn_firsts;

static int len_p, len_nc, len_s, len_f;
static int len_s, len_f;

void rn_init() {
  len_s=LEN_S; len_f=LEN_F; len_p=LEN_P; len_nc=LEN_NC;
  i_p=0; i_nc=0;
  rn_pattern=(int (*)[])calloc(len_p,sizeof(int[P_SIZE]));
  rn_nameclass=(int (*)[])calloc(len_nc,sizeof(int[NC_SIZE]));
  rn_string=(char*)calloc(len_s,sizeof(char));
  rn_first=(int*)calloc(len_p,sizeof(int));
  rn_first_a=(int*)calloc(len_p,sizeof(int));
  rn_first_b=(int*)calloc(len_p,sizeof(int));
  rn_first_to=(int*)calloc(len_p,sizeof(int));
  rn_firsts=(int*)calloc(len_f,sizeof(int));
}

/* hash function for array of int */
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

static int hash_p(int i) {return hash_ary(i,(int*)pattern,P_SIZE);}
static int hash_nc(int i) {return hash_ary(i,(int*)nameclass,NC_SIZE);}

static int hash_s(int i) {
  char *s=string+i;
  return 0;
}

static int equal_p(int p1,int p2) {return memcmp(pattern[p1],pattern[p2],P_SIZE)==0;}
static int equal_nc(int nc1,int nc2) {return memcmp(nameclass[nc1],pattern[nc2],NC_SIZE)==0;}
static int equal_s(int s1,int s2) {return strcmp(string+s1,string+s2)==0;}

int rn_accept_p() {
  return 0;
}

int rn_accept_nc() {
  return 0;
}

/* $Log$
 * Revision 1.1  2003/11/17 21:33:28  dvd
 * +cimpl
 * */
