/* $Id$ */

/* validation pipe:
 synopsis

   rvp -qvh grammar.rnc

 reads from 0, writes to 1, 2 for grammar parse errors only, then redirected.
   -q switches to numerical error codes
   -v displays version
   -h help message
 exit code: 0 on valid, non-zero on invalid
   
 protocol
  request ::= (start | start-tag-open | attribute | start-tag-close | text | end-tag) z.
   start ::= "start". 
   start-tag-open ::= "start-tag-open" patno name.
   attribute ::= "attribute" patno name value.
   start-tag-close :: = "start-tag-close" patno name.
   text ::= "text" patno text.
   end-tag ::= "end-tag" patno name.
  response ::= (ok | er | error) z.
   ok ::= "ok" patno. 
   er ::= "er" patno erno.
   error ::= "error" patno erno error.
  z ::= "\0" .
 
  conventions:
    last colon in name separates namespace uri and local part
    -q?er:error    
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

extern int rn_notAllowed;

static int explain=1, start, lasterr;

static void verror_handler(int erno,va_list ap) {
  lasterr=erno;
}

static void verror_handler_rnv(int erno,va_list ap) {verror_handler(erno|ERBIT_RNV,ap);}

static int initialized=0;
static void init() {
  if(!initialized) {initialized=1;
    rnl_init();
    rnv_init(); rnv_verror_handler=&verror_handler_rnv;
  }
}

#define START 0
#define QUIT 1                                                                      
#define STO 2
#define ATT 3
#define STC 4 
#define ENT 5

static int erp[2]; /* *erp to read error messages */
static FILE *nstderr=stderr;
static jmp_buf IOER;

#define OK "ok %u"
#define ER "er %u"
#define ERROR "error %u %s"

#define LEN_B 1024

static void resp(int ok,int patno) {
  int max,len,ofs;
  static char buf[LEN_B];
  char *f=ok?OK:explain?ERROR:ER;
  len=sprintf(buf,f,patno); assert(len<=LEN_B);
  write(1,buf,len);
  if(!ok&&explain) {
    buf[0]=' '; write(1,buf,1);
    for(;;) {
      len=read(0,buf,LEN_B);
      if(len<0) longjmp(IOER,1);
      if(len==0) goto Z;
      do {
        int n=write(1,buf,len);
        if(n==-1) longjmp(IOER,1);
        ofs+=n; len-=n;
      } while(len);
    }
  }
  Z: buf[0]='\0'; write(1,buf,1);
}

int main(int argc,char **argv) {
  while(*(++argv)&&**argv=='-') {
    int i=1;
    for(;;) {
      switch(*(*argv+i)) {
      case '\0': goto END_OF_OPTIONS;
      case 'q': numerr=1; break;
      case 'h': case '?': usage(); return 0;
      case 'v': version(); break;
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
  }
  return EXIT_FAILURE;
}
