/* $Id$ */

#include <stdio.h> /*stderr,fprintf*/
#include <string.h> /*strerror*/
#include <errno.h> /*errno*/
#include <stdarg.h> /*va_list,var_start,va_end*/
#include <assert.h> /*assert*/
#include "er.h"

static void default_er_handler(int er_no,...);

void (*er_handler)(int er_no,...)=&default_er_handler;

static void default_er_handler(int er_no,...) {
  va_list ap;
  va_start(ap,er_no);
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
  case ER_LEX:
    vfprintf(stderr,"lexical error: expected '%c', got '%c' (%s,%u,%u)\n",ap);
    break;
  case ER_LIT:
    vfprintf(stderr,"lexical error: unterminated literal (%s,%u,%u)\n",ap);
    break;
  default: assert(0);
  }
  va_end(ap);
}

/*
 * $Log$
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
