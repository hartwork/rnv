# $Id$
#
CC=cc
INC=-I.
OPT=-g -O -Wall
LIB=
LBL=
OBJ=er.o rnc.o u.o ht.o rn.o util.o

.c.o:
	${CC} ${INC} ${OPT} -c -o $@ $<

rnc: ${OBJ}
	${CC} ${OPT} ${LBL} -o rnc ${OBJ} ${LIB} 

# $Log$
# Revision 1.3  2003/11/27 21:00:23  dvd
# abspath,strhash
#
# Revision 1.2  2003/11/25 10:33:53  dvd
# documentation and comments
#
# Revision 1.1  2003/11/23 16:31:10  dvd
# Makefile added
#
