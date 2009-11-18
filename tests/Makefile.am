
DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

TESTS = \
	test-glib-layout \
	test-glib-properties \
	test-gtk-label \
	test-glib-simple-items \
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

check-local: test-mago-xvfb

######################
# Xvfb server stuff
######################

XVFB = Xvfb -ac -noreset -screen 0 800x600x16
XIDS = 101 102 103 104 105 106 107 197 199 211 223 227 293 307 308 309 310 311 \
   491 492 493 494 495 496 497 498 499 500 501 502 503 504 505 506 507 508 509 \
   991 992 993 994 995 996 997 998 999 1000 1001 1002 1003 1004 1005 1006 1007 \
  1008 1009 4703 4721 4723 4729 4733 4751 9973 9974 9975 9976 9977 9978 9979 \
  9980 9981 9982 9983 9984 9985 9986 9987 9988 9989 9990 9991 9992 9993 9994 \
  9995 9996 9997 9998 9999
XVFB_START = \
	${XVFB} -help 2>/dev/null 1>&2 \
	&& XID=`for id in $(XIDS) ; do test -e /tmp/.X$$id-lock || { echo $$id; exit 0; }; done; exit 1` \
	&& { ${XVFB} :$$XID -screen 0 800x600x16 -nolisten tcp -auth /dev/null >/dev/null 2>&1 & \
	       trap "kill -15 $$! " 0 HUP INT QUIT TRAP USR1 PIPE TERM ; } \
	|| { echo "Gtk+Tests:ERROR: Failed to start Xvfb environment for X11 target tests."; exit 1; } \
	&& DISPLAY=:$$XID && export DISPLAY
# call as: $(XVFB_START) && someprogram


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

test-mago-xvfb: test-mago
	$(XVFB_START) && ./test-mago

test-mago: test-gtk-label-client test-gtk-label-server $(srcdir)/dbusmenu-gtk/mago_tests/dbusmenu.xml Makefile.am
	@echo "#!/bin/sh" > test-mago
	@echo cd $(srcdir)/dbusmenu-gtk >> test-mago
	@echo /usr/lib/at-spi/at-spi-registryd & >> test-mago
	@echo echo Mago Results dir: $(abs_builddir)/mago.results >> test-mago
	@echo echo PYTHONPATH=$(abs_srcdir)/dbusmenu-gtk/mago_tests >> test-mago
	@echo export INDICATOR_BUILD_DIR=$(abs_builddir) >> test-mago
	@echo PYTHONPATH=$(abs_srcdir)/dbusmenu-gtk/mago_tests mago -f dbusmenu.xml -t $(abs_builddir)/mago.results --log-level=debug >> test-mago
	@chmod +x test-mago

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
