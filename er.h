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

extern void er_handler(int er_no,...);
extern void (*ver_handler_p)(int er_no,va_list ap);

#endif

/*
 * $Log$
 * Revision 1.17  2003/12/12 22:21:06  dvd
 * drv written, compiled, not yet debugged
 *
 * Revision 1.16  2003/12/08 21:23:47  dvd
 * +path restrictions
 *
 * Revision 1.15  2003/12/08 18:54:51  dvd
 * content-type checks
 *
 * Revision 1.14  2003/12/07 20:41:42  dvd
 * bugfixes, loops, traits
 *
 * Revision 1.13  2003/12/07 16:50:55  dvd
 * stage D, dereferencing and checking for loops
 *
 * Revision 1.12  2003/12/05 23:58:44  dvd
 * parses docbook
 *
 * Revision 1.11  2003/12/01 14:44:53  dvd
 * patterns in progress
 *
 * Revision 1.10  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.9  2003/11/29 17:47:48  dvd
 * decl
 *
 * Revision 1.8  2003/11/27 23:05:49  dvd
 * syntax and external files
 *
 * Revision 1.7  2003/11/26 23:49:00  dvd
 * syntax almost ready
 *
 * Revision 1.6  2003/11/26 00:37:47  dvd
 * parser in progress, documentation handling removed
 *
 * Revision 1.5  2003/11/25 13:14:21  dvd
 * scanner ready
 *
 * Revision 1.4  2003/11/24 23:00:27  dvd
 * literal, error reporting
 *
 * Revision 1.3  2003/11/21 00:20:06  dvd
 * lexer in progress
 *
 * Revision 1.2  2003/11/20 23:28:50  dvd
 * getu,getv debugged
 *
 * Revision 1.1  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 */
