/* $Id$ */

#include "rn.h"
#include "drv.h"

struct dtl {
  int uri;
  int (*equal)(char *typ,char *val,char *s);
  int (*allows)(char *typ,char *ps,char *s);
};

#define LEN_DTL 4

static struct dtl *dtls;
static int len_dtl,n_dtl;

void drv_init() {
}

void drv_clear() {
}

void drv_add_dtl(char *uri,int (*equal)(char *typ,char *val,char *s),int (*allows)(char *typ,char *ps,char *s)) {
}

int drv_start_tag_open(int p,char *uri,char *name) {
  return 0;
}

int drv_attribute(int p,char *uri,char *name,char *s) {
  return 0;
}

int drv_start_tag_close(int p) {
  return 0;
}

int drv_text(int p,char *s) {
  return 0;
}

int drv_end_tag(int p) {
  return 0;
}

/*
 * $Log$
 * Revision 1.2  2003/12/10 22:23:52  dvd
 * *** empty log message ***
 *
 * Revision 1.1  2003/12/08 22:39:15  dvd
 * +drv
 *
 */
