/* $Id$ */

#ifndef RNC_H
#define RNC_H 1

struct rnc_source;

extern int rnc_open(struct rnc_source *sp,char *fn);
extern int rnc_stropen(struct rnc_source *sp,char *fn,char *s,int len);
extern int rnc_bind(struct rnc_source *sp,char *fn,int fd);
extern int rnc_close(struct rnc_source *sp);

extern void rnc_parse(struct rnc_source *sp);

#endif

/*
 * $Log$
 * Revision 1.4  2003/11/26 00:37:47  dvd
 * parser in progress, documentation handling removed
 *
 * Revision 1.3  2003/11/20 23:28:50  dvd
 * getu,getv debugged
 *
 * Revision 1.2  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 * Revision 1.1  2003/11/17 21:33:28  dvd
 * +cimpl
 *
 */
