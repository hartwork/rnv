# $Id$
#
VERSION=1.2.0
CC=cc
EXPAT_H="<expat.h>"
UNISTD_H="<unistd.h>"
LIBEXPAT=-lexpat
INC=-I/usr/local/include
OPT=-g -O -pg 
WARN=-Wall -Wstrict-prototypes  -Wmissing-prototypes -Wcast-align
CFLAGS=${WARN} -shared -DEXPAT_H=${EXPAT_H} -DUNISTD_H=${UNISTD_H} -DRNV_VERSION="\"${VERSION}\""
LFLAGS=
LBL=-L/usr/local/lib
LIB=${LIBEXPAT}

LIBRNVA=librnv.a
LIBRNVSO=librnv.so
LIBRNV=${LIBRNVA}

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
xmlc.c xmlc.h \
strops.c strops.h

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
xmlc.o \
strops.o 

.c.o:
	${CC} ${INC} ${OPT} ${CFLAGS} -c -o $@ $<

all: rnv

rnv: rnv.o ${LIBRNV}
	${CC} ${OPT} ${LFLAGS} ${LBL} -o rnv rnv.o ${LIBRNV} ${LIB} 

rnd_test: ${OBJ} rnd_test.o
	${CC} ${OPT} ${LFLAGS} ${LBL} -o rnd_test rnd_test.o ${OBJ} ${LIB} 

${LIBRNVA}: ${OBJ}
	ar rc $@ ${OBJ}

${LIBRNVSO}: ${OBJ}
	gcc -shared -o $@ ${OBJ}

clean: 
	-rm -f *.o  *.a *.so rnv rnd_test *_test *.core *.gmon *.gprof rnv*.zip rnv.txt rnv.pdf rnv.html rnv.xml

DISTFILES=license.txt ${SRC} Makefile compile.bat rnv.exe readme.txt changes.txt
zip: rnv-${VERSION}.zip
rnv-${VERSION}.zip: ${DISTFILES}
	-rm -rf rnv.zip rnv-[0-9]*.[0-9]*.[0-9]*
	mkdir rnv-${VERSION}
	ln ${DISTFILES} rnv-${VERSION}/.
	zip -r rnv-${VERSION}.zip rnv-${VERSION}
	-rm -rf rnv-${VERSION}

install: rnv-${VERSION}.zip readme.txt changes.txt
	-cp -f rnv-${VERSION}.zip readme.txt changes.txt ${DISTDIR}
