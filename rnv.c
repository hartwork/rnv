/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <xmlparse.h>
#include "rnc.h"
#include "rnd.h"
#include "drv.h"

#define RNV_VERSION "1.0.0"

static int start;

static void init() {
  rnc_init();
  drv_init();
}

static int load_rnc(char *fn) {
  struct rnc_source *sp=rnc_alloc();
  if(rnc_open(sp,fn)!=-1) start=rnc_parse(sp); rnc_close(sp); 
  rnc_free(sp); if(rnc_errors(sp)) return 0;
  
  rnd_deref(start); if(rnd_errors()) return 0;
  rnd_restrictions(); if(rnd_errors()) return 0;
  rnd_traits();

  start=rnd_release(); 
  return 1;
}

int main(int argc,char **argv) {
  init();

  if(argc<2||3<argc) {
    fprintf(stderr,"rnv version  %s\nusage: rnv schema.rnc [document.xml]\n",RNV_VERSION);
    goto ERRORS;
  }
  
  if(load_rnc(*(++argv))) {
    return 0;
  }

ERRORS:
  fprintf(stderr,"exiting on errors\n");
  return 1;
}


/*
 * $Log$
 * Revision 1.9  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 */
