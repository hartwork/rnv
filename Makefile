# $Id$
#

#PUBLIC

VERSION=1.7.5
CC=cc

# optional features
M_STATIC=0
M_FILL=0
DSL_SCM=0
DXL_EXC=0

EXPAT_H="<expat.h>"
UNISTD_H="<unistd.h>"
SCM_H="<scm/scm.h>"

INC=-I/usr/local/include
LBL=-L/usr/local/lib

DEF=\
-DM_STATIC=${M_STATIC} \
-DM_FILL=${M_FILL} \
-DEXPAT_H=${EXPAT_H} \
-DUNISTD_H=${UNISTD_H} \
-DRNV_VERSION="\"${VERSION}\"" \
-DARX_VERSION="\"${VERSION}\"" \
-DRVP_VERSION="\"${VERSION}\""
WARN=-Wall -Wstrict-prototypes  -Wmissing-prototypes -Wcast-align
OPT=-O -g

CFLAGS=${INC} ${DEF} ${WARN} ${OPT}
LFLAGS=${OPT} ${LBL}

LIBEXPAT=-lexpat
LIB_SCM=-lscm -lm \
`sh -c '[ -f /usr/lib/libdl.a ] && echo -ldl \
      ; [ -f /usr/lib/libsocket.a ] && echo -lsocket \
'` 

LIB=${LIBEXPAT}

.if ${DSL_SCM}
DEF+=-DDSL_SCM=${DSL_SCM} -DSCM_H=${SCM_H}
LIB+=${LIB_SCM}
.endif

.if ${DXL_EXC}
DEF+=-DDXL_EXC=${DXL_EXC}
.endif

LIBRNVA=librnv.a
LIBRNVSO=librnv.so
LIBRNV=${LIBRNVA}

SRC=\
ll.h \
erbit.h \
xcl.c \
arx.c \
rvp.c \
xsdck.c \
test.c \
ary.c ary.h \
rn.c rn.h \
rnc.c rnc.h \
rnd.c rnd.h \
rnl.c rnl.h \
rnv.c rnv.h \
rnx.c rnx.h \
drv.c drv.h \
xsd.c xsd.h \
xsd_tm.c xsd_tm.h \
dxl.c dxl.h \
dsl.c dsl.h \
sc.c sc.h \
ht.c ht.h \
er.c er.h \
u.c u.h \
xmlc.c xmlc.h \
s.c s.h \
m.c m.h \
rx.c rx.h \
rx_cls_u.c \
rx_cls_ranges.c

OBJ=\
rn.o \
rnc.o \
rnd.o \
rnl.o \
rnv.o \
rnx.o \
drv.o \
ary.o \
xsd.o \
xsd_tm.o \
dxl.o \
dsl.o \
sc.o \
u.o \
ht.o \
er.o \
xmlc.o \
s.o \
m.o \
rx.o

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c -o $@ $<

all: rnv arx rvp xsdck test

rnv: xcl.o ${LIBRNV}
	${CC} ${LFLAGS} -o rnv xcl.o ${LIBRNV} ${LIB}

arx: arx.o ${LIBRNV}
	${CC} ${LFLAGS} -o arx arx.o ${LIBRNV} ${LIB}

rvp: rvp.o ${LIBRNV}
	${CC} ${LFLAGS} -o rvp rvp.o ${LIBRNV} ${LIB}

xsdck: xsdck.o ${LIBRNV}
	${CC} ${LFLAGS} -o xsdck xsdck.o ${LIBRNV} ${LIB}

test: test.o ${LIBRNV}
	${CC} ${LFLAGS} -o test test.o ${LIBRNV} ${LIB}

${LIBRNVA}: ${OBJ}
	ar rc $@ ${OBJ}
	ranlib ${LIBRNVA}

${LIBRNVSO}: ${OBJ}
	gcc -shared -o $@ ${OBJ}

depend: ${SRC}
	makedepend -Y ${DEF} ${SRC}

