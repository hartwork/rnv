#include <stdio.h>

char *(param_data[])[2]={{"a","b"},{"c","d"},{"e","f"},{(char*)0,(char*)0}};

int main() {
  char *(*param)[2];
  int i;
  param=param_data;
  i=0;
  while(param[i][0]!=0) {
    printf("%s=%s\n",param[i][0],param[i][1]);
    ++i;
  }
  return 0;

}
