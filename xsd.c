/* $Id$ */

#include <stdlib.h> /*calloc,free*/
#include <string.h> /*strncmp*/
#include <stdio.h> /*stdio*/
#include <assert.h>
#include "u.h"
#include "xmlc.h"
#include "strops.h"
#include "rx.h"
#include "xsd.h"

static void default_error_handler(char *msg) {fprintf(stderr,"msg\n");}
void (*xsd_error_handler)(char *msg)=&default_error_handler;

static int initialized=0;
void xsd_init(void) {
  if(!initialized) { initialized=1;
    rx_init();
  }
}

#define FCT_ENUMERATION 0
#define FCT_FRACTION_DIGITS 1
#define FCT_LENGTH 2
#define FCT_MAX_EXCLUSIVE 3
#define FCT_MAX_INCLUSIVE 4
#define FCT_MAX_LENGTH 5
#define FCT_MIN_EXCLUSIVE 6
#define FCT_MIN_INCLUSIVE 7
#define FCT_MIN_LENGTH 8
#define FCT_PATTERN 9
#define FCT_TOTAL_DIGITS 10
#define FCT_WHITE_SPACE 11
#define NFCT 12
static char *fcttab[NFCT]={
  "enumeration", "fractionDigits", "length", "maxExclusive", "maxInclusive", "maxLength",
  "minExclusive", "minInclusive", "minLength", "pattern", "totalDigits", "whiteSpace"};

#define FCT_SET(x) (fct.set&(1<<FCT_##x))

#define WS_PRESERVE 0
#define WS_REPLACE 1
#define WS_COLLAPSE 2

static int (*match[])(char *r,char *s,int n)={&rx_match,&rx_rmatch,&rx_cmatch};

#define TYP_ENTITIES 0
#define TYP_ENTITY 1
#define TYP_ID 2
#define TYP_IDREF 3
#define TYP_IDREFS 4
#define TYP_NCNAME 5
#define TYP_NMTOKEN 6
#define TYP_NMTOKENS 7
#define TYP_NOTATION 8
#define TYP_NAME 9
#define TYP_QNAME 10
#define TYP_ANY_URI 11
#define TYP_BASE64BINARY 12
#define TYP_BOOLEAN 13
#define TYP_BYTE 14
#define TYP_DATE 15
#define TYP_DATE_TIME 16
#define TYP_DECIMAL 17
#define TYP_DOUBLE 18
#define TYP_DURATION 19
#define TYP_FLOAT 20
#define TYP_G_DAY 21
#define TYP_G_MONTH 22
#define TYP_G_MONTH_DAY 23
#define TYP_G_YEAR 24
#define TYP_G_YEAR_MONTH 25
#define TYP_HEX_BINARY 26
#define TYP_INT 27
#define TYP_INTEGER 28
#define TYP_LANGUAGE 29
#define TYP_LONG 30
#define TYP_NEGATIVE_INTEGER 31
#define TYP_NON_NEGATIVE_INTEGER 32
#define TYP_NON_POSITIVE_INTEGER 33
#define TYP_NORMALIZED_STRING 34
#define TYP_POSITIVE_INTEGER 35
#define TYP_SHORT 36
#define TYP_STRING 37
#define TYP_TIME 38
#define TYP_TOKEN 39
#define TYP_UNSIGNED_BYTE 40
#define TYP_UNSIGNED_INT 41
#define TYP_UNSIGNED_LONG 42
#define TYP_UNSIGNED_SHORT 43
#define NTYP 44
static char *typtab[NTYP]={
"ENTITIES", "ENTITY", "ID", "IDREF", "IDREFS", "NCName", "NMTOKEN", "NMTOKENS",
"NOTATION", "Name", "QName", "anyURI", "base64Binary", "boolean", "byte", "date",
"dateTime", "decimal", "double", "duration", "float", "gDay", "gMonth",
"gMonthDay", "gYear", "gYearMonth", "hexBinary", "int", "integer", "language",
"long", "negativeInteger", "nonNegativeInteger", "nonPositiveInteger",
"normalizedString", "positiveInteger", "short", "string", "time", "token",
"unsignedByte", "unsignedInt", "unsignedLong", "unsignedShort"};

#define ERR_INVALID_PARAMETER "invalid XML Schema datatype parameter '%s'"
#define ERR_INVALID_DATATYPE "invalid XML Schema datatype name '%s'"

