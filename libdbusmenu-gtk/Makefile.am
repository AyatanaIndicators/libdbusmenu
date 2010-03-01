
CLEANFILES = 

EXTRA_DIST = \
	dbusmenu-gtk.pc.in

lib_LTLIBRARIES = \
	libdbusmenu-gtk.la

libdbusmenu_gtkincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-gtk/

libdbusmenu_gtkinclude_HEADERS = \
	client.h \
	menu.h \
	menuitem.h

libdbusmenu_gtk_la_SOURCES = \
	client.h \
	client.c \
	genericmenuitem.h \
	genericmenuitem.c \
	menu.h \
	menu.c \
	menuitem.h \
	menuitem.c

libdbusmenu_gtk_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_gtk_la_CFLAGS = \
	$(DBUSMENUGTK_CFLAGS) -I$(top_srcdir) -Wall -Werror -DG_DISABLE_DEPRECATED -DG_LOG_DOMAIN="\"LIBDBUSMENU-GTK\""

libdbusmenu_gtk_la_LIBADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGTK_LIBS)

pkgconfig_DATA = dbusmenu-gtk.pc
pkgconfigdir = $(libdir)/pkgconfig

#########################
# GObject Introsepction 
#########################

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = 
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --add-include-path=$(top_srcdir)/libdbusmenu-glib
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=$(top_srcdir)/libdbusmenu-glib

if HAVE_INTROSPECTION

introspection_sources = $(libdbusmenu_gtkinclude_HEADERS)

DbusmenuGtk-0.2.gir: libdbusmenu-gtk.la
DbusmenuGtk_0_2_gir_INCLUDES = \
	GObject-2.0 \
	Gtk-2.0 \
	DbusmenuGlib-0.2
DbusmenuGtk_0_2_gir_CFLAGS = $(DBUSMENUGTK_CFLAGS) -I$(top_srcdir)
DbusmenuGtk_0_2_gir_LIBS = libdbusmenu-gtk.la
DbusmenuGtk_0_2_gir_FILES = $(addprefix $(srcdir)/, $(introspection_sources))

INTROSPECTION_GIRS += DbusmenuGtk-0.2.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)

endif

#########################
# VAPI Files
#########################

if HAVE_INTROSPECTION

vapidir = $(datadir)/vala/vapi
vapi_DATA = DbusmenuGtk-0.2.vapi

DbusmenuGtk-0.2.vapi: DbusmenuGtk-0.2.tmp.gir Makefile.am
	$(VALA_API_GEN) --library=DbusmenuGtk-0.2 \
		--pkg gdk-pixbuf-2.0 \
		--pkg gtk+-2.0 \
		--pkg atk \
		--pkg DbusmenuGlib-0.2 \
		--vapidir=$(top_builddir)/libdbusmenu-glib \
		$<

DbusmenuGtk-0.2.tmp.gir: DbusmenuGtk-0.2.gir
	$(SED) \
		-e "s|GdkPixbuf.Pixbuf|Gdk.Pixbuf|g" \
		-e "s|Atk.ImplementorIface|Atk.Implementor|g" \
		$< > $@

CLEANFILES += $(vapi_DATA)

endif

