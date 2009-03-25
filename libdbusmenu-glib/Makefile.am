
EXTRA_DIST = \
	dbusmenu_glib.pc.in

lib_LTLIBRARIES = \
	libdbusmenu-glib.la

libdbusmenu_glibincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-glib/

libdbusmenu_glibinclude_HEADERS = \
	test.h

libdbusmenu_glib_la_SOURCES = \
	test.c

libdbusmenu_glib_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_glib_la_CFLAGS = \
	$(LIBDBUSMENU_GLIB_CFLAGS)

libdbusmenu_glib_la_LIBADD = \
	$(LIBDBUSMENU_GLIB_LIBS)

pkgconfig_DATA = dbusmenu_glib.pc
pkgconfigdir = $(libdir)/pkgconfig

