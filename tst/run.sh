#!/bin/sh

RNDIR=/usr/local/share/rng-c
HOME=/home/dvd
RNVDIR=${HOME}/work/PreTI/CImpl
FLAGS=${RNVFLAGS:-}

for a in 1 2 3 4 5
do
	rnv ${FLAGS} ${RNVDIR}/tst/dt/test${a}-compiled.rnc ${RNVDIR}/tst/dt/test${a}.xml
done

for a in docbook fo fo/main rddl rdfxml relaxng xhtml xmlschema xslt 
do
	r=${RNDIR}/${a}.rnc
	echo $r
	rnv ${FLAGS} -c $r
done

rnv ${FLAGS} -c tst/g/paths.rnc

rnv ${FLAGS} ${HOME}/Docu/RELAX-NG/testSuite/testSuite.rnc \
    ${HOME}/Docu/RELAX-NG/testSuite/spectest.xml
rnv ${FLAGS} ${RNDIR}/docbook.rnc ${HOME}/work/XEP/doc/docbook/*.dbx
rnv ${FLAGS} ${RNDIR}/xslt.rnc ${HOME}/work/docbook/xsl/*/*.xsl
rnv ${FLAGS} ${RNDIR}/xmlschema.rnc `find ${HOME}/Docu/mathml2 -name '*.xsd' -print`
rnv ${FLAGS} ${RNDIR}/fo.rnc ${HOME}/work/XEP/Tests/*/*.fo
rnv ${FLAGS} ${RNDIR}/fo/main.rnc ${HOME}/work/XEP/Tests/*/*.fo
xx ${HOME}/work/TEI/P5/P5-driver.xml | rnv ${FLAGS} ${HOME}/work/TEI/P5/p5odds.rnc 
