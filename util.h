/* $Id$ */

#ifndef UTIL_H
#define UTIL_H 1

/* hash value for a zero-terminated string */
extern int strhash(char *s);

/* compute the absolute path from a relative path and a base path;
 the caller must ensure that there is enough space in r:
 size(r) > strlen(r)+strlen(b)
 */
extern char *abspath(char *r,char *b);

#endif

/*
 * $Log$
 * Revision 1.1  2003/11/27 21:00:23  dvd
 * abspath,strhash
 *
 */