static int toknlen(char *s,int n) {
  char *end=s+n;
  int u,len=-2;
  SKIP_SPACE: ++len;
  for(;;) {
    if(s==end) return len;
    s+=u_get(&u,s); 
    if(!xmlc_white_space(u)) break; 
  }
  for(;;) {
    ++len;
    if(xmlc_white_space(u)) goto SKIP_SPACE;
    if(s==end) goto SKIP_SPACE;
    s+=u_get(&u,s); 
  }
}

static int tokncnt(char *s,int n) {
  char *end=s+n;
  int u,cnt=0;
  SKIP_SPACE:
  for(;;) {
    if(s==end) return cnt;
    s+=u_get(&u,s); 
    if(!xmlc_white_space(u)) break; 
  }
  ++cnt;
  for(;;) {
    if(xmlc_white_space(u)) goto SKIP_SPACE;
    if(s==end) goto SKIP_SPACE;
    s+=u_get(&u,s); 
  }
  return cnt;
}


#define NPAT 16

/* isn't it nice to have an implementation of unicode regular expressions */
#define PAT_ORDINAL "[0-9]+"
#define PAT_POSITIVE "\\+?"PAT_ORDINAL
#define PAT_NON_NEGATIVE "\\+?"PAT_ORDINAL
#define PAT_NON_POSITIVE "\\-"PAT_ORDINAL"|0+"
#define PAT_NEGATIVE "\\-"PAT_ORDINAL
#define PAT_INTEGER "[+-]?"PAT_ORDINAL
#define PAT_DECIMAL PAT_INTEGER"(\\.[0-9]+)?"
#define PAT_FLOATING PAT_DECIMAL"([Ee]"PAT_INTEGER")?|INF|-INF|NaN"
#define PAT_ANY_URI "(([a-zA-Z][0-9a-zA-Z+\\-\\.]*:)?/{0,2}[0-9a-zA-Z;/?:@&=+$\\.\\-_!~*'()%]+)?(#[0-9a-zA-Z;/?:@&=+$\\.\\-_!~*'()%]+)?"
#define PAT_NCNAME "[\\i-[:]][\\c-[:]]*"
#define PAT_QNAME "("PAT_NCNAME":)?"PAT_NCNAME
#define PAT_NMTOKEN "\\c+"
#define PAT_NAME "\\i\\c*"
#define PAT_NCNAMES PAT_NCNAME"( "PAT_NCNAME")*"
#define PAT_NMTOKENS PAT_NMTOKEN"( "PAT_NMTOKEN")*"
#define PAT_NAMES PAT_NAME"( "PAT_NAME")*"
#define PAT_LANGUAGE "([a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*"

