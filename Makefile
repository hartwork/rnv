# $Id$
#
CC=cc
INC=-I.
OPT=-g -O -Wall
LIB=
LBL=
OBJ=er.o rnc.o u.o 

.c.o:
	${CC} ${INC} ${OPT} -c -o $@ $<

rnc: ${OBJ}
	${CC} ${OPT} ${LBL} -o rnc ${OBJ} ${LIB} 

# $Log$
# Revision 1.2  2003/11/25 10:33:53  dvd
# documentation and comments
#
# Revision 1.1  2003/11/23 16:31:10  dvd
# Makefile added
#
