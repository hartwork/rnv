/* $Id$ */

#include <stdarg.h> /*va_list*/

#ifndef ER_H
#define ER_H

#define ER_IO 0
#define ER_UTF 1
#define ER_XESC 2
#define ER_LEXP 3
#define ER_LLIT 4
#define ER_LILL 5
#define ER_SEXP 6
#define ER_SILL 7
#define ER_UNAN 8

extern void er_handler(int er_no,...);
extern void (*ver_handler_p)(int er_no,va_list ap);

#endif

/*
 * $Log$
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
