/* $Id$ */

#include <fcntl.h> /* open, read, close */
#include <unistd.h> /* read, close */
#include <string.h> /* memcpy */
#include <stdlib.h> /* calloc,malloc,free */
#include <stdio.h> /*stderr*/
#include <stdarg.h> /*va_list,va_arg,va_end*/
#include <assert.h> /*assert*/

#include "u.h"
#include "er.h"
#include "rnc.h"

#define NKWD 19
static char *kwdtab[NKWD]={
  "attribute", "datatypes", "default", "div", "element", "empty", "external", 
  "grammar", "include", "inherit", "list", "mixed", "namespace", "notAllowed", 
  "parent", "start", "string", "text", "token"};

#define SYM_EOF -1

#define SYM_ATTRIBUTE 0
#define SYM_DEFAULT 1
#define SYM_DATATYPES 2
#define SYM_DIV 3
#define SYM_ELEMENT 4
#define SYM_EMPTY 5
#define SYM_EXTERNAL 6
#define SYM_GRAMMAR 7
#define SYM_INCLUDE 8
#define SYM_INHERIT 9
#define SYM_LIST 10
#define SYM_MIXED 11
#define SYM_NAMESPACE 12
#define SYM_NOT_ALLOWED 13
#define SYM_PARENT 14
#define SYM_START 15
#define SYM_STRING 16
#define SYM_TEXT 17
#define SYM_TOKEN 18

#define SYM_IDENT 19
#define SYM_QNAME 20

#define SYM_NSNAME 21

#define SYM_ASGN 22
#define SYM_ASGN_ILEAVE 23
#define SYM_ASGN_CHOICE 24
#define SYM_GROUP 25 /* , */
#define SYM_CHOICE 26 
#define SYM_ILEAVE 27
#define SYM_OPTIONAL 28
#define SYM_ZERO_OR_MORE 29
#define SYM_ONE_OR_MORE 30
#define SYM_LPAR 31
#define SYM_RPAR 32
#define SYM_LCUR 33
#define SYM_RCUR 34
#define SYM_LSQU 35
#define SYM_RSQU 36
#define SYM_EXCEPT 37
#define SYM_CONCAT 38
#define SYM_ANY_NAME SYM_ZERO_OR_MORE /* both are * */
#define SYM_QUOTE 39  /* \ */
#define SYM_FOLLOW_ANNOTATION 40 /* >> */
#define SYM_DOCUMENTATION 41 /* ## */
#define SYM_LITERAL 42

#define BUFSIZE 1024
#define BUFTAIL 6

#define SRC_FREE 1
#define SRC_CLOSE 2
#define SRC_ERRORS 4

struct sym {
  char *s; int slen;
  int line,col;
  int sym;
};

struct rnc_source {
  int flags;
  char *fn; int fd;
  char *buf; int i,n;
  int complete;
  int line,col;
  int u,v,w; int nx;
  int cur;
  struct sym sym[2];
};

#define CUR(sp) ((sp)->sym[(sp)->cur])
#define NXT(sp) ((sp)->sym[!(sp)->cur])

static void rnc_init(struct rnc_source *sp);
static int rnc_read(struct rnc_source *sp);

int rnc_stropen(struct rnc_source *sp,char *fn,char *s,int len) {
  rnc_init(sp);
  sp->buf=s; sp->n=len; sp->complete=1; 
  return 0;
}

int rnc_bind(struct rnc_source *sp,char *fn,int fd) {
  rnc_init(sp);
  sp->fn=fn; sp->fd=fd;
  sp->buf=(char*)calloc(BUFSIZE,sizeof(char));
  sp->complete=sp->fd==-1;
  sp->flags=SRC_FREE;
  rnc_read(sp);
  return sp->fd;
}

int rnc_open(struct rnc_source *sp,char *fn) {
  sp->flags|=SRC_CLOSE;
  return rnc_bind(sp,fn,open(fn,O_RDONLY));
}

