/* $Id$ */

#include <stdlib.h>
#include "dsl.h"

char *dsl_scm=NULL;

#ifndef DSL_SCM
#define DSL_SCM 0
#endif

#if DSL_SCM

#include <string.h>
#include UNISTD_H
#include SCM_H
#include "m.h"
#include "er.h"

/* simple rules better */
static char *implpath(void) {
  char *path=getenv("SCM_INIT_PATH");
  return path&&access(path,R_OK)!=-1?path:IMPLINIT;
}

static void init_user_scm_dsl(void) {}
static SCM  toplvl(void) {return scm_ldfile(dsl_scm)?BOOL_F:BOOL_T;}

static int initialized=0;
static void init(void) {
  if(!initialized) {initialized=1;
    init_user_scm=&init_user_scm_dsl;
    { char *argv[]={"dsl_scm",NULL};  /*Init.scm wants args*/
      scm_init_from_argv(1,argv,0,0,0L);
    }
    if(BOOL_F==scm_top_level(implpath(),&toplvl)) {
      (*er_printf)("dsl: cannot load %s\n",dsl_scm);
    }
  }
}

#define ALLOWS "(dsl-allows? \"%s\" '(%s) \"%s\")"
#define PARAM "(\"%s\".\"%s\")"
#define EQUAL "(dsl-equal? \"%s\" \"%s\" \"%s\")"

static int strnesc(char *d,char *s,int n) {
  char *t=d;
  while(n--!=0) {if(*s=='\\'||*s=='\"') *(t++)='\\'; *(t++)=*(s++);} *t=0;
  return t-d;
}
static int stresc(char *d,char *s) {return strnesc(d,s,strlen(s));}

#define shere(bp,sp) while(!((*(bp++)=*(sp++))=='%'&&(*(bp++)=*(sp++))=='s')); bp-=2;

int dsl_allows(char *typ,char *ps,char *s,int n) {
  char *buf,*sp,*bp, *p;
  int np,lenp;
  SCM ret=BOOL_F;

  if(dsl_scm) {init();
    p=ps; np=0;
    while(*p) {++np; while(*(p++)); while(*(p++));}
    lenp=p-ps-2*np;
    buf=(char*)m_alloc(
      strlen(ALLOWS)+np*strlen(PARAM)+2*(strlen(typ)+lenp+n)+1,
      sizeof(char));
    bp=buf; sp=ALLOWS;
    shere(bp,sp); bp+=stresc(bp,typ);
    shere(bp,sp); /* parameters */
    p=ps;
    while(np--) {
      char *sp=PARAM;
      shere(bp,sp); bp+=stresc(bp,p); while(*(p++));
      shere(bp,sp); bp+=stresc(bp,p); while(*(p++));
      while(*sp) *(bp++)=*(sp++);
    }
    shere(bp,sp); bp+=strnesc(bp,s,n);
    while((*(bp++)=*(sp++)));
    ret=scm_evstr(buf);
    m_free(buf); 
  }
  return ret!=BOOL_F;
}

int dsl_equal(char *typ,char *val,char *s,int n) {
  char *buf,*sp,*bp;
  SCM ret=BOOL_F;

  if(dsl_scm) {init();
    buf=(char*)m_alloc(
      strlen(EQUAL)+2*(strlen(typ)+strlen(val)+n)+1,
      sizeof(char));
    bp=buf; sp=EQUAL;
    shere(bp,sp); bp+=stresc(bp,typ);
    shere(bp,sp); bp+=stresc(bp,val);
    shere(bp,sp); bp+=strnesc(bp,s,n);
    while((*(bp++)=*(sp++)));
    ret=scm_evstr(buf);
    m_free(buf);
  }
  return ret!=BOOL_F;
}

#else

int dsl_allows(char *typ,char *ps,char *s,int n) {return 0;}
int dsl_equal(char *typ,char *val,char *s,int n) {return 0;}

#endif
