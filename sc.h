/* $Id$ */

#ifndef SC_H
#define SC_H 1

#define SC_RECSIZE 3 /* 0 - key, 1 - value, 2 - auxiliary */ 

struct sc_stack {
  int (*tab)[SC_RECSIZE];
  int len,base,top;
};

extern void sc_init(struct sc_stack *stp);
extern void sc_clear(struct sc_stack *stp);

extern void sc_open(struct sc_stack *stp);
extern void sc_lock(struct sc_stack *stp);
extern void sc_close(struct sc_stack *stp);

extern int sc_void(struct sc_stack *sp);
extern int sc_locked(struct sc_stack *stp);

extern int sc_find(struct sc_stack *stp,int key); /* returns 0 if not found, index in tab otherwise */
extern int sc_add(struct sc_stack *stp,int key,int val,int aux); /* returns index for the new record */

#endif

/* 
 * $Log$
 * Revision 1.5  2003/12/06 00:55:14  dvd
 * parses all grammars from nxml-mode samples
 *
 * Revision 1.4  2003/12/04 22:02:20  dvd
 * refactoring
 *
 * Revision 1.3  2003/12/01 14:44:54  dvd
 * patterns in progress
 *
 * Revision 1.2  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.1  2003/11/29 17:47:48  dvd
 * decl
 * 
 */
