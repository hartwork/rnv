# $Id$
#
VERSION=1.3.5
CC=cc

EXPAT_H="<expat.h>"
UNISTD_H="<unistd.h>"

INC=-I/usr/local/include
LBL=-L/usr/local/lib

DEF=-DEXPAT_H=${EXPAT_H} -DUNISTD_H=${UNISTD_H} -DRNV_VERSION="\"${VERSION}\"" -DARX_VERSION="\"${VERSION}\""
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
arx.c \
ary.c ary.h \
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
ary.o \
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

all: rnv arx

rnv: xcl.o ${LIBRNV}
	${CC} ${LFLAGS} -o rnv xcl.o ${LIBRNV} ${LIB}

arx: arx.o ${LIBRNV}
	${CC} ${LFLAGS} -o arx arx.o ${LIBRNV} ${LIB}

rnd_test: ${LIBRNV} tst/c/rnd_test.c
	${CC} ${LFLAGS} -I. -o rnd_test tst/c/rnd_test.c ${LIBRNV} ${LIB}

${LIBRNVA}: ${OBJ}
	ar rc $@ ${OBJ}

${LIBRNVSO}: ${OBJ}
	gcc -shared -o $@ ${OBJ}

depend: ${SRC}
	makedepend -Y ${DEF} ${SRC}

clean:
	-rm -f *.o tst/c/*.o  *.a *.so rnv arx rnd_test *_test *.core *.gmon *.gprof rnv*.zip rnv.txt rnv.pdf rnv.html rnv.xml

DIST=rnv+arx
DISTFILES=license.txt ${SRC} Makefile Makefile.bcc readme.txt changes.txt src.txt
DISTWIN32=rnv.exe arx.exe readme32.txt license.txt
DISTTOOLS=tools/xck tools/rnv.vim tools/arx.conf /usr/local/share/rng-c/xslt.rnc /usr/local/share/rng-c/fo.rnc /usr/local/share/rng-c/none.rnc /usr/local/share/rng-c/relaxng.rnc
zip: ${DIST}-${VERSION}.zip
${DIST}-${VERSION}.zip: ${DISTFILES} ${DISTWIN32} ${DISTTOOLS}
	-rm -rf rnv.zip ${DIST}-[0-9]*.[0-9]*.[0-9]*
	mkdir ${DIST}-${VERSION} ${DIST}-${VERSION}/tools
	cp ${DISTFILES} ${DIST}-${VERSION}/.
	cp ${DISTTOOLS} ${DIST}-${VERSION}/tools/.
	zip -9 -r ${DIST}-${VERSION}.zip ${DIST}-${VERSION}
	-rm -rf ${DIST}-${VERSION}
	zip -9 -r ${DIST}-${VERSION}-win32bin.zip ${DISTWIN32}

install: ${DIST}-${VERSION}.zip readme.txt changes.txt
	-cp -f ${DIST}-${VERSION}.zip ${DIST}-${VERSION}-win32bin.zip readme.txt changes.txt ${DISTDIR}
	(cd ${DISTDIR}; rm -f RNV.ZIP ; ln -s ${DIST}-${VERSION}.zip RNV.ZIP)

# DO NOT DELETE

xcl.o: memops.h erbit.h rnc.h rnd.h rnv.h rnx.h ll.h
arx.o: memops.h strops.h xmlc.h ht.h erbit.h rnc.h rnd.h rnv.h rx.h
rn.o: memops.h strops.h ht.h ll.h rn.h
rnc.o: u.h xmlc.h memops.h strops.h rn.h sc.h rnc.h
rnd.o: memops.h rn.h rnx.h ll.h rnd.h
rnv.o: memops.h xmlc.h erbit.h drv.h rnv.h
rnx.o: memops.h strops.h rn.h ll.h rnx.h
drv.o: xmlc.h memops.h strops.h ht.h rn.h xsd.h ll.h erbit.h drv.h
xsd.o: u.h xmlc.h strops.h erbit.h rx.h xsd_tm.h xsd.h
xsd_tm.o: xsd_tm.h
sc.o: memops.h ll.h sc.h
ht.o: memops.h ht.h
u.o: u.h
xmlc.o: u.h xmlc.h
strops.o: xmlc.h memops.h strops.h
memops.o: memops.h
rx.o: u.h xmlc.h memops.h strops.h ht.h ll.h rx.h rx_cls_u.c rx_cls_ranges.c
