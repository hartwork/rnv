/* $Id$ */

#ifndef LL_H
#define LL_H 1

/* all limits that can affect speed or memory consumption; 
 prefixes correspond to module names 
 */

#define RN_LEN_P 1024
#define RN_PRIME_P 0x3fd
#define RN_LIM_P 4*RN_LEN_P
#define RN_LEN_NC 256
#define RN_PRIME_NC 0xfb
#define RN_LEN_S 256

#define SC_LEN 64

#define DRV_LEN_M 1024
#define DRV_PRIME_M 0xffd
#define DRV_LIM_M 8*DRV_LEN_M

#define RNX_LEN_EXP 16
#define RNX_LIM_EXP 64

#define RNV_LEN_T 1024
#define RNV_LIM_T 16384

#define RX_LEN_P 256
#define RX_PRIME_P 0xfb
#define RX_LIM_P 4*RX_LEN_P
#define RX_LEN_R RX_LEN_P
#define RX_PRIME_R RX_PRIME_P
#define RX_LEN_2 32
#define RX_PRIME_2 0x1f
#define RX_LEN_M 1024
#define RX_PRIME_M 0x3fd
#define RX_LIM_M 8*RX_LEN_M

#endif
