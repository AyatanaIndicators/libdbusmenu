
SUBDIRS = testapp

libexec_PROGRAMS = dbusmenu-dumper

libexec_SCRIPTS = dbusmenu-bench

dbusmenu_dumper_SOURCES = \
	dbusmenu-dumper.c

dbusmenu_dumper_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) $(DBUSMENUTOOLS_CFLAGS) -Wall -Werror

dbusmenu_dumper_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS) $(DBUSMENUTOOLS_LIBS)

doc_DATA = README.dbusmenu-bench

EXTRA_DIST = \
	$(doc_DATA) \
	dbusmenu-bench
