/* $Id$ */

#include <string.h>
#include "xsd_tm.h"

void xsd_mktmn(struct xsd_tm *tmp,char *fmt,char *s,int n) {
}
void xsd_mktm(struct xsd_tm *tmp,char *fmt,char *val) {xsd_mktmn(tmp,fmt,val,strlen(val));}

extern int xsd_tmcmp(struct xsd_tm *tmp1, struct xsd_tm *tmp2) {
}
