/* $Id$ */

#include <stdarg.h>

#ifndef RNV_H
#define RNV_H 1

#define RNV_ER_ELEM 0
#define RNV_ER_AKEY 1
#define RNV_ER_AVAL 2
#define RNV_ER_EMIS 3
#define RNV_ER_AMIS 4
#define RNV_ER_UFIN 5
#define RNV_ER_TEXT 6
#define RNV_ER_MIXT 7

extern void (*rnv_verror_handler)(int erno,va_list ap);

extern void rnv_init(void);
extern void rnv_clear(void);

extern void rnv_text(int *curp,int *prevp,char *text,int n_t,int mixed);
extern void rnv_start_tag(int *curp,int *prevp,char *name,char **attrs);
extern void rnv_end_tag(int *curp,int *prevp,char *name);

#endif
