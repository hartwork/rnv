/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <string.h> /*memcpy*/
#include <assert.h> /*assert*/
#include <stdarg.h> /*va_list,va_start,va_end*/
#include "er.h"
#include "rn.h"
#include "rnd.h"

#define LEN_F 1024

static int len_f,n_f;
static int *flat;
static int errors;

int rnd_errors() {
  return errors!=0;
}

static void error(int er_no,...) {
  va_list ap; va_start(ap,er_no); (*ver_handler_p)(er_no,ap); va_end(ap);
  ++errors;
}

static void realloc_f() {
  int *newflat;
  newflat=(int*)calloc(len_f*=2,sizeof(int));
  memcpy(newflat,flat,n_f*sizeof(int)); free(flat);
  flat=newflat;
}

static int deref(int p) {
  int p0=p,p1,name;
  P_CHK(p,REF);
  for(;;) {
    Ref(p,p1,name);
    if(!P_IS(p1,REF)||p1==p0) break;
    p=p1;
  }
  return p1;
}

static void flatten(p) { if(!marked(p)) {flat[n_f++]=p; mark(p);}}

void rnd_deref(int start) {
  int p,p1,p2,nc,i,changed;

  flat=(int*)calloc(len_f=LEN_F,sizeof(int)); n_f=0;
  errors=0;

  if(P_IS(start,REF)) start=deref(start);
  flat[n_f++]=start; mark(start);

  i=0;
  do {
    p=flat[i++];
    switch(P_TYP(p)) {
    case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: case P_DATA: case P_VALUE:
      break;

    case P_CHOICE: Choice(p,p1,p2); goto BINARY;
    case P_INTERLEAVE: Interleave(p,p1,p2); goto BINARY;
    case P_GROUP: Group(p,p1,p2); goto BINARY;
    case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY;
    BINARY:
      changed=0;
      if(P_IS(p1,REF)) {p1=deref(p1); changed=1;}
      if(P_IS(p2,REF)) {p2=deref(p2); changed=1;}
      if(changed) {rn_del_p(p); rn_pattern[p][1]=p1; rn_pattern[p][2]=p2; rn_add_p(p);}
      if(n_f+2>len_f) realloc_f();
      flatten(p1); flatten(p2);
      break;

    case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
    case P_LIST: List(p,p1); goto UNARY;
    case P_ATTRIBUTE: Attribute(p,nc,p1); goto UNARY;
    case P_ELEMENT: Element(p,nc,p1); goto UNARY;
    UNARY:
      changed=0;
      if(P_IS(p1,REF)) {p1=deref(p1); changed=1;}
      if(changed) {rn_del_p(p); rn_pattern[p][1]=p1; rn_add_p(p);}
      if(n_f+1>len_f) realloc_f();
      flatten(p1);
      break;

    case P_REF: /* because of a loop, but will be handled in rnd_loops */
      break;

    default: assert(0);
    }
  } while(i!=n_f);
  for(i=0;i!=n_f;++i) unmark(flat[i]);
}

static int loop(int p) {
  int nc,p1,p2,ret=1;
  if(marked(p)) return 1;
  mark(p);
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: case P_DATA: case P_VALUE:
  case P_ELEMENT:
    ret=0; break;

  case P_CHOICE: Choice(p,p1,p2); goto BINARY;
  case P_INTERLEAVE: Interleave(p,p1,p2); goto BINARY;
  case P_GROUP: Group(p,p1,p2); goto BINARY;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY;
  BINARY:
    ret=loop(p1)||loop(p2); break;

  case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
  case P_LIST: List(p,p1); goto UNARY;
  case P_ATTRIBUTE:  Attribute(p,nc,p1); goto UNARY;
  UNARY:
    ret=loop(p1); break;

  case P_REF: ret=1; break;

  default: assert(0);
  }
  unmark(p);
  return ret;
}

static void loops() {
  int i=0,p=flat[i],nc=-1,p1;
  for(;;) {
    if(loop(p)) {
      if(i==0) error(ER_LOOPST); else {
	char *s=nc2str(nc);
	error(ER_LOOPEL,s);
	free(s);
      }
    }
    for(;;) {++i;
      if(i==n_f) return;
      p=flat[i];
      if(P_IS(p,ELEMENT)) {
	Element(p,nc,p1); p=p1;
	break;
      }
    }
  }
}

