
RNV -- Relax NG Compact Syntax Validator in C

Version 1.3 

   Table of Contents

   News since 1.2 
   News since 1.1 
   Package Contents 
   Installation 
   Invocation 
   Limitations 
   ARX 
   New versions 

   Abstract

   RNV is an implementation of Relax NG Compact Syntax,
   http://relaxng.org/compact-20021121.html. It is written in ANSI C,
   the command-line utility uses Expat,
   http://www.jclark.com/xml/expat.html. It is distributed under BSD
   license, see license.txt for details.

   RNV is a part of an on-going work, and the current code can have bugs
   and shortcomings; however, it validates documents against a number of
   grammars. I use it.

News since 1.2

   This release has many performance and convenience enhancements.
   Pattern pools are now one-dimensional arrays, which conserves space
   and gives better performance. Pass-through mode has been added to
   facilitate use of rnv in pipes. ARX, an utility to automatically
   associate documents and grammars, is included in the distribution;
   details are below. A simple plugin for vim, http://www.vim.org/,
   is provided to use RNV with the editor. The script uses ARX to
   automatically choose the grammar for a document.

News since 1.1

   I have implemented XML Schema datatypes checking. The support is not
   complete (details are in the list of limitations). On the bright side,
   the utility now includes a complete and sufficiently fast
   implementation of Unicode regular expressions (with ugly XML Schema
   syntax, but still useful).

Package Contents

   The package consists of:
     * the license, license.txt;
     * the source code, *.[ch];
     * the source code map, src.txt;
     * Makefile for unix-like systems;
     * Makefile.bcc for Win32 and Borland C/C++ Compiler;
     * tools/rnv.vim, a plugin for Vim;
     * tools/arx.conf, ARX configuration file;
     * tools/*.rnc, sample Relax NG grammars;
     * win32/rnv.exe and win32/arx.exe, Win32 executables statically
       linked with a current version of Expat from
       http://expat.sourceforge.net/;
     * the log of changes, changes.txt;
     * this file, readme.txt.

Installation

   On Unix-like systems, run make. If you are using Expat 1.2, define
   EXPAT_H as xmlparse.h (instead of expat.h).

   On Windows, use rnv.exe. To recompile from the sources, use
   Makefile.bcc with Borland C/C++ Compiler, or create a makefile or
   project for your environment.

Invocation

   The command-line syntax is

        rnv {-q|-p|-s|-v|-h} grammar.rnc {document1.xml}

   If no documents are specified, RNV attempts to read the XML document
   from the standard input. The options are:

   -q
          names of files being processed are not printed; in error
          messages, expected elements and attributes are not listed;

   -p
          xml input is copied to the output;

   -s
          rnv uses less memory and runs slower.

   -v
          rnv version is displayed

   -h or -?
          RNV displays usage summary and exits.

Limitations

     * RNV assumes that the encoding of the syntax file is UTF-8.
     * Support for XSL Schema Part 2: Datatypes is partial.
          + ordering for duration is not implemented;
          + only local parts of QName values are checked for equality,
            ENTITY values are only checked for lexical validity.
     * The schema parser does not check that all restrictions are obeyed,
       in particular, restrictions 7.3 and 7.4 are not checked.
     * RNV for Win32 platforms is a Unix program compiled on Win32. It
       expects file paths to be written with normal slashes; if a schema
       is in a different directory and includes or refers external files,
       then the schema's path must be written in the Unix way for the
       relative paths to work. For example, under Windows, rnv that uses
       ..\schema\docbook.rnc to validate userguide.dbx should be invoked
       as

      rnv.exe ../schema/docbook.rnc userguide.dbx

ARX

   ARX is a tool to automatically determine the type of a document from
   its name and contents. It is inspired by James Clark's schema location
   approach for nXML,
   http://groups.yahoo.com/group/emacs-nxml-mode/message/259, and is
   a development of the idea described in
   http://relaxng.org/pipermail/relaxng-user/2003-December/000214.htm
   l.

   ARX is a command-line utility. The invocation syntax is

        arx {-n|-v|-h} document.xml  arx.conf {arx.conf}

   ARX either prints a string corresponding to the document's type or
   nothing if the type cannot be determined. The options are:

   -n
          turns off prepending base path of the configuration file to the
          result, even if it looks like a relative path (useful when the
          configuration file and the grammars are in separate
          directories, or for association with something that is not a
          file);

   -v
          prints current version;

   -h or -?
          displays usage summary and exit.

   The configuration file must confrom to the following grammar:

      arx = grammars route*
      grammars = "grammars"  "{" type2string+ "}"
      type2string =  type "=" literal
      type = nmtoken
      route = match|nomatch|valid|invalid
      match = "=~" regexp "=>" type
      nomatch = "!~" regexp "=>" type
      valid = "valid" "{" rng "}" "=>" type
      invalid = "!valid" "{" rng "}" "=>" type

      literal=string in '"', '"' inside must be prepended by '\'
      regexp=string in '/', '/' inside must be prepended by '\'
      rng=Relax NG Compact Syntax

      Comments start with # and continue till the end of line.

   Rules are processed sequentially, the first matching rule determines
   the file's type. Relax NG templates are matched against file contents,
   regular expressions are applied to file names. The sample below
   associates documents with grammars for XSLT, DocBook or XSL FO.

      grammars {
        docbook="docbook.rnc"
        xslt="xslt.rnc"
        xslfo="fo.rnc"
      }

      valid {
        start = element (book|article|chapter|reference) {any}
        any = (element * {any}|attribute * {text}|text)*
      } => docbook

      !valid {
        default namespace xsl = "http://www.w3.org/1999/XSL/Transform"
        start = element *-xsl:* {not-xsl}
        not-xsl = (element *-xsl:* {not-xsl}|attribute * {text}|text)*
      } => xslt

      =~/.*\.xsl/ => xslt
      =~/.*\.fo/ => xslfo

   ARX can also be used to link documents to any type of information or
   processing.

New versions

   Visit http://davidashen.net/ for news and downloads.

