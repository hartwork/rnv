/* $Id$ */

#ifndef XSD_H
#define XSD_H 1

extern int xsd_allows(char *typ,char *ps,char *s,int n);
extern int xsd_equal(char *typ,char *val,char *s,int n);

extern void (*xsd_error_handler)(char *msg);

#endif
