/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <string.h> /*strncmp*/
#include <stdio.h> /*stderr for error_handler*/
#include <assert.h>
#include "u.h" /*u_get,u_strlen*/
#include "strops.h"
#include "ht.h"
#include "ll.h"
#include "rx.h"

#define LEN_P RX_LEN_P
#define PRIME_P RX_PRIME_P
#define LIM_P RX_LIM_P
#define LEN_2 RX_LEN_2
#define PRIME_2 RX_PRIME_2
#define LEN_R RX_LEN_R
#define PRIME_R RX_PRIME_R
#define R_LEN 16

#define P_ERROR 0
#define P_EMPTY 1
#define P_NOT_ALLOWED  2
#define P_CHOICE 3
#define P_GROUP 4
#define P_ONE_OR_MORE 5 /*+*/
#define P_EXCEPT 6 /*single-single*/ 
#define P_RANGE 7 /*lower,upper inclusive*/
#define P_CLASS 8 /*complement is .-*/
#define P_ANY 9
#define P_CHAR 10

#define P_SIZE 3

#define P_TYP(i) (pattern[i][0]&0xF)
#define P_IS(i,x)  (P_##x==P_TYP(i))
#define P_CHK(i,x)  assert(P_IS(i,x))

#define P_binop(TYP,p,p1,p2) P_CHK(p,TYP); p1=pattern[p][1]; p2=pattern[p][2]
#define P_unop(TYP,p,p1) P_CHK(p,TYP); p1=pattern[p][1]
#define Empty(p) P_CHK(p,Empty)
#define NotAllowed(p) P_CHK(p,NotAllowed)
#define Any(p) P_CHK(p,Empty)
#define Choice(p,p1,p2) P_binop(CHOICE,p,p1,p2)
#define Group(p,p1,p2) P_binop(GROUP,p,p1,p2)
#define OneOreMore(p,p1) P_unop(ONE_OR_MORE,p,p1)
#define Except(p,p1,p2) P_binop(EXCEPT,p,p1,p2)
#define Range(p,cf,cl) P_binop(RANGE,p,cf,cl)
#define Class(p,cn) P_unop(CLASS,p,cn)
#define Char(p,c) P_unop(CHAR,p,c)

#define P_NUL 0x100

#define setNullable(x) if(x) pattern[i_p][0]|=P_NUL
#define nullable(p) (pattern[p][0]&P_NUL)

int rx_compact=0;

static char *regex;
static int (*pattern)[P_SIZE];
static int (*r2p)[2];
static struct hashtable ht_r,ht_p,ht_2;
static int i_p,len_p,i_r,len_r,i_2,len_2;
static int empty,notAllowed,any;

static int accept_p(void) { 
  int j; 
  if((j=ht_get(&ht_p,i_p))==-1) { 
    ht_put(&ht_p,j=i_p++); 
    if(i_p==len_p) { 
      int (*newpattern)[P_SIZE]=(int (*)[P_SIZE])calloc(len_p*=2,sizeof(int[P_SIZE])); 
      memcpy(newpattern,pattern,i_p*sizeof(int[P_SIZE])); free(pattern); 
      newpattern=pattern; 
    } 
  } 
  memset(pattern[i_p],0,sizeof(int[P_SIZE])); 
  return j; 
}

#define P_NEW(x) (pattern[i_p][0]=P_##x)

