/* $Id$ */

#include <stdarg.h>

#ifndef RX_H
#define RX_H

#define RXER_BADCH 0
#define RXER_UNFIN 1
#define RXER_NOLSQ 2
#define RXER_NORSQ 3
#define RXER_NOLCU 4
#define RXER_NORCU 5
#define RXER_NOLPA 6
#define RXER_NORPA 7
#define RXER_BADCL 8
#define RXER_NODGT 9

extern void (*rx_verror_handler)(int erno,va_list ap);
extern int rx_compact;

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
