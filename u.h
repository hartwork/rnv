/* $Id$ */

#ifndef U_H
#define U_H 1

/* computes a unicode character u off the head of s; 
 returns number of bytes read. 0 means error.
 */
extern int u_get(int *u,char *s);

/* character classes required for parsing XML */
extern int u_base_char(int u);
extern int u_ideographic(int u);
extern int u_combining_char(int u);
extern int u_digit(int u);
extern int u_extender(int u);

#endif

/*
 * $Log$
 * Revision 1.3  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 * Revision 1.2  2003/11/19 11:05:55  dvd
 * binary search for u.c
 *
 */
