/* $Id$ */

#include <stdarg.h>

#ifndef RND_H
#define RND_H 1

#define RND_ER_LOOPST 0
#define RND_ER_LOOPEL 1
#define RND_ER_CTYPE 2
#define RND_ER_BADSTART 3
#define RND_ER_BADMORE 4
#define RND_ER_BADEXPT 5
#define RND_ER_BADLIST 6
#define RND_ER_BADATTR 7

extern void (*rnd_verror_handler)(int er_no,va_list ap);

extern void rnd_default_verror_handler(int erno,va_list ap);

extern void rnd_init(void);
extern void rnd_clear(void);

extern void rnd_deref(int start);
extern void rnd_restrictions(void);
extern void rnd_traits(void);
extern int rnd_release(void);
extern int rnd_errors(void);

#endif
