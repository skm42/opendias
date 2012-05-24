SUBDIRS = sane

CLEANFILES = defaultdatabase.sqlite3

test_DATA = \
	appLogUnify.sed \
	defaultdatabase.sqlite3 \
	testapp.conf \
	testappUnableToCreate.conf \
	testLogUnify.sed \
	valgrindUnify.sed 

testdir = $(pkgdatadir)/test/config

EXTRA_DIST = \
	$(test_DATA)

defaultdatabase.sqlite3:
	cp defaultdatabase.sqlite3.in defaultdatabase.sqlite3
	sqlite3 defaultdatabase.sqlite3 "update config set config_value='$(localstatedir)/log/opendias' where config_option = 'log_directory';"
