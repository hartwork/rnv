/* $Id$ */

#ifndef UTIL_H
#define UTIL_H 1

/* tests whether a character is an xml whitespace */
extern int xml_white_space(int v);

/* compares two tokens, s1 is null terminated, s2 is not */
extern int tokncmp(char *s1,char *s2,int n2);

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
 * Revision 1.3  2003/12/12 22:21:06  dvd
 * drv written, compiled, not yet debugged
 *
 * Revision 1.2  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 * Revision 1.1  2003/11/27 21:00:23  dvd
 * abspath,strhash
 *
 */
