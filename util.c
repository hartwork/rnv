/* $Id$ */

#include "util.h"

int strhash(char *s) {
  int h=0; 
  while(*s) h=h*31+*(s++); 
  return h;
}

extern char *abspath(char *r,char *b) {
  if(*r!='/') {
    char *c=b,*sep=(char*)0;
    for(;;) {if(!(*c)) break; if(*c++=='/') sep=c;}
    if(sep) {
      char *p,*q;
      p=r; while(*p++); q=p+(sep-b);
      do *(--q)=*(--p); while(p!=r);
      while(b!=sep) *r++=*b++;
    }
  }
  return r;
}

/*
 * $Log$
 * Revision 1.1  2003/11/27 21:00:23  dvd
 * abspath,strhash
 *
 */
