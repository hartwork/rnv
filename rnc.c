/* $Id$ */

/* taken from the specification:

topLevel ::= decl* (pattern | grammarContent*)
decl ::= "namespace" identifierOrKeyword "=" namespaceURILiteral
| "default" "namespace" [identifierOrKeyword] "=" namespaceURILiteral
| "datatypes" identifierOrKeyword "=" literal
pattern ::= "element" nameClass "{" pattern "}"
| "attribute" nameClass "{" pattern "}"
| pattern ("," pattern)+
| pattern ("&" pattern)+
| pattern ("|" pattern)+
| pattern "?"
| pattern "*"
| pattern "+"
| "list" "{" pattern "}"
| "mixed" "{" pattern "}"
| identifier
| "parent" identifier
| "empty"
| "text"
| [datatypeName] datatypeValue
| datatypeName ["{" param* "}"] [exceptPattern]
| "notAllowed"
| "external" anyURILiteral [inherit]
| "grammar" "{" grammarContent* "}"
| "(" pattern ")"
param ::= identifierOrKeyword "=" literal
exceptPattern ::= "-" pattern
grammarContent ::= start
| define
| "div" "{" grammarContent* "}"
| "include" anyURILiteral [inherit] ["{" includeContent* "}"]
includeContent ::= define
| start
| "div" "{" includeContent* "}"
start ::= "start" assignMethod pattern
define ::= identifier assignMethod pattern
assignMethod ::= "="
| "|="
| "&="
nameClass ::= name
| nsName [exceptNameClass]
| anyName [exceptNameClass]
| nameClass "|" nameClass
| "(" nameClass ")"
name ::= identifierOrKeyword
| CName
exceptNameClass ::= "-" nameClass
datatypeName ::= CName
| "string"
| "token"
datatypeValue ::= literal
anyURILiteral ::= literal
namespaceURILiteral ::= literal
| "inherit"
inherit ::= "inherit" "=" identifierOrKeyword
identifierOrKeyword ::= identifier
| keyword
identifier ::= (NCName - keyword)
| quotedIdentifier
quotedIdentifier ::= "\" NCName
CName ::= NCName ":" NCName
nsName ::= NCName ":*"
anyName ::= "*"
literal ::= literalSegment ("~" literalSegment)+
literalSegment ::= '"' (Char - ('"' | newline))* '"'
| "'" (Char - ("'" | newline))* "'"
| '"""' (['"'] ['"'] (Char - '"'))* '"""'
| "'''" (["'"] ["'"] (Char - "'"))* "'''"
keyword ::= "attribute"
| "default"
| "datatypes"
| "div"
| "element"
| "empty"
| "external"
| "grammar"
| "include"
| "inherit"
| "list"
| "mixed"
| "namespace"
| "notAllowed"
| "parent"
| "start"
| "string"
| "text"
| "token"
*/

#define SYM_EOF 0
#define SYM_DEFAULT 1
#define SYM_DATATYPE 2
#define SYM_DIV 3
#define SYM_ELEMENT 4
#define SYM_EMPTY 5
#define SYM_EXTERNAL 6
#define SYM_GRAMMAR 7
#define SYM_INCLUDE 8
#define SYM_INHERIT 9
#define SYM_LIST 10
#define SYM_MIXED 11
#define SYM_NAMESPACE 12
#define SYM_NOT_ALLOWED 13
#define SYM_PARENT 14
#define SYM_START 15
#define SYM_STRING 16
#define SYM_TEXT 17
#define SYM_TOKEN 18
#define SYM_ASGN 19
#define SYM_ASGN_ILEAVE 20
#define SYM_ASGN_CHOICE 21
#define SYM_SEQ 22 /* , */
#define SYM_CHOICE 23 
#define SYM_ILEAVE 24
#define SYM_OPTIONAL 25
#define SYM_ZERO_OR_MORE 26
#define SYM_ONE_OR_MORE 27
#define SYM_LPAR 28
#define SYM_RPAR 29
#define SYM_LCUR 30
#define SYM_RCUR 31
#define SYM_LSQU 32
#define SYM_RSQU 33
#define SYM_EXCEPT 34
#define SYM_QNAME 35   /* : */
#define SYM_NS_NAME 36 /* :* */
#define SYM_ANY_NAME 37 /* * */
#define SYM_QUOTE 38  /* \ */
#define SYM_ANNOTATION 39 /* >> */
#define SYM_COMMENT 40 
#define SYM_DOCUMENTATION 41 /* >> */
#define SYM_IDENT 42
#define SYM_LITERAL 43

static void sym() {
  int cc;

}


/*
 * $Log$
 * Revision 1.1  2003/11/19 00:28:57  dvd
 * back to lists of ranges
 *
 */
