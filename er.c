/* $Id$ */

#include <stdio.h>
#include "er.h"

int (*er_printf)(char *format,...)=&er_default_printf;
int (*er_vprintf)(char *format,va_list ap)=&er_default_vprintf;

int er_default_printf(char *format,...) {
  int ret;
  va_list ap; va_start(ap,format); ret=er_default_vprintf(format,ap); va_end(ap);
  return ret;
}
int er_default_vprintf(char *format,va_list ap) {return vfprintf(stderr,format,ap);}

