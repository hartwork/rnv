# $Id$
#
CC=cc
INC=-I/usr/local/include
OPT=-g -O -pg -Wall -Wno-parentheses
LIB=-lexpat
LBL=-L/usr/local/lib
OBJ=\
rn.o \
rnc.o \
rnd.o \
drv.o \
xsd.o \
er.o \
sc.o \
u.o \
ht.o \
util.o 

.c.o:
	${CC} ${INC} ${OPT} -c -o $@ $<

all: rnv

rnv: ${OBJ} rnv.o
	${CC} ${OPT} ${LBL} -o rnv rnv.o ${OBJ} ${LIB} 

rnd_test: ${OBJ} rnd_test.o
	${CC} ${OPT} ${LBL} -o rnd_test rnd_test.o ${OBJ} ${LIB} 

clean: 
	-rm -f *.o rnv rnd_test

# $Log$
# Revision 1.15  2003/12/13 22:31:54  dvd
# a few bugfixes
#
# Revision 1.14  2003/12/11 23:37:58  dvd
# derivative in progress
#
# Revision 1.13  2003/12/11 17:18:09  dvd
# rnv->rnd_test,rnv will be the command-line validator
#
# Revision 1.12  2003/12/11 17:01:31  dvd
# utf8 is handled properly
#
# Revision 1.11  2003/12/10 22:23:52  dvd
# *** empty log message ***
#
# Revision 1.10  2003/12/08 23:16:15  dvd
# multiple schema files as command-line arguments to rnv, cleanups in file handling code (rnc)
#
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
# DO NOT DELETE

drv.o: rn.h drv.h
er.o: er.h
ht.o: ht.h
ht_test.o: ht.h
rn.o: util.h ht.h rn.h
rnc.o: util.h u.h er.h rn.h sc.h rnc.h
rnd.o: er.h rn.h rnd.h
rnv.o: rnc.h rnd.h
sc.o: sc.h
u.o: u.h
u_test.o: u.h
u_test2.o: u.h
u_test3.o: u.h
util.o: util.h
util_test.o: util.h
xsd.o: xsd.h
