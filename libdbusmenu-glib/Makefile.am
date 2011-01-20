
CLEANFILES = 

EXTRA_DIST = \
	clean-namespaces.xslt \
	dbusmenu-glib-0.4.pc.in \
	dbus-menu.xml \
	client-marshal.list \
	menuitem-marshal.list \
	server-marshal.list

lib_LTLIBRARIES = \
	libdbusmenu-glib.la

libdbusmenu_glibincludedir=$(includedir)/libdbusmenu-0.4/libdbusmenu-glib/

libdbusmenu_glibinclude_HEADERS = \
	menuitem.h \
	menuitem-proxy.h \
	server.h \
	client.h

libdbusmenu_glib_la_SOURCES = \
	dbus-menu-clean.xml.h \
	dbus-menu-clean.xml.c \
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

pkgconfig_DATA = dbusmenu-glib-0.4.pc
pkgconfigdir = $(libdir)/pkgconfig

%.xml.h: %.xml
	echo "extern const char * $(subst -,_,$(subst .,_,$(basename $(notdir $@))));" > $@

%.xml.c: %.xml
	echo "const char * $(subst -,_,$(subst .,_,$(basename $(notdir $@)))) = " > $@
	sed -e "s:\":\\\\\":g" -e s:^:\": -e s:\$$:\\\\n\": $< >> $@
	echo ";" >> $@

dbus-menu-clean.xml: dbus-menu.xml
	$(XSLT_PROC) $(srcdir)/clean-namespaces.xslt $< > $@ || (rm -f $@ && /bin/false)

CLEANFILES += dbus-menu-clean.xml

BUILT_SOURCES = \
	dbus-menu-clean.xml.c \
	dbus-menu-clean.xml.h \
	client-marshal.h \
	client-marshal.c \
	menuitem-marshal.h \
	menuitem-marshal.c \
	server-marshal.h \
	server-marshal.c

CLEANFILES += $(BUILT_SOURCES)

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
	--warn-all \
        --add-include-path=$(srcdir) \
        $(addprefix --c-include=libdbusmenu-glib/, $(introspection_sources)) \
        --symbol-prefix=dbusmenu \
        --identifier-prefix=Dbusmenu
else
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) \
	--warn-all \
        --add-include-path=$(srcdir) \
        $(addprefix --c-include=libdbusmenu-glib/, $(introspection_sources))
endif

INTROSPECTION_COMPILER_ARGS = --includedir=$(builddir)

if HAVE_INTROSPECTION

introspection_sources = $(libdbusmenu_glibinclude_HEADERS)

Dbusmenu_Glib-0.4.gir: libdbusmenu-glib.la
Dbusmenu_Glib_0_4_gir_INCLUDES = \
	GObject-2.0
Dbusmenu_Glib_0_4_gir_CFLAGS = $(DBUSMENUGLIB_CFLAGS)
Dbusmenu_Glib_0_4_gir_LIBS = libdbusmenu-glib.la
Dbusmenu_Glib_0_4_gir_FILES = $(addprefix $(srcdir)/, $(introspection_sources))
Dbusmenu_Glib_0_4_gir_NAMESPACE = Dbusmenu
Dbusmenu_Glib_0_4_gir_VERSION = Glib-0.4
Dbusmenu_Glib_0_4_gir_EXPORT_PACKAGES = dbusmenu-glib-0.4
Dbusmenu_Glib_0_4_gir_SCANNER_FLAGS = $(INTROSPECTION_SCANNER_ARGS)

INTROSPECTION_GIRS += Dbusmenu-Glib-0.4.gir

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
vapi_DATA = Dbusmenu-Glib-0.4.vapi

Dbusmenu-Glib-0.4.vapi: Dbusmenu-Glib-0.4.gir
	$(VALA_API_GEN) --library=Dbusmenu-Glib-0.4 $<

CLEANFILES += $(vapi_DATA)

endif

