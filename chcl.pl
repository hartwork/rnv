$u=0;
while(<>) {
  chop;
  /"\\u(.*)" - "\\u(.*)",/ and do  {
    print <<_END_;
if(0x$u<u&&u<0x$1) return false;
if(0x$1<=u&&u<=0x$2) return true;
_END_
    $u=$2;
  };
  /^[ \t]*"\\u([^-]*)",$/ and do {

    print <<_END_;
if(0x$u<u&&u<0x$1) return false;
if(u==0x$1) return true;
_END_
  $u=$1;
  };
}
print "return false;\n"
    
