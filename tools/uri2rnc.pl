# $Id$

use strict;

my $TMPDIR="tmp";
my $RNGDIR="../../RNG-C";
my %SCHEMAS=(
  docbook=>"docbook.rnc",
  xslt=>"xslt.rnc",
  xhtml=>"xhtml.rnc",
  xslfo=>"fo.rnc",
  relaxng=>"relaxng.rnc"
);

my $ANY=<<END;
any = (element * {any}|attribute * {text}|text)*
END

my $DOCBOOK=<<END;
start = element (book|article|part|chapter|section) { any }
$ANY
END

my $XSLT=<<END;
default namespace xsl = "http://www.w3.org/1999/XSL/Transform"
start = element (stylesheet|transform) {any}
$ANY
END

my $RELAXNG=<<END;
default namespace rng = "http://relaxng.org/ns/structure/1.0"
start = element grammar {any}
$ANY
END

my $type;
for($ARGV[0]) {
  $type=( 0
  or valid($_,$DOCBOOK) and "docbook"
  or valid($_,$XSLT) 	and "xslt"
  or valid($_,$RELAXNG) and "relaxng"
  or /\.x?ht(ml?)?$/	and "xhtml"
  or /\.xsl$/		and "xslt"
  or /\.dbx$/ 		and "docbook"
  or /\.fo$/ 		and "xslfo"
  or /(.*)\.*$/ and do {my $f=$1.".rnc"; -f $f ? $f : 0; }
  or "unknown"
  );
}
print exists $SCHEMAS{$type}?$RNGDIR."/".$SCHEMAS{$type}:$type;

sub valid {
  my ($uri,$rnc)=@_;
  my $f=$TMPDIR."/".$$.".rnc";
  my $valid;

  open RNC,">$f"; print RNC $rnc; close RNC;
  system("rnv -q $f $uri 2>/dev/null");
  unlink $f; return !$?;
}
