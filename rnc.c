/* $Id$ */

#include <fcntl.h> /* open, read, close */
#include <unistd.h> /* read, close */
#include <string.h> /* memcpy,strlen,strcpy,strcat,strdup */
#include <stdlib.h> /* calloc,malloc,free */
#include <stdio.h> /*stderr*/
#include <stdarg.h> /*va_list,va_arg,va_end*/
#include <assert.h> /*assert*/

#include "util.h"
#include "u.h"
#include "er.h"
#include "rn.h"
#include "sc.h"
#include "rnc.h"

#define NKWD 19
static char *kwdtab[NKWD]={
  "attribute", "datatypes", "default", "div", "element", "empty", "external",
  "grammar", "include", "inherit", "list", "mixed", "namespace", "notAllowed",
  "parent", "start", "string", "text", "token"};

#define SYM_EOF -1

#define SYM_ATTRIBUTE 0
#define SYM_DATATYPES 1
#define SYM_DEFAULT 2
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
  int line,col,prevline/*when error reported*/;
  int u,v,w; int nx;
  int cur;
  struct sym sym[2];
};

#define CUR(sp) ((sp)->sym[(sp)->cur])
#define NXT(sp) ((sp)->sym[!(sp)->cur])

#define LEN_P 128

static int len_p;
static char *path;

static void rnc_source_init(struct rnc_source *sp);
static int rnc_read(struct rnc_source *sp);

int rnc_stropen(struct rnc_source *sp,char *fn,char *s,int len) {
  rnc_source_init(sp);
  sp->fn=strdup(fn);
  sp->buf=s; sp->n=len; sp->complete=1;
  return 0;
}

int rnc_bind(struct rnc_source *sp,char *fn,int fd) {
  rnc_source_init(sp);
  sp->fn=strdup(fn); sp->fd=fd;
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
  free(sp->fn); sp->fn=NULL;
  return ret;
}

static void rnc_source_init(struct rnc_source *sp) {
  sp->flags=0;
  sp->fn=sp->buf=NULL;
  sp->i=sp->n=0;
  sp->complete=sp->fd=-1;
  sp->line=1; sp->col=1; sp->prevline=-1;
  sp->u=-1; sp->v=0;  sp->nx=-1;
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

#define PFX_INHERITED 1
#define PFX_DEFAULT 2

#define DE_HEAD 4
#define DE_CHOICE 8
#define DE_ILEAVE 16

#define RE_

static struct sc_stack nss,dts,defs,refs,prefs;

static int initialized;
void rnc_init() {
  if(!initialized) {
    rn_init();
    len_p=LEN_P; path=(char*)calloc(len_p,sizeof(char));
    /* initialize scopes */
    sc_init(&nss); sc_init(&dts); sc_init(&defs); sc_init(&refs); sc_init(&prefs);
    initialized=1;
  }
}

static void error(int force,struct rnc_source *sp,int er_no,...) {
  if(force || sp->line != sp->prevline) {
    va_list ap; va_start(ap,er_no); (*ver_handler_p)(er_no,ap); va_end(ap);
    sp->prevline=sp->line;
  }
  sp->flags|=SRC_ERRORS;
}

/* read utf8 */
static void getu(struct rnc_source *sp) {
  int n,u0=sp->u;
  for(;;) {
    if(!sp->complete&&sp->i>sp->n-BUFTAIL) {
      if(rnc_read(sp)==-1) {error(1,sp,ER_IO,sp->fn);}
    }
    if(sp->i==sp->n) {
      sp->u=(u0=='\n'||u0=='\r'||u0==-1)?-1:'\n';
      u0=-1;
      break;
    } /* eof */
    n=u_get(&sp->u,sp->buf+sp->i);
    if(n==0) {
      error(0,sp,ER_UTF,sp->fn,sp->line,sp->col);
      ++sp->i;
      continue;
    } else if(n+sp->i>sp->n) {
      error(0,sp,ER_UTF,sp->fn,sp->line,sp->col);
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
	      error(0,sp,ER_XESC,sp->fn,CUR(sp).line,CUR(sp).col);
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
  for(;;) {
    NXT(sp).line=sp->line; NXT(sp).col=sp->col;
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
      if(sp->v!='>') error(0,sp,ER_LEXP,'>',sp->fn,sp->line,sp->col);
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
	      error(0,sp,ER_LLIT,sp->fn,sp->line,sp->col);
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
	  error(0,sp,ER_LILL,sp->v,sp->fn,sp->line,sp->col);
	  getv(sp);
	  continue;
	}
      }	
    }
  }
}

