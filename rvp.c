/* $Id$ */

/* validation pipe:
 synopsis

   rvp -qsvh grammar.rnc

 reads from 0, writes to 1, 2 for grammar parse errors only, then redirected.
   -q switches to numerical error codes
   -v displays version
   -s takes less space but more time
   -h help message
 exit code: 0 on valid, non-zero on invalid
   
 protocol
  query ::= (start | start-tag-open | attribute | start-tag-close | text | end-tag) z.
   start ::= "start". 
   start-tag-open ::= "start-tag-open" patno name.
   attribute ::= "attribute" patno name value.
   start-tag-close :: = "start-tag-close" patno name.
   text ::= ("text"|"mixed") patno text.
   end-tag ::= "end-tag" patno name.
  response ::= (ok | er | error) z.
   ok ::= "ok" patno. 
   er ::= "er" patno erno.
   error ::= "error" patno erno error.
  z ::= "\0" .
 
  conventions:
    last colon in name separates namespace uri and local part
    -q?er:error
    error==0 yields message 'protocol error' and happens when a query is not understood
*/

#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>  /*open,close*/
#include UNISTD_H   /*open,read,close*/
#include <stdio.h>  /*fprintf,stderr*/
#include <string.h> /*strerror*/
#include <setjmp.h>
#include <errno.h>
#include <assert.h>
##include "memops.h"
#include "strops.h"
#include "erbit.h"
#include "rnl.h"
#include "rnv.h"

extern int rn_notAllowed, drv_compact, rx_compact;

#define ATT 0
#define ENT 1
#define MIX 2
#define QUIT 3                                                                
#define START 4
#define STC 5 
#define STO 6
#define TXT 7
#define NKWD 8
char *kwdtab[NKWD]={ 
  "attribute",
  "end-tag"
  "mixed",
  "quit",
  "start",
  "start-tag-close",
  "start-tag-open",
  "text"
};

#define OK "ok %u"
#define ER "er %u"
#define ERROR "error %u %s"

#define PROTER 0
#define PROTOCOL_ERROR "protocol error\n"

#define LEN_B 1024

static int explain=1, start, lasterr;
static int len_q,n_q; char *quebuf;
static int erp[2]; /* *erp to read error messages */
static FILE *nstderr=stderr;
static jmp_buf IOER;

static void verror_handler(int erno,va_list ap) {lasterr=erno;}
static void verror_handler_rnv(int erno,va_list ap) {verror_handler(erno|ERBIT_RNV,ap);}

static int initialized=0;
static void init() {
  if(!initialized) {initialized=1;
    rnl_init();
    rnv_init(); rnv_verror_handler=&verror_handler_rnv;
    quebuf=(char*)memalloc(len_q=LEN_B,sizeof(char));
  }
}

static void clear() {}

#define SPACE "\t\n\r "

int tok(int i) {while(strchr(SPACE,quebuf[i])) ++i; return i;}
int endtok(int i) {while(quebuf[i]&&!strchr(SPACE,quebuf[i])) ++i; return i;}

static void resp(int ok,int patno) {
  int max,len,ofs;
  static char buf[LEN_B];
  char *f=ok?OK:explain?ERROR:ER;
  len=sprintf(buf,f,patno); assert(len<LEN_B);
  write(1,buf,len);
  if(!ok&&explain) {
    buf[0]=' '; write(1,buf,1);
  }
  for(;;) { /* read always, write if verbose */
    len=read(0,buf,LEN_B);
    if(len<0) longjmp(IOER,1);
    if(len==0) break;
    if(!ok&&explain) {
      ofs=0;
      do {
	int n=write(1,buf+ofs,len);
	if(n==-1) longjmp(IOER,1);
	ofs+=n; len-=n;
      } while(len);
    }
  }
  buf[0]='\0'; write(1,buf,1);
}

static int query(void) {
  int i,j,n,dn, kwd, patno,prevno, ok;
  n=0;
  for(;;) {
    if(n==n_q) {
      if(len_q-n_q<LEN_B) quebuf=(char*)memstretch(quebuf,len_q=n_q+LEN_B,n_q,sizeof(char));
      dn=read(0,quebuf+n_q,LEN_B);
      if(dn==-1) longjmp(IOER,1);
      n_q+=dn;
    }
    if(quebuf[n++]=='\0') break;
  }

  j=endtok(i=tok(0));
  switch((kwd=strntab(quebuf+i,j-i,kwdtab,NKWD))) {
  case START: resp(1,start); break;
  case QUIT: resp(1,0); return 0;
  case STO: case ATT: case STC: case TXT: case MIX: case ENT:
    j=endtok(i=tok(j)); if(i==j) goto PROTER;
    patno=0; do patno=patno*10+quebuf[i++]-'0'; while(i!=j);
    switch(kwd) {
    case STO: case ATT: case STC: case ENT:
      j=endtok(i=tok(j)); if(i==j||kwd==ATT&&quebuf[j]='\0') goto PROTER;
      
      break;
    case TXT: case MIX:
      if(quebuf[j]) ++j; i=j; while(quebuf[j]) ++j;
      ok=rnv_text(&patno,&prevno,quebuf+i,j-i,kwd==MIX);
      resp(ok,patno);
      break;
    }
  case NKWD: PROTER: fprintf(stderr,PROTOCOL_ERROR); resp(0,PROTER); break;
  default: assert(0);
  }
  

  i=0; while(n!=n_q) quebuf[i++]=quebuf[n++]; n_q=i;
  return 1;
}

int main(int argc,char **argv) {
  while(*(++argv)&&**argv=='-') {
    int i=1;
    for(;;) {
      switch(*(*argv+i)) {
      case '\0': goto END_OF_OPTIONS;
      case 'h': case '?': usage(); return 0;
      case 'v': version(); break;
      case 's': drv_compact=1; rx_compact=1; break;
      case 'q': numerr=1; break;
      default: fprintf(stderr,"unknown option '-%c'\n",*(*argv+i)); break;
      }
      ++i;
    }
    END_OF_OPTIONS:;
  }

  if(*argv==NULL || *(argv+1)!=NULL) {usage(); return 1;}

  if(setjmp(IOER)) {
    fprintf(nstderr,"I/O error: %s\n",strerror(erno));
    return EXIT_FAILURE;
  }

  if(start=rnl_fn(*(argv)) {
    int fd2;
    if((fd2=dup(2))==-1) longjmp(IOER,1);
    nstderr=fdopen(fd2,"w");
    if(pipe(erp)==-1||dup2(erp[1],2)==-1) longjmp(IOER,1);
    while(query());
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
