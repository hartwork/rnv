# $Id$
#
CC=cc
INC=-I.
OPT=-g -O -Wall -Wno-parentheses
LIB=
LBL=
OBJ=er.o rnc.o u.o ht.o rn.o util.o sc.o rnd.o

.c.o:
	${CC} ${INC} ${OPT} -c -o $@ $<

rnc: ${OBJ}
	${CC} ${OPT} ${LBL} -o rnc ${OBJ} ${LIB} 

# $Log$
# Revision 1.6  2003/12/07 09:06:16  dvd
# +rnd
#
# Revision 1.5  2003/12/06 00:55:13  dvd
# parses all grammars from nxml-mode samples
#
# Revision 1.4  2003/11/29 17:47:48  dvd
# decl
#
# Revision 1.3  2003/11/27 21:00:23  dvd
# abspath,strhash
#
# Revision 1.2  2003/11/25 10:33:53  dvd
# documentation and comments
#
# Revision 1.1  2003/11/23 16:31:10  dvd
# Makefile added
#
