
EXTRA_DIST = \
	dbusmenu-gtk.pc.in

lib_LTLIBRARIES = \
	libdbusmenu-gtk.la

libdbusmenu_gtkincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-gtk/

libdbusmenu_gtkinclude_HEADERS = \
	test.h

libdbusmenu_gtk_la_SOURCES = \
	test.c

libdbusmenu_gtk_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_gtk_la_CFLAGS = \
	$(LIBDBUSMENU_GTK_CFLAGS)

libdbusmenu_gtk_la_LIBADD = \
	$(LIBDBUSMENU_GTK_LIBS)

pkgconfig_DATA = dbusmenu-gtk.pc
pkgconfigdir = $(libdir)/pkgconfig

