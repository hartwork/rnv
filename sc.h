/* $Id$ */

#ifndef SC_H
#define SC_H 1

#define SC_NS 1
#define SC_DT 2
#define SC_DE 3

#define SC_NS_INHERITED 4

#define SC_DE_HEAD 4
#define SC_DE_CHOICE 8
#define SC_DE_ILEAVE 16

/* special prefixes */
#define SC_INHR -1 /* no string is at this index, used for inherited namespace */
#define SC_START -1 /* key for 'start' define */

/* sentinel urls */
#define SC_BASE -1
#define SC_LOCK -2

#define SC_RECSIZE 3 /* 0 - key, 1 - value, 2 - type & flags */ 

#define SC_TYP(i) (sc_tab[i][2]&0x3)

extern int (*sc_tab)[SC_RECSIZE];

extern void sc_init();

extern void sc_open();
extern void sc_lock();
extern int sc_locked();
extern void sc_close();

extern int sc_find(int key,int typ); /* returns 0 if not found, index in sc_tab otherwise */
extern int sc_add(int key,int val,int aux); /* returns index for the new record */

#endif

/* 
 * $Log$
 * Revision 1.2  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.1  2003/11/29 17:47:48  dvd
 * decl
 * 
 */
