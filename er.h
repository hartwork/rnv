/* $Id$ */

#ifndef ER_H
#define ER_H

#define ER_IO 0
#define ER_UTF 1
#define ER_XESC 2

extern void (*er_handler)(int er_no,...);

#endif

/*
 * $Log$
 * Revision 1.2  2003/11/20 23:28:50  dvd
 * getu,getv debugged
 *
 * Revision 1.1  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 */
