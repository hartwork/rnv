/* $Id$ */

#ifndef RN_H
#define RN_H 1

#include <assert.h>

/* Patterns */
#define P_EMPTY 0
#define P_NOT_ALLOWED 1
#define P_TEXT 2
#define P_CHOICE 3
#define P_INTERLEAVE 4
#define P_GROUP 5
#define P_ONE_OR_MORE 6
#define P_LIST 7
#define P_DATA 8
#define P_DATA_EXCEPT 9
#define P_VALUE 10
#define P_ATTRIBUTE 11
#define P_ELEMENT 12
#define P_AFTER 13

/* 
Patterns and nameclasses are stored in arrays of arrays of integers.
an integer is either an index in the same or another array,
or a value that denotes record type etc.

Each record has a macro that accesses its fields by assigning
them to variables in the local scope, and a creator.
*/

/* Pattern Bindings */
#define P_TYP(i) (rn_pattern[i][0]&0xf)
#define P_IS(i,x)  assert(P_##x==P_TYP(rn_pattern[i][0]))
#define P_NEW(i,x) rn_pattern[i][0]=P_##x

#define P_NUL_BIT 0x10
#define P_TXT_BIT 0x20

#define nullable(i) (rn_pattern[i][0]&P_NUL_BIT)
#define setNullable(i,x) if(x) rn_pattern[i][0]&=P_NUL_BIT

#define cdata(i) (rn_pattern[i][0]&P_TXT_BIT)
#define setCdata(i,x) if(x) rn_pattern[i][0]&=P_TXT_BIT

#define Empty(i) P_IS(i,EMPTY)
#define newEmpty(i) P_NEW(i,EMPTY); \
  setNullable(i,1)


#define NotAllowed(i) P_IS(i,NOT_ALLOWED)
#define newNotAllowed(i) P_NEW(i,NOT_ALLOWED)

#define Text(i) P_IS(i,TEXT)
#define newText(i) P_NEW(i,TEXT); \
  setNullable(i,1); setCdata(i,1)

#define Choice(i,p1,p2) P_IS(i,CHOICE) \
  p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define newChoice(i,p1,p2) P_NEW(i,CHOICE) \
  rn_pattern[i][1]=p1; rn_pattern[i][2]=p2; \
  setNullable(i,nullable(p1)||nullable(p2)); \
  setCdata(i,cdata(p1)||cdata(p2))
 
#define Interleave(i,p1,p2) P_IS(i,INTERLEAVE) \
  p1=rn_pattern[i][1]; p2=rn_pattern[i][2];
#define newInterleave(i,p1,p2) P_NEW(i,INTERLEAVE) \
  rn_pattern[i][1]=p1; rn_pattern[i][2]=p2; \
  setNullable(i,nullable(p1)&&nullable(p2)); \
  setCdata(i,cdata(p1)||cdata(p2))
 
