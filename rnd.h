/* $Id$ */

#ifndef RND_H
#define RND_H 1

extern void rnd_deref(int start);
extern void rnd_restrictions();
extern void rnd_traits();
extern void rnd_release();
extern int rnd_errors();

#endif

/*
 * $Log$
 * Revision 1.3  2003/12/07 20:41:42  dvd
 * bugfixes, loops, traits
 *
 * Revision 1.2  2003/12/07 16:50:55  dvd
 * stage D, dereferencing and checking for loops
 * 
 */