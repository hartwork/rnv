/* $Id$ */

#ifndef U_H
#define U_H 1

#define U_MAXLEN 6

/* computes a unicode character u off the head of s; 
 returns number of bytes read. 0 means error.
 */
extern int u_get(int *up,char *s);

/* encodes u in utf-8, returns number of octets taken */
extern int u_put(char *s,int u);

/* number of unicode characters in the string; -1 means error */
extern int u_strlen(char *s);

#endif