int rnc_close(struct rnc_source *sp) {
  int ret=0;
  {int i; for(i=0;i!=2;++i) if(sp->sym[i].s) free(sp->sym[i].s);}
  if(sp->flags&SRC_FREE) free(sp->buf); 
  sp->buf=NULL;
  sp->complete=-1;
  if(sp->flags&SRC_CLOSE) {
    if(sp->fd!=-1) {
      ret=close(sp->fd); sp->fd=-1;
    }
  }
  return ret;
}

static void rnc_init(struct rnc_source *sp) {
  sp->flags=0;
  sp->fn=sp->buf=NULL;
  sp->i=sp->n=0; 
  sp->complete=sp->fd=-1;
  sp->nx=-1;
  sp->u=-1; sp->v=0; sp->line=1; sp->col=1;
  sp->cur=0;
  {int i; for(i=0;i!=2;++i) sp->sym[i].s=(char*)calloc(sp->sym[i].slen=BUFSIZE,sizeof(char));}
}

static int rnc_read(struct rnc_source *sp) {
  int ni;
  memcpy(sp->buf,sp->buf+sp->i,sp->n-=sp->i);
  sp->i=0;
  for(;;) {
    ni=read(sp->fd,sp->buf+sp->n,BUFSIZE-sp->n);
    if(ni>0) {
      sp->n+=ni;
      if(sp->n>=BUFTAIL) break;
    } else {
      close(sp->fd); sp->fd=-1;
      sp->complete=1;
      break;
    }
  }
  return ni;
}

static void error(struct rnc_source *sp,int er_no,...) {
  va_list ap;
  va_start(ap,er_no);
  (*ver_handler_p)(er_no,ap);
  va_end(ap);
  sp->flags|=SRC_ERRORS;
}

/* read utf8 */
static void getu(struct rnc_source *sp) {
  int n,u0=sp->u;
  for(;;) {
    if(!sp->complete&&sp->i>sp->n-BUFTAIL) {
      if(rnc_read(sp)==-1) {error(sp,ER_IO,sp->fn);}
    }
    if(sp->i==sp->n) {
      sp->u=(u0=='\n'||u0=='\r'||u0==-1)?-1:'\n'; 
      u0=-1;
      break;
    } /* eof */
    n=u_get(&sp->u,sp->buf+sp->i);
    if(n==0) { 
      error(sp,ER_UTF,sp->fn,sp->line,sp->col);
      ++sp->i;
      continue;
    } else if(n+sp->i>sp->n) { 
      error(sp,ER_UTF,sp->fn,sp->line,sp->col);
      sp->i=sp->n;
      continue;
    } else {
      sp->i+=n;
      if(u0=='\r'&&sp->u=='\n') {u0='\n'; continue;}
    }
    break;
  }
  if(u0!=-1) {
    if(u0=='\r'||u0=='\n') {++sp->line; sp->col=0;} 
    if(!(sp->u=='\r'||sp->u=='\n')) {++sp->col;}
  }
}

/* newlines are replaced with \0; \x{<hex>+} are unescaped.
the result is in sp->v
*/
static void getv(struct rnc_source *sp) {
  if(sp->nx>0) {
    sp->v='x'; --sp->nx;
  } else if(sp->nx==0) {
    sp->v=sp->w;
    sp->nx=-1;
  } else {
    getu(sp);
    switch(sp->u) {
    case '\r': case '\n': sp->v=0; break;
    case '\\':
      getu(sp);
      if(sp->u=='x') {
	sp->nx=0;
	do {
	  ++sp->nx;
	  getu(sp);
	} while(sp->u=='x');
	if(sp->u=='{') { 
	  sp->nx=-1;
	  sp->v=0; 
	  for(;;) {
	    getu(sp);
	    if(sp->u=='}') goto END_OF_HEX_DIGITS;
	    sp->v<<=4;
	    switch(sp->u) {
            case '0': break;
            case '1': sp->v+=1; break;
            case '2': sp->v+=2; break;
            case '3': sp->v+=3; break;
            case '4': sp->v+=4; break;
            case '5': sp->v+=5; break;
            case '6': sp->v+=6; break;
            case '7': sp->v+=7; break;
            case '8': sp->v+=8; break;
            case '9': sp->v+=9; break;
	    case 'A': case 'a': sp->v+=10; break;
	    case 'B': case 'b': sp->v+=11; break;
	    case 'C': case 'c': sp->v+=12; break;
	    case 'D': case 'd': sp->v+=13; break;
	    case 'E': case 'e': sp->v+=14; break;
	    case 'F': case 'f': sp->v+=15; break;
            default: 
	      error(sp,ER_XESC,sp->fn,CUR(sp).line,CUR(sp).col);
	      goto END_OF_HEX_DIGITS;
            }
	  } END_OF_HEX_DIGITS:;
	} else {
	  sp->v='\\'; sp->w=sp->u;
	}
      } else {
	sp->nx=0;
	sp->v='\\'; sp->w=sp->u;
      }
      break;
    default:
      sp->v=sp->u;
      break;
    }
  }
}

