/* $Id$ */
#include <assert.h>
#include "ht.h"

static int ary[10]={'A','B','C','D','E','F','G','H','I','J'};

static int hash(i) {return 0;}
static int equal(i,j) {return ary[i]==ary[j];}

int main(int argc,char **argv) {
  struct hashtable ht;
  int i,j;
  ht_init(&ht,1,&hash,&equal);
  for(i=0;i!=10;++i) {
    printf("putting %i\n",i);
    ht_put(&ht,i);
    printf("hashtable:");
    for(j=0;j!=ht.tablen;++j) {
      printf(" (%i,%i)",ht.table[j],ht.table[j|ht.tablen]);
    }
    printf("\n");
  }

  for(i=0;i!=10;++i) {
    int ti=ht_get(&ht,i);
    printf("%i=='%c'?\n",i,ary[ti]);
    assert(i==ht_get(&ht,i));
  }
  for(i=0;i!=10;i+=2) {
    ht_del(&ht,i);
  }
  for(i=1;i!=11;i+=2) {
    int ti=ht_get(&ht,i);
    printf("%i=='%c'?\n",i,ary[ti]);
    assert(i==ht_get(&ht,i));
  }
  return 0;
}
