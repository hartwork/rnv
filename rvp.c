/* $Id$ */

/* validation pipe:
 synopsis

   rvp -qvh {grammar="grammar.rnc"}

 reads from 0, writes to 1, 2 for grammar parse errors only, then redirected.
   -q switches to numerical error codes
   -v displays version
   -h help message
 exit code: 0 on valid, non-zero on invalid
   
 protocol
  request ::= (grammar | start-tag-open | attribute | start-tag-close | text | end-tag) z.
   grammar ::= "grammar" name. 
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

#include "memops.h"
#include "strops.h"
#include "xmlc.h"
#include "ht.h"
#include "rnc.h"
#include "rnd.h"
#include "rnv.h"

extern in rn_notAllowed;

static int explain=1;

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

}
 