/* why \r is not a new line by itself when escaped? it is when not. */
#define newline(v) ((v)==0||(v)=='\n')
#define whitespace(v) ((v)==' '||(v)=='\t')
#define name_start(v) (u_base_char(v)||u_ideographic(v)||(v)=='_')
#define name_char(v) (name_start(v)||u_digit(v)||u_combining_char(v)||u_extender(v)||(v)=='.'||(v)=='-'||(v)==':')
#define skip_comment(sp) while(!newline(sp->v)) getv(sp); getv(sp)

static void realloc_s(struct sym *symp) {
  char *s; int slen=symp->slen*2;
  s=(char*)calloc(slen,sizeof(char));
  memcpy(s,symp->s,symp->slen); free(symp->s); 
  symp->s=s; symp->slen=slen;
}

static char *sym2str(int sym) {
  switch(sym) {
  case SYM_EOF: return "end of file"; 
  case SYM_ATTRIBUTE: return "\"attribute\"";
  case SYM_DEFAULT: return "\"default\"";
  case SYM_DATATYPES: return "\"datatypes\"";
  case SYM_DIV: return "\"div\"";
  case SYM_ELEMENT: return "\"element\"";
  case SYM_EMPTY: return "\"empty\"";
  case SYM_EXTERNAL: return "\"external\"";
  case SYM_GRAMMAR: return "\"grammar\"";
  case SYM_INCLUDE: return "\"include\"";
  case SYM_INHERIT: return "\"inherit\"";
  case SYM_LIST: return "\"list\"";
  case SYM_MIXED: return "\"mixed\"";
  case SYM_NAMESPACE: return "\"namespace\"";
  case SYM_NOT_ALLOWED: return "\"notAllowed\"";
  case SYM_PARENT: return "\"parent\"";
  case SYM_START: return "\"start\"";
  case SYM_STRING: return "\"string\"";
  case SYM_TEXT: return "\"text\"";
  case SYM_TOKEN: return "\"token\"";
  case SYM_IDENT: return "identifier"; 
  case SYM_QNAME: return "prefixed name";
  case SYM_NSNAME: return "namespace name";
  case SYM_ASGN: return "\"=\"";
  case SYM_ASGN_ILEAVE: return "\"&=\"";
  case SYM_ASGN_CHOICE: return "\"|=\"";
  case SYM_GROUP: return "\",\"";
  case SYM_CHOICE: return "\"|\"";
  case SYM_ILEAVE: return "\"&\"";
  case SYM_OPTIONAL: return "\"?\"";
  case SYM_ZERO_OR_MORE /*SYM_ANY_NAME*/: return "\"*\"";
  case SYM_ONE_OR_MORE: return "\"+\"";
  case SYM_LPAR: return "\"(\"";
  case SYM_RPAR: return "\")\"";
  case SYM_LCUR: return "\"{\"";
  case SYM_RCUR: return "\"}\"";
  case SYM_LSQU: return "\"[\"";
  case SYM_RSQU: return "\"]\"";
  case SYM_EXCEPT: return "\"-\"";
  case SYM_CONCAT: return "\"~\"";
  case SYM_QUOTE: return "\"\\\"";
  case SYM_FOLLOW_ANNOTATION: return "\">>\"";
  case SYM_DOCUMENTATION: return "\"##\"";
  case SYM_LITERAL: return "literal";
  default: assert(0);
  }
  return NULL;
}

