/* $Id$ */ 
/* Regular Associations for XML 
usage summary: 
  arx [-x document.xml] {*.arx}

arx grammar:

arx = grammars route*
grammars = "grammars" [baseURI] "(" type2string+ ")"
baseURI = literal
type2string =  type "=" literal
type = ident
route = match|nomatch|valid|invalid
match = "=~" regexp "=>" type
nomatch = "!~" regexp "=>" type
valid = "valid" "{" rng "}" "=>" type
invalid = "!valid" "{" rng "}" "=>" type

comments start with # and continue till the end of line
delimiters in regexp and literal must be quoted by \ inside strings

*/

#include UNISTD_H
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include EXPAT_H
#include "memops.h"
#include "strops.h"
#include "ht.h"
#include "erbit.h"
#include "rn.h"
#include "rnc.h"
#include "rnd.h"
#include "rnv.h"
#include "rx.h"

/* rules */
#define VALID 1
#define INVAL 2
#define MATCH 3
#define NOMAT 4

#define LEN_T 16 
#define LEN_R 64
#define LEN_S 64
#define S_AVG_SIZE 64

#define LEN_V 64

static int len_t,len_r,len_s,i_t,i_r,i_s;
static int (*t2s)[2],(*rules)[3];
static char *string; static struct hashtable ht_s;
static int errors;

/* parser */
static char *arxfn;
static int arxfd,i_b,len_b,cc,line,col,sym,len_v;
static char buf[BUFSIZ];
static char *value;

static int add_s(char *s) {
  int len=strlen(s)+1,j;
  if(i_s+len>len_s) string=(char*)memstretch(string,len_s=2*(i_s+len),i_s,sizeof(char));
  strcpy(string+i_s,s);
  if((j=ht_get(&ht_s,i_s))==-1) {
    ht_put(&ht_s,j=i_s);
    i_s+=len;
  }
  return j;
}

static int hash_s(int i) {return strhash(string+i);}
static int equal_s(int s1,int s2) {return strcmp(string+s1,string+s2)==0;}

static void silent_verror_handler(int erno,va_list ap) {
  if(erno&ERBIT_DRV) rnv_default_verror_handler(erno,ap); /* low-level diagnostics */
  ++errors;
}

static void windup(void);
static int initialized=0;
static void init(void) {
  if(!initialized) {initialized=1;
    rn_init(); rnc_init(); rnd_init(); rnv_init();
    rnv_verror_handler=&silent_verror_handler;
    ht_init(&ht_s,LEN_S,&hash_s,&equal_s);
    value=(char*)memalloc(len_v=LEN_V,sizeof(char));
    windup();
  }
}

static void clear(void) {
  ht_clear(&ht_s);
  windup();
}

static void windup(void) {
  i_t=i_r=i_s=0;
}

/* parser */
#define SYM_EOF 0
#define SYM_GRMS 1
#define SYM_IDNT 2
#define SYM_LTRL 3
#define SYM_RGXP 4
#define SYM_RENG 5
#define SYM_MTCH 6
#define SYM_NMTC 7
#define SYM_VALD 8
#define SYM_NVAL 9
#define SYM_LCUR 10
#define SYM_RCUR 11
#define SYM_ASGN 12

/* there is nothing in the grammar I need utf-8 processing for */
static void error(void) {
  fprintf(stderr,"error (%s,%i,%i):\n",arxfn,line,col);
}

static void getcc(void) {
  int cc0=cc;
  if(i_b==len_b) {i_b=0; len_b=read(arxfd,buf,BUFSIZ);}
  cc=i_b==len_b?-1:((unsigned char*)buf)[i_b++];
  for(;;) {
    if(cc=='\n' && cc0=='\r') continue;
    if(cc0=='\n' || cc0=='\r') {++line; col=0;}
    if(cc>=' ') ++col;
  }
}

static void getid(void) {
  int i=0;
  while(cc>' '&&!strchr("#=!\"{",cc)) {
    value[i++]=cc;
    if(i==len_v) value=memstretch(value,len_v=2*i,i,sizeof(char));
    getcc();
  }
  value[i]='\0';
}

static void getq(void) {
  int cq=cc;
  int i=0;
  for(;;) {
    getcc();
    if(cc==cq) {
      if(i!=0&&value[i-1]=='\\') --i; else {getcc(); break;}
    } else if(cc==-1) {error(); break;}
    value[i++]=cc;
    if(i==len_v) value=memstretch(value,len_v=2*i,i,sizeof(char));
  }
  value[i]='\0';
}