static void ctype(int p) {
  int p1,p2,nc;
  if(!contentType(p)) {
    switch(P_TYP(p)) {
    case P_EMPTY: setContentType(p,P_FLG_CTE,0); break;
    case P_NOT_ALLOWED: setContentType(p,P_FLG_CTE,0); break;
    case P_TEXT: setContentType(p,P_FLG_CTC,0); break;
    case P_CHOICE: Choice(p,p1,p2); ctype(p1); ctype(p2);
      setContentType(p,contentType(p1),contentType(p2)); break;
    case P_INTERLEAVE: Interleave(p,p1,p2); ctype(p1); ctype(p2);
      if(rn_groupable(p1,p2)) setContentType(p,contentType(p1),contentType(p2)); break;
    case P_GROUP: Group(p,p1,p2); ctype(p1); ctype(p2);
      if(rn_groupable(p1,p2)) setContentType(p,contentType(p1),contentType(p2)); break;
    case P_ONE_OR_MORE: OneOrMore(p,p1); ctype(p1);
      if(rn_groupable(p1,p1)) setContentType(p,contentType(p1),0); break;
    case P_LIST: List(p,p1); ctype(p1); 
      if(contentType(p1)) setContentType(p,P_FLG_CTS,0); break;
    case P_DATA: setContentType(p,P_FLG_CTS,0); break;
    case P_DATA_EXCEPT: DataExcept(p,p1,p2); ctype(p1); ctype(p2);
      if(contentType(p2)) setContentType(p,P_FLG_CTS,0); break;
    case P_VALUE: setContentType(p,P_FLG_CTS,0); break;
    case P_ATTRIBUTE: Attribute(p,nc,p1); ctype(p1);
      if(contentType(p1)) setContentType(p,P_FLG_CTE,0); break;
    case P_ELEMENT: setContentType(p,P_FLG_CTC,0); break;
    default: assert(0);
    }
  }
}

static void ctypes() {
  int i=0,p,p1,nc;
  for(i=0;i!=n_f;++i) {
    p=flat[i];
    if(P_IS(p,ELEMENT)) {
      Element(p,nc,p1);
      ctype(p1);
      if(!contentType(p1)) {
	char *s=nc2str(nc);
	error(ER_CTYPE,s);
	free(s);
      }
    }
  }
}

static int bad_start(int p) {
  int p1,p2;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_TEXT:
  case P_INTERLEAVE: case P_GROUP: case P_ONE_OR_MORE:
  case P_LIST: case P_DATA: case P_DATA_EXCEPT: case P_VALUE:
  case P_ATTRIBUTE:
    return 1;
  case P_NOT_ALLOWED:
  case P_ELEMENT:
    return 0;
  case P_CHOICE: Choice(p,p1,p2);
    return bad_start(p1)||bad_start(p2);
  default: assert(0);
  }
  return 1;
}

static int bad_data_except(int p) {
  int p1,p2;
  switch(P_TYP(p)) {
  case P_NOT_ALLOWED:
  case P_VALUE: case P_DATA:
    return 0;

  case P_CHOICE: Choice(p,p1,p2); goto BINARY;
  case P_DATA_EXCEPT: Choice(p,p1,p2); goto BINARY;
  BINARY: return bad_data_except(p1)||bad_data_except(p2);

  case P_EMPTY: case P_TEXT:
  case P_INTERLEAVE: case P_GROUP: case P_ONE_OR_MORE:
  case P_LIST:
  case P_ATTRIBUTE: case P_ELEMENT:
    return 1;
  default: assert(0);
  }
  return 1;
}

static int bad_one_or_more(int p,int in_group) {
  int nc,p1,p2;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_DATA: case P_VALUE:
  case P_ELEMENT:
    return 0;

  case P_CHOICE: Choice(p,p1,p2); goto BINARY;
  case P_INTERLEAVE: Interleave(p,p1,p2); in_group=1; goto BINARY;
  case P_GROUP: Group(p,p1,p2); in_group=1; goto BINARY;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY;
  BINARY: return  bad_one_or_more(p1,in_group)||bad_one_or_more(p2,in_group);

  case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
  case P_LIST: List(p,p1); goto UNARY;
  case P_ATTRIBUTE: if(in_group) return 1;
    Attribute(p,nc,p1); goto UNARY;
  UNARY: return  bad_one_or_more(p1,in_group);
  default: assert(0);
  }
  return 1;
}

static int bad_list(int p) {
  int p1,p2;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED:
  case P_DATA: case P_VALUE:
    return 0;

  case P_CHOICE: Choice(p,p1,p2); goto BINARY;
  case P_GROUP: Group(p,p1,p2); goto BINARY;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY;
  BINARY: return bad_list(p1)||bad_list(p2);

  case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
  case P_LIST: List(p,p1); goto UNARY;
  UNARY: return bad_list(p1);

  case P_TEXT:
  case P_INTERLEAVE:
  case P_ATTRIBUTE:
  case P_ELEMENT:
    return 1;
  default: assert(0);
  }
  return 1;
}

static int bad_attribute(int p) {
  int p1,p2;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_DATA: case P_VALUE:
    return 0;

  case P_CHOICE: Choice(p,p1,p2); goto BINARY;
  case P_INTERLEAVE: Interleave(p,p1,p2); goto BINARY;
  case P_GROUP: Group(p,p1,p2); goto BINARY;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY; 
  BINARY: return bad_attribute(p1)||bad_attribute(p2);
  

  case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
  case P_LIST: List(p,p1); goto UNARY;
  UNARY: return bad_attribute(p1);

  case P_ATTRIBUTE: case P_ELEMENT:
    return 1;
  default: assert(0);
  }
  return 1;
}

