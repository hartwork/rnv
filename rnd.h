/* $Id$ */

#ifndef RND_H
#define RND_H 1

void rnd_clear(void);
void rnd_init(void);

extern void rnd_deref(int start);
extern void rnd_restrictions(void);
extern void rnd_traits(void);
extern int rnd_release(void);
extern int rnd_errors(void);

#endif

/*
 * $Log$
 * Revision 1.7  2003/12/14 20:07:54  dvd
 * cleanups
 *
 * Revision 1.6  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 * Revision 1.5  2003/12/10 01:08:04  dvd
 * compressing schema, work in progress
 *
 * Revision 1.4  2003/12/09 19:47:35  dvd
 * start dereferenced
 *
 * Revision 1.3  2003/12/07 20:41:42  dvd
 * bugfixes, loops, traits
 *
 * Revision 1.2  2003/12/07 16:50:55  dvd
 * stage D, dereferencing and checking for loops
 * 
 */
