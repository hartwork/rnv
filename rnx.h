/* $Id$ */

#ifndef RNX_H
#define RNX_H 1

extern void rnx_init(void);
extern void rnx_clear(void);

extern int rnx_n_exp, *rnx_exp;
extern void rnx_expected(int p);

#endif

/*
 * $Log$
 * Revision 1.2  2003/12/14 20:07:54  dvd
 * cleanups
 *
 * Revision 1.1  2003/12/14 10:39:58  dvd
 * +rnx
 *
 */
