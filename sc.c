/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <string.h> /*memcpy*/
#include <assert.h> /*assert*/
#include "sc.h"

#define BASE -1
#define LOCK -2

#define LEN 64

static void windup(struct sc_stack *stp) {
  stp->top=0;
  sc_open(stp);
}

void sc_init(struct sc_stack *stp) {
  stp->tab=(int(*)[])calloc(stp->len=LEN,sizeof(int[SC_RECSIZE]));
  windup(stp);
}

void sc_clear(struct sc_stack *stp) {
  windup(stp);
}

static void realloc_sc(struct sc_stack *stp) {
  int (*tab)[SC_RECSIZE]=(int(*)[])calloc(stp->len*=2,sizeof(int[SC_RECSIZE]));
  memcpy(tab,stp->tab,stp->top*sizeof(int[SC_RECSIZE])); free(stp->tab);
  stp->tab=tab;
}

void sc_open(struct sc_stack *stp) {
  stp->tab[stp->base=stp->top++][1]=BASE; if(stp->top==stp->len) realloc_sc(stp);
}

int sc_void(struct sc_stack *stp) {
  return stp->base==0;
}

void sc_lock(struct sc_stack *stp) {
  stp->tab[stp->base][1]=LOCK;
}

int sc_locked(struct sc_stack *stp) {
  return stp->tab[stp->base][1]==LOCK;
}

void sc_close(struct sc_stack *stp) {
  stp->top=stp->base; while(stp->tab[--stp->base][1]>BASE);
}

int sc_find(struct sc_stack *stp,int key) {
  int i=stp->top; stp->tab[stp->base][0]=key;
  while(stp->tab[--i][0]!=key);
  return i!=stp->base?i:0;
}

int sc_add(struct sc_stack *stp,int key,int val,int aux) {
  int i=stp->top;
  assert(!sc_locked(stp));
  stp->tab[i][0]=key; stp->tab[i][1]=val; stp->tab[i][2]=aux;
  if(++stp->top==stp->len) realloc_sc(stp);
  return i;
}

/*
 * $Log$
 * Revision 1.7  2003/12/07 09:06:16  dvd
 * +rnd
 *
 * Revision 1.6  2003/12/06 00:55:14  dvd
 * parses all grammars from nxml-mode samples
 *
 * Revision 1.5  2003/12/05 23:58:44  dvd
 * parses docbook
 *
 * Revision 1.4  2003/12/04 22:02:20  dvd
 * refactoring
 *
 * Revision 1.3  2003/12/01 14:44:54  dvd
 * patterns in progress
 *
 * Revision 1.2  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.1  2003/11/29 17:47:48  dvd
 * decl
 *
 */
