/* $Id$ */

#ifndef MEMOPS_H
#define MEMOPS_H 1

extern void memfree(void *p);
extern void *memalloc(int length,int size);
extern void *memstretch(void *p,int newlen,int oldlen,int size);

#endif
