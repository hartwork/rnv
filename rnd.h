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
