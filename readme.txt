
RNV -- Relax NG Compact Syntax Validator in C

Version 1.0 

   Table of Contents

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

Package Contents

   The package consists of:
     * the license, license.txt;
     * the source code, *.[ch];
     * Makefile for unix-like systems;
     * compile.bat to compile with Borland C/C++ Compiler on Windows;
     * rnv.exe, a Win32 executable statically linked with a current
       version of Expat from http://expat.sourceforge.net/;
     * this file, readme.txt.

Installation

   On Unix and similar systems, execute make. If you use Expat 1.2,
   define EXPAT_H as xmlparse.h (instead of expat.h).

   On Windows, use rnv.exe. To recompile from the sources, either use
   compile.bat with Borland C/C++ Compiler or modify the Makefile for
   your environment.

Invocation

   The command-line syntax is

     rnv {-q|-v} grammar.rnc {document1.xml}

   If no documents are specified, RNV attempts to read the XML document
   from the standard input. When -q is specified, expected elements and
   attributes are not listed. -v prints version of the program.

Limitations

     * RNV assumes that the encoding of the syntax file is UTF-8.
     * The validator does not validate character data against XML Schema
       datatypes; anything that is not a string is treated as a token.
       The API has facilities to plug in multiple datatype libraries, but
       I currently don't need that particular datatype library for my
       work.
     * The schema parser does not check that all restrictions are obeyed,
       in particular, restrictions 7.3 and 7.4 are not checked.
     * RNV for Win32 platforms is a Unix program compiled on WIn32. It
       expects file paths to be written with normal slashes; if a schema
       is in a different directory and includes or refers external files,
       then the schema's path must be written in the Unix way for the
       relative paths to work. For example, under Windows, rnv that uses
       ..\schema\docbook.rnc to validate userguide.dbx should be invoked
       as:

     rnv.exe ../schema/docbook.rnc userguide.dbx

New versions

   Visit http://davidashen.net/ for news and downloads.

