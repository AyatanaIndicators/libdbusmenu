
libexec_PROGRAMS = \
	glib-server-nomenu

glib_server_nomenu_SOURCES = \
	glib-server-nomenu.c

glib_server_nomenu_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

glib_server_nomenu_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)




examplesdir = $(docdir)/examples/

examples_DATA = \
	$(glib_server_nomenu_SOURCES)

EXTRA_DIST = $(examples_DATA)
