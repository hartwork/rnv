/* $Id$ */

#ifndef HT_H
#define HT_H 1

struct hashtable {
  int (*hash)(int i);
  int (*equal)(int i1,int i2);
  int tablen,used,limit;
  int *table;
};

extern void ht_init(struct hashtable *ht,int len,int (*hash)(int),int (*equal)(int,int));
extern void ht_free(struct hashtable *ht);
extern void ht_clear(struct hashtable *ht);
extern int ht_get(struct hashtable *ht,int i);
extern void ht_put(struct hashtable *ht,int i);
extern int ht_del(struct hashtable *ht,int i);

#endif

/*
 * $Log$
 * Revision 1.2  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 * Revision 1.1  2003/11/17 21:33:28  dvd
 * +cimpl
 *
 */
