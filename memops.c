/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include "memops.h"

void memfree(void *p) {
  free(p);
}

extern void *memalloc(int length,int size) {
  void *p=calloc(length,size);
  if(p==NULL) {
    fprintf(stderr,"failed to allocate %i bytes of memory\n",length*size);
    exit(1);
  }
  return p;
}

void *memstretch(void *p,int newlen,int oldlen,int size) {
  char *newp=memalloc(newlen,size);
  memcpy(newp,p,oldlen*size); 
  memfree(p);
  return newp;
}