int xsd_allows(char *typ,char *ps,char *s,int n) {
  int ok=1,length;
  struct {
    int set;
    char *pattern[NPAT+1]; int npat;
    int length, minLength, maxLength, totalDigits, fractionDigits;
    char *maxExclusive, *maxInclusive, *minExclusive, *minInclusive;
    int whiteSpace;
  } fct;
  fct.set=0; fct.npat=0;
  { int n;
    while((n=strlen(ps))) {
      char *key=ps,*val=key+n+1,*end,i; 
      switch(i=strtab(key,fcttab,NFCT)) {
      case FCT_LENGTH: fct.length=(int)strtol(val,&end,10); if(!*val||*end) (*xsd_error_handler)("invalid value of 'Length'"); break;
      case FCT_MAX_LENGTH: fct.maxLength=(int)strtol(val,&end,10); if(!*val||*end) (*xsd_error_handler)("invalid value of 'maxLength'"); break;
      case FCT_MIN_LENGTH: fct.minLength=(int)strtol(val,&end,10); if(!*val||*end) (*xsd_error_handler)("invalid value of 'minLength'"); break;
      case FCT_FRACTION_DIGITS: fct.fractionDigits=(int)strtol(val,&end,10); if(!*val||*end) (*xsd_error_handler)("invalid value of 'fractionDigits'"); break;
      case FCT_TOTAL_DIGITS: fct.totalDigits=(int)strtol(val,&end,10); if(!*val||*end) (*xsd_error_handler)("invalid value of 'totalDigits'"); break;
      case FCT_PATTERN: 
	if(fct.npat==NPAT) (*xsd_error_handler)("no more than 16 patterns per datatype are supported"); else {
	  fct.pattern[fct.npat++]=val;
	} break;
      case FCT_MAX_EXCLUSIVE: fct.maxExclusive=val; break;
      case FCT_MAX_INCLUSIVE: fct.maxInclusive=val; break;
      case FCT_MIN_EXCLUSIVE: fct.minExclusive=val; break;
      case FCT_MIN_INCLUSIVE: fct.minInclusive=val; break;
      case FCT_WHITE_SPACE: (*xsd_error_handler)("'the builtin derived datatype that specifies the desired value for the whiteSpace facet should be used instead of 'whiteSpace'"); break;
      case FCT_ENUMERATION: (*xsd_error_handler)("'value' should be used instead of 'enumeration'"); break;
      case NFCT: 
	{ char *buf=(char*)calloc(strlen(ERR_INVALID_PARAMETER)+strlen(val)+1,sizeof(char));
	  sprintf(buf,ERR_INVALID_PARAMETER,val);
	  (*xsd_error_handler)(buf);
	  free(buf);
	} break;
      default: assert(0);
      }
      fct.set|=1<<i;
      ps=val+strlen(val)+1;
    }
  }

  fct.whiteSpace=WS_COLLAPSE; 
  length=-1;
  switch(strtab(typ,typtab,NTYP)) {
 /*primitive*/
  case TYP_STRING: fct.whiteSpace=WS_PRESERVE;
    length=u_strnlen(s,n); 
    break;
  case TYP_BOOLEAN: length=-1;
    fct.pattern[fct.npat++]="true|false|1|0";
    break;
  case TYP_DECIMAL: 
    fct.pattern[fct.npat++]=PAT_DECIMAL;
    break;
  case TYP_FLOAT: 
    fct.pattern[fct.npat++]=PAT_FLOATING;
    break;
  case TYP_DOUBLE:
    fct.pattern[fct.npat++]=PAT_FLOATING;
    break;
  case TYP_DURATION: break;
  case TYP_DATE_TIME: break;
  case TYP_DATE: break;
  case TYP_TIME: break;
  case TYP_G_YEAR_MONTH: break;
  case TYP_G_YEAR: break;
  case TYP_G_MONTH_DAY: break;
  case TYP_G_DAY: break;
  case TYP_G_MONTH: break;
  case TYP_HEX_BINARY: break;
  case TYP_BASE64BINARY: break;
  case TYP_ANY_URI: 
    fct.pattern[fct.npat++]=PAT_ANY_URI;
    length=toknlen(s,n);
    break;
  case TYP_QNAME:
    fct.pattern[fct.npat++]=PAT_QNAME;
    length=toknlen(s,n);
    break;
  case TYP_NOTATION:
    fct.pattern[fct.npat++]=PAT_QNAME;
    length=toknlen(s,n);
    break;
 /*derived*/
  case TYP_NORMALIZED_STRING: fct.whiteSpace=WS_REPLACE; 
    length=u_strnlen(s,n); 
    break;
  case TYP_TOKEN: 
    length=toknlen(s,n);
    break;
  case TYP_LANGUAGE: 
    fct.pattern[fct.npat++]=PAT_LANGUAGE;
    length=toknlen(s,n);
    break;
  case TYP_NMTOKEN: 
    fct.pattern[fct.npat++]=PAT_NMTOKEN;
    length=toknlen(s,n);
    break;
  case TYP_NMTOKENS: 
    fct.pattern[fct.npat++]=PAT_NMTOKENS;
    length=tokncnt(s,n);
    break;
  case TYP_NAME: 
    fct.pattern[fct.npat++]=PAT_NAME;
    length=toknlen(s,n);
    break;
  case TYP_NCNAME: 
    fct.pattern[fct.npat++]=PAT_NCNAME;
    length=toknlen(s,n);
    break;
  case TYP_ID: 
    fct.pattern[fct.npat++]=PAT_NCNAME;
    length=toknlen(s,n);
    break;
  case TYP_IDREF: 
    fct.pattern[fct.npat++]=PAT_NCNAME;
    length=toknlen(s,n);
    break;
  case TYP_IDREFS: 
    fct.pattern[fct.npat++]=PAT_NCNAMES;
    length=tokncnt(s,n);
    break;
  case TYP_ENTITY: 
    fct.pattern[fct.npat++]=PAT_NCNAME;
    length=toknlen(s,n);
    break;
  case TYP_ENTITIES: 
    fct.pattern[fct.npat++]=PAT_NCNAMES;
    length=tokncnt(s,n);
    break;
  case TYP_INTEGER:
    fct.pattern[fct.npat++]=PAT_INTEGER;
    break;
  case TYP_POSITIVE_INTEGER: 
    fct.pattern[fct.npat++]=PAT_POSITIVE;
    break;
  case TYP_NON_NEGATIVE_INTEGER: 
    fct.pattern[fct.npat++]=PAT_NON_NEGATIVE;
    break;
  case TYP_NON_POSITIVE_INTEGER: 
    fct.pattern[fct.npat++]=PAT_NON_POSITIVE;
    break;
  case TYP_NEGATIVE_INTEGER: 
    fct.pattern[fct.npat++]=PAT_NEGATIVE;
    break;
  case TYP_BYTE: 
    break;
  case TYP_UNSIGNED_BYTE: 
    break;
  case TYP_SHORT: 
    break;
  case TYP_UNSIGNED_SHORT: 
    break;
  case TYP_INT: 
    break;
  case TYP_UNSIGNED_INT: 
    break;
  case TYP_LONG: 
    break;
  case TYP_UNSIGNED_LONG: 
    break;
  case NTYP:
    { char *buf=(char*)calloc(strlen(ERR_INVALID_DATATYPE)+strlen(typ)+1,sizeof(char));
      sprintf(buf,ERR_INVALID_DATATYPE,typ);
      (*xsd_error_handler)(buf);
      free(buf);
    } break;
  default: assert(0);
  }

  while(fct.npat--) ok=ok&&match[fct.whiteSpace](fct.pattern[fct.npat],s,n);

  if(FCT_SET(LENGTH)) ok=ok&&length==fct.length;
  if(FCT_SET(MAX_LENGTH)) ok=ok&&length<=fct.maxLength;
  if(FCT_SET(MIN_LENGTH)) ok=ok&&length>=fct.minLength;

  return ok;
}

