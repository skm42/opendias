
sql_SCRIPTS = 

sql_DATA = \
	openDIAS.sqlite3.dmp.v1.sql \
	openDIAS.sqlite3.dmp.v2.sql \
	openDIAS.sqlite3.dmp.v3.sql \
	openDIAS.sqlite3.dmp.v4.sql \
	openDIAS.sqlite3.dmp.v5.sql \
	openDIAS.sqlite3.dmp.v6.sql

openDIAS.sqlite3.dmp.v4.sql:
	cat openDIAS.sqlite3.dmp.v4.sql.template | sed 's#LOG_LOCATION#$(localstatedir)/log/opendias#' > openDIAS.sqlite3.dmp.v4.sql

sqldir = \
	$(pkgdatadir)

CLEANFILES = openDIAS.sqlite3.dmp.v4.sql

EXTRA_DIST = \
	$(sql_DATA)

## File created by the gnome-build tools