static void advance(struct rnc_source *sp) {
  sp->cur=!sp->cur;
  NXT(sp).line=sp->line; NXT(sp).col=sp->col;
  for(;;) {
    if(newline(sp->v)||whitespace(sp->v)) {getv(sp); continue;}
    switch(sp->v) {
    case -1: NXT(sp).sym=SYM_EOF; return; 
    case '#': 
      getv(sp);
      if(sp->v=='#') {
	int i=0;
	for(;;) {
	  do getv(sp); while(sp->v=='#');
	  if(whitespace(sp->v)) getv(sp);
	  for(;;) {
	    if(i==NXT(sp).slen) realloc_s(&NXT(sp));
	    if(newline(sp->v)) {
	      do getv(sp); while(whitespace(sp->v));
	      if(sp->v=='#') {getv(sp);
		if(sp->v=='#') {NXT(sp).s[i++]='\n'; break;}
		skip_comment(sp);
	      }
	      NXT(sp).s[i]=0; NXT(sp).sym=SYM_DOCUMENTATION; return;
	    } else NXT(sp).s[i++]=(char)sp->v;
	    getv(sp);
	  } 
	}
      } else {skip_comment(sp); continue;}
    case '=': getv(sp); NXT(sp).sym=SYM_ASGN; return;
    case ',': getv(sp); NXT(sp).sym=SYM_GROUP; return;
    case '|': getv(sp); 
      if(sp->v=='=') {
	getv(sp); NXT(sp).sym=SYM_ASGN_CHOICE; return;
      } NXT(sp).sym=SYM_CHOICE; return;
    case '&': getv(sp); 
      if(sp->v=='=') {getv(sp); NXT(sp).sym=SYM_ASGN_ILEAVE;} else NXT(sp).sym=SYM_ILEAVE; return;
    case '?': getv(sp); NXT(sp).sym=SYM_OPTIONAL; return;
    case '*': getv(sp); NXT(sp).sym=SYM_ZERO_OR_MORE; return; /* SYM_ANY_NAME */
    case '+': getv(sp); NXT(sp).sym=SYM_ONE_OR_MORE; return;
    case '-': getv(sp); NXT(sp).sym=SYM_EXCEPT; return;
    case '~': getv(sp); NXT(sp).sym=SYM_CONCAT; return;	   
    case '(': getv(sp); NXT(sp).sym=SYM_LPAR; return;
    case ')': getv(sp); NXT(sp).sym=SYM_RPAR; return;
    case '{': getv(sp); NXT(sp).sym=SYM_LCUR; return;
    case '}': getv(sp); NXT(sp).sym=SYM_RCUR; return;
    case '[': getv(sp); NXT(sp).sym=SYM_LSQU; return;
    case ']': getv(sp); NXT(sp).sym=SYM_RSQU; return;
    case '>': getv(sp); 
      if(sp->v!='>') error(sp,ER_LEXP,'>',sp->fn,sp->line,sp->col);
      getv(sp); NXT(sp).sym=SYM_FOLLOW_ANNOTATION; return;
    case '"': case '\'': 
      { int q=sp->v;
	int triple=0;
	int i=0;
	getv(sp); 
	if(sp->v==q) {getv(sp);
	  if(sp->v==q) { // triply quoted string
	    triple=1; getv(sp);
	  } else {
	    NXT(sp).s[0]='\0'; NXT(sp).sym=SYM_LITERAL; return;
	  }
	} 
	for(;;) {
	  if(sp->v==q) {
	    if(triple) {
	      if(i>=2 && NXT(sp).s[i-2]==q && NXT(sp).s[i-1]==q) {
		NXT(sp).s[i-2]='\0'; break;
	      } else NXT(sp).s[i]=(char)sp->v;
	    } else {NXT(sp).s[i]='\0'; break;}
	  } else if(sp->v<=0) {
	    if(sp->v==-1 || !triple) {
	      error(sp,ER_LLIT,sp->fn,sp->line,sp->col);
	      NXT(sp).s[i]='\0'; break;
	    } else NXT(sp).s[i]='\n';
	  } else NXT(sp).s[i]=(char)sp->v;
	  getv(sp);
	  if(++i==NXT(sp).slen) realloc_s(&NXT(sp));
	}
	getv(sp); NXT(sp).sym=SYM_LITERAL; return;
      } 
    default: 
      { int escaped=0,prefixed=0;
        if(sp->v=='\\') {escaped=1; getv(sp);}
	if(name_start(sp->v)) {
	  int i=0;
	  for(;;) {
	    NXT(sp).s[i++]=sp->v;
	    if(i==NXT(sp).slen) realloc_s(&NXT(sp));
	    getv(sp);
	    if(!name_char(sp->v)) {NXT(sp).s[i]='\0'; break;}
	    if(sp->v==':') prefixed=1;
          }
	  if(!(escaped||prefixed)) {
	    int n=0,m=NKWD-1,i,cmp;
	    for(;;) {
	      if(n>m) break;
	      i=(n+m)/2;
	      cmp=strcmp(NXT(sp).s,kwdtab[i]);
	      if(cmp==0) {NXT(sp).sym=i; return;} else if(cmp<0) m=i-1; else n=i+1;
	    }
	  } 
	  if(prefixed) {
	    if(NXT(sp).s[i-1]==':'&&sp->v=='*') {
	      getv(sp); NXT(sp).s[i-1]='\0';
	      NXT(sp).sym=SYM_NSNAME;
	    } else NXT(sp).sym=SYM_QNAME;
	  } else NXT(sp).sym=SYM_IDENT; 
	  return;
	} else {
	  error(sp,ER_LILL,sp->v,sp->fn,sp->line,sp->col);
	  getv(sp);
	  continue;
	}
      }	
    }
  }
}

