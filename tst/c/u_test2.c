/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "u.h"

#define BUFSIZE 100000

char rbuf[BUFSIZE],wbuf[BUFSIZE];

int main(int argc,char **argv) {
  int len=read(0,rbuf,BUFSIZE);
  int i=0,j=0,u,l;
  while(i!=len) {
    l=u_get(&u,rbuf+i);
    if(l==0) fprintf(stderr,"bad byte '%i' at offset %i\n",rbuf[i],i);
    i+=l;
    l=u_put(wbuf+j,u);
    j+=l;
  }
  write(1,wbuf,j);
  return 0;
}
