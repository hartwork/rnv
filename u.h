#ifndef U_H
#define U_H

/* computes a unicode character u off the head of s; 
 returns number of bytes read. 0 means error.
 */
extern int u_get(int *u,char *s);

extern int u_letter(int u);
extern int u_digit(int u);

#endif