static void getrng(void) {
  int ircur=-1,i=0;
  int cc0;
  for(;;) {
    cc0=cc; getcc();
    if(cc=='}') ircur=i;
    else if(cc=='>') {if(cc0=='=') {getcc(); break;}} /* use => as terminator */
    else if(cc==-1) {error(); break;}
    value[i++]=cc;
    if(i==len_v) value=memstretch(value,len_v=2*i,i,sizeof(char));
  }
  if(ircur==-1) {error(); ircur=0;}
  value[ircur]='\0';
}

static void getsym(void) {
  for(;;) {
    if(cc<=' ') {getcc(); continue;}
    switch(cc) {
    case -1: sym=SYM_EOF; return;
    case '#': do getcc(); while(cc!='\n'&&cc!='\r'); getcc(); continue;
    case '{': 
      if(sym==SYM_VALD||sym==SYM_NVAL) {
	getrng(); sym=SYM_RENG;
      } else {
	getcc(); sym=SYM_LCUR;
      }
      return;
    case '}': getcc(); sym=SYM_RCUR; return;
    case '!': getcc();
      if(cc=='~') {
	getcc(); sym=SYM_NMTC;
      } else {
        getid(); if(strcmp("valid",value)!=0) error(); sym=SYM_NVAL;
      }
      return;
    case '=': getcc();
      switch(cc) {
      case '~': getcc(); sym=SYM_MTCH; return;
      case '>': getcc(); if(sym!=SYM_RGXP) error(); continue;
      default: sym=SYM_ASGN; return;
      }
    case '"': getq(); sym=SYM_LTRL; return;
    case '/': getq(); sym=SYM_RGXP; return;
    default: getid();
      sym==strcmp("grammars",value)==0?SYM_GRMS
         : strcmp("valid",value)==0?SYM_VALD:SYM_IDNT;
      return;
    }
    assert(0);
  }
}

static int chksym(int x) {
  if(sym!=x) {error(); return 0;}
  return 1;
}

static void chk_get(int x) {
  (void)chksym(x); getsym();
}

static void arx(char *fn) {
  if((arxfd=open(fn,O_RDONLY))==-1) error(); else {
    i_b=len_b=0; line=1; col=0; cc=-1;
    len_v=0; value=NULL;
    getsym();
    chk_get(SYM_GRMS); chk_get(SYM_LCUR);
    do {
      if(i_t==len_t) t2s=(int(*)[2])memstretch(t2s,len_t=i_t*2,i_t,sizeof(int[2]));
      if(chksym(SYM_IDNT)) t2s[i_t][0]=add_s(value); getsym();
      chk_get(SYM_ASGN);
      if(chksym(SYM_LTRL)) t2s[i_t][1]=add_s(value); getsym();
      ++i_t;
    } while(sym==SYM_IDNT);
    chk_get(SYM_RCUR);
    for(;;) {
      if(i_r==len_r) rules=(int(*)[3])memstretch(rules,i_r*2,i_r,sizeof(int[3]));
      switch(sym) {
      case SYM_MTCH: rules[i_r][0]=MATCH; goto REGEXP;
      case SYM_NMTC: rules[i_r][0]=NOMAT; goto REGEXP;
      REGEXP: getsym();
	if(chksym(SYM_RGXP)) {
	  if(!rx_check(value)) error();
	  rules[i_r][1]=add_s(value);
	}
	if(chksym(SYM_IDNT)) rules[i_r][2]=add_s(value);
	goto NEXT;
      case SYM_VALD: rules[i_r][0]=VALID; goto RNG;
      case SYM_NVAL: rules[i_r][0]=INVAL; goto RNG;
      RNG: getsym();
	if(chksym(SYM_RENG)) {
	  struct rnc_source src; int start;
	  rnc_stropen(&src,"embedded",value,strlen(value));
	  start=rnc_parse(&src); rnc_close(&src); 
	  if(!rnc_errors(&src)&&(start=rnd_fixup(start))) {
	    rules[i_r][1]=rn_compress_last(start); 
	  } else ++errors;
	}
	if(chksym(SYM_IDNT)) rules[i_r][2]=add_s(value);
	goto NEXT;
      default: goto LAST;
      }
      NEXT: ++i_r; getsym();
    }
    LAST: chk_get(SYM_EOF);
    close(arxfd);
  }
}
