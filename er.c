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

    
#define printerr(msg) vfprintf(stderr,msg" (%s,%u,%u)\n",ap)

static void default_ver_handler(int er_no,va_list ap) {
  switch(er_no) {
  case ER_IO: fprintf(stderr,"I/O error (%s): %s\n",va_arg(ap,char*),strerror(errno)); break;
  case ER_UTF: printerr("invalid UTF-8 sequence"); break;
  case ER_XESC: printerr("unterminated escape"); break;
  case ER_LEXP: printerr("lexical error: '%c' expected"); break;
  case ER_LLIT: printerr("lexical error: unterminated literal"); break;
  case ER_LILL: printerr("lexical error: illegal character \\x{%x}"); break;
  case ER_SEXP: printerr("syntax error: %s expected, %s found"); break;
  case ER_SILL: printerr("syntax error: %s unexpected "); break;
  case ER_NOTGR: printerr("included schema is not a grammar"); break;
  case ER_EXT: printerr("cannot open external grammar '%s'"); break;
  case ER_DUPNS: printerr("duplicate namespace prefix '%s'"); break;
  case ER_DUPDT: printerr("duplicate datatype prefix '%s'"); break;
  case ER_NONS: printerr("undeclared namespace prefix '%s'"); break;
  case ER_NODT: printerr("undeclared datatype prefix '%s'"); break;
  case ER_NCEX: printerr("first argument for '-' is not '*' or 'prefix:*'"); break;
  case ER_2HEADS: printerr("repeated define or start"); break;
  case ER_COMBINE: printerr("conflicting combine methods in define or start"); break;
  case ER_OVRIDE: printerr("'%s' overrides nothing"); break;
  case ER_EXPT: printerr("first argument for '-' is not data"); break;
  case ER_NOSTART: printerr("missing start"); break;
  case ER_UNDEF: printerr("undefined reference to '%s'"); break;
  default: assert(0);
  }
}

/*
 * $Log$
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
