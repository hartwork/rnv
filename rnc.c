/* $Id$ */

#include <fcntl.h> /* open, read, close */
#include <string.h> /* memcpy */
#include <stdlib.h> /* calloc,malloc,free */

#include "u.h"
#include "rn.h"
#include "er.h"
#include "rnc.h"

/* taken from the specification:

topLevel ::= decl* (pattern | grammarContent*)
decl ::= "namespace" identifierOrKeyword "=" namespaceURILiteral
| "default" "namespace" [identifierOrKeyword] "=" namespaceURILiteral
| "datatypes" identifierOrKeyword "=" literal
pattern ::= "element" nameClass "{" pattern "}"
| "attribute" nameClass "{" pattern "}"
| pattern ("," pattern)+
| pattern ("&" pattern)+
| pattern ("|" pattern)+
| pattern "?"
| pattern "*"
| pattern "+"
| "list" "{" pattern "}"
| "mixed" "{" pattern "}"
| identifier
| "parent" identifier
| "empty"
| "text"
| [datatypeName] datatypeValue
| datatypeName ["{" param* "}"] [exceptPattern]
| "notAllowed"
| "external" anyURILiteral [inherit]
| "grammar" "{" grammarContent* "}"
| "(" pattern ")"
param ::= identifierOrKeyword "=" literal
exceptPattern ::= "-" pattern
grammarContent ::= start
| define
| "div" "{" grammarContent* "}"
| "include" anyURILiteral [inherit] ["{" includeContent* "}"]
includeContent ::= define
| start
| "div" "{" includeContent* "}"
start ::= "start" assignMethod pattern
define ::= identifier assignMethod pattern
assignMethod ::= "="
| "|="
| "&="
nameClass ::= name
| nsName [exceptNameClass]
| anyName [exceptNameClass]
| nameClass "|" nameClass
| "(" nameClass ")"
name ::= identifierOrKeyword
| CName
exceptNameClass ::= "-" nameClass
datatypeName ::= CName
| "string"
| "token"
datatypeValue ::= literal
anyURILiteral ::= literal
namespaceURILiteral ::= literal
| "inherit"
inherit ::= "inherit" "=" identifierOrKeyword
identifierOrKeyword ::= identifier
| keyword
identifier ::= (NCName - keyword)
| quotedIdentifier
quotedIdentifier ::= "\" NCName
CName ::= NCName ":" NCName
nsName ::= NCName ":*"
anyName ::= "*"
literal ::= literalSegment ("~" literalSegment)+
literalSegment ::= '"' (Char - ('"' | newline))* '"'
| "'" (Char - ("'" | newline))* "'"
| '"""' (['"'] ['"'] (Char - '"'))* '"""'
| "'''" (["'"] ["'"] (Char - "'"))* "'''"
keyword ::= "attribute"
| "default"
| "datatypes"
| "div"
| "element"
| "empty"
| "external"
| "grammar"
| "include"
| "inherit"
| "list"
| "mixed"
| "namespace"
| "notAllowed"
| "parent"
| "start"
| "string"
| "text"
| "token"
*/

#define SYM_EOF 0
#define SYM_DEFAULT 1
#define SYM_DATATYPE 2
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
#define SYM_ASGN 19
#define SYM_ASGN_ILEAVE 20
#define SYM_ASGN_CHOICE 21
#define SYM_SEQ 22 /* , */
#define SYM_CHOICE 23 
#define SYM_ILEAVE 24
#define SYM_OPTIONAL 25
#define SYM_ZERO_OR_MORE 26
#define SYM_ONE_OR_MORE 27
#define SYM_LPAR 28
#define SYM_RPAR 29
#define SYM_LCUR 30
#define SYM_RCUR 31
#define SYM_LSQU 32
#define SYM_RSQU 33
#define SYM_EXCEPT 34
#define SYM_QNAME 35   /* : */
#define SYM_NS_NAME 36 /* :* */
#define SYM_ANY_NAME 37 /* * */
#define SYM_QUOTE 38  /* \ */
#define SYM_ANNOTATION 39 /* >> */
#define SYM_COMMENT 40 
#define SYM_DOCUMENTATION 41 /* ## */
#define SYM_IDENT 42
#define SYM_LITERAL 43

#define BUFSIZE 1024
#define CUFSIZE 1
#define BUFTAIL 6

struct utf_source {
  char *fn; int fd;
  char *buf; int i,n;
  int complete;
  int line,col;
  int u,v,w; int nx;
};

