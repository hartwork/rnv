#include <stdlib.h> /*calloc,free*/
#include <assert.h> /*assert*/
#include "rn.h"
#include "rnd.h"

static int deref(int ref) {do {Ref(ref,ref);} while(P_IS(ref,REF)); return ref;}

void unary(int p,int p1) {
  if(P_IS(p1,REF)) {rn_del_p(p); rn_pattern[p][1]=deref(p1); rn_add_p(p);}
}

static void binary(int p,int p1,int p2) {
  int changed=0;
  if(P_IS(p1,REF)) {p1=deref(p1); changed=1;}
  if(P_IS(p2,REF)) {p2=deref(p2); changed=1;}
  if(changed) {rn_del_p(p); rn_pattern[p][1]=p1; rn_pattern[p][2]=p2; rn_add_p(p);}
}

int rnd_deref(int start) {
  int p,p1,p2,nc;
  if(P_IS(start,REF)) start=deref(start);
  p=start;
  for(;;) {
    switch(P_TYP(p)) {
    case P_EMPTY:
    case P_NOT_ALLOWED:
    case P_TEXT:
    case P_DATA:
    case P_VALUE:
      break;
    case P_CHOICE: Choice(p,p1,p2); binary(p,p1,p2); break;
    case P_INTERLEAVE: Interleave(p,p1,p2); binary(p,p1,p2); break;
    case P_GROUP: Group(p,p1,p2); binary(p,p1,p2); break;
    case P_DATA_EXCEPT: DataExcept(p,p1,p2); binary(p,p1,p2); break;

    case P_ONE_OR_MORE: OneOrMore(p,p1); unary(p,p1); break;
    case P_LIST: List(p,p1); unary(p,p1); break;
    case P_ATTRIBUTE: Attribute(p,p1,nc); unary(p,p1); break;
    case P_ELEMENT: Element(p,p1,nc); unary(p,p1); break;
    default:
      assert(0);
    }
    break;
  }
  return start;
}


