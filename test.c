#include <stdio.h>
#include <string.h>
#include "u.h"

int main(int argc,char **argv) {
  printf("%i\n",u_strlen(*(argv+1)));
  return 0;
}
