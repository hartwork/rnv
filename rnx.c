/* $Id$ */

#include <stdlib.h> /*NULL*/
#include <string.h> /*strcat*/
#include "memops.h"
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
    rnx_exp=(int*)memalloc(len_exp=LEN_EXP,sizeof(int));
  }
}

void rnx_clear(void) {}

static void expected(int p,int first) {
  int p1,p2,px=0,i;
  switch(RN_P_TYP(p)) {
  case RN_P_ERROR: break;
  case RN_P_EMPTY: break;
  case RN_P_NOT_ALLOWED: break;
  case RN_P_TEXT: px=p; break;
  case RN_P_CHOICE: rn_Choice(p,p1,p2); expected(p1,first); expected(p2,first); break;
  case RN_P_INTERLEAVE: rn_Interleave(p,p1,p2); expected(p1,first); expected(p2,first); break;
  case RN_P_GROUP: rn_Group(p,p1,p2); expected(p1,first); expected(p2,first&&rn_nullable(p1)); break;
  case RN_P_ONE_OR_MORE: rn_OneOrMore(p,p1); expected(p1,first); break;
  case RN_P_LIST: rn_List(p,p1); expected(p1,first); break;
  case RN_P_DATA: px=p; break;
  case RN_P_DATA_EXCEPT: rn_DataExcept(p,p1,p2); expected(p1,first); break;
  case RN_P_VALUE: px=p; break;
  case RN_P_ATTRIBUTE: px=p; break;
  case RN_P_ELEMENT: px=p; break;
  case RN_P_AFTER: rn_After(p,p1,p2); expected(p1,first); if(rn_nullable(p1)) px=p; break;
  case RN_P_REF: break;
  default: assert(0);
  }
  if(px&&(first||RN_P_IS(px,ATTRIBUTE))) {
    for(i=0;i!=rnx_n_exp;++i) {
      if(rnx_exp[i]==px) {px=0; break;}
    }
    if(px) {
      if(rnx_n_exp==len_exp) rnx_exp=(int*)memstretch(rnx_exp,len_exp=2*rnx_n_exp,rnx_n_exp,sizeof(int));
      rnx_exp[rnx_n_exp++]=px;
    }
  }
}
void rnx_expected(int p) {
  if(len_exp>LIM_EXP) {
    memfree(rnx_exp);
    rnx_exp=(int*)memalloc(len_exp=LIM_EXP,sizeof(int));
  }
  rnx_n_exp=0;
  expected(p,1);
}

char *rnx_p2str(int p) {
  char *s=NULL,*s1;
  int dt,ps,val,nc,p1;
  switch(RN_P_TYP(p)) {
  case RN_P_ERROR: s=strclone("error"); break;
  case RN_P_EMPTY: s=strclone("empty"); break;
  case RN_P_NOT_ALLOWED: s=strclone("notAllowed"); break;
  case RN_P_TEXT: s=strclone("text"); break;
  case RN_P_CHOICE: s=strclone("choice (|)"); break;
  case RN_P_INTERLEAVE: s=strclone("interleave (&)"); break;
  case RN_P_GROUP: s=strclone("group (,)"); break;
  case RN_P_ONE_OR_MORE: s=strclone("one or more (+)"); break;
  case RN_P_LIST: s=strclone("list"); break;
  case RN_P_DATA: rn_Data(p,dt,ps);
    s1=rnx_nc2str(dt);
    s=(char*)memalloc(strlen("data ")+1+strlen(s1),sizeof(char));
    strcpy(s,"data "); strcat(s,s1);
    memfree(s1);
    break;
  case RN_P_DATA_EXCEPT: s=strclone("dataExcept (-)");  break;
  case RN_P_VALUE: rn_Value(p,dt,val);
    s1=rnx_nc2str(dt);
    s=(char*)memalloc(strlen("value \"\" ")+1+strlen(s1)+strlen(rn_string+val),sizeof(char));
    strcpy(s,"value "); strcat(s,s1); strcat(s," \""); strcat(s,rn_string+val); strcat(s,"\"");
    memfree(s1);
    break;
  case RN_P_ATTRIBUTE: rn_Attribute(p,nc,p1);
    s1=rnx_nc2str(nc);
    s=(char*)memalloc(strlen("attribute ")+1+strlen(s1),sizeof(char));
    strcpy(s,"attribute "); strcat(s,s1);
    memfree(s1);
    break;
  case RN_P_ELEMENT: rn_Element(p,nc,p1);
    s1=rnx_nc2str(nc);
    s=(char*)memalloc(strlen("element ")+1+strlen(s1),sizeof(char));
    strcpy(s,"element "); strcat(s,s1);
    memfree(s1);
    break;
  case RN_P_REF: s=strclone("ref"); break;
  case RN_P_AFTER: s=strclone("after"); break;
  default: assert(0);
  }
  return s;
}