static void skipAnnotationContent(struct rnc_source *sp) {
 /* syntax of annotations is not checked; it is not a purpose of this parser to handle them anyway */
  int line=CUR(sp).line, col=CUR(sp).col;
  if(CUR(sp).sym==SYM_LSQU) {
    advance(sp);
    for(;;) {
      switch(CUR(sp).sym) {
      case SYM_RSQU: advance(sp); return;
      case SYM_LSQU: skipAnnotationContent(sp); break;
      case SYM_IDENT: case SYM_QNAME: /* keywords are in the default: clause */
      case SYM_ASGN:
      case SYM_LITERAL: case SYM_CONCAT: advance(sp); break;
      default:
        if(0<=CUR(sp).sym&&CUR(sp).sym<NKWD) { /* keywords */
	  advance(sp);
	  break;
	} else {
	  error(sp,ER_SILL,sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
	  error(sp,ER_UNAN,sp->fn,line,col);
	  return;
	}
      }
    }
  }
}

/* advance, join literal fragments and skip annotations and documentation comments */
static void getsym(struct rnc_source *sp) {
  advance(sp);
  for(;;) {
    switch(CUR(sp).sym) {
    case SYM_DOCUMENTATION: 
      advance(sp); 
      continue;
    case SYM_FOLLOW_ANNOTATION: advance(sp); advance(sp); 
      if(CUR(sp).sym<0||CUR(sp).sym>SYM_QNAME) {
	error(sp,ER_SEXP,"identifier, prefixed name or keyword",sp->fn,CUR(sp).line,CUR(sp).col);
	while(CUR(sp).sym!=SYM_LSQU&&CUR(sp).sym!=SYM_EOF) advance(sp);
      } else {
	advance(sp); 
        if(CUR(sp).sym!=SYM_LSQU) error(sp,ER_SEXP,sym2str(SYM_LSQU),sp->fn,CUR(sp).line,CUR(sp).col);
      }
    case SYM_LSQU: 
      skipAnnotationContent(sp);
      continue;
    case SYM_LITERAL: 
     /* alternatively, either a literal() non-terminal, or a separate filter;
      - one more filtering layer is not worth the effort, 
      - the non-terminal would later need extra buffer for concatenated strings.
      Since the concatenation is only applied to constants anyway, merging them
      into single terminal looks appropriate. 
      */
      if(NXT(sp).sym==SYM_CONCAT) {
	sp->cur=!sp->cur; advance(sp);
	if(NXT(sp).sym!=SYM_LITERAL) {
	  error(sp,ER_SEXP,"literal",sp->fn,NXT(sp).line,NXT(sp).col);
	  break;
	}
	{ int newslen=strlen(CUR(sp).s)+strlen(NXT(sp).s);
	  char *s;
	  if(newslen>CUR(sp).slen) {
	    s=(char*)calloc(newslen+1,sizeof(char));
	    strcpy(s,CUR(sp).s); free(CUR(sp).s);
	    CUR(sp).s=s;
	  } else s=CUR(sp).s;
	  strcat(s,NXT(sp).s);
	}
	sp->cur=!sp->cur; advance(sp);
	continue;
      }
      break;
    }
    return;
  }
}

/* parser helpers: weak symbols, syntax errors */
static void skipto(struct rnc_source *sp,int sym) {
  while(CUR(sp).sym!=sym&&CUR(sp).sym!=SYM_EOF) getsym(sp);
}

static int chkskip(struct rnc_source *sp,int symc,int syms) {
  if(CUR(sp).sym!=symc) {
    error(sp,ER_SEXP,sym2str(symc),sp->fn,CUR(sp).line,CUR(sp).col);
    skipto(sp,syms);
    return 0;
  } else {
    return 1;
  }
}

static int chksym(struct rnc_source *sp,int sym) {
  return chkskip(sp,sym,CUR(sp).sym);
}

static void chk_get(struct rnc_source *sp,int sym) {
  int ok=chksym(sp,sym); getsym(sp);
}

/* check and skip to the symbol if failed */
static void chk_skip(struct rnc_source *sp,int symc,int syms) {
  if(chkskip(sp,symc,syms)) getsym(sp);
}

/* go past the symbol */
static void chk_skip_get(struct rnc_source *sp,int sym) {
  int ok=chkskip(sp,sym,sym); getsym(sp);
}

/* in the code below non-terminals that can be repeated return int, non-terminals
 that cannot do not return a value. Allows use of ``while(nonterminal())''.
 */

static int decl(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_NAMESPACE: 
    getsym(sp);
    if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {
      getsym(sp);
    } else {
      error(sp,ER_SEXP,"identifier or keyword",sp->fn,CUR(sp).line,CUR(sp).col);
      skipto(sp,SYM_ASGN);
    }
    chk_get(sp,SYM_ASGN);
    if(chksym(sp,SYM_LITERAL)) {
    }
    getsym(sp);
    return 1;
  case SYM_DEFAULT: 
    getsym(sp);
    chk_get(sp,SYM_NAMESPACE);
    if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {
      getsym(sp);
    }
    chk_get(sp,SYM_ASGN);
    if(chksym(sp,SYM_LITERAL)) {
    }
    getsym(sp);
    return 1;
  case SYM_DATATYPES:
    getsym(sp);
    if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {
      getsym(sp);
    } else {
      error(sp,ER_SEXP,"identifier or keyword",sp->fn,CUR(sp).line,CUR(sp).col);
      skipto(sp,SYM_ASGN);
    }
    chk_get(sp,SYM_ASGN);
    if(chksym(sp,SYM_LITERAL)) {
    }
    getsym(sp);
    return 1;
  default: return 0;
  }
}