#define P_newbinop(TYP,p1,p2) P_NEW(TYP); pattern[i_p][1]=p1; pattern[i_p][2]=p2
#define P_newunop(TYP,p1) P_NEW(TYP); pattern[i_p][1]=p1
static int newEmpty(void) {P_NEW(EMPTY); setNullable(1); return accept_p();}
static int newAny(void) {P_NEW(ANY); return accept_p();}
static int newNotAllowed(void) {P_NEW(NOT_ALLOWED); return accept_p();}
static int newChoice(int p1,int p2) {P_newbinop(CHOICE,p1,p2); setNullable(nullable(p1)||nullable(p2)); return accept_p();}
static int newGroup(int p1,int p2) {P_newbinop(GROUP,p1,p2); setNullable(nullable(p1)&&nullable(p2)); return accept_p();}
static int newOneOrMore(int p1) {P_newunop(ONE_OR_MORE,p1); setNullable(nullable(p1)); return accept_p();}
static int newExcept(int p1,int p2) {P_newbinop(EXCEPT,p1,p2); return accept_p();}
static int newRange(int cf,int cl) {P_newbinop(RANGE,cf,cl); return accept_p();}
static int newClass(int cn) {P_newunop(CLASS,cn); return accept_p();}
static int newChar(int c) {P_newunop(CHAR,c); return accept_p();}

static int one_or_more(int p) {
  if(P_IS(p,EMPTY)) return p;
  if(P_IS(p,NOT_ALLOWED)) return p;
  return newOneOrMore(p);
}

static int group(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  if(P_IS(p1,EMPTY)) return p2;
  if(P_IS(p2,EMPTY)) return p1;
  return newGroup(p1,p2);
}

static int samechoice(int p1,int p2) {
  if(P_IS(p1,CHOICE)) {
    int p11,p12; Choice(p1,p11,p12);
    return p12==p2||samechoice(p11,p2);
  } else return p1==p2;
}

static int choice(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p2;
  if(P_IS(p2,NOT_ALLOWED)) return p1;
  if(P_IS(p2,CHOICE)) {
    int p21,p22; Choice(p2,p21,p22);
    p1=newChoice(p1,p21); return choice(p1,p22);
  }
  if(samechoice(p1,p2)) return p1;
  if(nullable(p1) && (P_IS(p2,EMPTY))) return p1;
  if(nullable(p2) && (P_IS(p1,EMPTY))) return p2;
  return newChoice(p1,p2);
}


static int equal_r(int r1,int r2) {return strcmp(regex+r1,regex+r2)==0;}
static int hash_r(int r) {return strhash(regex+r);}

static int equal_p(int p1,int p2) {
  int *pp1=pattern[p1],*pp2=pattern[p2];
  return (pp1[0]&0xF)==(pp2[0]&0xF) && pp1[1] == pp2[1]&&pp1[2] == pp2[2];
}
static int hash_p(int p) {
  int *pp=pattern[p];
  return ((pp[0]&0xF)|((pp[1]^pp[2])<<4))*PRIME_P;
}

static int equal_2(int x1,int x2) {return r2p[x1][0]==r2p[x2][0];}
static int hash_2(int x) {return r2p[x][0]*PRIME_2;}

static int add_r(char *rx) {
  int len=strlen(rx);
  if(i_r+len>len_r) {
    char *newregex=(char*)calloc(len_r=(i_r+len)*2,sizeof(char));
    memcpy(newregex,regex,i_r*sizeof(char)); free(regex);
    regex=newregex;
  }
  strcpy(regex+i_r,rx);
  return len+1;
}

static void default_error_handler(char *msg) {fprintf(stderr,"%s\n",msg);}
void (*rx_error_handler)(char *msg)=&default_error_handler;

static void windup(void);
static int initialized=0;
void rx_init(void) {
  if(!initialized) { initialized=1;
    pattern=(int (*)[P_SIZE])calloc(len_p=LEN_P,sizeof(int[P_SIZE]));
    r2p=(int (*)[2])calloc(len_2=LEN_2,sizeof(int[2]));
    regex=(char*)calloc(len_r=LEN_R,sizeof(char));

    ht_init(&ht_p,len_p,&hash_p,&equal_p);
    ht_init(&ht_2,len_2,&hash_2,&equal_2);
    ht_init(&ht_r,len_r/R_LEN,&hash_r,&equal_r);

    windup();
  }
}

