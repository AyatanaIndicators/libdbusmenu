
EXTRA_DIST = \
	dbusmenu-glib.pc.in \
	dbus-menu.xml

lib_LTLIBRARIES = \
	libdbusmenu-glib.la

libdbusmenu_glibincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-glib/

libdbusmenu_glibinclude_HEADERS = \
	menuitem.h \
	server.h \
	client.h

libdbusmenu_glib_la_SOURCES = \
	dbusmenu-server.h \
	dbusmenu-client.h \
	menuitem.h \
	menuitem.c \
	server.h \
	server.c \
	server-marshal.h \
	server-marshal.c \
	client.h \
	client.c

libdbusmenu_glib_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_glib_la_CFLAGS = \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

libdbusmenu_glib_la_LIBADD = \
	$(DBUSMENUGLIB_LIBS)

pkgconfig_DATA = dbusmenu-glib.pc
pkgconfigdir = $(libdir)/pkgconfig

BUILT_SOURCES = \
	dbusmenu-client.h \
	dbusmenu-server.h \
	server-marshal.h \
	server-marshal.c

dbusmenu-server.h: dbus-menu.xml
	dbus-binding-tool \
		--prefix=_dbusmenu_server \
		--mode=glib-server \
		--output=dbusmenu-server.h \
		$(srcdir)/dbus-menu.xml

dbusmenu-client.h: dbus-menu.xml
	dbus-binding-tool \
		--prefix=_dbusmenu_client \
		--mode=glib-client \
		--output=dbusmenu-client.h \
		$(srcdir)/dbus-menu.xml

server-marshal.h: $(srcdir)/server-marshal.list
	glib-genmarshal --header \
		--prefix=_dbusmenu_server_marshal $(srcdir)/server-marshal.list \
		> server-marshal.h

server-marshal.c: $(srcdir)/server-marshal.list
	glib-genmarshal --body \
		--prefix=_dbusmenu_server_marshal $(srcdir)/server-marshal.list \
		> server-marshal.c

