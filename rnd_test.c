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

  fprintf(stderr,"dereferencing\n");
  rnd_deref(start); if(rnd_errors()) return 0;

  fprintf(stderr,"checking restrictions\n");
  rnd_restrictions(); if(rnd_errors()) return 0;

  fprintf(stderr,"computing auxiliary traits\n");
  rnd_traits();
  start=rnd_release();

#if 0
  start=rn_compress_last(start);
#endif

  fprintf(stderr,"start=%i\n",start);

  starts[n_st++]=start;

  return 1;
}

static void dump(void) {
  int p=1; char *s;
  while(P_TYP(p)) {
    if(!P_IS(p,VOID)) {
      fprintf(stderr,"%c%4i: %s\n",marked(p)?'+':' ',p,s=p2str(p));
      free(s);
    }
    ++p;
  }
}

int main(int argc,char **argv) {
  struct rnc_source *sp;

  rnc_init();
  rnd_init();
  rnx_init();


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

  rn_compress(starts,n_st);

  while(n_st--) printf("start=%i\n",starts[n_st]);

  dump();

  return 0;

ERRORS:
  fprintf(stderr,"exiting on errors \n");
  return 1;
}
