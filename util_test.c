#include <stdio.h>
#include <string.h>
#include "util.h"

#if 0
/* abspath */
char *samples[]={
  "file.txt","/otherfile/name",
  "some/file.txt","/otherfile/name",
  "/path/to/file.txt","/otherfile/name",
  "file.txt","otherfile/name",
  "some/file.txt","otherfile/name",
  "/path/to/file.txt","otherfile/name",
  "file.txt","name",
  "some/file.txt","name",
  "/path/to/file.txt","name",
  0};
int main() {
  int i=0;
  char buf[1024];
  while(samples[i]) {
    strcpy(buf,samples[i++]);
    abspath(buf,samples[i++]);
    printf("%s\n",buf);
  }
  return 0;
}
#endif

#if 1
int main(int argc,char **argv) {
  char *s1=*(++argv),*s2=*(++argv);
  int cmp=tokncmp(s1,s2,strlen(s2));
  printf("%s%c%s\n",s1,cmp<0?'<':cmp>0?'>':'=',s2);
  return 0;
}
#endif
