/* $Id$ */

#ifndef ER_H
#define ER_H

#define ER_INTERNAL 0
#define ER_IO 1
#define ER_UTF 2

extern void (*er_handler)(int erno,...);

#endif

/*
 * $Log$
 * Revision 1.1  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 */
