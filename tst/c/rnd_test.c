/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include "rnc.h"
#include "rnd.h"
#include "rn.h"
#include "rnx.h"

int starts[1024];
int n_st=0;

static int parse(struct rnc_source *sp) {
  int start;

  fprintf(stderr,"parsing\n");
  start=rnc_parse(sp);
  rnc_close(sp); if(rnc_errors(sp)) return 0;

  fprintf(stderr,"fixing\n");
  start=rnd_fixup(start);

#if 0
  fprintf(stderr,"collecting garbage\n");
  start=rn_compress_last(start);
#endif

  fprintf(stderr,"start=%i\n",start);

  starts[n_st++]=start;

  return 1;
}

int main(int argc,char **argv) {
  struct rnc_source src;

  rnc_init();
  rnd_init();
  rnx_init();


  if(*(++argv)) {
    do {
      fprintf(stderr,"\n*** processing '%s'\n",*argv);
      if(rnc_open(&src,*argv)!=-1) if(!parse(&src)) goto ERRORS;
      rnc_close(&src);
    } while(*(++argv));
  } else {
    rnc_bind(&src,"stdin",0);
    if(!parse(&src)) goto ERRORS;
  }

  rn_compress(starts,n_st);

  while(n_st--) printf("start=%i\n",starts[n_st]);

  return 0;

ERRORS:
  fprintf(stderr,"exiting on errors \n");
  return 1;
}
