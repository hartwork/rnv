/* $Id$ */

#include <string.h> /*memcpy*/
#include <stdio.h>
#include <assert.h>
#include "memops.h"
#include "rn.h"
#include "rnx.h"
#include "ll.h"
#include "rnd.h"

#define LEN_F RND_LEN_F
#define LEN_R RND_LEN_R

static int len_f,n_f,len_r,n_r;
static int *flat;
static int *refs;
static int errors;

#define err(msg) vfprintf(stderr,"error: "msg"\n",ap)
static void default_verror_handler(int er_no,va_list ap) {
  switch(er_no) {
  case RND_ER_LOOPST: err("loop in start pattern"); break;
  case RND_ER_LOOPEL: err("loop in pattern for element '%s'"); break;
  case RND_ER_CTYPE: err("content of element '%s' does not have a content-type"); break;
  case RND_ER_BADSTART: err("bad path in start pattern"); break;
  case RND_ER_BADMORE: err("bad path before '*' or '+' in element '%s'"); break;
  case RND_ER_BADEXPT: err("bad path after '-' in element '%s'"); break;
  case RND_ER_BADLIST: err("bad path after 'list' in element '%s'"); break;
  case RND_ER_BADATTR: err("bad path in attribute '%s' of element '%s'"); break;
  default: assert(0);
  }
}

void (*rnd_verror_handler)(int er_no,va_list ap)=&default_verror_handler;

static int initialized=0;
void rnd_init(void) {
  if(!initialized) {
    rn_init();
    initialized=1;
  }
}

void rnd_clear(void) {}


int rnd_errors(void) {
  return errors!=0;
}

static void error(int er_no,...) {
  va_list ap; va_start(ap,er_no); (*rnd_verror_handler)(er_no,ap); va_end(ap);
  ++errors;
}

static int deref(int p) {
  int p0=p,p1;
  P_CHK(p,REF);
  for(;;) {
    if(!marked(p)) {
      if(n_r==len_r) refs=(int*)memstretch(refs,len_r=(n_r*2),n_r,sizeof(int));
      refs[n_r++]=p;
    }
    mark(p);
    Ref(p,p1);
    if(!P_IS(p1,REF)||p1==p0) break;
    p=p1;
  }
  return p1;
}

static void flatten(int p) { if(!marked(p)) {flat[n_f++]=p; mark(p);}}

void rnd_deref(int start) {
  int p,p1,p2,nc,i,changed;

  flat=(int*)memalloc(len_f=LEN_F,sizeof(int)); n_f=0;
  refs=(int*)memalloc(len_r=LEN_R,sizeof(int)); n_r=0;
  errors=0;

  if(P_IS(start,REF)) start=deref(start);
  flatten(start);

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
      if(changed) {rn_del_p(p); rn_pattern[p+1]=p1; rn_pattern[p+2]=p2; rn_add_p(p);}
      if(n_f+2>len_f) flat=(int*)memstretch(flat,len_f=2*(n_f+2),n_f,sizeof(int));
      flatten(p1); flatten(p2);
      break;

    case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
    case P_LIST: List(p,p1); goto UNARY;
    case P_ATTRIBUTE: Attribute(p,nc,p1); goto UNARY;
    case P_ELEMENT: Element(p,nc,p1); goto UNARY;
    UNARY:
      changed=0;
      if(P_IS(p1,REF)) {p1=deref(p1); changed=1;}
      if(changed) {rn_del_p(p); rn_pattern[p+1]=p1; rn_add_p(p);}
      if(n_f+1>len_f) flat=(int*)memstretch(flat,len_f=2*(n_f+1),n_f,sizeof(int));
      flatten(p1);
      break;

    case P_REF: /* because of a loop, but will be handled in rnd_loops */
      break;

    default: 
      assert(0);
    }
  } while(i!=n_f);
  for(i=0;i!=n_f;++i) unmark(flat[i]);
  for(i=0;i!=n_r;++i) {p=refs[i]; rn_pattern[p+1]=0; unmark(p);}
  memfree(refs);
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

static void loops(void) {
  int i=0,p=flat[i],nc=-1,p1;
  for(;;) {
    if(loop(p)) {
      if(i==0) error(RND_ER_LOOPST); else {
	char *s=rnx_nc2str(nc);
	error(RND_ER_LOOPEL,s);
	memfree(s);
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
    case P_LIST: setContentType(p,P_FLG_CTS,0); break;
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

static void ctypes(void) {
  int i,p,p1,nc;
  for(i=0;i!=n_f;++i) {
    p=flat[i];
    if(P_IS(p,ELEMENT)) {
      Element(p,nc,p1);
      ctype(p1);
      if(!contentType(p1)) {
	char *s=rnx_nc2str(nc);
	error(RND_ER_CTYPE,s);
	memfree(s);
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
    if(bad_data_except(p2)) {char *s=rnx_nc2str(nc); error(RND_ER_BADEXPT,s); memfree(s);}
    goto BINARY;
  BINARY: path(p1,nc); path(p2,nc); break;

  case P_ONE_OR_MORE: OneOrMore(p,p1); 
    if(bad_one_or_more(p1,0)) {char *s=rnx_nc2str(nc); error(RND_ER_BADMORE,s); memfree(s);}
    goto UNARY;
  case P_LIST: List(p,p1); 
    if(bad_list(p1)) {char *s=rnx_nc2str(nc); error(RND_ER_BADLIST,s); memfree(s);}
    goto UNARY;
  case P_ATTRIBUTE: Attribute(p,nc1,p1); 
    if(bad_attribute(p1)) {char *s=rnx_nc2str(nc),*s1=rnx_nc2str(nc1); error(RND_ER_BADATTR,s1,s); memfree(s1); memfree(s);}
    goto UNARY;
  UNARY: path(p1,nc); break; 

  default: assert(0);
  }
}

static void paths(void) {
  int i,p,p1,nc;
  if(bad_start(flat[0])) error(RND_ER_BADSTART);
  for(i=1;i!=n_f;++i) {
    p=flat[i];
    if(P_IS(p,ELEMENT)) {
      Element(p,nc,p1);
      path(p,nc);
    }
  }
}

void rnd_restrictions(void) {
  loops(); if(errors) return; /* loops can cause endless loops in subsequent calls */
  ctypes();
  paths();
}

static void nullables(void) {
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

static void cdatas(void) {
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

void rnd_traits(void) {
  nullables();
  cdatas();
}

int rnd_release(void) {
  int start=flat[0];
  memfree(flat); flat=NULL;
  return start;
}
