/* $Id$ */

#include <stdlib.h> /*calloc*/
#include <assert.h> /*assert*/
#include "sc.h"

#define LEN 64

int (*sc_tab)[SC_RECSIZE];
static int len,sc_base,sc_top;

static int initialized=0;
void sc_init() {
  if(!initialized) {
    sc_tab=(int(*)[])calloc(len=LEN,sizeof(int[SC_RECSIZE]));
    sc_base=sc_top=0;
    sc_open();
    initialized=1;
  }
}

static void realloc_sc() {
  int (*sc_tab_new)[SC_RECSIZE]=(int(*)[])calloc(len*=2,sizeof(int[SC_RECSIZE]));
  memcpy(sc_tab_new,sc_tab,sc_top*sizeof(int[SC_RECSIZE])); free(sc_tab);
  sc_tab=sc_tab_new;
}

void sc_open() {
  sc_tab[sc_base=sc_top++][1]=SC_BASE; if(sc_top==len) realloc_sc();
  sc_add(0,0,SC_NS|SC_NS_INHERITED); /* default namespace */
  sc_add(SC_INHR,0,SC_NS|SC_NS_INHERITED); /* inherited namespace */
}

void sc_lock() {
  sc_tab[sc_top++][1]=SC_LOCK; if(sc_top==len) realloc_sc();
}

int sc_locked() {
  return sc_tab[sc_top-1][1]==SC_LOCK;
}

void sc_close() {
  sc_top=sc_base; while(sc_tab[--sc_base][1]!=SC_BASE);
}

int sc_find(int key,int typ) {
  int i=sc_top; sc_tab[sc_base][0]=key; sc_tab[sc_base][2]=typ;
  while(sc_tab[--i][0]!=key||SC_TYP(i)!=typ);
  return i!=sc_base?i:0;
}

int sc_add(int key,int val,int aux) {
  int i=sc_top;
  assert(!sc_locked());
  sc_tab[i][0]=key; sc_tab[i][1]=val; sc_tab[i][2]=aux;
  if(++sc_top==len) realloc_sc();
  return i;
}

/*
 * $Log$
 * Revision 1.2  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.1  2003/11/29 17:47:48  dvd
 * decl
 *
 */
