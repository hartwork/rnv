/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include "rnc.h"
#include "rnd.h"

static int parse(struct rnc_source *sp) {
  int start;

  fprintf(stderr,"parsing\n");
  start=rnc_parse(sp);
  rnc_close(sp); if(rnc_errors(sp)) return 0;

  fprintf(stderr,"dereferencing\n");
  rnd_deref(start); if(rnd_errors()) return 0;

  fprintf(stderr,"checking restrictions\n");
  rnd_restrictions(); if(rnd_errors()) return 0;

  fprintf(stderr,"computing auxiliary traits\n");
  rnd_traits();
  start=rnd_release();

  fprintf(stderr,"start=%i\n",start);

  return 1;
}

int main(int argc,char **argv) {
  struct rnc_source *sp;

  rnc_init();
  rnd_init();

  sp=rnc_alloc();
  if(*(++argv)) {
    do {
      fprintf(stderr,"\n*** processing '%s'\n",*argv);
      if(rnc_open(sp,*argv)!=-1) if(!parse(sp)) goto ERRORS;
      rnc_close(sp);
    } while(*(++argv));
  } else {
    rnc_bind(sp,"stdin",0);
    if(!parse(sp)) goto ERRORS;
  }
  rnc_free(sp);

  return 0;

ERRORS:
  fprintf(stderr,"exiting on errors \n");
  return 1;
}

/* 
 * $Log$
 * Revision 1.2  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 */
