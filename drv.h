/* $Id$ */

#ifndef DRV_H
#define DRV_H 1

extern void register_datatype_library(char *uri,int (*equal)(char *typ,char *val,char *s),int (*allows)(char *typ,char *ps,char *s));

extern int drv_start_tag_open(int p,char *uri,char *name);
extern int drv_attribute(int p,char *uri,char *name,char *s);
extern int drv_start_tag_close(int p);
extern int drv_text(int p,char *s);
extern int drv_end_tag(int p);

#endif

/*
 * $Log$
 * Revision 1.2  2003/12/10 21:41:26  dvd
 * +xsd
 *
 * Revision 1.1  2003/12/08 22:39:15  dvd
 * +drv
 *
 */
