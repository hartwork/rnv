/* $Id$ */

#ifndef STROPS_H
#define STROPS_H 1

/* compares two tokens, s1 is null terminated, s2 is not */
extern int tokncmp(char *s1,char *s2,int n2);

/* hash value for a zero-terminated string */
extern int strhash(char *s);

/* strdup is a non-standard function */
extern char *strclone(char *s);

/* compute the absolute path from a relative path and a base path;
 the caller must ensure that there is enough space in r:
 size(r) > strlen(r)+strlen(b)
 */
extern char *abspath(char *r,char *b);

/* find a string in a sorted array, return the index, 
  or size on failure */
extern int strtab(char *s,char *tab[],int size);
extern int strntab(char *s,int len,char *tab[],int size);

#endif
