/* $Id$ */

#ifndef RNX_H
#define RNX_H 1

extern void rnx_init();
extern void rnx_clear();

extern int rnx_n_exp, *rnx_exp;
extern void rnx_expected(int p);

#endif

/*
 * $Log$
 * Revision 1.1  2003/12/14 10:39:58  dvd
 * +rnx
 *
 */
