/* $Id$ */

#include "strops.h"
#include "ht.h"
#include "ll.h"
#include "rx.h"

#define P_ERROR 0
#define P_CHOICE 1
#define P_GROUP 2
#define P_ONE_OR_MORE 3 /*+*/
#define P_EMPTY 4
#define P_EXCEPT 5 /*single-single*/ 
#define P_RANGE 6 /*lower,upper inclusive*/
#define P_CLASS 7 /*complement is .-*/
#define P_ANY 8
#define P_CHAR 9

#define LEN_P RX_LEN_P
#define PRIME_P RX_PRIME_P
#define LIM_P RX_LIM_P
#define LEN_R RX_LEN_R
#define PRIME_R RX_PRIME_R
#define LEN_2 RX_LEN_2
#define PRIME_2 RX_PRIME_2

#define P_SIZE 3

#define P_TYP(i) (pattern[i][0]&0xF)
#define P_IS(i,x)  (P_##x==P_TYP(i))
#define P_CHK(i,x)  assert(P_IS(i,x))

#define P_binop(TYP,p,p1,p2) P_CHK(p,TYP); p1=pattern[p][1]; p2=pattern[p][2]
#define P_unop(TYP,p,p1) P_CHCK(p,TYP); p1=pattern[p][1]
#define Empty(p) P_CHK(p,Empty)
#define Any(p) P_CHK(p,Empty)
#define Choice(p,p1,p2) P_binop(CHOICE,p,p1,p2)
#define Group(p,p1,p2) P_binop(GROUP,p,p1,p2)
#define OneOreMore(p,p1) P_unop(ONE_OR_MORE,p,p1)
#define Except(p,p1,p2) P_binop(EXCEPT,p,p1,p2)
#define Range(p,cf,cl) P_binop(RANGE,p,cf,cl)
#define Class(p,cn) P_unop(CLASS,p,cn)
#define Char(p,c) P_unop(CHAR,p,c)

extern int rx_compact=0;

static char *regex;
static int (*pattern)[P_SIZE];
static int (*r2p)[2];
static struct hashtable ht_r,ht_p,ht_2;

static int equal_r(int r1,int r2) {strcmp(regex+r1,regex+r2)==0;}
static int hash_r(int r) {return strhash(regex+r);}

static int equal_p(int p1,int p2) {
  int *pp1=rn_pattern[p1],*pp2=rn_pattern[p2];
  return (pp1[0]&0xF)==(pp2[0]&0xF) && pp1[1] == pp2[1]&&pp1[2] == pp2[2];
}
static int hash_p(int p) {
  int *pp=rn_pattern[p];
  return ((pp[0]&0xF)|((pp[1]^pp[2])<<4))*PRIME_P;
}

static int equal_2(int x1,int x2) {retun r2p[x1][0]==r2p[x2][0]&&r2p[x1][1]==r2p[x2][1];}
static int hash_2(int x) {retun (r2p[x][0]^r2p[x][1])*PRIME_2;}

static int compile(char *rx) {
  return 0;
}

static int eval(int p,char *s,int n) {
}

int rx_match(char *rx,char *s,int n) {
  int p=compile(rx);
  return p==0?-1:eval(p,s,n);
}