static void nameclass(struct rnc_source *sp);

static void simplenc(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_QNAME:
    getsym(sp);
    break;
  case SYM_NSNAME:
    getsym(sp);
    break;
  case SYM_ANY_NAME:
    getsym(sp);
    break;
  case SYM_LPAR:
    getsym(sp); 
    nameclass(sp);
    chk_skip(sp,SYM_RPAR,SYM_LCUR);
  default:
    if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {
      getsym(sp);
      break;
    } else {
      error(sp,ER_SILL,sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
      skipto(sp,SYM_LCUR);
    }
  }
}

static void nameclass(struct rnc_source *sp) {
  simplenc(sp);
  switch(CUR(sp).sym) {
  case SYM_CHOICE:
    do {
      getsym(sp);
      simplenc(sp);
    } while(CUR(sp).sym==SYM_CHOICE);
    break;
  case SYM_EXCEPT:
    getsym(sp); /* check that the first argument is NCName or nsName */
    simplenc(sp);
    break;
  }
}

static void value(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_TOKEN:
  case SYM_STRING: 
  case SYM_QNAME:
    getsym(sp);
  case SYM_LITERAL:
    getsym(sp);
    break;
  }
}

static void data(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_TOKEN: 
  case SYM_STRING: 
  case SYM_QNAME:
    getsym(sp);
  }
  if(CUR(sp).sym==SYM_LCUR) {
   /* parameters */
    for(;;) {
      if(CUR(sp).sym==SYM_RCUR) {
	getsym(sp);
	break;
      }
      getsym(sp);
      if(0<CUR(sp).sym||CUR(sp).sym<=SYM_IDENT) {
	error(sp,ER_SILL,sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
	skipto(sp,SYM_RCUR);
	break;
      }
      chk_get(sp,SYM_ASGN);
      if(chksym(sp,SYM_LITERAL)) {
      }
      getsym(sp);
    }
  }
}

