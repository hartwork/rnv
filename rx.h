/* $Id$ */

#ifndef RX_H
#define RX_H

extern int rx_compact;

extern void (*rx_error_handler)(char *msg);
extern void rx_init(void);

/* 
 returns positive value if the s[0..n] ~= rx, 0 if not, -1 on regex error; 
 rx and s are in utf-8, rx is 0-terminated, s is n bytes long;
 rmatch replaces white space in s with 0x20,
 cmatch collapses white space. 
 */
extern int rx_match(char *rx,char *s,int n);
extern int rx_rmatch(char *rx,char *s,int n);
extern int rx_cmatch(char *rx,char *s,int n);

#endif
