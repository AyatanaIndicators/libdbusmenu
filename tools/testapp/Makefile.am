
if USE_GTK3
VER=3
else
VER=
endif

libexec_PROGRAMS = dbusmenu-testapp

dbusmenu_testapp_SOURCES = \
	main.c

dbusmenu_testapp_CFLAGS = \
	-I $(srcdir)/../.. \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) \
	-Wall -Werror

dbusmenu_testapp_LDADD = \
	$(builddir)/../../libdbusmenu-glib/libdbusmenu-glib.la \
	$(builddir)/../../libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)
