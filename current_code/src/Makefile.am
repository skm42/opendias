## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(OPENDIAS_CFLAGS)

AM_CFLAGS =\
	 -Wall

bin_PROGRAMS = opendias opendiasconfig
BUILT_SOURCES = dirconfig.h
CLEANFILES = dirconfig.h

dirconfig.h:
	echo "#define LOG_LOCATION \"$(localstatedir)/log/opendias\"" > dirconfig.h
	echo "#define DOCUMENT_ROOT \"$(datadir)/opendias/webcontent\"" >> dirconfig.h
	echo "#define DEFAULT_CONF_FILE \"$(sysconfdir)/opendias/opendias.conf\"" >> dirconfig.h
	echo "#define PIDFILE \"$(localstatedir)/run/opendias/opendias.pid\"" >> dirconfig.h
	echo "#define PDF2TXTPROG \"/usr/bin/pdftotext\"" >> dirconfig.h
	echo "#define TMPLOCATION \"$(localstatedir)/tmp\"" >> dirconfig.h

opendiasconfig_SOURCES = \
	db.c    \
	db.h    \
	debug.c    \
	debug.h    \
	dirconfig.h \
	utils.c    \
	utils.h    \
        opendiasconfig.h \
        opendiasconfig.c \
        simpleLinkedList.c    \
        simpleLinkedList.h

opendiasconfig_LDFLAGS = 

opendiasconfig_LDADD = $(OPENDIAS_LIBS)

opendias_SOURCES = \
	db.c    \
	db.h    \
	debug.c    \
	debug.h    \
	dirconfig.h \
	doc_editor.c    \
	doc_editor.h    \
	web_handler.c    \
	web_handler.h    \
	pageRender.c    \
	pageRender.h    \
	main.h    \
	ocr_plug.cc    \
	ocr_plug.h    \
	scan.c    \
	scan.h    \
	scanner.c    \
	scanner.h    \
	speak.cc    \
	speak.h    \
	utils.c    \
	utils.h    \
	main.c    \
	read_odf.c    \
	read_odf.h    \
	dbaccess.c    \
	dbaccess.h    \
	imageProcessing.h    \
	imageProcessing.c    \
	validation.c    \
	validation.h    \
	import_doc.c    \
	import_doc.h    \
        simpleLinkedList.c    \
        simpleLinkedList.h

opendias_LDFLAGS = 

opendias_LDADD = $(OPENDIAS_LIBS)

