#include <stdio.h>
#include "rx.h"

int main(int argc,char **argv) {
  int ok;
  rx_init();
  ok=rx_cmatch(*(argv+1),*(argv+2),strlen(*(argv+2)));
  fprintf(stderr,"%s\n",ok?"ok":"error");
  return !ok;
}

