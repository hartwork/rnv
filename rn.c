/* $Id$ */

#include <string.h> /* strcmp,memcmp,strlen,strcpy,strclone,memcpy,memset */

#include "memops.h"
#include "strops.h"
#include "ht.h"
#include "ll.h"
#include "rn.h"

#define LEN_P RN_LEN_P
#define PRIME_P RN_PRIME_P
#define LIM_P RN_LIM_P
#define LEN_NC RN_LEN_NC
#define PRIME_NC RN_PRIME_NC
#define LEN_S RN_LEN_S

#define P_SIZE 3
#define NC_SIZE 3
#define P_AVG_SIZE 2
#define NC_AVG_SIZE 2
#define S_AVG_SIZE 16

#define erased(i) (rn_pattern[i]&P_FLG_ERS)
#define erase(i) (rn_pattern[i]|=P_FLG_ERS)

static int p_size[]={1,1,1,1,3,3,3,2,2,3,3,3,3,3,3,3};
static int nc_size[]={1,3,2,1,3,3,3};

int *rn_pattern;
int *rn_nameclass;
char *rn_string;
int rn_empty,rn_text,rn_notAllowed,rn_dt_string,rn_dt_token,rn_xsd_uri;

static struct hashtable ht_p, ht_nc, ht_s;

static int i_p,i_nc,i_s,BASE_P,base_p,i_ref;
static int len_p,len_nc,len_s;
static int adding_ps;

void rn_new_schema(void) {base_p=i_p; i_ref=0;}

void rn_del_p(int i) {if(ht_get(&ht_p,i)==i) ht_del(&ht_p,i);}
void rn_add_p(int i) {if(ht_get(&ht_p,i)==-1) ht_put(&ht_p,i);}

void setNullable(int i,int x) {if(x) rn_pattern[i]|=P_FLG_NUL;}
void setCdata(int i,int x) {if(x) rn_pattern[i]|=P_FLG_TXT;}
void setContentType(int i,int t1,int t2) {rn_pattern[i]|=(t1>t2?t1:t2);}

static int add_s(char *s) {
  int len=strlen(s)+1;
  if(i_s+len>len_s) rn_string=(char*)memstretch(rn_string,len_s=2*(i_s+len),i_s,sizeof(char));
  strcpy(rn_string+i_s,s);
  return len;
}

int newString(char *s) {
  int d_s,j;
  assert(!adding_ps);
  d_s=add_s(s);
  if((j=ht_get(&ht_s,i_s))==-1) {
    ht_put(&ht_s,j=i_s);
    i_s+=d_s;
  }
  return j;
}

#define P_NEW(x) rn_pattern[i_p]=P_##x

#define P_LABEL "pattern"
#define NC_LABEL "nameclass"

