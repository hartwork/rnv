/* $Id$ */

#include <string.h> /*strncmp*/
#include "util.h"
#include "xsd.h"

int xsd_allows(char *typ,char *ps,char *s,int n) {
  return 1;
}

int xsd_equal(char *typ,char *val,char *s,int n) {
  return (strcmp(typ,"string")?tokncmp(val,s,n):strncmp(val,s,n))==0;
}
