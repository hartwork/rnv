AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip

bin_PROGRAMS = rnv arx rvp xsdck rnvtest

TESTS = rnvtest



rnv_CPPFLAGS = -DRNV_VERSION=\"@VERSION@\"

rnv_SOURCES = \
	ary.h ary.c \
	drv.h drv.c \
	er.h er.c \
	erbit.h \
	ht.h ht.c \
	m.h m.c \
	rn.h rn.c \
	rnc.h rnc.c \
	rnd.h rnd.c \
	rnl.h rnl.c \
	rnv.h rnv.c \
	rnx.h rnx.c \
	rx.h rx.c \
	s.h s.c \
	sc.h sc.c \
	u.h u.c \
	xmlc.h xmlc.c \
	xsd.h xsd.c \
	xsd_tm.h xsd_tm.c \
	\
	dxl.h dxl.c \
	dsl.h dsl.c \
	\
	ll.h \
	xcl.c



arx_CPPFLAGS = -DARX_VERSION=\"@VERSION@\"

arx_SOURCES = \
	ary.h ary.c \
	drv.h drv.c \
	er.h er.c \
	erbit.h \
	ht.h ht.c \
	m.h m.c \
	rn.h rn.c \
	rnc.h rnc.c \
	rnd.h rnd.c \
	rnl.h rnl.c \
	rnv.h rnv.c \
	rnx.h rnx.c \
	rx.h rx.c \
	s.h s.c \
	sc.h sc.c \
	u.h u.c \
	xmlc.h xmlc.c \
	xsd.h xsd.c \
	xsd_tm.h xsd_tm.c \
	\
	arx.c



rvp_CPPFLAGS = -DRVP_VERSION=\"@VERSION@\"

rvp_SOURCES = \
	ary.h ary.c \
	drv.h drv.c \
	er.h er.c \
	erbit.h \
	ht.h ht.c \
	m.h m.c \
	rn.h rn.c \
	rnc.h rnc.c \
	rnd.h rnd.c \
	rnl.h rnl.c \
	rnv.h rnv.c \
	rnx.h rnx.c \
	rx.h rx.c \
	s.h s.c \
	sc.h sc.c \
	u.h u.c \
	xmlc.h xmlc.c \
	xsd.h xsd.c \
	xsd_tm.h xsd_tm.c \
	\
	dxl.h dxl.c \
	dsl.h dsl.c \
	\
	rvp.c



xsdck_SOURCES = \
	ary.h ary.c \
	drv.h drv.c \
	er.h er.c \
	erbit.h \
	ht.h ht.c \
	m.h m.c \
	rn.h rn.c \
	rnc.h rnc.c \
	rnd.h rnd.c \
	rnl.h rnl.c \
	rnv.h rnv.c \
	rnx.h rnx.c \
	rx.h rx.c \
	s.h s.c \
	sc.h sc.c \
	u.h u.c \
	xmlc.h xmlc.c \
	xsd.h xsd.c \
	xsd_tm.h xsd_tm.c \
	\
	xsdck.c



rnvtest_SOURCES = \
	ary.h ary.c \
	drv.h drv.c \
	er.h er.c \
	erbit.h \
	ht.h ht.c \
	m.h m.c \
	rn.h rn.c \
	rnc.h rnc.c \
	rnd.h rnd.c \
	rnl.h rnl.c \
	rnv.h rnv.c \
	rnx.h rnx.c \
	rx.h rx.c \
	s.h s.c \
	sc.h sc.c \
	u.h u.c \
	xmlc.h xmlc.c \
	xsd.h xsd.c \
	xsd_tm.h xsd_tm.c \
	\
	test.c

EXTRA_DIST = \
	scm/dsl.scm \
	scm/M.scm \
	scm/rx.scm \
	scm/rx-ranges.scm \
	scm/s48.scm \
	scm/spat.scm \
	scm/u.scm \
	scm/xml-ranges.scm \
	\
	tools/addr-spec.rnc \
	tools/addr-spec-dsl.rnc \
	tools/arx.conf \
	tools/b64gen.c \
	tools/gn.sed \
	tools/pr.c \
	tools/rgen.pl \
	tools/rnv.vim \
	tools/rvp.pl \
	tools/rvp.py \
	tools/xck \
	tools/xsd2rnc.xsl \
	tools/xslt-dsl.rnc \
	\
	readme.txt \
	readme32.txt \
	src.txt \
	\
	build_vms.com \
	Makefile___TRY_CONFIGURE_INSTEAD \
	Makefile.bcc___TRY_CONFIGURE_INSTEAD \
	Makefile.bsd___TRY_CONFIGURE_INSTEAD \
	Makefile.gnu___TRY_CONFIGURE_INSTEAD \
	\
	rx_cls_ranges.c \
	rx_cls_u.c

## NOTE: The two source files above are included by rx.c