static void pattern(struct rnc_source *sp);
static int grammarContent(struct rnc_source *sp);

static void primary(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_ELEMENT: getsym(sp);
    nameclass(sp); chk_get(sp,SYM_LCUR); pattern(sp); chk_skip_get(sp,SYM_RCUR);
    break;
  case SYM_ATTRIBUTE: getsym(sp);
    nameclass(sp); chk_get(sp,SYM_LCUR); pattern(sp); chk_skip_get(sp,SYM_RCUR);
    break;
  case SYM_LIST: getsym(sp);
    chk_get(sp,SYM_LCUR); pattern(sp); chk_skip_get(sp,SYM_RCUR);
    break;
  case SYM_MIXED: getsym(sp);
    chk_get(sp,SYM_LCUR); pattern(sp); chk_skip_get(sp,SYM_RCUR);
    break;
  case SYM_PARENT: getsym(sp);
    if(chksym(sp,SYM_IDENT)) {
    }
    getsym(sp);
    break;
  case SYM_EMPTY: getsym(sp);
    break;
  case SYM_TEXT: getsym(sp);
    break;
  case SYM_NOT_ALLOWED: getsym(sp);
    break;
  case SYM_EXTERNAL: getsym(sp);
    if(chksym(sp,SYM_LITERAL)) {
    }
    getsym(sp); 
    if(CUR(sp).sym==SYM_INHERIT) getsym(sp);
    break;
  case SYM_GRAMMAR: 
    chk_get(sp,SYM_LCUR);
    while(grammarContent(sp));
    chk_skip_get(sp,SYM_RCUR);
    break;
    if(NXT(sp).sym==SYM_LITERAL) {
      value(sp); 
    } else {
      data(sp);
    }
    break;
  case SYM_IDENT: /* ref */
    getsym(sp);
    break;
  case SYM_STRING: 
  case SYM_TOKEN: 
  case SYM_QNAME:  
    if(NXT(sp).sym==SYM_LITERAL) {
      value(sp); 
    } else {
      data(sp);
    }
    break;
  case SYM_LPAR: 
    getsym(sp); 
    pattern(sp); 
    chk_skip(sp,SYM_RPAR,SYM_RCUR); 
    break;
  case SYM_LITERAL: 
    value(sp); 
    break;
  default: 
    error(sp,ER_SILL,sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
    getsym(sp);
    break;
  }
}

static void unary(struct rnc_source *sp) {
  primary(sp);
  switch(CUR(sp).sym) {
  case SYM_OPTIONAL: getsym(sp); 
    break;
  case SYM_ZERO_OR_MORE:
  case SYM_ONE_OR_MORE: getsym(sp);
   /* check that the argument are not data-derived (?) */
    break;
  }
}

