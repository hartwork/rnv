/* $Id$ */

#ifndef ER_H
#define ER_H

#define ER_IO 0
#define ER_UTF 1
#define ER_XESC 2
#define ER_LEX 3
#define ER_LIT 4

extern void (*er_handler)(int er_no,...);

#endif

/*
 * $Log$
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
