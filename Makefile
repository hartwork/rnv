# $Id$
#
CC=cc
INC=-I.
OPT=-g -O -pg -Wall -Wno-parentheses
LIB=
LBL=
OBJ=\
rn.o \
rnc.o \
rnd.o \
rnv.o \
er.o \
sc.o \
u.o \
ht.o \
util.o 

.c.o:
	${CC} ${INC} ${OPT} -c -o $@ $<

rnv: ${OBJ}
	${CC} ${OPT} ${LBL} -o rnv ${OBJ} ${LIB} 

# $Log$
# Revision 1.9  2003/12/08 21:23:47  dvd
# +path restrictions
#
# Revision 1.8  2003/12/07 20:41:42  dvd
# bugfixes, loops, traits
#
# Revision 1.7  2003/12/07 16:50:55  dvd
# stage D, dereferencing and checking for loops
#
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