static void pattern(struct rnc_source *sp) {
  int op;
  unary(sp);
  switch(CUR(sp).sym) {
  case SYM_GROUP: 
  case SYM_CHOICE:
  case SYM_ILEAVE: /* check that the arguments are not data-derived (?) */
    op=CUR(sp).sym;
    do {
      getsym(sp);
      unary(sp);
    } while(CUR(sp).sym==op);
    break;
  case SYM_EXCEPT:
   /*check that the pattern's content-type is simple or unknown*/
    getsym(sp);
    primary(sp);
  }
}

static void define(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_ASGN: case SYM_ASGN_CHOICE: case SYM_ASGN_ILEAVE: getsym(sp); break;
  default: error(sp,ER_SEXP,"assign method",sp->fn,CUR(sp).line,CUR(sp).col);
  }
  pattern(sp);
}

static void division(struct rnc_source *sp) {
  chk_get(sp,SYM_LCUR);
  while(grammarContent(sp)); 
  chk_skip_get(sp,SYM_RCUR);
}

static void include(struct rnc_source *sp) {
 /* check for include inside includeContent */
  getsym(sp);
  if(CUR(sp).sym==SYM_INHERIT) getsym(sp);
  chk_get(sp,SYM_LCUR);
  while(grammarContent(sp));
  chk_skip_get(sp,SYM_RCUR);
}

static int grammarContent(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_IDENT: 
    switch(NXT(sp).sym) {
    case SYM_LSQU: getsym(sp); return 1; /* skip grammar annotation */
    case SYM_ASGN: 
    case SYM_ASGN_CHOICE: 
    case SYM_ASGN_ILEAVE: getsym(sp); define(sp); return 1;
    default: return 0;
    }
  case SYM_QNAME:
    switch(NXT(sp).sym) {
    case SYM_LSQU: getsym(sp); return 1;
    default: return 0;
    }
  case SYM_START: getsym(sp); define(sp); return 1;
  case SYM_DIV: getsym(sp); division(sp); return 1;
  case SYM_INCLUDE: getsym(sp); include(sp); return 1;
  default: return 0;
  }
}

static void topLevel(struct rnc_source *sp) {
  while(decl(sp));
  if(grammarContent(sp)) {
    while(grammarContent(sp));
  } else pattern(sp);
  chk_get(sp,SYM_EOF);
}

void rnc_parse(struct rnc_source *sp) {
  getsym(sp); getsym(sp);
  topLevel(sp);
 /* second pass here */ 
}

int main(int argc,char **argv) {
  struct rnc_source src;
  rnc_bind(&src,"stdin",0);
  rnc_parse(&src);
  rnc_close(&src);
  if(src.flags&=SRC_ERRORS) {
    fprintf(stderr,"errors happened\n");
    return 1;
  } 
  return 0;
}

/*
 * $Log$
 * Revision 1.14  2003/11/27 14:19:15  dvd
 * syntax done, now to includes
 *
 * Revision 1.13  2003/11/26 23:49:00  dvd
 * syntax almost ready
 *
 * Revision 1.12  2003/11/26 00:37:47  dvd
 * parser in progress, documentation handling removed
 *
 * Revision 1.11  2003/11/25 13:14:21  dvd
 * scanner ready
 *
 * Revision 1.10  2003/11/25 10:33:53  dvd
 * documentation and comments
 *
 * Revision 1.9  2003/11/24 23:00:27  dvd
 * literal, error reporting
 *
 * Revision 1.8  2003/11/23 16:16:06  dvd
 * no roles for elements
 *
 * Revision 1.7  2003/11/21 00:20:06  dvd
 * lexer in progress
 *
 * Revision 1.4  2003/11/20 23:28:50  dvd
 * getu,getv debugged
 *
 * Revision 1.3  2003/11/20 16:29:08  dvd
 * x escapes sketched
 *
 * Revision 1.2  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 * Revision 1.1  2003/11/19 00:28:57  dvd
 * back to lists of ranges
 *
 */
