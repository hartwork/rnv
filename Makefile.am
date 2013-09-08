AUTOMAKE_OPTIONS = 1.12.1 foreign dist-bzip2 dist-zip dist-xz no-dist-gzip

# Silent rule support for AsciiDoc
V ?= $(AM_DEFAULT_VERBOSITY)
SILENT_RULE_A2X_MESSAGE_PLAIN  = $(if $(subst 0,,$(V)),@:,@echo "  A2X     " $@)
HIDE_A2X = $(if $(subst 0,,$(V)),,@)

# Man page config
_MANUAL_PACKAGE = @PACKAGE_NAME@
_MANUAL_TITLE = @PACKAGE_NAME@ Manual
_MANUAL_VERSION = @PACKAGE_VERSION@


bin_PROGRAMS = rnv arx rvp xsdck
check_PROGRAMS = rnvtest
noinst_LIBRARIES = librnv1.a librnv2.a

TESTS = rnvtest

man1_MANS = \
	man/arx.1 \
	man/rnv.1 \
	man/rvp.1 \
	man/xsdck.1


librnv1_a_SOURCES = \
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
	xsd_tm.h xsd_tm.c



librnv2_a_SOURCES = \
	dxl.h dxl.c \
	dsl.h dsl.c



rnv_CPPFLAGS = -DRNV_VERSION=\"@VERSION@\"
rnv_LDADD = librnv1.a librnv2.a

rnv_SOURCES = \
	ll.h \
	xcl.c



arx_CPPFLAGS = -DARX_VERSION=\"@VERSION@\"
arx_LDADD = librnv1.a

arx_SOURCES = \
	arx.c



rvp_CPPFLAGS = -DRVP_VERSION=\"@VERSION@\"
rvp_LDADD = librnv1.a librnv2.a

rvp_SOURCES = \
	rvp.c



xsdck_LDADD = librnv1.a

xsdck_SOURCES = \
	xsdck.c



rnvtest_LDADD = librnv1.a

rnvtest_SOURCES = \
	test.c


man/%.1: man/%.1.txt man/asciidoc.conf Makefile.am configure.ac
	@chmod u+w docbook-xsl.css 2>/dev/null || true
	@$(MKDIR_P) man
	$(SILENT_RULE_A2X_MESSAGE_PLAIN)
	$(HIDE_A2X)a2x \
		--conf-file=$(srcdir)/man/asciidoc.conf \
		--attribute="manual_package=$(_MANUAL_PACKAGE)" \
		--attribute="manual_title=$(_MANUAL_TITLE)" \
		--attribute="manual_version=$(_MANUAL_VERSION)" \
		--format=manpage -D man \
		"$<"

clean-local:
	rm -f "$(builddir)"/man/*.1


EXTRA_DIST = \
	man/asciidoc.conf \
	man/arx.1 \
	man/arx.1.txt \
	man/rnv.1 \
	man/rnv.1.txt \
	man/rvp.1 \
	man/rvp.1.txt \
	man/xsdck.1 \
	man/xsdck.1.txt \
	\
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
	\
	rx_cls_ranges.c \
	rx_cls_u.c

## NOTE: The two source files above are included by rx.c

