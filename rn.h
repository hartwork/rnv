/* $Id$ */

#ifndef RN_H
#define RN_H 1

#include <assert.h>

/* Patterns */
#define P_ERROR 0
#define P_EMPTY 1
#define P_NOT_ALLOWED 2
#define P_TEXT 3
#define P_CHOICE 4
#define P_INTERLEAVE 5
#define P_GROUP 6
#define P_ONE_OR_MORE 7
#define P_LIST 8
#define P_DATA 9
#define P_DATA_EXCEPT 10
#define P_VALUE 11
#define P_ATTRIBUTE 12
#define P_ELEMENT 13
#define P_AFTER 14

/*
Patterns and nameclasses are stored in arrays of arrays of integers.
an integer is either an index in the same or another array,
or a value that denotes record type etc.

Each record has a macro that accesses its fields by assigning
them to variables in the local scope, and a creator.
*/

/* Pattern Bindings */
#define P_TYP(i) (rn_pattern[i][0]&0xff)
#define P_IS(i,x)  (P_##x==P_TYP(i))
#define P_CHK(i,x)  assert(P_IS(i,x))
#define P_NEW(x) rn_pattern[rn_i_p][0]=P_##x

#define P_NUL_BIT 0x100
#define P_TXT_BIT 0x200

#define nullable(i) (rn_pattern[i][0]&P_NUL_BIT)
#define setNullable(x) if(x) rn_pattern[rn_i_p][0]|=P_NUL_BIT

#define cdata(i) (rn_pattern[i][0]&P_TXT_BIT)
#define setCdata(x) if(x) rn_pattern[rn_i_p][0]|=P_TXT_BIT

#define Empty(i) P_CHK(i,EMPTY)
#define newEmpty() P_NEW(EMPTY); \
  setNullable(1)

#define NotAllowed(i) P_CHK(i,NOT_ALLOWED)
#define newNotAllowed() P_NEW(NOT_ALLOWED)

#define Text(i) P_CHK(i,TEXT)
#define newText() P_NEW(TEXT); \
  setNullable(1); setCdata(1)

#define Choice(i,p1,p2) P_CHK(i,CHOICE); \
  p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define newChoice(p1,p2) P_NEW(CHOICE); \
  rn_pattern[rn_i_p][1]=p1; rn_pattern[rn_i_p][2]=p2; \
  setNullable(nullable(p1)||nullable(p2)); \
  setCdata(cdata(p1)||cdata(p2))

#define Interleave(i,p1,p2) P_CHK(i,INTERLEAVE); \
  p1=rn_pattern[i][1]; p2=rn_pattern[i][2];
#define newInterleave(p1,p2) P_NEW(INTERLEAVE); \
  rn_pattern[rn_i_p][1]=p1; rn_pattern[rn_i_p][2]=p2; \
  setNullable(nullable(p1)&&nullable(p2)); \
  setCdata(cdata(p1)||cdata(p2))

#define Group(i,p1,p2) P_CHK(i,GROUP); \
  p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define newGroup(p1,p2) P_NEW(GROUP); \
  rn_pattern[rn_i_p][1]=p1; rn_pattern[rn_i_p][2]=p2; \
  setNullable(nullable(p1)&&nullable(p2)); \
  setCdata(cdata(p1)||cdata(p2))

#define OneOrMore(i,p1) P_CHK(i,ONE_OR_MORE); \
  p1=rn_pattern[i][1]
#define newOneOrMore(p1) P_NEW(ONE_OR_MORE); \
  rn_pattern[rn_i_p][1]=p1; \
  setNullable(nullable(p1)); setCdata(cdata(p1))

#define List(i,p1) P_CHK(i,LIST); \
  p1=rn_pattern[i][1]
#define newList(p1) P_NEW(LIST); \
  rn_pattern[rn_i_p][1]=p1; \
  setCdata(1)

#define Data(i,dt,ps) P_CHK(i,DATA); \
  dt=rn_pattern[i][1]
#define newData(dt,ps) P_NEW(DATA); \
  rn_pattern[rn_i_p][1]=dt; \
  setCdata(1)

#define DataExcept(i,dt,p1) P_CHK(i,DATA_EXCEPT); \
  dt=rn_pattern[i][1]; p1=rn_pattern[i][2]
#define newDataExcept(dt,p1) P_NEW(DATA_EXCEPT); \
  rn_pattern[rn_i_p][1]=dt; rn_pattern[rn_i_p][2]=p1; \
  setCdata(1)

#define Value(i,dt,s,ctx) P_CHK(i,VALUE); \
  dt=rn_pattern[i][1]; s=rn_pattern[i][2]
#define newValue(dt,s,ctx) P_NEW(VALUE); \
  rn_pattern[rn_i_p][1]=dt; rn_pattern[rn_i_p][2]=s; \
  setCdata(1)

#define Attribute(i,nc,p1) P_CHK(i,ATTRIBUTE); \
  nc=rn_pattern[i][1]; p1=rn_pattern[i][2]
#define newAttribute(nc,p1) P_NEW(ATTRIBUTE); \
  rn_pattern[rn_i_p][1]=nc; rn_pattern[rn_i_p][2]=p1