static void clear(void) {
  ht_clear(&ht_p); ht_clear(&ht_2); ht_clear(&ht_r);
  windup();
}

static void windup(void) {
  i_p=i_r=i_2=0;
  memset(pattern[0],0,sizeof(int[P_SIZE]));
  pattern[0][0]=P_ERROR;  accept_p(); 
  empty=newEmpty(); notAllowed=newNotAllowed(); any=newAny();
}

#define SYM_END 0
#define SYM_CLS 1
#define SYM_ESC 2
#define SYM_CHR 3

static int r0,ri,sym,val,errors;

#define ERRMSG "bad character in \"%s\" at offset %i"

static void error(void) {
  if(!errors) {
    int n=strlen(ERRMSG)+strlen(regex+r0)+11;
    char *buf=(char*)calloc(n,sizeof(char));
    fprintf(stderr,"r0=%i ri=%i\n",r0,ri);
    snprintf(buf,n,ERRMSG,regex+r0,u_strlen(regex+r0)-u_strlen(regex+ri));
    (*rx_error_handler)(buf);
    free(buf);
  }
  errors=1;
}

#include "rx_cls_u.c"

static int chclass(void) {
  int u,cl,rj;
  ri+=u_get(&u,regex+ri);
  if(u=='\0') {--ri; error(); return 0;}
  if(u!='{') {error(); return 0;}
  rj=ri;
  for(;;) {
    if(regex[rj]=='\0') {ri=rj; error(); return 0;}
    if(regex[rj]=='}') {
      int n=0,m=NUM_CLS_U-1,i,cmp;
      for(;;) {
	if(n>m) {error(); cl=0; break;}
	i=(n+m)/2;
	cmp=strncmp(regex+ri,clstab[i],rj-ri);
	if(cmp==0) {cl=i; break;} else if(cmp<0) m=i-1; else n=i+1;
      }
      ri=rj+1;
      return cl;
    }
    ++rj;
  }
}

#define CLS_NL (NUM_CLS_U+1)
#define CLS_S (NUM_CLS_U+2)
#define CLS_I (NUM_CLS_U+3)
#define CLS_C (NUM_CLS_U+4)
#define CLS_W (NUM_CLS_U+5)
#define NUM_CLS (NUM_CLS_U+6)

static void getsym(void) {
  int u;
  if(regex[ri]=='\0') sym=SYM_END; else {
    ri+=u_get(&u,regex+ri); 
    if(u=='\\') {
      ri+=u_get(&u,regex+ri);
      switch(u) {
      case '\0': --ri; error(); sym=SYM_END; break;
      case 'p': sym=SYM_CLS; val=chclass(); break;
      case 'P': sym=SYM_CLS; val=-chclass(); break;
      case 's': sym=SYM_CLS; val=CLS_S; break;
      case 'S': sym=SYM_CLS; val=-CLS_S; break;
      case 'i': sym=SYM_CLS; val=CLS_I; break;
      case 'I': sym=SYM_CLS; val=-CLS_I; break;
      case 'c': sym=SYM_CLS; val=CLS_C; break;
      case 'C': sym=SYM_CLS; val=-CLS_C; break;
      case 'd': sym=SYM_CLS; val=CLS_U_Nd; break;
      case 'D': sym=SYM_CLS; val=-CLS_U_Nd; break;
      case 'w': sym=SYM_CLS; val=CLS_W; break;
      case 'W': sym=SYM_CLS; val=-CLS_W; break;
      case 'n': sym=SYM_ESC; val=0xA; break;
      case 'r': sym=SYM_ESC; val=0xD; break;
      case 't': sym=SYM_ESC; val=0x9; break;
      case '\\': case '|': case '.': case '-': case '^': case '?': case '*': case '+': 
      case '{': case '}': case '[': case ']': case '(': case ')':
        sym=SYM_ESC; val=u; break;
      default: error(); sym=SYM_ESC; val=u; break;
      }
    } else {
      switch(u) {
      case '.': sym=SYM_CLS; val=-CLS_NL; break;
      default: sym=SYM_CHR; val=u; break;
      }
    }
  }
}