#define Group(i,p1,p2) P_IS(i,GROUP) \
  p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define newGroup(i,p1,p2) P_NEW(i,GROUP) \
  rn_pattern[i][1]=p1; rn_pattern[i][2]=p2; \
  setNullable(i,nullable(p1)&&nullable(p2)); \
  setCdata(i,cdata(p1)||cdata(p2)
 
#define OneOrMore(i,p1) P_IS(i,ONE_OR_MORE) \
  p1=rn_pattern[i][1]
#define newOneOrMore(i,p1) P_NEW(i,ONE_OR_MORE) \
  rn_pattern[i][1]=p1; \
  setNullable(i,nullable(p1)); setCdata(i,cdata(p1))
 
#define List(i,p1) P_IS(i,LIST) \
  p1=rn_pattern[i][1]
#define newList(i,p1) P_NEW(i,LIST) \
  rn_pattern[i][1]=p1; \
  setCdata(i,1)
 
#define Data(i,dt,ps) P_IS(i,DATA) \
  dt=rn_pattern[i][1]
#define newData(i,dt,ps) P_NEW(i,DATA) \
  rn_pattern[i][1]=dt; \
  setCdata(i,1)

#define DataExcept(i,dt,ps,p1) P_IS(i,DATA_EXCEPT) \
  dt=rn_pattern[i][1]; p1=rn_pattern[i][2]
#define newDataExcept(i,dt,ps,p1) P_NEW(i,DATA_EXCEPT) \
  rn_pattern[i][11=dt; rn_pattern[i][2]=p1; \
  setCdata(i,1)

#define Value(i,dt,s,ctx) P_IS(i,VALUE) \
  dt=rn_pattern[i][1]; s=rn_string+rn_pattern[i][2]
#define newValue(i,dt,s,ctx) P_NEW(i,VALUE) \
  rn_pattern[i][1]=dt; rn_pattern[i][2]=s-rn_string; \
  setCdata(i,1)

#define Attribute(i,nc,p1) P_IS(i,ATTRIBUTE) \
  nc=rn_pattern[i][1]; p1=rn_pattern[i][2]
#define newAttribute(i,nc,p1) P_NEW(i,ATTRIBUTE) \
  rn_pattern[i][1]=nc; rn_pattern[i][2]=p1

#define Element(i,nc,p1) P_IS(i,ELEMENT) \
  nc=rn_pattern[i][1]; p1=rn_pattern[i][2]
#define newElement(i,nc,p1) P_NEW(i,ELEMENT) \
  rn_pattern[i][1]=nc; rn_pattern[i][2]=p1

#define After(i,qn,p1,p2) P_IS(i,AFTER) \
  qn=rn_pattern[i][1]; p1=rn_pattern[i][2]; p2=rn_pattern[i][3]
#define newAfter(i,qn,p1,p2) P_NEW(i,AFTER) \
  rn_pattern[i][1]=qn; rn_pattern[i][2]=p1; rn_pattern[i][3]=p2;
  setCdata(i,cdata(p1))

/* Name Classes */
#define NC_ANY_NAME 0
#define NC_ANY_NAME_EXCEPT 1
#define NC_NAME 2
#define NC_NSNAME 3
#define NC_NSNAME_EXCEPT 4
#define NC_CHOICE 5

/* Name Class Bindings  */
#define NC_TYP(i) (rn_nameclass[i][0]&0x7)
#define NC_IS(i,x) assert(NC_##x==NC_TYP(rn_nameclass[i][0]))
#define NC_NEW(i,x) rn_nameclass[i][0]=NC_##x

#define AnyName(i) NC_IS(i,ANY_NAME)
#define newAnyName(i) NC_NEW(i,ANY_NAME)
#define AnyNameExcept(i,nc) NC_IS(i,ANY_NAME_EXCEPT); \
  nc=rn_nameclass[i][1]
#define newAnyNameExcept(i,nc) NC_NEW(i,ANY_NAME_EXCEPT); \
  rn_nameclass[i][1]=nc
#define Name(i,uri,localname) NC_IS(i,NAME); \
  uri=rn_string+rn_nameclass[i][1]; localname=rn_string+rn_nameclass[i][2]
#define newName(i,uri,localname) NC_NEW(i,NAME); \
  rn_nameclass[i][1]=uri-rn_string; rn_nameclass[i][2]=localname-rn_string
#define NsName(i,uri) NC_IS(i,NSNAME); \
  uri=rn_string+rn_nameclass[i][1]
#define newNsName(i,uri) NC_NEW(i,NSNAME); \
  rn_string+rn_nameclass[i][1]=uri-rn_string
#define NsNameExcept(i,uri,nc) NC_IS(i,NSNAME_EXCEPT); \
  uri=rn_string+rn_nameclass[i][1]; nc=rn_nameclass[i][2]
#define newNsNameExcept(i,uri,nc) NC_NEW(i,NSNAME_EXCEPT); \
  rn_nameclass[i][1]=uri-rn_string; rn_nameclass[i][2]=nc-rn_string
#define NameClassChoice(i,nc1,nc2) NC_IS(i,CHOICE); \
  rn_nameclass[i][1]=nc1; rn_nameclass[i][2]=nc2

#define QName(i,uri,localname) NC_IS(i,NAME); \
  uri=rn_nameclass[i][1]; localname=rn_nameclass[i][2]
#define newQName(i,uri,localname) NC_NEW(i,NAME); \
  rn_nameclass[i][1]=uri; rn_nameclass[i][2]=localname

/* Built-in Datatypes */
#define DT_TOKEN 0
#define DT_STRING 1

/* Element Roles */
#define R_INLINE 1
#define R_BLOCK 2
#define R_SECTION 4

extern int rn_i_p,rn_i_nc; /* current index, a new element is created at this index */
extern int rn_accept_p(); /* the pattern at i_p is either new and i_p is incremented, or the same pattern is in the table */
extern int rn_accept_nc(); /* same for name class */

#define P_SIZE 4
#define NC_SIZE 3

extern int (*rn_pattern)[P_SIZE];
extern int (*rn_nameclass)[NC_SIZE];
extern char *rn_string; /* string pool */
extern int *rn_first, *rn_first_a, *rn_first_b, *rn_first_to; /* -1 if not computed, otherwise index in firsts */
extern int *rn_firsts; /* -1 terminates a list */

extern void rn_init();

#endif

/*
 * $Log$
 * Revision 1.3  2003/11/23 16:16:06  dvd
 * no roles for elements
 *
 * Revision 1.2  2003/11/20 07:46:16  dvd
 * +er, rnc in progress
 *
 * Revision 1.1  2003/11/17 21:33:28  dvd
 * +cimpl
 *
 */
