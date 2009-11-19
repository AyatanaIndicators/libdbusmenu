
DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

TESTS = \
	test-glib-layout \
	test-glib-properties \
	test-glib-simple-items \
	test-mago

GTK_TESTS = \
	test-gtk-label \
	test-gtk-reorder

check_PROGRAMS = \
	glib-server-nomenu \
	test-glib-layout-client \
	test-glib-layout-server \
	test-glib-properties-client \
	test-glib-properties-server \
	test-gtk-label-client \
	test-gtk-label-server \
	test-glib-simple-items \
	test-gtk-reorder-server

XVFB_RUN=". run-xvfb.sh"

######################
# Test GLib server
######################

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

test-glib-layout: test-glib-layout-client test-glib-layout-server Makefile.am
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

test-glib-properties: test-glib-properties-client test-glib-properties-server Makefile.am
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

test-gtk-label: test-gtk-label-client test-gtk-label-server test-gtk-label.json Makefile.am
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
# Test GTK Reorder
#########################

test-gtk-reorder: test-gtk-label-client test-gtk-reorder-server Makefile.am
	@echo "#!/bin/sh" > test-gtk-reorder
	@echo $(DBUS_RUNNER) --task ./test-gtk-label-client --task-name Client --task ./test-gtk-reorder-server --parameter $(srcdir)/test-gtk-label.json --task-name Server --ignore-return >> test-gtk-reorder
	@chmod +x test-gtk-reorder

test_gtk_reorder_server_SOURCES = \
	test-gtk-reorder-server.c

test_gtk_reorder_server_CFLAGS = \
	-I $(srcdir)/.. \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_reorder_server_LDADD = \
	../libdbusmenu-glib/libdbusmenu-glib.la \
	../libdbusmenu-gtk/libdbusmenu-gtk.la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

#########################
# Test Mago
#########################

test-mago: test-gtk-label-client test-gtk-label-server $(srcdir)/dbusmenu-gtk/mago_tests/dbusmenu.xml Makefile.am
	@echo "#!/bin/sh" > $@
	@echo $(XVFB_RUN) >> $@
	@echo cd $(srcdir)/dbusmenu-gtk >> $@
	@echo /usr/lib/at-spi/at-spi-registryd \& >> $@
	@echo echo Mago Results dir: $(abs_builddir)/mago.results >> $@
	@echo echo PYTHONPATH=$(abs_srcdir)/dbusmenu-gtk/mago_tests >> $@
	@echo export INDICATOR_BUILD_DIR=$(abs_builddir) >> $@
	@echo PYTHONPATH=$(abs_srcdir)/dbusmenu-gtk/mago_tests mago -f dbusmenu.xml -t $(abs_builddir)/mago.results --log-level=debug >> $@
	@chmod +x $@

#########################
# Other
#########################

examplesdir = $(docdir)/examples/

examples_DATA = \
	$(glib_server_nomenu_SOURCES)

EXTRA_DIST = \
	$(examples_DATA) \
	test-gtk-label.json \
	dbusmenu-gtk/dbusMenuTest \
	dbusmenu-gtk/mago_tests/dbusmenu.xml \
	dbusmenu-gtk/mago_tests/dbusmenu.py \
	dbusmenu-gtk/mago_tests/data/blank_label_2levels.json \
	dbusmenu-gtk/mago_tests/data/blank_label.json \
	dbusmenu-gtk/mago_tests/data/blank_submenus.json \
	dbusmenu-gtk/mago_tests/data/dynamic.json \
	dbusmenu-gtk/mago_tests/data/long_label.json \
	dbusmenu-gtk/mago_tests/data/no_id.json \
	dbusmenu-gtk/mago_tests/data/no_label.json \
	dbusmenu-gtk/mago_tests/data/sameid_submenus_diff_sizes.json \
	dbusmenu-gtk/mago_tests/data/sameid_submenus.json \
	dbusmenu-gtk/mago_tests/data/sameid_top_and_submenus.json \
	dbusmenu-gtk/mago_tests/data/sameid_topmenu.json \
	dbusmenu-gtk/mago_tests/data/several_submenus.json \
	dbusmenu-gtk/mago_tests/data/several_submenus_recursive.json \
	dbusmenu-gtk/mago_tests/data/several_submenus_utf8.json \
	dbusmenu-gtk/mago_tests/data/static.json \
	dbusmenu-gtk/mago_tests/data/test-gtk-label.json

CLEANFILES = \
	dbusmenu-gtk/mago_tests/dbusmenu.xml \
	dbusmenu-gtk/mago_tests/dbusmenu.pyc

distclean-local:
	-rm -rf $(builddir)/dbusmenu-gtk/mago.results

DISTCLEANFILES = \
	$(TESTS)