static int rnc_stropen(struct utf_source *src,char *s,int len) {
  src->fn="";
  src->buf=s; 
  src->i=0; src->n=len; 
  nx=-1;
  src->complete=1; 
  src->fd=-1;
  return 0;
}

static int rnc_open(struct utf_source *src,char *fn) {
  src->fn=fn; 
  src->buf=(char*)calloc(BUFSIZE,sizeof(char));
  src->fd=open(fn,O_RDONLY);
  src->i=src->n=0;
  nx=-1;
  complete=-1;
  src->fd==-1;
  return src->fd;
}

static int rnc_read(struct utf_source *src) {
  int ni;
  memcpy(src->buf,src->buf+src->i,src->n-=src->i);
  src->n=src->i=ni=0;
  for(;;) {
    ni=read(src->fd,src->buf+src->n,BUFSIZE);
    if(ni>0) {
      src->n+=ni;
      if(src->n>=BUFTAIL) break;
    } else {
      close(src->fd); src->fd=-1;
      src->complete=1;
      break;
    }
  }
  return ni;
}

static int rnc_close(struct utf_source *src) {
  int ret=0;
  free(src->buf); src->buf=NULL;
  src->complete=-1;
  if(src->fd!=-1) {
    ret=close(src->fd); src->fd=-1;
  }
  return ret;
}

static void getu(struct utf_source *src) {
  int n,u0=src->u;
  for(;;) {
    if(!src->complete&&src->i>BUFSIZE-BUFTAIL) {
      if(rnc_read(src)==-1) (*er_handler)(ER_IO,src->fn);
    }
    if(src->i==src->n) {
      src->u=u0=='\n'?-1:'\n'; 
      return;
    } /* eof */
    n=u_get(&src->u,src->buf+src->i);
    if(n==0) { 
      (*er_handler)(ER_UTF,src->fn,src->line,src->col);
      ++src->i;
      continue;
    } else if(n+src->n>sizeof(src->buf)) { 
      (*er_handler)(ER_UTF,src->fn,src->line,src->col);
      src->i=src->n;
      continue;
    } else {
      src->i+=n;
      if(u0=='\r'&&src->u=='\n') continue;
    }
    return;
  }
}

static void getv(struct utf_source *src) {
  if(nx>0) {
    src->v='x'; --nx;
  } else if(nx==0) {
    src->v=src->w;
    nx=-1;
  } else {
    getu(src);
    switch(u) {
    case '\r': case '\n': src->v=0; break;
    case '\\':
      getu(src);
      if(src->u=='x') {
	nx=0;
	do {
	  ++nx;
	  getu(src);
	} while(src->u=='x');
	if(src->u=='{') { 
	  nx=-1;
	  src->v=0; 
	  for(;;) {
	    getu(src);
	    if(src->u=='}') goto END_OF_HEX_DIGITS;
	    src->v<<=4;
	    switch(src->u) {
            case '0': break;
            case '1': src->v+=1; break;
            case '2': src->v+=2; break;
            case '3': src->v+=3; break;
            case '4': src->v+=4; break;
            case '5': src->v+=5; break;
            case '6': src->v+=6; break;
            case '7': src->v+=7; break;
            case '8': src->v+=8; break;
            case '9': src->v+=9; break;
	    case 'A': case 'a': src->v+=10; break;
	    case 'B': case 'b': src->v+=11; break;
	    case 'C': case 'c': src->v+=12; break;
	    case 'D': case 'd': src->v+=13; break;
	    case 'E': case 'e': src->v+=14; break;
	    case 'F': case 'f': src->v+=15; break;
            default: 
	      (*er_handler)(ER_XESC,src->fn,src->line,src->col);
	      goto END_OF_HEX_DIGITS;
            }
	  } END_OF_HEX_DIGITS:;
	} else {
	  src->v='\\'; src->w=src->u;
	}
      } else {
	nx=0;
	src->v='\\'; src->w=src->u;
      }
      break;
    default:
      src->v=src->u;
      break;
    }
  }
}

static int sym(struct utf_source *src) {
  switch(src->v) {
  case -1: return SYM_EOF;
  case '#':
    break;
  case '=':
    break;
  case ',':
    break;
  case '|':
    break;
  case '&':
    break;
  case '?':
    break;
  case '*':
    break;
  case '+':
    break;
  case '-':
    break;
  case '(':
    break;
  case ')':
    break;
  case '{':
    break;
  case '}':
    break;
  case '"':
    break;
  case '\'':
    break;
  case '~':	   
    break;
  case '[':
    break;
  case ']':
    break;
  case '>':
    break;
  default:
  }
}

/*
 * $Log$
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