static int nrmncmp(char *s1,char *s2,int n) {
  char *end=s2+n;
  for(;;) {
    if(!*s1) return (s2==end);
    if(s2==end) return 0;
    if(*s1!=*s2&&(!xmlc_white_space(*s1)||!xmlc_white_space(*s2))) return 0;
    ++s1; ++s2;
  }
}

int xsd_equal(char *typ,char *val,char *s,int n) {
  switch(strtab(typ,typtab,NTYP)) {
 /*primitive*/
  case TYP_STRING: return strncmp(val,s,n)==0;
  case TYP_BOOLEAN: return (tokncmp("true",val,strlen(val))||tokncmp("1",val,strlen(val)))==(tokncmp("true",s,n)||tokncmp("1",s,n));
  case TYP_DECIMAL:
  case TYP_FLOAT:
  case TYP_DOUBLE: return atof(val)==atof(s);
  case TYP_DURATION:
  case TYP_DATE_TIME:
  case TYP_DATE:
  case TYP_TIME:
  case TYP_G_YEAR_MONTH:
  case TYP_G_YEAR:
  case TYP_G_MONTH_DAY:
  case TYP_G_DAY:
  case TYP_G_MONTH:
    return 1;
  case TYP_HEX_BINARY:
  case TYP_BASE64BINARY:
    return 1;
  case TYP_ANY_URI: return tokncmp(val,s,n);
  case TYP_QNAME: case TYP_NOTATION: /* context is not passed over; compare local parts */  
    { char *ln=strchr(val,':'),m=val-ln+1; 
      return ln?m<n?tokncmp(ln+1,s,n-m):0:tokncmp(val,s,n);
    }
 /*derived*/
  case TYP_NORMALIZED_STRING: return nrmncmp(val,s,n);
  case TYP_TOKEN:
  case TYP_LANGUAGE:
  case TYP_NMTOKEN:
  case TYP_NMTOKENS:
  case TYP_NAME:
  case TYP_NCNAME:
  case TYP_ID:
  case TYP_IDREF:
  case TYP_IDREFS: 
  case TYP_ENTITY:
  case TYP_ENTITIES: return tokncmp(val,s,n);
  case TYP_INTEGER:
  case TYP_POSITIVE_INTEGER:
  case TYP_NON_NEGATIVE_INTEGER:
  case TYP_NON_POSITIVE_INTEGER:
  case TYP_NEGATIVE_INTEGER:
  case TYP_BYTE:
  case TYP_UNSIGNED_BYTE:
  case TYP_SHORT:
  case TYP_UNSIGNED_SHORT:
  case TYP_INT:
  case TYP_UNSIGNED_INT:
  case TYP_LONG:
  case TYP_UNSIGNED_LONG: return atol(val)==atol(s);
  case NTYP:
    { char *buf=(char*)calloc(strlen(ERR_INVALID_DATATYPE)+strlen(typ)+1,sizeof(char));
      sprintf(buf,ERR_INVALID_DATATYPE,typ);
      (*xsd_error_handler)(buf);
      free(buf);
    } return 0;
  }
  assert(0);
  return 0;
}
