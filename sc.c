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
  stp->tab=(int(*)[SC_RECSIZE])calloc(stp->len=LEN,sizeof(int[SC_RECSIZE]));
  windup(stp);
}

void sc_clear(struct sc_stack *stp) {
  windup(stp);
}

static void realloc_sc(struct sc_stack *stp) {
  int (*tab)[SC_RECSIZE]=(int(*)[SC_RECSIZE])calloc(stp->len*=2,sizeof(int[SC_RECSIZE]));
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
