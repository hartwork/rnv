
RNV -- Relax NG Compact Syntax Validator in C

Version 1.2 

   Table of Contents

   News since 1.1 
   Package Contents 
   Installation 
   Invocation 
   Limitations 
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
     * compile.bat to compile with Borland C/C++ Compiler on Windows;
     * rnv.exe, a Win32 executable statically linked with a current
       version of Expat from http://expat.sourceforge.net/;
     * the log of changes, changes.txt;
     * this file, readme.txt.

Installation

   On Unix-like systems, run make. If you are using Expat 1.2, define
   EXPAT_H as xmlparse.h (instead of expat.h).

   On Windows, use rnv.exe. To recompile from the sources, either use
   compile.bat with Borland C/C++ Compiler or modify the Makefile for
   your environment.

Invocation

   The command-line syntax is

        rnv {-q|-s|-v|-h} grammar.rnc {document1.xml}

   If no documents are specified, RNV attempts to read the XML document
   from the standard input. The options are:

   -q
          in error messages, expected elements and attributes are not
          listed.

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
          + value equality for dateTime and derived types is not
            implemented (any two dates are equal);
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
       as:
       rnv.exe ../schema/docbook.rnc userguide.dbx

New versions

   Visit http://davidashen.net/ for news and downloads.

