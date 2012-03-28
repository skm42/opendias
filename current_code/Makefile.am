## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src sql webcontent scripts

opendiasdocdir = ${prefix}/doc/opendias

opendiasdoc_DATA = ChangeLog

EXTRA_DIST = $(opendiasdoc_DATA)

install-exec-hook:
	test ! -e $(DESTDIR)/$(localstatedir)/log/opendias && mkdir -p $(DESTDIR)/$(localstatedir)/log/opendias || echo Log directory already exists
	test ! -e $(DESTDIR)/$(localstatedir)/lib/opendias && mkdir -p $(DESTDIR)/$(localstatedir)/lib/opendias || echo Lib directory already exists
	test ! -e $(DESTDIR)/$(localstatedir)/run/opendias && mkdir -p $(DESTDIR)/$(localstatedir)/run/opendias || echo Lib directory already exists
	grep -q opendias /etc/passwd && echo "opendias user exists. Adjusting permission"
	grep -q opendias /etc/passwd && test -e $(DESTDIR)/$(localstatedir)/log/opendias && chown opendias:opendias $(DESTDIR)/$(localstatedir)/log/opendias || echo Cannot change ownership for opendias logs
	grep -q opendias /etc/passwd && test -e $(DESTDIR)/$(localstatedir)/lib/opendias && chown opendias:opendias $(DESTDIR)/$(localstatedir)/lib/opendias || echo Cannot change ownership for opendias database and archive sink
	grep -q opendias /etc/passwd && test -e $(DESTDIR)/$(localstatedir)/run/opendias && chown opendias:opendias $(DESTDIR)/$(localstatedir)/run/opendias || echo Cannot change ownership for opendias pidfile
	test ! -e $(DESTDIR)/$(sysconfdir)/opendias && mkdir -p $(DESTDIR)/$(sysconfdir)/opendias || echo openDIAS config directory already exists
	test ! -e $(DESTDIR)/$(sysconfdir)/opendias/opendias.conf && echo $(localstatedir)/lib/opendias >> $(DESTDIR)/$(sysconfdir)/opendias/opendias.conf || echo config file already exists
	test ! -e $(DESTDIR)/$(sysconfdir)/init.d && mkdir $(DESTDIR)/$(sysconfdir)/init.d || echo "init.d already exists"
	test ! -e $(DESTDIR)/$(sysconfdir)/init.d/openDIAS && install -m 755 scripts/openDIAS $(DESTDIR)/$(sysconfdir)/init.d/openDIAS || echo "rc-script already in place"
		

## Copy all the spec files. Of cource, only one is actually used.
#dist-hook:
#	for specfile in *.spec; do \
#		if test -f $$specfile; then \
#			cp -p $$specfile $(distdir); \
#		fi \
#	done

