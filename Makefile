# $Id$
#
VERSION=1.3.2
CC=cc

EXPAT_H="<expat.h>"
UNISTD_H="<unistd.h>"

INC=-I/usr/local/include
LBL=-L/usr/local/lib

DEF=-DEXPAT_H=${EXPAT_H} -DUNISTD_H=${UNISTD_H} -DRNV_VERSION="\"${VERSION}\""
WARN=-Wall -Wstrict-prototypes  -Wmissing-prototypes -Wcast-align
OPT=-O -g

CFLAGS=${INC} ${DEF} ${WARN} ${OPT}
LFLAGS=${OPT} ${LBL}

LIBEXPAT=-lexpat
LIB=${LIBEXPAT}

LIBRNVA=librnv.a
LIBRNVSO=librnv.so
LIBRNV=${LIBRNVA}

SRC=\
ll.h \
erbit.h \
xcl.c \
rn.c rn.h \
rnc.c rnc.h \
rnd.c rnd.h \
rnv.c rnv.h \
rnx.c rnx.h \
drv.c drv.h \
xsd.c xsd.h \
xsd_tm.c xsd_tm.h \
sc.c sc.h \
ht.c ht.h \
u.c u.h \
xmlc.c xmlc.h \
strops.c strops.h \
memops.c memops.h \
rx.c rx.h \
rx_cls_u.c \
rx_cls_ranges.c 

OBJ=\
rn.o \
rnc.o \
rnd.o \
rnv.o \
rnx.o \
drv.o \
xsd.o \
xsd_tm.o \
sc.o \
u.o \
ht.o \
xmlc.o \
strops.o \
memops.o \
rx.o 

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c -o $@ $<

all: rnv

rnv: xcl.o ${LIBRNV}
	${CC} ${LFLAGS} -o rnv xcl.o ${LIBRNV} ${LIB} 

rnd_test: ${OBJ} rnd_test.o
	${CC} ${LFLAGS} -o rnd_test rnd_test.o ${OBJ} ${LIB} 

${LIBRNVA}: ${OBJ}
	ar rc $@ ${OBJ}

${LIBRNVSO}: ${OBJ}
	gcc -shared -o $@ ${OBJ}

depend: ${SRC}
	makedepend -Y ${DEF} ${SRC}

clean: 
	-rm -f *.o  *.a *.so rnv rnd_test *_test *.core *.gmon *.gprof rnv*.zip rnv.txt rnv.pdf rnv.html rnv.xml

DISTFILES=license.txt ${SRC} Makefile readme.txt changes.txt src.txt
DISTWIN32=rnv.exe compile.bat
DISTTOOLS=tools/rnv.vim tools/uri2rnc.pl
zip: rnv-${VERSION}.zip
rnv-${VERSION}.zip: ${DISTFILES} ${DISTWIN32} ${DISTTOOLS}
	-rm -rf rnv.zip rnv-[0-9]*.[0-9]*.[0-9]*
	mkdir rnv-${VERSION} rnv-${VERSION}/tools rnv-${VERSION}/win32
	ln ${DISTFILES} rnv-${VERSION}/.
	ln ${DISTWIN32} rnv-${VERSION}/win32/.
	ln ${DISTTOOLS} rnv-${VERSION}/tools/.
	zip -r rnv-${VERSION}.zip rnv-${VERSION}
	-rm -rf rnv-${VERSION}

install: rnv-${VERSION}.zip readme.txt changes.txt
	-cp -f rnv-${VERSION}.zip readme.txt changes.txt ${DISTDIR}
	(cd ${DISTDIR}; rm -f RNV.ZIP ; ln -s rnv-${VERSION}.zip RNV.ZIP)

# DO NOT DELETE

xcl.o: erbit.h rn.h rnc.h rnd.h rnv.h rnx.h ll.h
rn.o: strops.h ht.h ll.h rn.h
rnc.o: u.h xmlc.h strops.h rn.h sc.h rnc.h
rnd.o: rn.h rnx.h ll.h rnd.h
rnv.o: xmlc.h erbit.h rn.h drv.h rnv.h
rnx.o: strops.h rn.h ll.h rnx.h
drv.o: xmlc.h strops.h ht.h rn.h xsd.h ll.h erbit.h drv.h
xsd.o: u.h xmlc.h strops.h erbit.h rx.h xsd_tm.h xsd.h
xsd_tm.o: xsd_tm.h
sc.o: ll.h sc.h
ht.o: ht.h
u.o: u.h
xmlc.o: u.h xmlc.h
strops.o: xmlc.h strops.h
rx.o: u.h xmlc.h strops.h ht.h ll.h rx.h rx_cls_u.c rx_cls_ranges.c
