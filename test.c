#include <stdio.h>
#include <string.h>
#include "util.h"

int main(int argc,char **argv) {
  char *s1=*(++argv),*s2=*(++argv);
  int cmp=tokncmp(s1,s2,strlen(s2));
  printf("%s%c%s\n",s1,cmp<0?'<':cmp>0?'>':'=',s2);
  return 0;
}