static void path(int p,int nc) {
  int p1,p2,nc1;
  switch(P_TYP(p)) {
  case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT:
  case P_DATA: case P_VALUE:
  case P_ELEMENT:
    break;

  case P_CHOICE: Choice(p,p1,p2); goto BINARY;
  case P_INTERLEAVE: Interleave(p,p1,p2); goto BINARY;
  case P_GROUP: Group(p,p1,p2); goto BINARY;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2); 
    if(bad_data_except(p2)) {char *s=nc2str(nc); error(ER_BADEXPT,s); free(s);}
    goto BINARY;
  BINARY: path(p1,nc); path(p2,nc); break;

  case P_ONE_OR_MORE: OneOrMore(p,p1); 
    if(bad_one_or_more(p1,0)) {char *s=nc2str(nc); error(ER_BADMORE,s); free(s);}
    goto UNARY;
  case P_LIST: List(p,p1); 
    if(bad_list(p1)) {char *s=nc2str(nc); error(ER_BADLIST,s); free(s);}
    goto UNARY;
  case P_ATTRIBUTE: Attribute(p,nc1,p1); 
    if(bad_attribute(p1)) {char *s=nc2str(nc),*s1=nc2str(nc1); error(ER_BADATTR,s1,s); free(s1); free(s);}
    goto UNARY;
  UNARY: path(p1,nc); break; 

  default: assert(0);
  }
}

static void paths() {
  int i=0,p,p1,nc=-1;
  if(bad_start(flat[0])) error(ER_BADSTART);
  for(i=1;i!=n_f;++i) {
    p=flat[i];
    if(P_IS(p,ELEMENT)) {
      Element(p,nc,p1);
      path(p,nc);
    }
  }
}

void rnd_restrictions() {
  loops(); if(errors) return; /* loops can cause endless loops in subsequent calls */
  ctypes();
  paths();
}

static void nullables() {
  int i,p,p1,p2,changed;
  do {
    changed=0;
    for(i=0;i!=n_f;++i) {
      p=flat[i];
      if(!nullable(p)) {
	switch(P_TYP(p)) {
	case P_NOT_ALLOWED:
	case P_DATA: case P_DATA_EXCEPT: case P_VALUE: case P_LIST:
	case P_ATTRIBUTE: case P_ELEMENT:
	  break;

	case P_CHOICE: Choice(p,p1,p2); setNullable(p,nullable(p1)||nullable(p2)); break;
	case P_INTERLEAVE: Interleave(p,p1,p2); setNullable(p,nullable(p1)&&nullable(p2)); break;
	case P_GROUP: Group(p,p1,p2);  setNullable(p,nullable(p1)&&nullable(p2)); break;

	case P_ONE_OR_MORE: OneOrMore(p,p1); setNullable(p,nullable(p1)); break;

	default: assert(0);
	}
	changed=changed||nullable(p);
      }
    }
  } while(changed);
}

static void cdatas() {
  int i,p,p1,p2,changed;
  do {
    changed=0;
    for(i=0;i!=n_f;++i) {
      p=flat[i];
      if(!cdata(p)) {
	switch(P_TYP(p)) {
	case P_NOT_ALLOWED: case P_EMPTY:
	case P_ATTRIBUTE: case P_ELEMENT:
	  break;

	case P_CHOICE: Choice(p,p1,p2); setCdata(p,cdata(p1)||cdata(p2)); break;
	case P_INTERLEAVE: Interleave(p,p1,p2); setCdata(p,cdata(p1)||cdata(p2)); break;
	case P_GROUP: Group(p,p1,p2);  setCdata(p,cdata(p1)||cdata(p2)); break;

	case P_ONE_OR_MORE: OneOrMore(p,p1); setCdata(p,cdata(p1)); break;

	default: assert(0);
	}
	changed=changed||cdata(p);
      }
    }
  } while(changed);
}

void rnd_traits() {
  nullables();
  cdatas();
}

void rnd_release() {
  free(flat); flat=NULL;
}

/*
 * $Log$
 * Revision 1.6  2003/12/08 23:18:48  dvd
 * string.h included from rnd for memcpy
 *
 * Revision 1.5  2003/12/08 21:23:47  dvd
 * +path restrictions
 *
 * Revision 1.4  2003/12/08 18:54:51  dvd
 * content-type checks
 *
 * Revision 1.3  2003/12/07 20:41:42  dvd
 * bugfixes, loops, traits
 *
 * Revision 1.2  2003/12/07 16:50:55  dvd
 * stage D, dereferencing and checking for loops
 *
 */
