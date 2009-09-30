SUBDIRS = dbusmenu-gtk

DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

TESTS = \
	test-glib-layout \
	test-glib-properties \
	test-gtk-label \
	test-glib-simple-items

check_PROGRAMS = \
	glib-server-nomenu \
	test-glib-layout-client \
	test-glib-layout-server \
	test-glib-properties-client \
	test-glib-properties-server \
	test-gtk-label-client \
	test-gtk-label-server \
	test-glib-simple-items

glib_server_nomenu_SOURCES = \
	glib-server-nomenu.c

glib_server_nomenu_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

glib_server_nomenu_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

######################
# Test Glib Layout
######################

test-glib-layout: test-glib-layout-client test-glib-layout-server
	@echo "#!/bin/sh" > test-glib-layout
	@echo $(DBUS_RUNNER) --task ./test-glib-layout-client --task-name Client --task ./test-glib-layout-server --task-name Server --ignore-return >> test-glib-layout
	@chmod +x test-glib-layout

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


######################
# Test Glib Properties
######################

test-glib-properties: test-glib-properties-client test-glib-properties-server
	@echo "#!/bin/sh" > test-glib-properties
	@echo $(DBUS_RUNNER) --task ./test-glib-properties-client --task-name Client --task ./test-glib-properties-server --task-name Server --ignore-return >> test-glib-properties
	@chmod +x test-glib-properties

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

#########################
# Test Glib Simple Items
#########################

test_glib_simple_items_SOURCES = \
	test-glib-simple-items.c

test_glib_simple_items_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_simple_items_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

#########################
# Test GTK Label
#########################

test-gtk-label: test-gtk-label-client test-gtk-label-server test-gtk-label.json
	@echo "#!/bin/sh" > test-gtk-label
	@echo $(DBUS_RUNNER) --task ./test-gtk-label-client --task-name Client --task ./test-gtk-label-server --parameter $(srcdir)/test-gtk-label.json --task-name Server --ignore-return >> test-gtk-label
	@chmod +x test-gtk-label

test_gtk_label_server_SOURCES = \
	test-gtk-label-server.c

test_gtk_label_server_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_label_server_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	../libdbusmenu-gtk/libdbusmenu-gtk.la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

test_gtk_label_client_SOURCES = \
	test-gtk-label-client.c

test_gtk_label_client_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_label_client_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	../libdbusmenu-gtk/libdbusmenu-gtk.la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)


#########################
# Other
#########################

examplesdir = $(docdir)/examples/

examples_DATA = \
	$(glib_server_nomenu_SOURCES)

EXTRA_DIST = \
	$(examples_DATA) \
	test-gtk-label.json
