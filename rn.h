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
#define P_REF 14
#define P_AFTER 15
#define P_VOID 255

/*
Patterns and nameclasses are stored in arrays of arrays of integers.
an integer is either an index in the same or another array,
or a value that denotes record type etc.

Each record has a macro that accesses its fields by assigning
them to variables in the local scope, and a creator.
*/

/* Pattern Bindings */
#define P_TYP(i) (rn_pattern[i][0]&0xFF)
#define P_IS(i,x)  (P_##x==P_TYP(i))
#define P_CHK(i,x)  assert(P_IS(i,x))

#define P_FLG_NUL 0x00000100
#define P_FLG_TXT 0x00000200
#define P_FLG_CTE 0x00000400
#define P_FLG_CTC 0x00000800
#define P_FLG_CTS 0x00001000
#define P_FLG_MRK 0x10000000

#define marked(i) (rn_pattern[i][0]&P_FLG_MRK)
#define mark(i) (rn_pattern[i][0]|=P_FLG_MRK)
#define unmark(i) (rn_pattern[i][0]&=~P_FLG_MRK)

#define nullable(i) (rn_pattern[i][0]&P_FLG_NUL)
#define cdata(i) (rn_pattern[i][0]&P_FLG_TXT)
#define contentType(i) (rn_pattern[i][0]&0x1C00)

/* assert: p1 at 1, p2 at 2 */

#define Empty(i) P_CHK(i,EMPTY)
#define NotAllowed(i) P_CHK(i,NOT_ALLOWED)
#define Text(i) P_CHK(i,TEXT)
#define Choice(i,p1,p2) P_CHK(i,CHOICE); p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define Interleave(i,p1,p2) P_CHK(i,INTERLEAVE); p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define Group(i,p1,p2) P_CHK(i,GROUP); p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define OneOrMore(i,p1) P_CHK(i,ONE_OR_MORE); p1=rn_pattern[i][1]
#define List(i,p1) P_CHK(i,LIST); p1=rn_pattern[i][1]
#define Data(i,dt,ps) P_CHK(i,DATA); dt=rn_pattern[i][1]; ps=rn_pattern[i][2]
#define DataExcept(i,p1,p2) P_CHK(i,DATA_EXCEPT); p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define Value(i,dt,s) P_CHK(i,VALUE); dt=rn_pattern[i][1]; s=rn_pattern[i][2]
#define Attribute(i,nc,p1) P_CHK(i,ATTRIBUTE);  p1=rn_pattern[i][1]; nc=rn_pattern[i][2]
#define Element(i,nc,p1) P_CHK(i,ELEMENT); p1=rn_pattern[i][1]; nc=rn_pattern[i][2]
#define After(i,p1,p2) P_CHK(i,AFTER); p1=rn_pattern[i][1]; p2=rn_pattern[i][2]
#define Ref(i,p) P_CHK(i,REF); p=rn_pattern[i][1]

/* Name Classes */
#define NC_ERROR 0
#define NC_QNAME 1
#define NC_NSNAME 2
#define NC_ANY_NAME 3
#define NC_EXCEPT 4
#define NC_CHOICE 5
#define NC_DATATYPE 6

/* Name Class Bindings  */
#define NC_TYP(i) (rn_nameclass[i][0]&0xFF)
#define NC_IS(i,x) (NC_##x==NC_TYP(i))
#define NC_CHK(i,x) assert(NC_IS(i,x))

#define QName(i,uri,name) NC_CHK(i,QNAME); uri=rn_nameclass[i][1]; name=rn_nameclass[i][2]
#define NsName(i,uri) NC_CHK(i,NSNAME); uri=rn_nameclass[i][1]
#define AnyName(i) NC_CHK(i,ANY_NAME) 
#define NameClassExcept(i,nc1,nc2) NC_CHK(i,EXCEPT); nc1=rn_nameclass[i][1]; nc2=rn_nameclass[i][2]
#define NameClassChoice(i,nc1,nc2) NC_CHK(i,CHOICE); nc1=rn_nameclass[i][1]; nc2=rn_nameclass[i][2]
#define Datatype(i,lib,typ) NC_CHK(i,DATATYPE); lib=rn_nameclass[i][1]; typ=rn_nameclass[i][2]

extern int rn_empty,rn_text,rn_notAllowed,rn_dt_string,rn_dt_token,rn_xsd_uri;

extern char *rn_string;

#define P_SIZE 3
#define NC_SIZE 3

extern int (*rn_pattern)[P_SIZE];
extern int (*rn_nameclass)[NC_SIZE];

extern void rn_new_schema(void);

extern void setNullable(int i,int x);
extern void setCdata(int i,int x);
extern void setContentType(int i,int t1,int t2);

extern void rn_del_p(int i);
extern void rn_add_p(int i);

extern int newString(char *s);

extern int newEmpty(void);
extern int newNotAllowed(void);
extern int newText(void);
extern int newChoice(int p1,int p2);
extern int newInterleave(int p1,int p2);
extern int newGroup(int p1,int p2);
extern int newOneOrMore(int p1);
extern int newList(int p1);
extern int newData(int dt,int ps);
extern int newDataExcept(int p1,int p2);
extern int newValue(int dt,int s);
extern int newAttribute(int nc,int p1);
extern int newElement(int nc,int p1);
extern int newAfter(int p1,int p2);
extern int newRef(void);

extern int rn_groupable(int p1,int p2);
extern int rn_one_or_more(int p);
extern int rn_group(int p1,int p2);
extern int rn_choice(int p1,int p2);
extern int rn_ileave(int p1,int p2);
extern int rn_after(int p1,int p2);

extern int newAnyName(void);
extern int newAnyNameExcept(int nc);
extern int newQName(int uri,int name);
extern int newNsName(int uri);
extern int newNameClassExcept(int nc1,int nc2);
extern int newNameClassChoice(int nc1,int nc2);
extern int newDatatype(int lib,int typ);

extern int rn_i_ps(void);
extern void rn_add_pskey(char *s);
extern void rn_add_psval(char *s);
extern void rn_end_ps(void);

extern void rn_init(void);
extern void rn_clear(void);

extern void rn_compress(int *starts,int n);
extern int rn_compress_last(int start);

#endif
