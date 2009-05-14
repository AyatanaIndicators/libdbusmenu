check: tests

DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

tests: test-glib-layout test-glib-properties

libexec_PROGRAMS = \
	glib-server-nomenu \
	test-glib-layout-client \
	test-glib-layout-server \
	test-glib-properties-client \
	test-glib-properties-server

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
	test-glib-layout.h \
	test-glib-layout-server.c

test_glib_layout_server_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_layout_server_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_layout_client_SOURCES = \
	test-glib-layout.h \
	test-glib-layout-client.c

test_glib_layout_client_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_layout_client_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)



test-glib-properties: test-glib-properties-client test-glib-properties-server
	$(DBUS_RUNNER) --task ./test-glib-properties-client --task-name Client --task ./test-glib-properties-server --task-name Server --ignore-return

test_glib_properties_server_SOURCES = \
	test-glib-properties.h \
	test-glib-properties-server.c

test_glib_properties_server_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_properties_server_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_properties_client_SOURCES = \
	test-glib-properties.h \
	test-glib-properties-client.c

test_glib_properties_client_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_properties_client_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)



examplesdir = $(docdir)/examples/

examples_DATA = \
	$(glib_server_nomenu_SOURCES)

EXTRA_DIST = $(examples_DATA)
