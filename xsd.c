/* $Id$ */

#include <string.h> /*strncmp*/
#include <stdio.h> /*stdio*/
#include "strops.h"
#include "rx.h"
#include "xsd.h"

static void default_error_handler(char *msg) {fprintf(stderr,"msg\n");}
void (*xsd_error_handler)(char *msg)=&default_error_handler;

static int initialized=0;
void xsd_init(void) {
  if(!initialized) { initialized=1;
    rx_init();
  }
}

int xsd_allows(char *typ,char *ps,char *s,int n) {
  int ok=1,len;
  while((len=strlen(ps))) {
    char *key=ps,*val=key+len+1; 
    if(strcmp(key,"pattern")==0) ok=ok&&rx_match(val,s,n);
    ps=val+strlen(val)+1;
  }
  return ok;
}

int xsd_equal(char *typ,char *val,char *s,int n) {
  return (strcmp(typ,"string")?tokncmp(val,s,n):strncmp(val,s,n))==0;
}
