#include <stdio.h>
#include <string.h>
#include "util.h"

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
