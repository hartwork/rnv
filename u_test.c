/* $Id$ */
#include <assert.h>
#include "u.h"

int main(int argc,char **argv) {
  int i;
  for(i=-1;i!=32768;++i) {
    printf("%#x is",i);
    if(u_base_char(i)) printf(" base");
    if(u_ideographic(i)) printf(" ideographic");
    if(u_combining_char(i)) printf(" combining");
    if(u_extender(i)) printf(" extender");
    if(u_digit(i)) printf(" digit");
    printf("\n");
  }
  return 0;
}

/*
 * $Log$
 * Revision 1.2  2003/11/25 13:14:21  dvd
 * scanner ready
 *
 * Revision 1.1  2003/11/19 11:05:55  dvd
 * binary search for u.c
 *
 */