static void chk_get(int v) {if(sym!=SYM_CHR||val!=v) error(); getsym();}

static int chgroup(void) {
  int p=notAllowed,c;
  for(;;) {
    switch(sym) {
    case SYM_CHR: if(val==']'||(val=='-'&&regex[ri]=='[')) goto END_OF_GROUP;
    case SYM_ESC: c=val; getsym();
      if(sym==SYM_CHR&&val=='-') {
	if(regex[ri]=='[') {
	  p=choice(p,newChar(c));
	  goto END_OF_GROUP;
	} else if(regex[ri]==']') {
	  p=choice(p,newChar(c));
	} else {
	  getsym();
	  switch(sym) {
	  case SYM_CHR:
	  case SYM_ESC: p=choice(p,newRange(c,val)); getsym(); break;
	  default: error(); getsym(); break;
	  }
	}
      } else {
	p=choice(p,newChar(c));
      }
      break;
    case SYM_CLS: p=choice(p,newClass(val)); getsym(); break;
    case SYM_END: error(); goto END_OF_GROUP;
    default: assert(0);
    }
  }
  END_OF_GROUP:;
  return p;
}

static int chexpr(void) {
  int p;
  if(sym==SYM_CHR&&val=='^') { getsym(); 
    p=newExcept(any,chgroup());
  } else {
    p=chgroup();
  }
  if(sym==SYM_CHR&&val=='-') { getsym(); 
    chk_get('['); p=newExcept(p,chexpr()); chk_get(']');
  }
  return p;
}

static int expression(void);
static int atom(void) {
  int p=0;
  switch(sym) {
  case SYM_CHR:
    switch(val) {
    case '[': getsym(); p=chexpr(); chk_get(']'); break;
    case '(': getsym(); p=expression(); chk_get(')'); break;
    case '{': case '?': case '*': case '+': case '|': 
    case ')': case ']': case '}': error(); getsym(); break;
    default: p=newChar(val); getsym(); break;
    }
    break;
  case SYM_ESC: p=newChar(val); getsym(); break;
  case SYM_CLS: p=newClass(val); getsym(); break;
  default: error(); getsym(); break;
  }
  return p;
}

static int number(void) {
  int n=0,m;
  for(;;) {
    if(sym!=SYM_CHR) goto END_OF_DIGITS;
    switch(val) {
    case '0': m=0; break;
    case '1': m=1; break;
    case '2': m=2; break;
    case '3': m=3; break;
    case '4': m=4; break;
    case '5': m=5; break;
    case '6': m=6; break;
    case '7': m=7; break;
    case '8': m=8; break;
    case '9': m=9; break;
    default: goto END_OF_DIGITS;
    }
    n=n*10+m;
  }
  END_OF_DIGITS:;
  return n;
}

static int quantifier(int p0) {
  int p=empty,n;
  n=number();
  while(n--) p=group(p,p0);
  if(sym==SYM_CHR) {
    if(val==',') {
      getsym();
      if(sym==SYM_CHR && val=='}') {
	p=group(p,choice(empty,one_or_more(p0)));
      } else {
	n=number();
	while(n--) p=group(p,choice(empty,p0));
      }
    }
  } else error();
  return p;
}

static int piece(void) {
  int p;
  p=atom();
  if(sym==SYM_CHR) {
    switch(val) {
    case '{': getsym(); p=quantifier(p); chk_get('}'); break;
    case '?': getsym(); p=choice(empty,p); break;
    case '*': getsym(); p=choice(empty,one_or_more(p)); break;
    case '+': getsym(); p=one_or_more(p); break;
    default: break;
    } 
  }
  return p;
}

static int branch(void) {
  int p;
  p=empty;
  while(!(sym==SYM_END||(sym==SYM_CHR&&(val=='|'||val==')')))) p=group(p,piece());
  return p;
}

