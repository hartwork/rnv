/* $Id$ */

#include <string.h> /*strncmp*/
#include "util.h"
#include "xsd.h"

int xsd_allows(char *typ,char **ps,char *s,int n) {
  return 1;
}

int xsd_equal(char *typ,char *val,char *s,int n) {
  return (strcmp(typ,"string")?tokncmp(val,s,n):strncmp(val,s,n))==0;
}

/*
 * $Log$
 * Revision 1.2  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 * Revision 1.1  2003/12/10 21:41:26  dvd
 * +xsd
 *
 */