char *rnx_nc2str(int nc) {
  char *s=NULL,*s1,*s2;
  int nc1,nc2,uri,name;
  switch(RN_NC_TYP(nc)) {
  case RN_NC_ERROR: s=strclone("?"); break;
  case RN_NC_NSNAME:
    rn_NsName(nc,uri);
    s=(char*)memalloc(strlen(rn_string+uri)+3,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,":*");
    break;
  case RN_NC_QNAME:
    rn_QName(nc,uri,name);
    s=(char*)memalloc(strlen(rn_string+uri)+strlen(rn_string+name)+2,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,"^"); strcat(s,rn_string+name);
    break;
  case RN_NC_ANY_NAME: s=strclone("*"); break;
  case RN_NC_EXCEPT:
    rn_NameClassExcept(nc,nc1,nc2);
    s1=rnx_nc2str(nc1); s2=rnx_nc2str(nc2);
    s=(char*)memalloc(strlen(s1)+strlen(s2)+2,sizeof(char));
    strcpy(s,s1); strcat(s,"-"); strcat(s,s2);
    memfree(s1); memfree(s2);
    break;
  case RN_NC_CHOICE:
    rn_NameClassChoice(nc,nc1,nc2);
    s1=rnx_nc2str(nc1); s2=rnx_nc2str(nc2);
    s=(char*)memalloc(strlen(s1)+strlen(s2)+2,sizeof(char));
    strcpy(s,s1); strcat(s,"|"); strcat(s,s2);
    memfree(s1); memfree(s2);
    break;
  case RN_NC_DATATYPE:
    rn_Datatype(nc,uri,name);
    s=(char*)memalloc(strlen(rn_string+uri)+strlen(rn_string+name)+2,sizeof(char));
    strcpy(s,rn_string+uri); strcat(s,"^"); strcat(s,rn_string+name);
    break;
  default: assert(0);
  }
  return s;
}

/*
isany::Pattern->Bool
isany p =
  let
    isanycontent
      p@(OneOrMore
	  (Choice
	    (Choice
	      (Element AnyName p1)
	      (Attribute AnyName Text))
	    Text)) = p == p1
    isanycontent _ = False
    isanymixed (OneOrMore (Choice (Element AnyName p1) Text)) = isanycontent p1
    isanymixed _ = False
  in
     case p of
       (After p1 Empty) -> isanymixed p1
       (After p1 p2) -> isanymixed p1 && isany p2
       _ -> False
*/

static int isanycont(int p) {
  int p0,nc,p1,p2,i,res,flat[3];
  p0=p; if(!RN_P_IS(p0,ONE_OR_MORE)) return 0;
  rn_OneOrMore(p0,p1);
  p0=p1; if(!RN_P_IS(p0,CHOICE)) return 0;
  rn_Choice(p0,p1,p2); flat[0]=p2;
  p0=p1; if(!RN_P_IS(p0,CHOICE)) return 0;
  rn_Choice(p0,p1,p2); flat[1]=p1; flat[2]=p2;
  res=0;
  for(i=0;i!=3;++i) {
    p0=flat[i];
    switch(RN_P_TYP(p0)) {
    case RN_P_ELEMENT: rn_Element(p0,nc,p1); 
      if(!(RN_NC_IS(nc,ANY_NAME)&&p==p1)) return 0;
      res|=1; break;
    case RN_P_ATTRIBUTE: rn_Attribute(p0,nc,p1); 
      if(!(RN_NC_IS(nc,ANY_NAME)&&p1==rn_text)) return 0;
      res|=2; break;
    case RN_P_TEXT: break;
    default: return 0;
    }
  }
  return res==3;
}

static int isanymix(int p) {
  int p0,nc,p1,p2,i,res,flat[2];
  p0=p; if(!RN_P_IS(p0,ONE_OR_MORE)) return 0;
  rn_OneOrMore(p0,p1);
  p0=p1; if(!RN_P_IS(p0,CHOICE)) return 0;
  rn_Choice(p0,p1,p2); flat[0]=p1; flat[1]=p2;
  res=0;
  for(i=0;i!=2;++i) {
    p0=flat[i];
    switch(RN_P_TYP(p0)) {
    case RN_P_ELEMENT: rn_Element(p0,nc,p1); 
      if(!(RN_NC_IS(nc,ANY_NAME)&& isanycont(p1))) return 0;
      res|=1; break;
    case RN_P_TEXT: break;
    default: return 0;
    }
  }
  return res==1;
}

int rnx_isany(int p) {
  int p1,p2;
  if(!RN_P_IS(p,AFTER)) return 0;
  rn_After(p,p1,p2); return isanymix(p1)&&(p2==rn_empty||rnx_isany(p2));
}
