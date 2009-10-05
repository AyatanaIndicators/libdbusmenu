
libexec_PROGRAMS = dbusmenu-dumper

dbusmenu_dumper_SOURCES = \
	dbusmenu-dumper.c

dbusmenu_dumper_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

dbusmenu_dumper_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