static void skipAnnotationContent(struct rnc_source *sp) {
 /* syntax of annotations is not checked; it is not a purpose of this parser to handle them anyway */
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
	  error(0,sp,ER_SILL,sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
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
    case SYM_FOLLOW_ANNOTATION: advance(sp);
      if(CUR(sp).sym<0||CUR(sp).sym>SYM_QNAME) {
	error(0,sp,ER_SEXP,"identifier, prefixed name or keyword",sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
	while(CUR(sp).sym!=SYM_LSQU&&CUR(sp).sym!=SYM_EOF) advance(sp);
      } else {
	advance(sp);
        if(CUR(sp).sym!=SYM_LSQU) error(0,sp,ER_SEXP,sym2str(SYM_LSQU),sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
      }
    case SYM_LSQU:
      skipAnnotationContent(sp);
      continue;
    case SYM_LITERAL:
     /* alternatively, either a non-terminal, or a separate filter;
      - one more filtering layer is not worth the effort,
      - the non-terminal would later need extra buffer for concatenated strings.
      Since the concatenation is only applied to constants anyway, merging them
      into a single terminal looks appropriate.
      */
      if(NXT(sp).sym==SYM_CONCAT) {
	sp->cur=!sp->cur; advance(sp);
	if(NXT(sp).sym!=SYM_LITERAL) {
	  error(0,sp,ER_SEXP,sym2str(SYM_LITERAL),sym2str(CUR(sp).sym),sp->fn,NXT(sp).line,NXT(sp).col);
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
    error(0,sp,ER_SEXP,sym2str(symc),sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
    skipto(sp,syms);
    return 0;
  } else {
    return 1;
  }
}

static int chksym(struct rnc_source *sp,int sym) {
  return chkskip(sp,sym,CUR(sp).sym);
}

static int chkwd(struct rnc_source *sp) {
  if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {
    return 1;
  } else {
    error(0,sp,ER_SEXP,"identifier or keyword",sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
    return 0;
  }
}

static void chk_get(struct rnc_source *sp,int sym) {
  (void)chksym(sp,sym); getsym(sp);
}

/* check and skip to the symbol if failed */
static void chk_skip(struct rnc_source *sp,int symc,int syms) {
  if(chkskip(sp,symc,syms)) getsym(sp);
}

/* go past the symbol */
static void chk_skip_get(struct rnc_source *sp,int sym) {
  (void)chkskip(sp,sym,sym); getsym(sp);
}

/* a grammar without stop symbols provides weak capabilities for recovery. when
  in doubt, always move forward */

static int nsuri(struct rnc_source *sp) {
  int uri=-1;
  switch(CUR(sp).sym) {
  case SYM_LITERAL: uri=newString(CUR(sp).s); break;
  case SYM_INHERIT: uri=nss.tab[(sc_find(&nss,-1))][1]; break;
  default:
    error(0,sp,ER_SEXP,"literal or 'inherit'",sp->fn,CUR(sp).line,CUR(sp).col);	
    break;
  }
  getsym(sp);
  return uri;
}

static void open_scope(struct rnc_source *sp) {
  sc_open(&defs);
  sc_open(&refs);
  sc_open(&prefs);
}

static void close_scope(struct rnc_source *sp) {
  int i,j,name;
  for(i=refs.base+1;i!=refs.top;++i) {
    name=refs.tab[i][0];
    if(j=sc_find(&defs,name)) {
      rn_pattern[refs.tab[i][1]][1]=defs.tab[j][1];
    } else {
      error(1,sp,ER_UNDEF,rn_string+name,sp->fn,CUR(sp).line,CUR(sp).col);
    }
  }
  sc_close(&defs); sc_close(&refs);
  for(i=prefs.base+1;i!=prefs.top;++i) {
    if(sc_void(&refs)) error(1,sp,ER_UNDEF,rn_string+prefs.tab[i][0],sp->fn,CUR(sp).line,CUR(sp).col);
    else sc_add(&refs,prefs.tab[i][0],prefs.tab[i][1],prefs.tab[i][2]);
  }
  sc_close(&prefs);
}

static void fold_efs(struct rnc_source *sp,struct sc_stack *stp,void (*fold)(struct rnc_source *sp,struct sc_stack *rp,int key,int val,int flags)) {
  int len=stp->top-stp->base-1;
  if(len!=0) {
    int i;
    int (*tab)[SC_RECSIZE]=(int(*)[])calloc(len,sizeof(int[SC_RECSIZE]));
    memcpy(tab,stp->tab+stp->base+1,len*sizeof(int[SC_RECSIZE]));
    sc_close(stp);
    for(i=0;i!=len;++i) fold(sp,stp,tab[i][0],tab[i][1],tab[i][2]);
    free(tab);
  } else sc_close(stp);
}

static void adddef(struct rnc_source *sp,int name,int pat,int flags);

static void folddef(struct rnc_source *sp,struct sc_stack *rp,int key,int val,int flags) {
  adddef(sp,key,val,flags);
}

static void foldref(struct rnc_source *sp,struct sc_stack *rp,int key,int val,int flags) {
  sc_add(rp,key,val,flags);
}

static void fold_scope(struct rnc_source *sp) {
  fold_efs(sp,&defs,&folddef);
  fold_efs(sp,&refs,&foldref); 
  fold_efs(sp,&prefs,&foldref);
}

static void addns(struct rnc_source *sp,int pfx,int url) {
  int i;
  if(i=sc_find(&nss,pfx)) {
    if(nss.tab[i][2]&(PFX_INHERITED|PFX_DEFAULT)) {
      nss.tab[i][1]=url; nss.tab[i][2]&=~(PFX_INHERITED|PFX_DEFAULT);
    } else error(1,sp,ER_DUPNS,rn_string+pfx,sp->fn,CUR(sp).line,CUR(sp).col);
  } else sc_add(&nss,pfx,url,0);
}

static void adddt(struct rnc_source *sp,int pfx,int url) {
  int i;
  if(i=sc_find(&dts,pfx)) {
    if(dts.tab[i][2]&PFX_DEFAULT) {
      dts.tab[i][1]=url; dts.tab[i][2]&=~PFX_DEFAULT;
    } else error(1,sp,ER_DUPDT,rn_string+pfx,sp->fn,CUR(sp).line,CUR(sp).col);
  } else sc_add(&dts,pfx,url,0);
}

static void adddef(struct rnc_source *sp,int name,int pat,int flags) {
  int i;
  if(i=sc_find(&defs,name)) {
    if(sc_locked(&defs)) {
      defs.tab[i][1]=pat; defs.tab[i][2]=flags;
    } else {
      int old_flags=defs.tab[i][2];
      if(DE_HEAD&flags&old_flags) error(1,sp,ER_2HEADS,sp->fn,CUR(sp).line,CUR(sp).col);
      if(((flags|old_flags)&(DE_CHOICE|DE_ILEAVE))==(DE_CHOICE|DE_ILEAVE)) error(1,sp,ER_COMBINE,sp->fn,CUR(sp).line,CUR(sp).col);
      flags=defs.tab[i][2]=old_flags|flags;
      if(DE_CHOICE&flags) {
	defs.tab[i][1]=rn_choice(defs.tab[i][1],pat);
      } else if(DE_ILEAVE&flags) {
	defs.tab[i][1]=rn_ileave(defs.tab[i][1],pat);
      }
    }
  } else {
    if(sc_locked(&defs)) error(1,sp,ER_OVRIDE,name!=0?rn_string+name:"start",sp->fn,CUR(sp).line,CUR(sp).col);
    else sc_add(&defs,name,pat,flags);
  }
}

static int decl(struct rnc_source *sp) {
  int pfx=-1,uri=-1;
  switch(CUR(sp).sym) {
  case SYM_NAMESPACE:
    getsym(sp);
    if(chkwd(sp)) pfx=newString(CUR(sp).s); getsym(sp);
    chk_get(sp,SYM_ASGN);
    uri=nsuri(sp);
    if(uri!=-1&&pfx!=-1) addns(sp,pfx,uri);
    return 1;
  case SYM_DEFAULT:
    getsym(sp);
    chk_get(sp,SYM_NAMESPACE);
    if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {pfx=newString(CUR(sp).s); getsym(sp);}
    chk_get(sp,SYM_ASGN);
    uri=nsuri(sp);
    if(uri!=-1) {if(pfx!=-1) addns(sp,pfx,uri); addns(sp,0,uri);}
    return 1;
  case SYM_DATATYPES:
    getsym(sp);
    if(chkwd(sp)) pfx=newString(CUR(sp).s); getsym(sp);
    chk_get(sp,SYM_ASGN);
    if(chksym(sp,SYM_LITERAL)) uri=newString(CUR(sp).s); getsym(sp);
    if(pfx!=-1&&uri!=-1) adddt(sp,pfx,uri);
    return 1;
  default: return 0;
  }
}

static int ns2uri(struct rnc_source *sp,int p) {
  int i=sc_find(&nss,p);
  if(!i) {
    error(1,sp,ER_NONS,rn_string+p,sp->fn,CUR(sp).line,CUR(sp).col);
  }
  return i?nss.tab[i][1]:0;
}

static int dt2uri(struct rnc_source *sp,int p) {
  int i=sc_find(&dts,p);
  if(!i) error(1,sp,ER_NODT,rn_string+p,sp->fn,CUR(sp).line,CUR(sp).col);
  return i?dts.tab[i][1]:0;
}

static int inherit(struct rnc_source *sp) {
  int uri=0;
  if(CUR(sp).sym==SYM_INHERIT) {
    getsym(sp); chk_get(sp,SYM_ASGN);
    if(chkwd(sp)) uri=ns2uri(sp,newString(CUR(sp).s));
    getsym(sp);
  } else uri=nss.tab[sc_find(&nss,0)][1];
  return uri;
}

static int name(struct rnc_source *sp,int p,int s) {
  int nc=newQName(ns2uri(sp,p),p);
  getsym(sp);
  return nc;
}

static int qname(struct rnc_source *sp) {
  char *s=CUR(sp).s; while(*s!=':') ++s; *(s++)='\0';
  return name(sp,newString(CUR(sp).s),newString(s));
}

static int nsname(struct rnc_source *sp) {
  int nc=newNsName(ns2uri(sp,newString(CUR(sp).s)));
  getsym(sp);
  return nc;
}

static int nameclass(struct rnc_source *sp);

static int simplenc(struct rnc_source *sp) {
  int nc=0;
  switch(CUR(sp).sym) {
  case SYM_QNAME: nc=qname(sp); break;
  case SYM_NSNAME: nc=nsname(sp); break;
  case SYM_ANY_NAME: nc=newAnyName(); getsym(sp); break;
  case SYM_LPAR: getsym(sp); nc=nameclass(sp); chk_skip(sp,SYM_RPAR,SYM_LCUR); break;
  default:
    if(chkwd(sp)) {
      nc=name(sp,0,newString(CUR(sp).s));
      break;
    } else skipto(sp,SYM_LCUR);
  }
  return nc;
}

static int nameclass(struct rnc_source *sp) {
  int nc=simplenc(sp);
  switch(CUR(sp).sym) {
  case SYM_CHOICE:
    do {
      int nci;
      getsym(sp);
      nci=simplenc(sp);
      if(nc==nci||NC_IS(nc,ANY_NAME)) {
	;
      } else if(NC_IS(nci,ANY_NAME)) {
	nc=nci;
      } else {
	nc=newNameClassChoice(nc,nci);
      }
    } while(CUR(sp).sym==SYM_CHOICE);
    break;
  case SYM_EXCEPT:
    if(!(NC_IS(nc,ANY_NAME)||NC_IS(nc,NSNAME))) error(1,sp,ER_NCEX,sp->fn,CUR(sp).line,CUR(sp).col);
    getsym(sp);
    nc=newNsNameExcept(nc,simplenc(sp));
    break;
  }
  return nc;
}

static int pattern(struct rnc_source *sp);

static int element(struct rnc_source *sp) {
  int nc,p;
  nc=nameclass(sp); chk_get(sp,SYM_LCUR); p=newElement(nc,pattern(sp)); chk_skip_get(sp,SYM_RCUR);
  return p;
}

static int attribute(struct rnc_source *sp) {
  int nc,p,i=sc_find(&nss,0),nsuri=nss.tab[i][1];
  nss.tab[i][1]=0; nc=nameclass(sp);  nss.tab[i][1]=nsuri;
  chk_get(sp,SYM_LCUR); p=newAttribute(nc,pattern(sp)); chk_skip_get(sp,SYM_RCUR);
  return p;
}

static int refname(struct rnc_source *sp,struct sc_stack *stp) {
  int name=newString(CUR(sp).s),i,p;
  if(i=sc_find(stp,name)) {
    p=stp->tab[i][1];
  } else {
    p=newRef();
    sc_add(stp,name,p,0);
  }
  return p;
}

static int ref(struct rnc_source *sp) {
  int p=refname(sp,&refs);
  getsym(sp);
  return p;
}

static int parent(struct rnc_source *sp) {
  int p=0;
  getsym(sp);
  if(chksym(sp,SYM_IDENT)) p=refname(sp,&prefs);
  getsym(sp);
  return p;
}

static int relpath(struct rnc_source *sp) {
  int ret=0;
  if(ret=chksym(sp,SYM_LITERAL)) {
    int len=strlen(sp->fn)+strlen(CUR(sp).s)+1;
    if(len>len_p) {free(path); len_p=len; path=(char*)calloc(len_p,sizeof(char));}
    strcpy(path,CUR(sp).s); abspath(path,sp->fn);
  }
  getsym(sp);
  return ret;
}

static int topLevel(struct rnc_source *sp);

static void add_well_known_nss(int dflt) {
  sc_add(&nss,newString("xml"),newString("http://www.w3.org/XML/1998/namespace"),0);
  sc_add(&nss,newString("xmlns"),newString("http://www.w3.org/2000/xmlns"),0);
  sc_add(&nss,0,dflt,PFX_INHERITED); sc_add(&nss,-1,dflt,PFX_INHERITED); 
}

static int file(struct rnc_source *sp,int nsuri) {
  int ret=0;
  struct rnc_source src;
  add_well_known_nss(nsuri);
  rnc_source_init(&src);
  if(rnc_open(&src,path)!=-1) {
    ret=topLevel(&src);
    sp->flags|=src.flags&SRC_ERRORS;
  } else {
    error(1,sp,ER_EXT,path,sp->fn,CUR(sp).line,CUR(sp).col);
  }
  rnc_close(&src);
  return ret;
}

static int external(struct rnc_source *sp) {
  int ret=0;
  if(relpath(sp)) {
    int nsuri=inherit(sp);
    sc_open(&nss);
    open_scope(sp);
    if((ret=file(sp,nsuri))==-1) { /* grammar */
      int i;
      if(i=sc_find(&defs,0)) {
	ret=defs.tab[i][1];
      }
      close_scope(sp);
      sc_close(&nss);
    } else {
      fold_scope(sp);
      sc_close(&nss);
    }
  }
  return ret;
}

static int list(struct rnc_source *sp) {
  int p;
  chk_get(sp,SYM_LCUR);
  p=newList(pattern(sp));
  chk_skip_get(sp,SYM_RCUR);
  return p;
}

static int mixed(struct rnc_source *sp) {
  int mixed;
  chk_get(sp,SYM_LCUR);
  mixed=rn_choice(pattern(sp),rn_text);
  chk_skip_get(sp,SYM_RCUR);
  return mixed;
}

static int param(struct rnc_source *sp) {
  if(0<=CUR(sp).sym&&CUR(sp).sym<=SYM_IDENT) {
    getsym(sp);
    chk_get(sp,SYM_ASGN);
    if(chksym(sp,SYM_LITERAL)) {
    }
    getsym(sp);
    return 1;
  } else return 0;
}

static int datatype(struct rnc_source *sp) {
  int dt=0;
  switch(CUR(sp).sym) {
  case SYM_TOKEN: dt=newDatatype(0,newString("token")); break;
  case SYM_STRING: dt=newDatatype(0,newString("string")); break;
  case SYM_QNAME: {
      char *s=CUR(sp).s; while(*s!=':') ++s; *(s++)='\0';
      dt=newDatatype(dt2uri(sp,newString(CUR(sp).s)),newString(s));
      break;
    }
    break;
  case SYM_LITERAL: dt=newDatatype(0,newString("token")); return dt;
  }
  getsym(sp);
  return dt;
}

static int params(struct rnc_source *sp) {
  if(CUR(sp).sym==SYM_LCUR) {
    getsym(sp);
    while(param(sp));
    chk_skip_get(sp,SYM_RCUR);
  }
  return 0;
}

static int data(struct rnc_source *sp) {
  int dt,ps; dt=datatype(sp); ps=params(sp);
  return newData(dt,ps);
}

static int value(struct rnc_source *sp) {
  int dt,val=0; dt=datatype(sp);
  if(chksym(sp,SYM_LITERAL)) val=newString(CUR(sp).s);
  getsym(sp);
  return newValue(dt,val);
}

static int grammarContent(struct rnc_source *sp);

static int grammar(struct rnc_source *sp) {
  int start=0,i;
  open_scope(sp);
  chk_get(sp,SYM_LCUR);
  while(grammarContent(sp));
  chk_skip_get(sp,SYM_RCUR);
  if(i=sc_find(&defs,0)) {
    start=defs.tab[i][1];
  } else error(1,sp,ER_NOSTART,sp->fn,CUR(sp).line,CUR(sp).col);
  close_scope(sp);
  return start;
}

static int primary(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_ELEMENT: getsym(sp); return element(sp);
  case SYM_ATTRIBUTE: getsym(sp); return attribute(sp);
  case SYM_IDENT: return ref(sp);
  case SYM_PARENT: return parent(sp);
  case SYM_EXTERNAL: getsym(sp); return external(sp);

  case SYM_LIST: getsym(sp); return list(sp);
  case SYM_MIXED: getsym(sp); return mixed(sp);

  case SYM_STRING:
  case SYM_TOKEN:
  case SYM_QNAME: return NXT(sp).sym==SYM_LITERAL?value(sp):data(sp);
  case SYM_LITERAL: return value(sp);

  case SYM_EMPTY: getsym(sp); return rn_empty;
  case SYM_TEXT: getsym(sp); return rn_text;
  case SYM_NOT_ALLOWED: getsym(sp); return rn_notAllowed;

  case SYM_GRAMMAR: getsym(sp); return grammar(sp);

  case SYM_LPAR: getsym(sp); {int ret=pattern(sp); chk_skip(sp,SYM_RPAR,SYM_RCUR); return ret;}

  default:
    error(0,sp,ER_SILL,sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
    getsym(sp);
    return 0;
  }
}

static int unary(struct rnc_source *sp) {
  int p;
  p=primary(sp);
  switch(CUR(sp).sym) {
  case SYM_OPTIONAL: getsym(sp); p=rn_choice(p,rn_empty); break;
  case SYM_ZERO_OR_MORE: getsym(sp); p=rn_choice(rn_one_or_more(p),rn_empty); break;
  case SYM_ONE_OR_MORE: getsym(sp); p=rn_one_or_more(p); break;
  }
  return p;
}

static int (*op_handler[])(int p1,int p2)={&rn_group,&rn_choice,&rn_ileave};

static int pattern(struct rnc_source *sp) {
  int p,op;
  p=unary(sp);
  switch(CUR(sp).sym) {
  case SYM_GROUP:
  case SYM_CHOICE:
  case SYM_ILEAVE: /* check that the arguments are not data-derived (?) */
    op=CUR(sp).sym;
    do {
      getsym(sp);
      p=(*op_handler[op-SYM_GROUP])(p,unary(sp));
    } while(CUR(sp).sym==op);
    break;
  case SYM_EXCEPT:
    if(!P_IS(p,DATA)) error(1,sp,ER_EXPT,sp->fn,CUR(sp).line,CUR(sp).col);
    getsym(sp);
    p=newDataExcept(p,primary(sp));
  }
  return p;
}

static void define(struct rnc_source *sp,int name) {
  int pat=0,flags=0;
  switch(CUR(sp).sym) {
  case SYM_ASGN: flags=DE_HEAD; break;
  case SYM_ASGN_CHOICE: flags=DE_CHOICE; break;
  case SYM_ASGN_ILEAVE: flags=DE_ILEAVE; break;
  default: error(0,sp,ER_SEXP,"assign method",sym2str(CUR(sp).sym),sp->fn,CUR(sp).line,CUR(sp).col);
  }
  getsym(sp);
  pat=pattern(sp);
  adddef(sp,name,pat,flags);
}

static void division(struct rnc_source *sp) {
  chk_get(sp,SYM_LCUR);
  while(grammarContent(sp));
  chk_skip_get(sp,SYM_RCUR);
}

static void include(struct rnc_source *sp) {
 /* check for include inside includeContent */
  int nsuri;
  if(relpath(sp)) {
    nsuri=inherit(sp);
    sc_open(&nss); open_scope(sp);
    if(file(sp,nsuri)!=-1) error(1,sp,ER_NOTGR,sp->fn,CUR(sp).line,CUR(sp).col);
    sc_lock(&defs);
    if(CUR(sp).sym==SYM_LCUR) {
      getsym(sp);
      while(grammarContent(sp));
      chk_skip_get(sp,SYM_RCUR);
    }
    fold_scope(sp);
    sc_close(&nss);
  }
}

static int grammarContent(struct rnc_source *sp) {
  switch(CUR(sp).sym) {
  case SYM_IDENT:
    switch(NXT(sp).sym) {
    case SYM_LSQU: getsym(sp); return 1; /* skip grammar annotation */
    case SYM_ASGN:
    case SYM_ASGN_CHOICE:
    case SYM_ASGN_ILEAVE: {
	int name=newString(CUR(sp).s); getsym(sp); define(sp,name);
	return 1;
      }
    default: return 0;
    }
  case SYM_QNAME:
    switch(NXT(sp).sym) {
    case SYM_LSQU: getsym(sp); return 1;
    default: return 0;
    }
  case SYM_START: getsym(sp); define(sp,0); return 1;
  case SYM_DIV: getsym(sp); division(sp); return 1;
  case SYM_INCLUDE: getsym(sp); include(sp); return 1;
  default: return 0;
  }
}

/* returns -1 if it is a grammar, and a non-negative value if it is a pattern
 and is not a grammar. the returned value is then used by external()
 */
static int topLevel(struct rnc_source *sp) {
  int ret=-1,is_grammar;
  sc_open(&dts);
  sc_add(&dts,newString("xsd"),newString("http://www.w3.org/2001/XMLSchema-datatypes"),PFX_DEFAULT);

  getsym(sp); getsym(sp);
  while(decl(sp));
  if(is_grammar=(CUR(sp).sym==SYM_GRAMMAR)) {
    chk_get(sp,SYM_LCUR);
  }
  if(grammarContent(sp)) {
    while(grammarContent(sp));
  } else if(!is_grammar) {
    ret=pattern(sp);
  }
  if(is_grammar) chk_skip(sp,SYM_RCUR,SYM_EOF);
  chk_skip(sp,SYM_EOF,SYM_EOF);
  sc_close(&dts);
  return ret;
}

int rnc_parse(struct rnc_source *sp) {
  int start,i;

  sc_open(&nss); add_well_known_nss(0);
  open_scope(sp);

  start=topLevel(sp); if(start!=-1) sc_add(&defs,0,start,0);

 /* second pass here */

  if(i=sc_find(&defs,0)) {
    start=defs.tab[i][1];
  } else {
    error(1,sp,ER_NOSTART,sp->fn,CUR(sp).line,CUR(sp).col);
    start=0;
  }

  close_scope(sp);
  sc_close(&nss);

  return start;
}

int main(int argc,char **argv) {
  struct rnc_source src;
  rnc_init();
  if(*(++argv)) rnc_open(&src,*argv); else rnc_bind(&src,"stdin",0);
  rnc_parse(&src);
  rnc_close(&src);
  if(src.flags&=SRC_ERRORS) {
    fprintf(stderr,"errors happened\n");
    return 1;
  }
  return 1;
}

/*
 * $Log$
 * Revision 1.27  2003/12/06 00:55:13  dvd
 * parses all grammars from nxml-mode samples
 *
 * Revision 1.26  2003/12/06 00:08:49  dvd
 * fixed error reporting
 *
 * Revision 1.25  2003/12/05 23:58:44  dvd
 * parses docbook
 *
 * Revision 1.24  2003/12/05 14:28:39  dvd
 * separate stacks for references
 *
 * Revision 1.23  2003/12/04 22:09:30  dvd
 * bug in define
 *
 * Revision 1.22  2003/12/04 22:02:20  dvd
 * refactoring
 *
 * Revision 1.21  2003/12/04 00:37:03  dvd
 * refactoring
 *
 * Revision 1.20  2003/12/01 14:44:53  dvd
 * patterns in progress
 *
 * Revision 1.19  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.18  2003/11/29 18:06:02  dvd
 * fixes
 *
 * Revision 1.17  2003/11/29 17:47:48  dvd
 * decl
 *
 * Revision 1.16  2003/11/27 23:19:31  dvd
 * syntax and external files
 *
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