#define Element(i,nc,p1) P_CHK(i,ELEMENT); \
  nc=rn_pattern[i][1]; p1=rn_pattern[i][2]
#define newElement(nc,p1) P_NEW(ELEMENT); \
  rn_pattern[rn_i_p][1]=nc; rn_pattern[rn_i_p][2]=p1

#define After(i,qn,p1,p2) P_CHK(i,AFTER); \
  qn=rn_pattern[i][1]; p1=rn_pattern[i][2]; p2=rn_pattern[i][3]
#define newAfter(qn,p1,p2) P_NEW(AFTER); \
  rn_pattern[rn_i_p][1]=qn; rn_pattern[rn_i_p][2]=p1; rn_pattern[rn_i_p][3]=p2; \
  setCdata(cdata(p1))

/* Name Classes */
#define NC_ERROR 0
#define NC_ANY_NAME 1
#define NC_ANY_NAME_EXCEPT 2
#define NC_NAME 3
#define NC_NSNAME 4
#define NC_NSNAME_EXCEPT 5
#define NC_CHOICE 6
#define NC_DATATYPE 7

/* Name Class Bindings  */
#define NC_TYP(i) (rn_nameclass[i][0]&0xff)
#define NC_IS(i,x) (NC_##x==NC_TYP(i))
#define NC_CHK(i,x) assert(NC_IS(i,x))
#define NC_NEW(x) rn_nameclass[rn_i_nc][0]=NC_##x

#define AnyName(i) NC_CHK(i,ANY_NAME)
#define newAnyName() NC_NEW(ANY_NAME)

#define AnyNameExcept(i,nc) NC_CHK(i,ANY_NAME_EXCEPT); \
  nc=rn_nameclass[i][1]
#define newAnyNameExcept(nc) NC_NEW(ANY_NAME_EXCEPT); \
  rn_nameclass[rn_i_nc][1]=nc

#define Name(i,uri,localname) NC_CHK(i,NAME); \
  uri=rn_nameclass[i][1]; localname=rn_nameclass[i][2]
#define newName(uri,localname) NC_NEW(NAME); \
  rn_nameclass[rn_i_nc][1]=uri; rn_nameclass[rn_i_nc][2]=localname

#define NsName(i,uri) NC_CHK(i,NSNAME); \
  uri=rn_nameclass[i][1]
#define newNsName(uri) NC_NEW(NSNAME); \
  rn_nameclass[rn_i_nc][1]=uri

#define NsNameExcept(i,uri,nc) NC_CHK(i,NSNAME_EXCEPT); \
  uri=rn_nameclass[i][1]; nc=rn_nameclass[i][2]
#define newNsNameExcept(uri,nc) NC_NEW(NSNAME_EXCEPT); \
  rn_nameclass[rn_i_nc][1]=uri; rn_nameclass[rn_i_nc][2]=nc

#define NameClassChoice(i,nc1,nc2) NC_CHK(i,CHOICE); \
  nc1=rn_nameclass[i][1]; nc2=rn_nameclass[i][2]
#define newNameClassChoice(nc1,nc2) NC_NEW(CHOICE); \
  rn_nameclass[rn_i_nc][1]=nc1; rn_nameclass[rn_i_nc][2]=nc2

#define QName(i,uri,localname) NC_CHK(i,NAME); \
  uri=rn_nameclass[i][1]; localname=rn_nameclass[i][2]
#define newQName(uri,localname) NC_NEW(NAME); \
  rn_nameclass[rn_i_nc][1]=uri; rn_nameclass[rn_i_nc][2]=localname

#define Datatype(i,lib,dt) NC_CHK(i,DATATYPE); \
  lib=rn_nameclass[i][1]; dt=rn_nameclass[i][2]
#define newDatatype(lib,dt) NC_NEW(DATATYPE); \
  rn_nameclass[rn_i_nc][1]=lib; rn_nameclass[rn_i_nc][2]=dt

extern int rn_empty,rn_text,rn_notAllowed;

extern int rn_i_p,rn_i_nc,rn_i_s; /* current index, a new element is created at this index */
extern int rn_accept_p(); /* the pattern at i_p is either new and i_p is incremented, or the same pattern is in the table */
extern int rn_accept_nc(); /* same for name class */
extern int rn_accept_s(char *s); /* accept a string, store it in the string pool or return a pointer to one already in the pool */

#define P_SIZE 4
#define NC_SIZE 3

extern int (*rn_pattern)[P_SIZE];
extern int (*rn_nameclass)[NC_SIZE];

extern char *rn_string; /* string pool */
extern int *rn_first, *rn_first_a, *rn_first_c, *rn_first_to; /* -1 if not computed, otherwise index in firsts */
extern int *rn_firsts; /* -1 terminates a list */

extern void rn_init();

#endif

/*
 * $Log$
 * Revision 1.8  2003/12/01 14:44:53  dvd
 * patterns in progress
 *
 * Revision 1.7  2003/11/29 20:51:39  dvd
 * nameclasses
 *
 * Revision 1.6  2003/11/29 17:47:48  dvd
 * decl
 *
 * Revision 1.5  2003/11/26 00:37:47  dvd
 * parser in progress, documentation handling removed
 *
 * Revision 1.4  2003/11/24 23:00:27  dvd
 * literal, error reporting
 *
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
