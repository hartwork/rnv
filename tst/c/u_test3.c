/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "u.h"

#define BUFSIZE 100000

char rbuf[BUFSIZE];

int main(int argc,char **argv) {
  int len=read(0,rbuf,BUFSIZE);
  int i=0,u,l;
  while(i!=len) {
    l=u_get(&u,rbuf+i);
    if(l==0) fprintf(stderr,"bad byte '%i' at offset %i\n",rbuf[i],i);
    i+=l;
    if(u<0x80) printf("%c",u); else printf("&#x%x;",u);
  }
  return 0;
}
