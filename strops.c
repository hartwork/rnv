/* $Id$ */

#include <stdlib.h> /*calloc*/
#include <string.h> /*strcpy,strlen*/
#include "xmlc.h"
#include "strops.h"

int strcmpn(char *s1,char *s2,int n2) {
  char *end=s2+n2;
  for(;;) {
    if(s2==end) return *s1;
    if(*s1=='\0') return -*s2;
    if(*s1!=*s2) return *s1-*s2;
    ++s1; ++s2;
  }
}

int tokcmpn(char *s1,char *s2,int n2) {
  char *end2=s2+n2;
 /* all white space characters are one byte long */
  while(xmlc_white_space(*s1)) ++s1; 
  while(s2!=end2&&xmlc_white_space(*s2)) ++s2;
  for(;;) {
    if(s2==end2) {
      while(xmlc_white_space(*s1)) ++s1;
      return *s1;
    }
    if(*s1=='\0') {
      while(s2!=end2&&xmlc_white_space(*s2)) ++s2;
      return s2==end2?0:-*s2;
    }
    if(xmlc_white_space(*s1)&&xmlc_white_space(*s2)) {
      do ++s1; while(xmlc_white_space(*s1));
      do ++s2; while(s2!=end2&&xmlc_white_space(*s2));
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

char *strclone(char *s) {
  return strcpy((char*)calloc(strlen(s)+1,sizeof(char)),s);
}

char *abspath(char *r,char *b) {
  if(*r!='/') {
    char *c=b,*sep=(char*)0;
    for(;;) {if(!(*c)) break; if(*c++=='/') sep=c;}
    if(sep) {
      char *p,*q;
      p=r; while(*p++); q=p+(sep-b);
      do *(--q)=*(--p); while(p!=r);
      p=r; while(b!=sep) *p++=*b++;
    }
  }
  return r;
}

int strtab(char *s,char *tab[],int size) {return strntab(s,strlen(s),tab,size);}
int strntab(char *s,int len,char *tab[],int size) {
  int n=0,m=size-1,i,cmp;
  for(;;) {
    if(n>m) return size;
    i=(n+m)/2;
    if((cmp=strcmpn(tab[i],s,len))==0) return i; else {if(cmp>0) m=i-1; else n=i+1;}
  }
}
