/* $Id$ */

#ifndef DRV_H
#define DRV_H 1

extern void drv_init();
extern void drv_clear();

/* Expat passes character data unterminated.  Hence functions that can deal with cdata expect the length of the data */

extern void drv_add_dtl(char *suri,int (*equal)(char *typ,char *val,char *s,int n),int (*allows)(char *typ,char *ps,char *s,int n));

extern int drv_start_tag_open(int p,char *suri,char *sname);
extern int drv_attribute(int p,char *suri,char *sname,char *s);
extern int drv_start_tag_close(int p);
extern int drv_text(int p,char *s,int n);
extern int drv_end_tag(int p);

#endif

/*
 * $Log$
 * Revision 1.5  2003/12/12 22:21:06  dvd
 * drv written, compiled, not yet debugged
 *
 * Revision 1.4  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 * Revision 1.3  2003/12/10 22:23:52  dvd
 * *** empty log message ***
 *
 * Revision 1.2  2003/12/10 21:41:26  dvd
 * +xsd
 *
 * Revision 1.1  2003/12/08 22:39:15  dvd
 * +drv
 *
 */
