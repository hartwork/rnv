#!/usr/bin/perl
# $Id$

use strict;

if(@ARGV!=1) {
  print STDERR "usage: uri2rnc <file.xml>\n";
  exit(1);
}

my $TMPDIR="/tmp";
my $RNGDIR="/usr/local/share/rng-c";
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

my $XML=<<END;
start = element * {any}
$ANY
END

my $DOCBOOK=<<END;
start = element (set|setindex|book|part|reference|preface|chapter|appendix|article|bibliography|glossary|index|refentry|sect1|sect2|sect3|sect4|sect5|section) { any }
$ANY
END

my $NOTXSLT=<<END;
default namespace xsl = "http://www.w3.org/1999/XSL/Transform"
start = element *-xsl:* {not-xsl}
not-xsl = (element *-xsl:* {not-xsl}|attribute * {text}|text)*
END

my $RELAXNG=<<END;
default namespace rng = "http://relaxng.org/ns/structure/1.0"
start = element rng:* {any}
$ANY
END

my $type;
for($ARGV[0]) {
  $type=(0
  or valid($_,$XML) 
    and (0
      or valid($_,$DOCBOOK) and "docbook"
      or!valid($_,$NOTXSLT) and "xslt"
      or valid($_,$RELAXNG) and "relaxng")
  or /\.x?ht(ml?)?$/	and "xhtml"
  or /\.xsl$/		and "xslt"
  or /\.dbx$/ 		and "docbook"
  or /\.fo$/ 		and "xslfo"
  or /(.*)\.*$/ and do {my $f=$1.".rnc"; -f $f ? $f : 0;}
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
