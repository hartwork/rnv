/* $Id$ */

#ifndef U_H
#define U_H 1

#define U_MAXLEN 5

/* computes a unicode character u off the head of s; 
 returns number of bytes read. 0 means error.
 */
extern int u_get(int *up,char *s);

/* encodes u in utf-8, returns number of octets taken */
extern int u_put(char *s,int u);

/* character classes required for parsing XML */
extern int u_base_char(int u);
extern int u_ideographic(int u);
extern int u_combining_char(int u);
extern int u_digit(int u);
extern int u_extender(int u);

#endif

/*
 * $Log$
 * Revision 1.7  2003/12/11 17:01:32  dvd
 * utf8 is handled properly
 *
 * Revision 1.6  2003/12/10 23:02:14  dvd
 * prepared to add u_put
 *
 * Revision 1.5  2003/11/25 10:33:53  dvd
 * documentation and comments
 *
 * Revision 1.4  2003/11/20 16:29:08  dvd
 * x escapes sketched
 *
 * Revision 1.3  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 * Revision 1.2  2003/11/19 11:05:55  dvd
 * binary search for u.c
 *
 */
