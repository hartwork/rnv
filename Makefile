# $Id$
#
VERSION=1.0.1
CC=cc
EXPAT_H="<expat.h>"
UNISTD_H="<unistd.h>"
LIBEXPAT=-lexpat
INC=-I/usr/local/include
CFLAGS=-Wall -DEXPAT_H=${EXPAT_H} -DUNISTD_H=${UNISTD_H} -DRNV_VERSION="\"${VERSION}\""
LFLAGS=
OPT=-g -O -pg 
LIB=${LIBEXPAT}
LBL=-L/usr/local/lib
SRC=\
rnv.c \
rn.c rn.h \
rnc.c rnc.h \
rnd.c rnd.h \
rnx.c rnx.h \
drv.c drv.h \
xsd.c xsd.h \
er.c er.h \
sc.c sc.h \
ht.c ht.h \
u.c u.h \
util.c util.h

OBJ=\
rn.o \
rnc.o \
rnd.o \
rnx.o \
drv.o \
xsd.o \
er.o \
sc.o \
u.o \
ht.o \
util.o 

.c.o:
	${CC} ${INC} ${OPT} ${CFLAGS} -c -o $@ $<

all: rnv

rnv: ${OBJ} rnv.o
	${CC} ${OPT} ${LFLAGS} ${LBL} -o rnv rnv.o ${OBJ} ${LIB} 

rnd_test: ${OBJ} rnd_test.o
	${CC} ${OPT} ${LFLAGS} ${LBL} -o rnd_test rnd_test.o ${OBJ} ${LIB} 

clean: 
	-rm -f *.o rnv rnd_test *_test *.core *.gmon *.gprof rnv*.zip

DISTFILES=license.txt ${SRC} Makefile compile.bat rnv.exe readme.txt 
zip: ${DISTFILES}
	-rm -rf rnv.zip rnv-[0-9]*.[0-9]*.[0-9]*
	mkdir rnv-${VERSION}
	ln ${DISTFILES} rnv-${VERSION}/.
	zip -r rnv-${VERSION}.zip rnv-${VERSION}
	-rm -rf rnv-${VERSION}

