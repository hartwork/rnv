/* $Id$ */

#include <stdlib.h>
#include <string.h>
#include "er.h"
#include "m.h"

void m_free(void *p) {
  free(p);
}

extern void *m_alloc(int length,int size) {
  void *p=calloc(length,size);
  if(p==NULL) {
    er_printf("failed to allocate %i bytes of memory\n",length*size);
    exit(1);
  }
  return p;
}

void *m_stretch(void *p,int newlen,int oldlen,int size) {
  void *newp=m_alloc(newlen,size);
  memcpy(newp,p,oldlen*size);
  m_free(p);
  return newp;
}
