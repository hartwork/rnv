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

static void default_ver_handler(int er_no,va_list ap) {
  switch(er_no) {
  case ER_IO:
    fprintf(stderr,"I/O error (%s): %s\n",va_arg(ap,char*),strerror(errno));
    break;
  case ER_UTF: 
    vfprintf(stderr,"invalid UTF-8 sequence (%s,%u,%u)\n",ap);
    break;
  case ER_XESC:
    vfprintf(stderr,"unterminated escape (%s,%u,%u)\n",ap);
    break;
  case ER_LEXP:
    vfprintf(stderr,"lexical error: '%c' expected (%s,%u,%u)\n",ap);
    break;
  case ER_LLIT:
    vfprintf(stderr,"lexical error: unterminated literal (%s,%u,%u)\n",ap);
    break;
  case ER_LILL:
    vfprintf(stderr,"lexical error: illegal character \\x{%x} (%s,%u,%u)\n",ap);
    break;
  case ER_SEXP:
    vfprintf(stderr,"syntax error: %s expected, %s found (%s,%u,%u)\n",ap);
    break;
  case ER_SILL:
    vfprintf(stderr,"syntax error: %s unexpected  (%s,%u,%u)\n",ap);
    break;
  case ER_EXT:
    vfprintf(stderr,"cannot open external grammar '%s' (%s,%u,%u)\n",ap);
    break;
  case ER_DUPNS:
    vfprintf(stderr,"duplicate namespace prefix '%s' (%s,%u,%u)\n",ap);
    break;
  case ER_DUPDT:
    vfprintf(stderr,"duplicate datatype prefix '%s' (%s,%u,%u)\n",ap);
    break;
  case ER_NONS:
    vfprintf(stderr,"undeclared namespace prefix '%s' (%s,%u,%u)\n",ap);
    break;
  case ER_NODT:
    vfprintf(stderr,"undeclared datatype prefix '%s' (%s,%u,%u)\n",ap);
    break;
  case ER_NCEX:
    vfprintf(stderr,"first argument for '-' is not '*' or 'prefix:*' (%s,%u,%u)\n",ap);
    break;
  default: assert(0);
  }
}

/*
 * $Log$
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
