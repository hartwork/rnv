#!/usr/local/bin/perl
# $Id$

use FileHandle; use IPC::Open2;
use XML::Parser::Expat;

use strict;

my $RVP="rvp";
my $parser=new XML::Parser::Expat(Namespaces=>1);

# rvp wrapper
$|=1;  # using pipes
$/="\0"; # zero byte is separator

# write to queries RVPIN, get responses from RVPOUT
open2(\*RVPOUT,\*RVPIN,$RVP,shift @ARGV);

sub resp {
  $_=<RVPOUT>;
  chop;
  /^ok (\d+).*/ and return $1;
  /^error (\d+) (\d+) (.*)/ and do {
      my ($pat,$msg)=($1,$3); 

     # if the message is empty, don't print the message, 
     # the error has occured in already erroneous state 
      $msg and printf STDERR "%u,%u: %s\n",
        $parser->current_line(),
        $parser->current_column(),
        $msg;
      return $pat;
    };
  die "protocol error, stopped ";
}

sub start_tag_open {
  my($cur,$name)=@_;
  print RVPIN "start-tag-open $cur $name\0";
  return resp();
}

sub attribute {
  my($cur,$name,$val)=@_;
  print RVPIN "attribute $cur $name $val\0";
  return resp();
}

sub start_tag_close {
  my($cur,$name)=@_;
  print RVPIN "start-tag-close $cur $name\0";
  return resp();
}

sub end_tag {
  my($cur,$name)=@_;
  print RVPIN "end-tag $cur $name\0";
  return resp();
}

sub text {
  my($cur,$text)=@_;
  print RVPIN "text $cur $text\0";
  return resp();
}

sub mixed {
  my($cur,$text)=@_;
  print RVPIN "mixed $cur $text\0";
  return resp();
}

sub start {
  print RVPIN "start\0";
  return resp();
}

sub quit {
  print RVPIN "quit\0";
  return resp();
}

my ($current,$text,$mixed)=(0,"",0);

# expat handlers
sub flush_text {
  $current=$mixed?mixed($current,$text):text($current,$text);
  $text="";
}

sub qname {
   my ($p,$name)=@_;
   return join(':',$p->namespace($name),$name);
}
   
sub start_element {
  my ($p,$el,%attrs)=@_;

  $mixed=1;
  flush_text();
  $current=start_tag_open($current,qname($p,$el));
  $mixed=0;
  for my $atname (keys %attrs) {
    $current=attribute($current,qname($p,$atname),$attrs{$atname});
  }
  $current=start_tag_close($current,qname($p,$el));
}

sub end_element {
  my ($p,$el)=@_;
  flush_text();
  $current=end_tag($current,qname($p,$el));
  $mixed=1;
}

sub characters {
  my ($p,$chars)=@_;
  $text=$text.$chars;
}

$parser->setHandlers(
  Start=>\&start_element,
  End=>\&end_element,
  Char=>\&characters);

$current=start();

$parser->parse(*STDIN);

quit();
