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
