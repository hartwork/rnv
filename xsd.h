/* $Id$ */

#ifndef XSD_H
#define XSD_H 1

#define XSDER_TYP 0
#define XSDER_PAR 1
#define XSDER_PARVAL 2
#define XSDER_VAL 3
#define XSDER_NPAT 4
#define XSDER_WS 5
#define XSDER_ENUM 6

extern void (*xsd_verror_handler)(int erno,va_list ap);

extern void xsd_init(void);

extern int xsd_allows(char *typ,char *ps,char *s,int n);
extern int xsd_equal(char *typ,char *val,char *s,int n);

#endif