#define accept(name,n,N)  \
static int accept_##n(void) { \
  int j; \
  if((j=ht_get(&ht_##n,i_##n))==-1) { \
    ht_put(&ht_##n,j=i_##n); \
    i_##n+=n##_size[N##_TYP(i_##n)]; \
    if(i_##n+N##_SIZE>len_##n) rn_##name=(int *)memstretch(rn_##name,len_##n=2*(i_##n+N##_SIZE),i_##n,sizeof(int)); \
  } \
  return j; \
}

accept(pattern,p,P)
accept(nameclass,nc,NC)

int newEmpty(void) { P_NEW(EMPTY);
  setNullable(i_p,1);
  return accept_p();
}

int newNotAllowed(void) { P_NEW(NOT_ALLOWED);
  return accept_p();
}

int newText(void) { P_NEW(TEXT);
  setNullable(i_p,1); 
  setCdata(i_p,1);
  return accept_p();
}

int newChoice(int p1,int p2) { P_NEW(CHOICE);
  rn_pattern[i_p+1]=p1; rn_pattern[i_p+2]=p2;
  setNullable(i_p,nullable(p1)||nullable(p2)); 
  setCdata(i_p,cdata(p1)||cdata(p2)); 
  return accept_p();
}

int newInterleave(int p1,int p2) { P_NEW(INTERLEAVE);
  rn_pattern[i_p+1]=p1; rn_pattern[i_p+2]=p2;
  setNullable(i_p,nullable(p1)&&nullable(p2));
  setCdata(i_p,cdata(p1)||cdata(p2)); 
  return accept_p();
}

int newGroup(int p1,int p2) { P_NEW(GROUP);
  rn_pattern[i_p+1]=p1; rn_pattern[i_p+2]=p2;
  setNullable(i_p,nullable(p1)&&nullable(p2));
  setCdata(i_p,cdata(p1)||cdata(p2)); 
  return accept_p();
}

int newOneOrMore(int p1) { P_NEW(ONE_OR_MORE);
  rn_pattern[i_p+1]=p1;
  setNullable(i_p,nullable(p1));
  setCdata(i_p,cdata(p1));
  return accept_p();
}

int newList(int p1) { P_NEW(LIST);
  rn_pattern[i_p+1]=p1;
  setCdata(i_p,1);
  return accept_p();
}

int newData(int dt,int ps) { P_NEW(DATA);
  rn_pattern[i_p+1]=dt;
  rn_pattern[i_p+2]=ps;
  setCdata(i_p,1);
  return accept_p();
}

int newDataExcept(int p1,int p2) { P_NEW(DATA_EXCEPT);
  rn_pattern[i_p+1]=p1; rn_pattern[i_p+2]=p2;
  setCdata(i_p,1);
  return accept_p();
}

int newValue(int dt,int s) { P_NEW(VALUE);
  rn_pattern[i_p+1]=dt; rn_pattern[i_p+2]=s;
  setCdata(i_p,1);
  return accept_p();
}

int newAttribute(int nc,int p1) { P_NEW(ATTRIBUTE);
  rn_pattern[i_p+2]=nc; rn_pattern[i_p+1]=p1;
  return accept_p();
}

int newElement(int nc,int p1) { P_NEW(ELEMENT);
  rn_pattern[i_p+2]=nc; rn_pattern[i_p+1]=p1; 
  return accept_p();
}

int newAfter(int p1,int p2) { P_NEW(AFTER);
  rn_pattern[i_p+1]=p1; rn_pattern[i_p+2]=p2;
  setCdata(i_p,cdata(p1));
  return accept_p();
}

int newRef(void) { P_NEW(REF);
  rn_pattern[i_p+2]=i_ref++;
  return accept_p();
}

int rn_groupable(int p1,int p2) {
  int ct1=contentType(p1),ct2=contentType(p2);
  return ((ct1&ct2&P_FLG_CTC)||((ct1|ct2)&P_FLG_CTE));
}

int rn_one_or_more(int p) {
  if(P_IS(p,EMPTY)) return p;
  if(P_IS(p,NOT_ALLOWED)) return p;
  if(P_IS(p,TEXT)) return p;
  return newOneOrMore(p);
}

int rn_group(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  if(P_IS(p1,EMPTY)) return p2;
  if(P_IS(p2,EMPTY)) return p1;
  return newGroup(p1,p2);
}

static int samechoice(int p1,int p2) {
  if(P_IS(p1,CHOICE)) {
    int p11,p12; Choice(p1,p11,p12);
    return p12==p2||samechoice(p11,p2);
  } else return p1==p2;
}

int rn_choice(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p2;
  if(P_IS(p2,NOT_ALLOWED)) return p1;
  if(P_IS(p2,CHOICE)) {
    int p21,p22; Choice(p2,p21,p22);
    p1=newChoice(p1,p21); return rn_choice(p1,p22);
  }
  if(samechoice(p1,p2)) return p1;
  if(nullable(p1) && (P_IS(p2,EMPTY))) return p1;
  if(nullable(p2) && (P_IS(p1,EMPTY))) return p2;
  return newChoice(p1,p2);
}

int rn_ileave(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  if(P_IS(p1,EMPTY)) return p2;
  if(P_IS(p2,EMPTY)) return p1;
  return newInterleave(p1,p2);
}

int rn_after(int p1,int p2) {
  if(P_IS(p1,NOT_ALLOWED)) return p1;
  if(P_IS(p2,NOT_ALLOWED)) return p2;
  return newAfter(p1,p2);
}

#define NC_NEW(x) rn_nameclass[i_nc]=NC_##x

int newQName(int uri,int name) { NC_NEW(QNAME);
  rn_nameclass[i_nc+1]=uri; rn_nameclass[i_nc+2]=name;
  return accept_nc();
}

int newNsName(int uri) { NC_NEW(NSNAME);
  rn_nameclass[i_nc+1]=uri;
  return accept_nc();
}

int newAnyName(void) { NC_NEW(ANY_NAME);
  return accept_nc();
}

int newNameClassExcept(int nc1,int nc2) { NC_NEW(EXCEPT);
  rn_nameclass[i_nc+1]=nc1; rn_nameclass[i_nc+2]=nc2;
  return accept_nc();
}

int newNameClassChoice(int nc1,int nc2) { NC_NEW(CHOICE);
  rn_nameclass[i_nc+1]=nc1; rn_nameclass[i_nc+2]=nc2;
  return accept_nc();
}

int newDatatype(int lib,int typ) { NC_NEW(DATATYPE);
  rn_nameclass[i_nc+1]=lib; rn_nameclass[i_nc+2]=typ;
  return accept_nc();
}

int rn_i_ps(void) {adding_ps=1; return i_s;}
void rn_add_pskey(char *s) {i_s+=add_s(s);}
void rn_add_psval(char *s) {i_s+=add_s(s);}
void rn_end_ps(void) {i_s+=add_s(""); adding_ps=0;}

static int hash_p(int i);
static int hash_nc(int i);
static int hash_s(int i);

static int equal_p(int p1,int p2);
static int equal_nc(int nc1,int nc2); 
static int equal_s(int s1,int s2);

static void windup(void);

static int initialized=0;
void rn_init(void) {
  if(!initialized) { initialized=1;
    rn_pattern=(int *)memalloc(len_p=P_AVG_SIZE*LEN_P,sizeof(int));
    rn_nameclass=(int *)memalloc(len_nc=NC_AVG_SIZE*LEN_NC,sizeof(int));
    rn_string=(char*)memalloc(len_s=S_AVG_SIZE*LEN_S,sizeof(char));

    ht_init(&ht_p,LEN_P,&hash_p,&equal_p);
    ht_init(&ht_nc,LEN_NC,&hash_nc,&equal_nc);
    ht_init(&ht_s,LEN_S,&hash_s,&equal_s);

    windup();
  }
}

void rn_clear(void) {
  ht_clear(&ht_p); ht_clear(&ht_nc); ht_clear(&ht_s);
  windup();
}

static void windup(void) {
  i_p=i_nc=i_s=0;
  adding_ps=0;
  rn_pattern[0]=P_ERROR;  accept_p(); 
  rn_nameclass[0]=NC_ERROR; accept_nc();
  newString("");
  rn_empty=newEmpty(); rn_notAllowed=newNotAllowed(); rn_text=newText(); BASE_P=i_p;
  rn_dt_string=newDatatype(0,newString("string")); rn_dt_token=newDatatype(0,newString("token"));
  rn_xsd_uri=newString("http://www.w3.org/2001/XMLSchema-datatypes");
}

static int hash_p(int p) {
  int *pp=rn_pattern+p; int h=0;
  switch(p_size[P_TYP(p)]) {
  case 1: h=pp[0]&0xF; break;
  case 2: h=(pp[0]&0xF)|(pp[1]<<4); break;
  case 3: h=(pp[0]&0xF)|((pp[1]^pp[2])<<4); break;
  default: assert(0);
  }
  return h*PRIME_P;
}

static int hash_nc(int nc) {
  int *ncp=rn_nameclass+nc; int h=0;
  switch(nc_size[NC_TYP(nc)]) {
  case 1: h=ncp[0]&0x7; break;
  case 2: h=(ncp[0]&0x7)|(ncp[1]<<3); break;
  case 3: h=(ncp[0]&0x7)|((ncp[1]^ncp[2])<<3); break;
  default: assert(0);
  }
  return h*PRIME_NC;
}

static int hash_s(int i) {return strhash(rn_string+i);}

static int equal_p(int p1,int p2) {
  int *pp1=rn_pattern+p1,*pp2=rn_pattern+p2;
  if(P_TYP(p1)!=P_TYP(p2)) return 0;
  switch(p_size[P_TYP(p1)]) {
  case 3: if(pp1[2]!=pp2[2]) return 0;
  case 2: if(pp1[1]!=pp2[1]) return 0;
  case 1: return 1;
  default: assert(0);
  }
  return 0;
}

static int equal_nc(int nc1,int nc2) {
  int *ncp1=rn_nameclass+nc1,*ncp2=rn_nameclass+nc2;
  if(NC_TYP(nc1)!=NC_TYP(nc2)) return 0;
  switch(nc_size[NC_TYP(nc1)]) {
  case 3: if(ncp1[2]!=ncp2[2]) return 0;
  case 2: if(ncp1[1]!=ncp2[1]) return 0;
  case 1: return 1;
  default: assert(0);
  }
  return 0;
}

static int equal_s(int s1,int s2) {return strcmp(rn_string+s1,rn_string+s2)==0;}

/* marks patterns reachable from start, assumes that the references are resolved */
static void mark_p(int start) {
  int p,p1,p2,nc,i;
  int n_f=0;
  int *flat=(int*)memalloc(i_p,sizeof(int));

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
      if(!marked(p2)) {flat[n_f++]=p2; mark(p2);}
      goto UNARY;

    case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
    case P_LIST: List(p,p1); goto UNARY;
    case P_ATTRIBUTE: Attribute(p,nc,p1); goto UNARY;
    case P_ELEMENT: Element(p,nc,p1); goto UNARY;
    UNARY:
      if(!marked(p1)) {flat[n_f++]=p1; mark(p1);}
      break;

    default: 
      assert(0);
    }
  } while(i!=n_f);
  memfree(flat);
}

/* assumes that used patterns are marked */
static void sweep_p(int *starts,int n_st,int since) {
  int p,p1,p2,nc,q,changed,touched;
  int *xlat;
  xlat=(int*)memalloc(i_p-since,sizeof(int));
  changed=0;
  for(p=since;p!=i_p;p+=p_size[P_TYP(p)]) {
    if(!marked(p)) {
      rn_del_p(p);
      xlat[p-since]=-1;
    } else if((q=ht_get(&ht_p,p))!=p) {
      unmark(p);
      xlat[p-since]=q;
      changed=1;
    } else {
      xlat[p-since]=p;
    }
  }
  while(changed) {
    changed=0;
    for(p=since;p!=i_p;p+=p_size[P_TYP(p)]) {
      if(xlat[p-since]==p) {
	touched=0;
	switch(P_TYP(p)) {
	case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: case P_DATA: case P_VALUE:
	  break;

	case P_CHOICE: Choice(p,p1,p2); goto BINARY;
	case P_INTERLEAVE: Interleave(p,p1,p2); goto BINARY;
	case P_GROUP: Group(p,p1,p2); goto BINARY;
	case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY;
	BINARY:
	  if(p2>=since && (q=xlat[p2-since])!=p2) {
	    ht_del(&ht_p,p); 
	    touched=1; 
	    rn_pattern[p+2]=q;
	  }
	  goto UNARY;

	case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
	case P_LIST: List(p,p1); goto UNARY;
	case P_ATTRIBUTE: Attribute(p,nc,p1); goto UNARY;
	case P_ELEMENT: Element(p,nc,p1); goto UNARY;
	UNARY:
	  if(p1>=since && (q=xlat[p1-since])!=p1) {
	    if(!touched) ht_del(&ht_p,p);
	    touched=1; 
	    rn_pattern[p+1]=q;
	  }
	  break;

	default: 
	  assert(0);
	}
	if(touched) {
	  changed=1;
	  if((q=ht_get(&ht_p,p))==-1) {
	    ht_put(&ht_p,p);
	  } else {
	    unmark(p);
	    xlat[p-since]=q;
	  }
	}
      }
    }
  }
  while(n_st--!=0) {if(*starts>=since) *starts=xlat[*starts-since]; ++starts;}
  memfree(xlat);
}

static void unmark_p(int since) {
  int p; 
  for(p=0;p!=since;p+=p_size[P_TYP(p)]) unmark(p);
  for(p=since;p!=i_p;p+=p_size[P_TYP(p)]) {
    if(marked(p)) unmark(p); else erase(p);
  }
}

static void compress_p(int *starts,int n_st,int since) {
  int p,p1,p2,q,nc,i_q,touched;
  int *xlat=(int*)memalloc(i_p-since,sizeof(int));
  q=since;
  for(p=since;p!=i_p;p+=p_size[P_TYP(p)]) {
    if(erased(p)) {
      xlat[p-since]=-1;
    } else {
      xlat[p-since]=q; 
      q+=p_size[P_TYP(p)];
    }
  }
  i_q=q;
  for(p=since;p!=i_p;p+=p_size[P_TYP(p)]) {
    touched=0;
    if(xlat[p-since]!=-1) {
      switch(P_TYP(p)) {
      case P_EMPTY: case P_NOT_ALLOWED: case P_TEXT: case P_DATA: case P_VALUE:
	break;

      case P_CHOICE: Choice(p,p1,p2); goto BINARY;
      case P_INTERLEAVE: Interleave(p,p1,p2); goto BINARY;
      case P_GROUP: Group(p,p1,p2); goto BINARY;
      case P_DATA_EXCEPT: DataExcept(p,p1,p2); goto BINARY;
      BINARY:
	if(p2>=since && (q=xlat[p2-since])!=p2) {
	  ht_del(&ht_p,p); touched=1; 
	  rn_pattern[p+2]=q;
	}
	goto UNARY;

      case P_ONE_OR_MORE: OneOrMore(p,p1); goto UNARY;
      case P_LIST: List(p,p1); goto UNARY;
      case P_ATTRIBUTE: Attribute(p,nc,p1); goto UNARY;
      case P_ELEMENT: Element(p,nc,p1); goto UNARY;
      UNARY:
	if(p1>=since && (q=xlat[p1-since])!=p1) {
	  if(!touched) {ht_del(&ht_p,p); touched=1;}
	  rn_pattern[p+1]=q;
	}
	break;

      default: 
	assert(0);
      }
      if((q=xlat[p-since])!=p) {
	int i;
	if(!touched) {ht_del(&ht_p,p); touched=1;}
	for(i=0;i!=p_size[P_TYP(p)];++i) rn_pattern[q+i]=rn_pattern[p+i];
      }
      if(touched) ht_put(&ht_p,q);
    }
  }
  while(n_st--!=0) {if(*starts>=since) *starts=xlat[*starts-since]; ++starts;}
  if(i_q!=i_p) {
    int len_q=i_q*2;
    i_p=i_q;
    if(len_p>P_AVG_SIZE*LIM_P&&len_q<len_p) rn_pattern=(int*)memstretch(rn_pattern,len_p=len_q>P_AVG_SIZE*LEN_P?len_q:P_AVG_SIZE*LEN_P,i_p,sizeof(int));
  }
}

void rn_compress(int *starts,int n_st) {
  int i;
  for(i=0;i!=n_st;++i) mark_p(starts[i]);
  sweep_p(starts,n_st,BASE_P);
  unmark_p(BASE_P);
  compress_p(starts,n_st,BASE_P);
}

int rn_compress_last(int start) {
  mark_p(start);
  sweep_p(&start,1,base_p);
  unmark_p(base_p);
  compress_p(&start,1,base_p);
  return start;
}
