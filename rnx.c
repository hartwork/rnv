/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <string.h> /*memcpy*/
#include "strops.h"
#include "rn.h"
#include "ll.h"
#include "rnx.h"

#define LEN_EXP RNX_LEN_EXP
#define LIM_EXP RNX_LIM_EXP

int *rnx_exp=NULL;
int rnx_n_exp;
static int len_exp;

static int initialized=0;
void rnx_init(void) {
  if(!initialized) { initialized=1;
    rnx_exp=(int*)calloc(len_exp=LEN_EXP,sizeof(int));
  }
}

void rnx_clear(void) {}

static void expected(int p,int first) {
  int p1,p2,px=0,i;
  switch(P_TYP(p)) {
  case P_ERROR: break;
  case P_EMPTY: break;
  case P_NOT_ALLOWED: break;
  case P_TEXT: px=p; break;
  case P_CHOICE: Choice(p,p1,p2); expected(p1,first); expected(p2,first); break;
  case P_INTERLEAVE: Interleave(p,p1,p2); expected(p1,first); expected(p2,first); break;
  case P_GROUP: Group(p,p1,p2); expected(p1,first); expected(p2,first||nullable(p1)); break;
  case P_ONE_OR_MORE: OneOrMore(p,p1); expected(p1,first); break;
  case P_LIST: List(p,p1); expected(p1,first); break;
  case P_DATA: px=p; break;
  case P_DATA_EXCEPT: DataExcept(p,p1,p2); expected(p1,first); break;
  case P_VALUE: px=p; break;
  case P_ATTRIBUTE: px=p; break;
  case P_ELEMENT: px=p; break;
  case P_AFTER: After(p,p1,p2); expected(p1,first); if(nullable(p1)) px=p; break;
  case P_REF: break;
  default: assert(0);
  }
  if(px) {
    for(i=0;i!=rnx_n_exp;++i) {
      if(rnx_exp[i]==px) {px=0; break;}
    }
    if(px) {
      if(rnx_n_exp==len_exp) {
	int *newexp=(int*)calloc(len_exp*=2,sizeof(int));
	memcpy(newexp,rnx_exp,rnx_n_exp*sizeof(int)); free(rnx_exp);
	rnx_exp=newexp;
      }
      rnx_exp[rnx_n_exp++]=px;
    }
  }
}
void rnx_expected(int p) {
  if(len_exp>LIM_EXP) {
    free(rnx_exp);
    rnx_exp=(int*)calloc(len_exp=LIM_EXP,sizeof(int));
  }
  rnx_n_exp=0;
  expected(p,1);
}

char *rnx_p2str(int p) {
  char *s=NULL,*s1;
  int dt,ps,val,nc,p1;
  switch(P_TYP(p)) {
  case P_ERROR: s=strclone("error"); break;
  case P_EMPTY: s=strclone("empty"); break;
  case P_NOT_ALLOWED: s=strclone("notAllowed"); break;
  case P_TEXT: s=strclone("text"); break;
  case P_CHOICE: s=strclone("choice (|)"); break;
  case P_INTERLEAVE: s=strclone("interleave (&)"); break;
  case P_GROUP: s=strclone("group (,)"); break;
  case P_ONE_OR_MORE: s=strclone("one or more (+)"); break;
  case P_LIST: s=strclone("list"); break;
  case P_DATA: Data(p,dt,ps);
    s1=rnx_nc2str(dt);
    s=(char*)calloc(strlen("data ")+1+strlen(s1),sizeof(char));
    strcpy(s,"data "); strcat(s,s1);
    free(s1);
    break;
  case P_DATA_EXCEPT: s=strclone("dataExcept (-)");  break;
  case P_VALUE: Value(p,dt,val);
    s1=rnx_nc2str(dt);
    s=(char*)calloc(strlen("value \"\" ")+1+strlen(s1)+strlen(rn_string+val),sizeof(char));
    strcpy(s,"value "); strcat(s,s1); strcat(s," \""); strcat(s,rn_string+val); strcat(s,"\"");
    free(s1);
    break;
  case P_ATTRIBUTE: Attribute(p,nc,p1);
    s1=rnx_nc2str(nc);
    s=(char*)calloc(strlen("attribute ")+1+strlen(s1),sizeof(char));
    strcpy(s,"attribute "); strcat(s,s1);
    free(s1);
    break;
  case P_ELEMENT: Element(p,nc,p1);
    s1=rnx_nc2str(nc);
    s=(char*)calloc(strlen("element ")+1+strlen(s1),sizeof(char));
    strcpy(s,"element "); strcat(s,s1);
    free(s1);
    break;
  case P_REF: s=strclone("ref"); break;
  case P_AFTER: s=strclone("after"); break;
  default: assert(0);
  }
  return s;
}

char *rnx_nc2str(int nc) {
  char *s=NULL,*s1,*s2;
  int nc1,nc2,uri,name;
  switch(NC_TYP(nc)) {
  case NC_ERROR: s=strclone("?"); break;
  case NC_NSNAME:
    NsName(nc,uri);
    s=(char*)calloc(strlen(rn_string+uri)+3,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,":*");
    break;
  case NC_QNAME:
    QName(nc,uri,name); 
    s=(char*)calloc(strlen(rn_string+uri)+strlen(rn_string+name)+2,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,"^"); strcat(s,rn_string+name);
    break;
  case NC_ANY_NAME: s=strclone("*"); break;
  case NC_EXCEPT:
    NameClassExcept(nc,nc1,nc2);
    s1=rnx_nc2str(nc1); s2=rnx_nc2str(nc2);
    s=(char*)calloc(strlen(s1)+strlen(s2)+2,sizeof(char));
    strcpy(s,s1); strcat(s,"-"); strcat(s,s2);
    free(s1); free(s2);
    break;
  case NC_CHOICE:
    NameClassChoice(nc,nc1,nc2);
    s1=rnx_nc2str(nc1); s2=rnx_nc2str(nc2);
    s=(char*)calloc(strlen(s1)+strlen(s2)+2,sizeof(char));
    strcpy(s,s1); strcat(s,"|"); strcat(s,s2);
    free(s1); free(s2);
    break;
  case NC_DATATYPE:
    Datatype(nc,uri,name); 
    s=(char*)calloc(strlen(rn_string+uri)+strlen(rn_string+name)+2,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,"^"); strcat(s,rn_string+name);
    break;
  default: assert(0);
  }
  return s;
}
