/* $Id$ */

#include "util.h"

int xml_white_space(int v) {return v=='\t'||v=='\n'||v=='\r'||v==' ';}

int tokncmp(char *s1,char *s2,int n2) {
  char *end2=s2+n2;
  while(xml_white_space(*s1)) ++s1;
  while(s2!=end2&&xml_white_space(*s2)) ++s2;
  for(;;) {
    if(s2==end2) {
      while(xml_white_space(*s1)) ++s1;
      return *s1;
    }
    if(*s1=='\0') {
      while(s2!=end2&&xml_white_space(*s2)) ++s2;
      return s2==end2?0:-*s2;
    }
    if(xml_white_space(*s1)&&xml_white_space(*s2)) {
      do ++s1; while(xml_white_space(*s1));
      do ++s2; while(s2!=end2&&xml_white_space(*s2));
    } else {
      if(*s1!=*s2) return *s1-*s2;
      ++s1; ++s2;
    }
  }
}

int strhash(char *s) {
  int h=0; 
  while(*s) h=h*31+*(s++); 
  return h;
}

extern char *abspath(char *r,char *b) {
  if(*r!='/') {
    char *c=b,*sep=(char*)0;
    for(;;) {if(!(*c)) break; if(*c++=='/') sep=c;}
    if(sep) {
      char *p,*q;
      p=r; while(*p++); q=p+(sep-b);
      do *(--q)=*(--p); while(p!=r);
      while(b!=sep) *r++=*b++;
    }
  }
  return r;
}