clean:
	-rm -f *.o tst/c/*.o  *.a *.so rnv arx rnd_test *_test *.core *.gmon *.gprof rnv*.zip rnv.txt rnv.pdf rnv.html rnv.xml

rnd_test: ${LIBRNV} tst/c/rnd_test.c
	${CC} ${LFLAGS} -I. -o rnd_test tst/c/rnd_test.c ${LIBRNV} ${LIB}

#PRIVATE

Makefile.gnu: Makefile.bsd
	cat $> | \
	sed -E	-e 's/^\.if ([$${}A-Za-z_]+)/ifeq (\1,1)/' \
		-e 's/^\.endif/endif/' | \
	cat > $@

Makefile.bsd: Makefile
	cat $> | \
	sed -En -e '/^#PUBLIC/,/^#PRIVATE/ p' | \
	grep -Ev '^#PUBLIC|^#PRIVATE' | \
	cat > $@

DIST=rnv
DISTFILES=license.txt ${SRC} Makefile.gnu Makefile.bsd Makefile.bcc readme.txt changes.txt src.txt
DISTWIN32=rnv.exe arx.exe readme32.txt license.txt
DISTTOOLS=\
tools/xck tools/rnv.vim tools/arx.conf \
tools/rvp.pl tools/rvp.py \
/usr/local/share/rng-c/xslt.rnc \
/usr/local/share/rng-c/fo.rnc \
/usr/local/share/rng-c/none.rnc \
/usr/local/share/rng-c/relaxng.rnc \
tools/addr-spec.rnc tools/addr-spec-dsl.rnc tools/xslt-dsl.rnc
DISTSCM=\
scm/u.scm \
scm/xml-ranges.scm \
scm/rx-ranges.scm \
scm/rx.scm \
scm/spat.scm \
scm/dsl.scm

zip: ${DIST}-${VERSION}.zip
${DIST}-${VERSION}.zip: ${DISTFILES} ${DISTWIN32} ${DISTTOOLS} ${DISTSCM}
	-rm -rf rnv.zip ${DIST}-[0-9]*.[0-9]*.[0-9]*
	mkdir ${DIST}-${VERSION} ${DIST}-${VERSION}/tools ${DIST}-${VERSION}/scm
	cp ${DISTFILES} ${DIST}-${VERSION}/.
	cp ${DISTTOOLS} ${DIST}-${VERSION}/tools/.
	cp ${DISTSCM} ${DIST}-${VERSION}/scm/.
	ln -s Makefile.gnu ${DIST}-${VERSION}/Makefile
	zip -9 -r ${DIST}-${VERSION}.zip ${DIST}-${VERSION}
	-rm -rf ${DIST}-${VERSION}
	zip -9 -r ${DIST}-${VERSION}-win32bin.zip ${DISTWIN32}

install: ${DIST}-${VERSION}.zip readme.txt changes.txt
	-cp -f ${DIST}-${VERSION}.zip ${DIST}-${VERSION}-win32bin.zip readme.txt changes.txt doc/rnv.pdf ${DISTDIR}
	(cd ${DISTDIR}; rm -f RNV.ZIP ; ln -s ${DIST}-${VERSION}.zip RNV.ZIP)

# DO NOT DELETE

xcl.o: m.h erbit.h rnl.h rnv.h rnx.h er.h ll.h
arx.o: u.h m.h s.h xmlc.h ht.h erbit.h rnl.h rnv.h rx.h er.h ary.h
rvp.o: m.h s.h erbit.h drv.h rnl.h rnv.h dxl.h er.h dsl.h
xsdck.o: er.h xsd.h
ary.o: rn.h ary.h
rn.o: m.h s.h ht.h ll.h rn.h
rnc.o: u.h xmlc.h m.h s.h rn.h sc.h er.h rnc.h
rnd.o: m.h rn.h rnx.h ll.h er.h rnd.h
rnl.o: erbit.h rn.h rnc.h rnd.h rnl.h
rnv.o: m.h xmlc.h erbit.h drv.h er.h rnv.h
rnx.o: m.h s.h rn.h ll.h rnx.h
drv.o: xmlc.h m.h s.h ht.h rn.h xsd.h ll.h erbit.h er.h drv.h
xsd.o: u.h xmlc.h s.h erbit.h rx.h xsd_tm.h er.h xsd.h
xsd_tm.o: xsd_tm.h
dxl.o: m.h er.h dxl.h
dsl.o: dsl.h
sc.o: m.h ll.h sc.h
ht.o: m.h ht.h
er.o: er.h
u.o: u.h
xmlc.o: u.h xmlc.h
s.o: xmlc.h m.h s.h
m.o: er.h m.h
rx.o: u.h xmlc.h m.h s.h ht.h ll.h er.h rx.h rx_cls_u.c rx_cls_ranges.c
