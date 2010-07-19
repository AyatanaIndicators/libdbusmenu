
SUBDIRS = testapp

libexec_PROGRAMS = dbusmenu-dumper

libexec_SCRIPTS = dbusmenu-bench

dbusmenu_dumper_SOURCES = \
	dbusmenu-dumper.c

dbusmenu_dumper_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -I/usr/include/dbus-1.0 -Wall -Werror

dbusmenu_dumper_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS) -lX11 -ldbus-glib-1

doc_DATA = README.dbusmenu-bench

EXTRA_DIST = \
	$(doc_DATA) \
	dbusmenu-bench
