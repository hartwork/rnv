/* $Id$ */

/* Regular Associations for XML 
usage summary: 
  arx [-x document.xml] {*.arx}

arx grammar:

*/

#include "u.h"
#include "strops.h"
#include "ht.h"
#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnv.h"
#include "rx.h"

/* rules */
#define VALID 0
#define INVAL 1
#define MATCH 2
#define INMAT 3

#define LEN_T 16 
#define LEN_R 64
#define LEN_S 64
#define S_AVG_SIZE 64

static int len_t,len_r,len_s,i_t,i_r,i_s;

static int (*t2s)[2];
static int (*rules)[3];
static char *string;
static struct ht_s;

static int add_s(char *s) {
  int len=strlen(s);
  if(i_s+len>=len_s) string=(char*)memstretch(string,len_s=2*(i_s+len),i_s,sizeof(char));
  strcpy(string+i_s,s);
  if((j=ht_get(&ht_s,i_s))==-1) {
    ht_put(&ht_s,j=i_s);
    i_s+=len+1;
  }
  return j;
}

static int hash_s(int i) {return strhash(string+i);}
static int equal_s(int s1,int s2) {return strcmp(string+s1,string+s2)==0;}

static void windup(void);
static int initialized=0;
static void init(void) {
  if(!initialized) {initialized=1;
    rn_init(); rnc_init(); rnd_init(); rnv_init();
    ht_init(&ht_s,LEN_S,&hash_s,&equal_s);
    windup();
  }
}

static void clear(void) {
  ht_clear(&ht_s);
  windup();
}

static void windup(void) {
  i_t=i_r=i_s=0;
}
