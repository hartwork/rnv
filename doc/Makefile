# $Id$

XSLT=java -Xms16m -Xmx16m com.sun.xt.xsl.sax.Driver
#XSLT=java com.jclark.xsl.sax.Driver
#XSLT=java com.icl.saxon.StyleSheet -x com.sun.resolver.tools.ResolvingXMLReader -y com.sun.resolver.tools.ResolvingXMLReader -r com.sun.resolver.tools.CatalogResolver
XSLFO=java -Dcom.renderx.sax.entityresolver=com.sun.resolver.tools.CatalogResolver -Dcom.renderx.jaxp.uriresolver=com.sun.resolver.tools.CatalogResolver com.renderx.xep.XSLDriver -DROOT=/home/dvd/work/XEP
DBX2HTML_XSL=~/work/docbook/xsl/html/docbook.xsl
DBX2PDF_XSL=~/work/docbook/xsl/fo/docbook.xsl -param draft.mode=no -param xep.extensions=1
#DBX2PDF_XSL=~/work/XEP/doc/docbook/stylesheets/dbx2pdf.xsl

.SUFFIXES: .dbx .html .pdf .txt

.dbx.html:
	${XSLT} $< ${DBX2HTML_XSL} > $@

.dbx.pdf:
	${XSLFO} $< -xsl ${DBX2PDF_XSL} $@

.html.txt:
	lynx -dump $< \
	| sed -e '/^References$$/,$$d' \
	     -e '/^ *__* *$$/d' \
	     -e 's/\[[0-9][0-9]*\]//g' \
	     > $@

