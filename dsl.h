/* $Id$ */

#ifndef DSL_H
#define DSL_H 1

#define DSL_URL "http://davidashen.net/relaxng/scheme-datatypes"

extern char *dsl_scm;

extern int dsl_allows(char *typ,char *ps,char *s,int n);
extern int dsl_equal(char *typ,char *val,char *s,int n);

#endif
