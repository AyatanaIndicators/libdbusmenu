check: tests

DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

tests: test-glib-layout

libexec_PROGRAMS = \
	glib-server-nomenu \
	test-glib-layout-client \
	test-glib-layout-server

glib_server_nomenu_SOURCES = \
	glib-server-nomenu.c

glib_server_nomenu_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

glib_server_nomenu_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test-glib-layout: test-glib-layout-client test-glib-layout-server
	$(DBUS_RUNNER) --task ./test-glib-layout-client --task-name Client --task ./test-glib-layout-server --task-name Server --ignore-return

test_glib_layout_server_SOURCES = \
	test-glib-layout-server.c

test_glib_layout_server_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_layout_server_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_layout_client_SOURCES = \
	test-glib-layout-client.c

test_glib_layout_client_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_layout_client_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)




examplesdir = $(docdir)/examples/

examples_DATA = \
	$(glib_server_nomenu_SOURCES)

EXTRA_DIST = $(examples_DATA)
