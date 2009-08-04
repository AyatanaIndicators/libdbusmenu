
EXTRA_DIST = \
	dbusmenu-gtk.pc.in

lib_LTLIBRARIES = \
	libdbusmenu-gtk.la

libdbusmenu_gtkincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-gtk/

libdbusmenu_gtkinclude_HEADERS = \
	client.h \
	menu.h

libdbusmenu_gtk_la_SOURCES = \
	client.h \
	client.c \
	menu.h \
	menu.c

libdbusmenu_gtk_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_gtk_la_CFLAGS = \
	$(DBUSMENUGTK_CFLAGS) -I$(srcdir)/.. -Wall -Werror

libdbusmenu_gtk_la_LIBADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGTK_LIBS)

pkgconfig_DATA = dbusmenu-gtk.pc
pkgconfigdir = $(libdir)/pkgconfig

