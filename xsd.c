/* $Id$ */

#include <string.h> /*strncmp*/
#include <stdio.h> /*stdio*/
#include "strops.h"
#include "xsd.h"

static void default_nodt_handler(char *typ) {
  fprintf(stderr,"unknown datatype 'xsd:%s'\n",typ);
}

void (*xsd_nodt_handler)(char *typ)=&default_nodt_handler;

int xsd_allows(char *typ,char *ps,char *s,int n) {
  if(strlen(ps)) {
    do {
      printf("%s=",ps); ps+=strlen(ps)+1;
      printf("\"%s\" ",ps); ps+=strlen(ps)+1;
    } while(strlen(ps));
    printf("\n");
  }
  return 1;
}

int xsd_equal(char *typ,char *val,char *s,int n) {
  return (strcmp(typ,"string")?tokncmp(val,s,n):strncmp(val,s,n))==0;
}
