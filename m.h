/* $Id$ */

#ifndef MEMOPS_H
#define MEMOPS_H 1

extern void m_free(void *p);
extern void *m_alloc(int length,int size);
extern void *m_stretch(void *p,int newlen,int oldlen,int size);

#endif