static int expression(void) {
  int p;
  p=branch();
  while(sym==SYM_CHR&&val=='|') {
    getsym();
    p=choice(p,branch());
  }
  return p;
}

static void bind(int r) {
  r0=ri=r; sym=-1; errors=0; 
  getsym();
}

static int compile(char *rx) {
  int r=0,p=0,d_r;
  d_r=add_r(rx);
  if((r=ht_get(&ht_r,i_r))==-1) {
    if(i_p>=LIM_P) {clear(); d_r=add_r(rx);}
    ht_put(&ht_r,r=i_r);
    i_r+=d_r;
    bind(r); p=expression(); if(sym!=SYM_END) error();
    r2p[i_2][0]=r; r2p[i_2][1]=p;
    ht_put(&ht_2,i_2++);
    if(i_2==len_2) {
      int (*newr2p)[2]=(int (*)[2])calloc(len_2*=2,sizeof(int[2]));
      memcpy(newr2p,r2p,i_2*sizeof(int[2])); free(r2p);
      r2p=newr2p;
    }
  } else {
    r2p[i_2][0]=r;
    p=r2p[ht_get(&ht_2,i_2)][1];
  }
  return p;
}

static int drv(int p,int c) {
  int p1,p2,cf,cl,cn;
  assert(!P_IS(p,ERROR));
  switch(P_TYP(p)) {
  case  P_EMPTY: case  P_NOT_ALLOWED: p=notAllowed; break;
  case  P_CHOICE: Choice(p,p1,p2); p=choice(drv(p1,c),drv(p2,c)); break;
  case  P_GROUP: Group(p,p1,p2); {int p11=group(drv(p1,c),p2); p=nullable(p1)?choice(p11,drv(p2,c)):p11;} break;
  case  P_ONE_OR_MORE: OneOreMore(p,p1); p=group(drv(p1,c),choice(empty,p)); break;
  case  P_EXCEPT: Except(p,p1,p2); p=nullable(drv(p1,c))&&!nullable(drv(p2,c))?empty:notAllowed; break;
  case  P_RANGE: Range(p,cf,cl); p=cf<=c&&c<=cl?empty:notAllowed; break;
  case  P_CLASS: Class(p,cn); p=empty; break;
  case  P_ANY: p=empty; break;
  case  P_CHAR: Char(p,cf); p=c==cf?empty:notAllowed; break;
  default: assert(0);
  }
  return p;
}

static int match(int p,char *s,int n) {
  int u,l;
  while(n!=0) {l=u_get(&u,s); n-=l; s+=l; p=drv(p,u);}
  return nullable(p);
}

int rx_match(char *rx,char *s,int n) {
  int p=compile(rx);
  return !errors&&match(p,s,n);
}

void rx_dump(void) {
  int i;
  for(i=0;i!=i_p;++i) {
    printf("%i: ",i);
    switch(P_TYP(i)) {
    case  P_ERROR: printf("?"); break;
    case  P_EMPTY: printf("-"); break;
    case  P_NOT_ALLOWED: printf("!"); break;
    case  P_CHOICE: printf("#%i|#%i",pattern[i][1],pattern[i][2]); break;
    case  P_GROUP: printf("#%i#%i",pattern[i][1],pattern[i][2]); break;
    case  P_ONE_OR_MORE: printf("#%i+",pattern[i][1]); break;
    case  P_EXCEPT: printf("#%i/#%i",pattern[i][1],pattern[i][2]); break;
    case  P_RANGE: printf("%c-%c",pattern[i][1],pattern[i][2]); break;
    case  P_CLASS: printf("@%i",pattern[i][1]); break;
    case  P_ANY: printf("."); break;
    case  P_CHAR: printf("%c",pattern[i][1]); break;
    default: assert(0);
    }
    printf("\n");
  }
}
