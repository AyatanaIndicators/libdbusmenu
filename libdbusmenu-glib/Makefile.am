
EXTRA_DIST = \
	dbusmenu-glib.pc.in

lib_LTLIBRARIES = \
	libdbusmenu-glib.la

libdbusmenu_glibincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-glib/

libdbusmenu_glibinclude_HEADERS = \
	menuitem.h

libdbusmenu_glib_la_SOURCES = \
	menuitem.c

libdbusmenu_glib_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_glib_la_CFLAGS = \
	$(DBUSMENUGLIB_CFLAGS)

libdbusmenu_glib_la_LIBADD = \
	$(DBUSMENUGLIB_LIBS)

pkgconfig_DATA = dbusmenu-glib.pc
pkgconfigdir = $(libdir)/pkgconfig

