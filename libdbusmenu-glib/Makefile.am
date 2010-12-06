
CLEANFILES = 

EXTRA_DIST = \
	dbusmenu-glib.pc.in \
	dbus-menu.xml \
	client-marshal.list \
	menuitem-marshal.list \
	server-marshal.list

lib_LTLIBRARIES = \
	libdbusmenu-glib.la

libdbusmenu_glibincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-glib/

libdbusmenu_glibinclude_HEADERS = \
	menuitem.h \
	menuitem-proxy.h \
	server.h \
	client.h

libdbusmenu_glib_la_SOURCES = \
	dbusmenu-server.h \
	dbusmenu-client.h \
	menuitem.h \
	menuitem.c \
	menuitem-marshal.h \
	menuitem-marshal.c \
	menuitem-private.h \
	menuitem-proxy.h \
	menuitem-proxy.c \
	server.h \
	server.c \
	server-marshal.h \
	server-marshal.c \
	client-marshal.h \
	client-marshal.c \
	client-menuitem.h \
	client-menuitem.c \
	client.h \
	client.c

libdbusmenu_glib_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_glib_la_CFLAGS = \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror -DG_DISABLE_DEPRECATED -DG_LOG_DOMAIN="\"LIBDBUSMENU-GLIB\""

libdbusmenu_glib_la_LIBADD = \
	$(DBUSMENUGLIB_LIBS)

pkgconfig_DATA = dbusmenu-glib.pc
pkgconfigdir = $(libdir)/pkgconfig

BUILT_SOURCES = \
	dbusmenu-client.h \
	dbusmenu-server.h \
	client-marshal.h \
	client-marshal.c \
	menuitem-marshal.h \
	menuitem-marshal.c \
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

client-marshal.h: $(srcdir)/client-marshal.list
	glib-genmarshal --header \
		--prefix=_dbusmenu_client_marshal $(srcdir)/client-marshal.list \
		> client-marshal.h

client-marshal.c: $(srcdir)/client-marshal.list
	glib-genmarshal --body \
		--prefix=_dbusmenu_client_marshal $(srcdir)/client-marshal.list \
		> client-marshal.c

server-marshal.h: $(srcdir)/server-marshal.list
	glib-genmarshal --header \
		--prefix=_dbusmenu_server_marshal $(srcdir)/server-marshal.list \
		> server-marshal.h

server-marshal.c: $(srcdir)/server-marshal.list
	glib-genmarshal --body \
		--prefix=_dbusmenu_server_marshal $(srcdir)/server-marshal.list \
		> server-marshal.c

menuitem-marshal.h: $(srcdir)/menuitem-marshal.list
	glib-genmarshal --header \
		--prefix=_dbusmenu_menuitem_marshal $(srcdir)/menuitem-marshal.list \
		> menuitem-marshal.h

menuitem-marshal.c: $(srcdir)/menuitem-marshal.list
	glib-genmarshal --body \
		--prefix=_dbusmenu_menuitem_marshal $(srcdir)/menuitem-marshal.list \
		> menuitem-marshal.c

#########################
# GObject Introsepction 
#########################

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = 

if INTROSPECTION_TEN
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) \
        --add-include-path=$(srcdir) \
        $(addprefix --c-include=libdbusmenu-glib/, $(introspection_sources)) \
        --symbol-prefix=dbusmenu \
        --identifier-prefix=Dbusmenu
else
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) \
        --add-include-path=$(srcdir) \
        $(addprefix --c-include=libdbusmenu-glib/, $(introspection_sources))
endif

INTROSPECTION_COMPILER_ARGS = --includedir=$(builddir)

if HAVE_INTROSPECTION

introspection_sources = $(libdbusmenu_glibinclude_HEADERS)

Dbusmenu_Glib-0.2.gir: libdbusmenu-glib.la
Dbusmenu_Glib_0_2_gir_INCLUDES = \
	GObject-2.0
Dbusmenu_Glib_0_2_gir_CFLAGS = $(DBUSMENUGLIB_CFLAGS)
Dbusmenu_Glib_0_2_gir_LIBS = libdbusmenu-glib.la
Dbusmenu_Glib_0_2_gir_FILES = $(addprefix $(srcdir)/, $(introspection_sources))
Dbusmenu_Glib_0_2_gir_NAMESPACE = Dbusmenu
Dbusmenu_Glib_0_2_gir_VERSION = Glib-0.2
Dbusmenu_Glib_0_2_gir_PACKAGES = dbusmenu-glib
Dbusmenu_Glib_0_2_gir_SCANNER_FLAGS = $(INTROSPECTION_SCANNER_ARGS)

INTROSPECTION_GIRS += Dbusmenu-Glib-0.2.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif

#########################
# VAPI Files
#########################

if HAVE_INTROSPECTION

vapidir = $(datadir)/vala/vapi
vapi_DATA = Dbusmenu-Glib-0.2.vapi

Dbusmenu-Glib-0.2.vapi: Dbusmenu-Glib-0.2.gir
	$(VALA_API_GEN) --library=Dbusmenu-Glib-0.2 $<

CLEANFILES += $(vapi_DATA)

endif

