/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include "rnc.h"
#include "rnd.h"
#include "rn.h"
#include "rnx.h"

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

void dump() {
  int p=1,q; char *s;
  while(P_TYP(p)) {
    fprintf(stderr,"%s\n",s=p2str(p));
    free(s);
    rnx_expected(p);
    if(rnx_n_exp!=0) {
      int i;
      fprintf(stderr,">\n");
      for(i=0;i!=rnx_n_exp;++i) {
	fprintf(stderr,"\t%s\n",s=p2str(rnx_exp[i]));
	free(s);
      }
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

  dump();

  return 0;

ERRORS:
  fprintf(stderr,"exiting on errors \n");
  return 1;
}

/* 
 * $Log$
 * Revision 1.4  2003/12/14 15:21:49  dvd
 * much better hash functions
 *
 * Revision 1.3  2003/12/13 22:03:31  dvd
 * rnv works
 *
 * Revision 1.2  2003/12/11 23:37:58  dvd
 * derivative in progress
 *
 */
