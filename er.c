/* $Id$ */

#include <stdio.h> /*stderr,fprintf*/
#include <string.h> /*strerror*/
#include <errno.h> /*errno*/
#include <stdarg.h> /*va_list,var_start,va_end*/
#include <assert.h> /*assert*/
#include "er.h"

static void default_ver_handler(int er_no,va_list ap);

void (*ver_handler_p)(int er_no,va_list ap)=&default_ver_handler;

void er_handler(int er_no,...) {
  va_list ap;
  va_start(ap,er_no);
  (*ver_handler_p)(er_no,ap);
  va_end(ap);
}

    
#define cerr(msg) vfprintf(stderr,msg" (%s,%u,%u)\n",ap)
#define derr(msg) vfprintf(stderr,msg"\n",ap)
#define verr(msg) vfprintf(stderr,msg"\n",ap)

static void default_ver_handler(int er_no,va_list ap) {
  switch(er_no) {
  case ER_IO: fprintf(stderr,"I/O error (%s): %s\n",va_arg(ap,char*),strerror(errno)); break;
  case ER_UTF: cerr("invalid UTF-8 sequence"); break;
  case ER_XESC: cerr("unterminated escape"); break;
  case ER_LEXP: cerr("lexical error: '%c' expected"); break;
  case ER_LLIT: cerr("lexical error: unterminated literal"); break;
  case ER_LILL: cerr("lexical error: illegal character \\x{%x}"); break;
  case ER_SEXP: cerr("syntax error: %s expected, %s found"); break;
  case ER_SILL: cerr("syntax error: %s unexpected "); break;
  case ER_NOTGR: cerr("included schema is not a grammar"); break;
  case ER_EXT: cerr("cannot open external grammar '%s'"); break;
  case ER_DUPNS: cerr("duplicate namespace prefix '%s'"); break;
  case ER_DUPDT: cerr("duplicate datatype prefix '%s'"); break;
  case ER_DFLTNS: cerr("overriding default namespace prefix '%s'"); break;
  case ER_DFLTDT: cerr("overriding default datatype prefix '%s'"); break;
  case ER_NONS: cerr("undeclared namespace prefix '%s'"); break;
  case ER_NODT: cerr("undeclared datatype prefix '%s'"); break;
  case ER_NCEX: cerr("first argument for '-' is not '*' or 'prefix:*'"); break;
  case ER_2HEADS: cerr("repeated define or start"); break;
  case ER_COMBINE: cerr("conflicting combine methods in define or start"); break;
  case ER_OVRIDE: cerr("'%s' overrides nothing"); break;
  case ER_EXPT: cerr("first argument for '-' is not data"); break;
  case ER_NOSTART: cerr("missing start"); break;
  case ER_UNDEF: cerr("undefined reference to '%s'"); break;
  case ER_LOOPST: derr("loop in start pattern"); break;
  case ER_LOOPEL: derr("loop in pattern for element '%s'"); break;
  case ER_CTYPE: derr("content of element '%s' does not have a content-type"); break;
  case ER_BADSTART: derr("bad path in start pattern"); break;
  case ER_BADMORE: derr("bad path before '*' or '+' in element '%s'"); break;
  case ER_BADEXPT: derr("bad path after '-' in element '%s'"); break;
  case ER_BADLIST: derr("bad path after 'list' in element '%s'"); break;
  case ER_BADATTR: derr("bad path in attribute '%s' of element '%s'"); break;
  case ER_NODTL: verr("no datatype library for URI '%s'"); break;
  default: assert(0);
  }
}
