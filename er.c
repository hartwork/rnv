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
  default: assert(0);
  }
}

/*
 * $Log$
 * Revision 1.16  2003/12/08 18:54:51  dvd
 * content-type checks
 *
 * Revision 1.15  2003/12/07 20:41:42  dvd
 * bugfixes, loops, traits
 *
 * Revision 1.14  2003/12/07 16:50:55  dvd
 * stage D, dereferencing and checking for loops
 *
 * Revision 1.13  2003/12/05 23:58:44  dvd
 * parses docbook
 *
 * Revision 1.12  2003/12/01 14:44:53  dvd
 * patterns in progress
 *
 * Revision 1.11  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.10  2003/11/29 17:47:48  dvd
 * decl
 *
 * Revision 1.9  2003/11/27 23:05:49  dvd
 * syntax and external files
 *
 * Revision 1.8  2003/11/27 14:19:15  dvd
 * syntax done, now to includes
 *
 * Revision 1.7  2003/11/26 23:49:00  dvd
 * syntax almost ready
 *
 * Revision 1.6  2003/11/26 00:37:47  dvd
 * parser in progress, documentation handling removed
 *
 * Revision 1.5  2003/11/25 13:14:21  dvd
 * scanner ready
 *
 * Revision 1.4  2003/11/24 23:00:27  dvd
 * literal, error reporting
 *
 * Revision 1.3  2003/11/21 00:20:06  dvd
 * lexer in progress
 *
 * Revision 1.2  2003/11/20 23:28:50  dvd
 * getu,getv debugged
 *
 * Revision 1.1  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 */
