/* $Id$ */

/* Regular Associations for XML 
usage summary: 
  arx [-x document.xml] {*.arx}

arx grammar:

*/

#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnv.h"
#include "rx.h"

/* rules */
#define VALID 0
#define INVAL 1
#define MATCH 2
#define INMAT 3

#define LEN_T 16 
#define LEN_R 64
#define LEN_S 64
#define S_AVG_SIZE 64

static int len_t,len_r,len_s,i_t,i_r,i_s;

static int (*t2s)[2];
static int (*rules)[3];
static char *string;

static int initialized=0;
static int init() {
  if(!initialized) {initialized=1;
  }
}
