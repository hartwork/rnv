#include <stdlib.h>
#include <stdio.h>
#include "rnc.h"
#include "rnd.h"

int main(int argc,char **argv) {
  struct rnc_source *sp=rnc_alloc();
  int start;
  rnc_init();
  if(*(++argv)) rnc_open(sp,*argv); else rnc_bind(sp,"stdin",0);
  fprintf(stderr,"parsing\n");
  start=rnc_parse(sp);
  rnc_close(sp); if(rnc_errors(sp)) {goto ERRORS;}
  free(sp);
  fprintf(stderr,"dereferencing\n");
  rnd_deref(start); if(rnd_errors()) goto ERRORS;
  fprintf(stderr,"checking restrictions\n");
  rnd_restrictions(); if(rnd_errors()) goto ERRORS;
  fprintf(stderr,"computing auxiliary traits\n");
  rnd_traits();
  rnd_release();
  return 0;
ERRORS:
  fprintf(stderr,"exiting on errors \n");
  return 1;
}

