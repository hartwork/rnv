/* $Id$ */

#include <stdarg.h> /*va_list*/

#ifndef ER_H
#define ER_H

#define ER_IO 0
#define ER_UTF 10
#define ER_XESC 20
#define ER_LEXP 30
#define ER_LLIT 31
#define ER_LILL 32
#define ER_SEXP 40
#define ER_SILL 41
#define ER_NOTGR 42
#define ER_EXT 50
#define ER_DUPNS 51
#define ER_DUPDT 52
#define ER_DFLTNS 53
#define ER_DFLTDT 54
#define ER_NONS 55
#define ER_NODT 56
#define ER_NCEX 57
#define ER_2HEADS 58
#define ER_COMBINE 59
#define ER_OVRIDE 60
#define ER_EXPT 61
#define ER_NOSTART 70
#define ER_UNDEF 71
#define ER_LOOPST 80
#define ER_LOOPEL 81
#define ER_CTYPE 82
#define ER_BADSTART 83
#define ER_BADMORE 84
#define ER_BADEXPT 85
#define ER_BADLIST 86
#define ER_BADATTR 87
#define ER_NODTL 100

#define ER_BIT 0x1000

extern void er_handler(int er_no,...);
extern void (*ver_handler_p)(int er_no,va_list ap);

#endif
