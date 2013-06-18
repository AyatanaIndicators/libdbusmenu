
DBUS_RUNNER=dbus-test-runner --max-wait=0

CLEANFILES=
DISTCLEANFILES=
EXTRA_DIST = 

TESTS = \
	test-glib-objects-test \
	test-glib-events \
	test-glib-events-nogroup \
	test-glib-layout \
	test-glib-properties \
	test-glib-proxy \
	test-glib-simple-items \
	test-glib-submenu

if WANT_DBUSMENUDUMPER
if HAVE_VALGRIND
TESTS += \
	test-json \
	test-json-instruction
endif
endif

if WANT_LIBDBUSMENUGTK
TESTS += \
	test-gtk-objects-test \
	test-gtk-label \
	test-gtk-shortcut \
	test-gtk-reorder \
	test-gtk-remove \
	test-gtk-parser-test
# Not working with GTK3 and a critical grab that is in
# the GTK3 code.
#	test-gtk-submenu 
endif

if ENABLE_GTK_DOC
TESTS += \
	test_libdbusmenu_glib_gtkdoc \
	test_libdbusmenu_gtk_gtkdoc
endif

# The Python test only work on the system copy of
# dbusmenu, so while they can be usefule they're not
# good tests of what you're currently building.  Handy
# to check GI support though. FIXME!
PYTHON_TESTS = \
	test-gtk-shortcut-python \
	test-glib-simple-items.py

check_PROGRAMS = \
	glib-server-nomenu \
	test-glib-objects \
	test-glib-events-client \
	test-glib-events-server \
	test-glib-events-nogroup-client \
	test-glib-layout-client \
	test-glib-layout-server \
	test-glib-properties-client \
	test-glib-properties-server \
	test-glib-proxy-client \
	test-glib-proxy-server \
	test-glib-proxy-proxy \
	test-glib-submenu-client \
	test-glib-submenu-server \
	test-glib-simple-items

if WANT_DBUSMENUDUMPER
if HAVE_VALGRIND
check_PROGRAMS += \
	test-json-client \
	test-json-server
endif
endif

if WANT_LIBDBUSMENUGTK
check_PROGRAMS += \
	test-gtk-objects \
	test-gtk-label-client \
	test-gtk-label-server \
	test-gtk-shortcut-client \
	test-gtk-shortcut-server \
	test-gtk-remove-server \
	test-gtk-reorder-server \
	test-gtk-submenu-server \
	test-gtk-submenu-client \
	test-gtk-parser
endif

XVFB_RUN=". $(srcdir)/run-xvfb.sh"

# for the GI tests, prefer/use the typelibs from the local build tree
TESTS_ENVIRONMENT = env GI_TYPELIB_PATH=$(top_builddir)/libdbusmenu-glib:$(top_builddir)/libdbusmenu-gtk:$(GI_TYPELIB_PATH)

######################
# JSON Loader lib
######################

lib_LTLIBRARIES = libdbusmenu-jsonloader.la

libdbusmenu_jsonloaderincludedir=$(includedir)/libdbusmenu-glib-0.4/libdbusmenu-jsonloader/

libdbusmenu_jsonloaderinclude_HEADERS = \
	json-loader.h

libdbusmenu_jsonloader_la_SOURCES = \
	json-loader.h \
	json-loader.c

libdbusmenu_jsonloader_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_jsonloader_la_CFLAGS = \
	$(DBUSMENUGLIB_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	-I $(top_srcdir) \
	-Wall \
	-Werror \
	-DG_DISABLE_DEPRECATED \
	-DG_LOG_DOMAIN="\"LIBDBUSMENU-JSONLOADER\""

libdbusmenu_jsonloader_la_LIBADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS) \
	$(DBUSMENUTESTS_LIBS)

pkgconfig_DATA = dbusmenu-jsonloader-0.4.pc
pkgconfigdir = $(libdir)/pkgconfig

######################
# Test GLib server
######################

glib_server_nomenu_SOURCES = \
	glib-server-nomenu.c

glib_server_nomenu_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

glib_server_nomenu_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

######################
# Test Glib Layout
######################

test-glib-layout: test-glib-layout-client test-glib-layout-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task ./test-glib-layout-client --task-name Client --task ./test-glib-layout-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_glib_layout_server_SOURCES = \
	test-glib-layout.h \
	test-glib-layout-server.c

test_glib_layout_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_layout_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_layout_client_SOURCES = \
	test-glib-layout.h \
	test-glib-layout-client.c

test_glib_layout_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_layout_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

######################
# Test Glib Events
######################

test-glib-events: test-glib-events-client test-glib-events-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task ./test-glib-events-client --task-name Client --task ./test-glib-events-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_glib_events_server_SOURCES = \
	test-glib-events-server.c

test_glib_events_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_events_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_events_client_SOURCES = \
	test-glib-events-client.c

test_glib_events_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_events_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

################################
# Test Glib Events No Grouping
################################

test-glib-events-nogroup: test-glib-events-nogroup-client test-glib-events-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task ./test-glib-events-nogroup-client --task-name Client --task ./test-glib-events-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_glib_events_nogroup_client_SOURCES = \
	test-glib-events-nogroup-client.c

test_glib_events_nogroup_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_events_nogroup_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

######################
# Test JSON
######################

test-json: test-json-client test-json-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task ./test-json-client --wait-for org.dbusmenu.test --task-name Client --parameter $(top_builddir)/tools/dbusmenu-dumper --parameter test-json-01.output.json --task ./test-json-server --task-name Server --parameter $(srcdir)/test-json-01.json >> $@
	@echo diff $(srcdir)/test-json-01.json test-json-01.output.json \> /dev/null >> $@
	@chmod +x $@

CLEANFILES += test-json-01.output.json

test_json_server_SOURCES = \
	test-json-server.c

test_json_server_CFLAGS = \
	-I $(top_srcdir) \
	-I $(srcdir) \
	$(DBUSMENUGLIB_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUTESTSVALGRIND_CFLAGS) \
	-Wall -Werror

test_json_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	libdbusmenu-jsonloader.la \
	$(DBUSMENUTESTS_LIBS) \
	$(DBUSMENUGLIB_LIBS)

test_json_client_SOURCES = \
	test-json-client.c

test_json_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_json_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUTESTS_LIBS) \
	$(DBUSMENUGLIB_LIBS)

#########################
# Test JSON Instructions
#########################

test-json-instruction: test-json-client test-json-server test-json-instruction-count Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task $(builddir)/test-json-client --wait-for org.dbusmenu.test --task-name Client --parameter $(top_builddir)/tools/dbusmenu-dumper --parameter /dev/null --task libtool --parameter --mode=execute --parameter $(srcdir)/test-json-instruction-count --parameter $(builddir)/test-json-server --task-name Server --parameter $(srcdir)/test-json-01.json >> $@
	@chmod +x $@

EXTRA_DIST += \
	test-json-instruction-count


######################
# Test Glib Submenu
######################

test-glib-submenu: test-glib-submenu-client test-glib-submenu-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
# Removing the check for criticals here as there's one coming from GTK
# in the grabbing code on GTK3.  Since we can't add events to the stack
# we can't remove this error from getting thrown :-(
#	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task ./test-glib-submenu-client --task-name Client --task ./test-glib-submenu-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_glib_submenu_server_SOURCES = \
	test-glib-submenu.h \
	test-glib-submenu-server.c

test_glib_submenu_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_submenu_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_submenu_client_SOURCES = \
	test-glib-submenu.h \
	test-glib-submenu-client.c

test_glib_submenu_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_submenu_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

######################
# Test Glib Object
######################

OBJECT_XML_REPORT = test-glib-objects.xml

test-glib-objects-test: test-glib-objects Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task gtester --task-name test --parameter --verbose --parameter -k --parameter -o --parameter $(OBJECT_XML_REPORT) --parameter ./test-glib-objects >> $@
	@chmod +x $@

test_glib_objects_SOURCES = \
	test-glib-objects.c

test_glib_objects_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_objects_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

DISTCLEANFILES += $(OBJECT_XML_REPORT)

######################
# Test Glib Properties
######################

test-glib-properties: test-glib-properties-client test-glib-properties-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task ./test-glib-properties-client --task-name Client --task ./test-glib-properties-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_glib_properties_server_SOURCES = \
	test-glib-properties.h \
	test-glib-properties-server.c

test_glib_properties_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_properties_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_properties_client_SOURCES = \
	test-glib-properties.h \
	test-glib-properties-client.c

test_glib_properties_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_properties_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

######################
# Test Glib Proxy
######################

test-glib-proxy: test-glib-proxy-client test-glib-proxy-server test-glib-proxy-proxy Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(DBUS_RUNNER) --task ./test-glib-proxy-client --task-name Client --task ./test-glib-proxy-server --task-name Server --ignore-return \\ >> $@
	@echo --task ./test-glib-proxy-proxy --parameter test.proxy.first_proxy --parameter test.proxy.second_proxy --task-name Proxy01 --ignore-return \\ >> $@
	@echo --task ./test-glib-proxy-proxy --parameter test.proxy.second_proxy --parameter test.proxy.third_proxy --task-name Proxy02 --ignore-return \\ >> $@
	@echo --task ./test-glib-proxy-proxy --parameter test.proxy.third_proxy --parameter test.proxy.fourth_proxy --task-name Proxy03 --ignore-return \\ >> $@
	@echo --task ./test-glib-proxy-proxy --parameter test.proxy.fourth_proxy --parameter test.proxy.last_proxy --task-name Proxy04 --ignore-return \\ >> $@
	@echo --task ./test-glib-proxy-proxy --parameter test.proxy.last_proxy --parameter test.proxy.server --task-name Proxy05 --ignore-return >> $@
	@chmod +x $@

test_glib_proxy_server_SOURCES = \
	test-glib-proxy.h \
	test-glib-proxy-server.c

test_glib_proxy_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_proxy_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_proxy_client_SOURCES = \
	test-glib-proxy.h \
	test-glib-proxy-client.c

test_glib_proxy_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_proxy_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

test_glib_proxy_proxy_SOURCES = \
	test-glib-proxy.h \
	test-glib-proxy-proxy.c

test_glib_proxy_proxy_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_proxy_proxy_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

#########################
# Test Glib Simple Items
#########################

test_glib_simple_items_SOURCES = \
	test-glib-simple-items.c

test_glib_simple_items_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_glib_simple_items_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(DBUSMENUGLIB_LIBS)

EXTRA_DIST += test-glib-simple-items.py

######################
# Test GTK Object
######################

GTK_OBJECT_XML_REPORT = test-gtk-objects.xml

test-gtk-objects-test: test-gtk-objects Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task gtester --task-name test --parameter --verbose --parameter -k --parameter -o --parameter $(GTK_OBJECT_XML_REPORT) --parameter ./test-gtk-objects >> $@
	@chmod +x $@

test_gtk_objects_SOURCES = \
	test-gtk-objects.c

test_gtk_objects_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) \
	$(DBUSMENUGTK_CFLAGS) \
	-DSRCDIR="\"$(srcdir)\"" \
	-Wall -Werror

test_gtk_objects_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGLIB_LIBS) \
	$(DBUSMENUGTK_LIBS)

DISTCLEANFILES += $(GTK_OBJECT_XML_REPORT)

######################
# Test GTK Parser
######################

GTK_PARSER_XML_REPORT = test-gtk-parser.xml

test-gtk-parser-test: test-gtk-parser Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(GTK_PARSER_XML_REPORT) ./test-gtk-parser >> $@
	@chmod +x $@

test_gtk_parser_SOURCES = \
	test-gtk-parser.c

test_gtk_parser_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGLIB_CFLAGS) \
	$(DBUSMENUGTK_CFLAGS) \
	-DSRCDIR="\"$(srcdir)\"" \
	-Wall -Werror

test_gtk_parser_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGLIB_LIBS) \
	$(DBUSMENUGTK_LIBS)

DISTCLEANFILES += $(GTK_PARSER_XML_REPORT)

#########################
# Test GTK Label
#########################

test-gtk-label: test-gtk-label-client test-gtk-label-server test-gtk-label.json Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task ./test-gtk-label-client --task-name Client --task ./test-gtk-label-server --parameter $(srcdir)/test-gtk-label.json --task-name Server --ignore-return >> $@
	@chmod +x $@

test_gtk_label_server_SOURCES = \
	test-gtk-label-server.c

test_gtk_label_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_label_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	libdbusmenu-jsonloader.la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

test_gtk_label_client_SOURCES = \
	test-gtk-label-client.c

test_gtk_label_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_label_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

#########################
# Test GTK Shortcut
#########################

test-gtk-shortcut: test-gtk-shortcut-client test-gtk-shortcut-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task ./test-gtk-shortcut-client --task-name Client --task ./test-gtk-shortcut-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_gtk_shortcut_server_SOURCES = \
	test-gtk-shortcut-server.c

test_gtk_shortcut_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_shortcut_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

test_gtk_shortcut_client_SOURCES = \
	test-gtk-shortcut-client.c

test_gtk_shortcut_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_shortcut_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

#########################
# Test GTK Shortcut Python
#########################

test-gtk-shortcut-python: test-gtk-shortcut-server test-gtk-shortcut-client.py Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task $(srcdir)/test-gtk-shortcut-client.py --task-name Client --task ./test-gtk-shortcut-server --task-name Server --ignore-return >> $@
	@chmod +x $@

EXTRA_DIST += test-gtk-shortcut-client.py
CLEANFILES += test-gtk-shortcut-client.pyc

#########################
# Test GTK Remove
#########################

test-gtk-remove: test-gtk-remove-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task ./test-gtk-remove-server --parameter $(srcdir)/test-gtk-label.json --task-name Server --ignore-return >> $@
	@chmod +x $@

test_gtk_remove_server_SOURCES = \
	test-gtk-remove-server.c

test_gtk_remove_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_remove_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)


#########################
# Test GTK Reorder
#########################

test-gtk-reorder: test-gtk-label-client test-gtk-reorder-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task ./test-gtk-label-client --task-name Client --task ./test-gtk-reorder-server --parameter $(srcdir)/test-gtk-label.json --task-name Server --ignore-return >> $@
	@chmod +x $@

test_gtk_reorder_server_SOURCES = \
	test-gtk-reorder-server.c

test_gtk_reorder_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_reorder_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

#########################
# Test GTK Submenu
#########################

test-gtk-submenu: test-gtk-submenu-client test-gtk-submenu-server Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
	@echo $(XVFB_RUN) >> $@
	@echo $(DBUS_RUNNER) --task ./test-gtk-submenu-client --task-name Client --task ./test-gtk-submenu-server --task-name Server --ignore-return >> $@
	@chmod +x $@

test_gtk_submenu_server_SOURCES = \
	test-gtk-submenu-server.c

test_gtk_submenu_server_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_submenu_server_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

test_gtk_submenu_client_SOURCES = \
	test-gtk-submenu-client.c

test_gtk_submenu_client_CFLAGS = \
	-I $(top_srcdir) \
	$(DBUSMENUGTK_CFLAGS) \
	$(DBUSMENUTESTS_CFLAGS) \
	$(DBUSMENUGLIB_CFLAGS) -Wall -Werror

test_gtk_submenu_client_LDADD = \
	$(top_builddir)/libdbusmenu-glib/libdbusmenu-glib.la \
	$(top_builddir)/libdbusmenu-gtk/libdbusmenu-gtk$(VER).la \
	$(DBUSMENUGTK_LIBS) \
	$(DBUSMENUTESTS_LIBS)

#########################
# Test Mago
#########################

test-mago: test-gtk-label-client test-gtk-label-server $(srcdir)/dbusmenu-gtk/mago_tests/dbusmenu.xml Makefile.am
	@echo "#!/bin/bash" > $@
	@echo export UBUNTU_MENUPROXY="" >> $@
	@echo export G_DEBUG=fatal_criticals >> $@
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

test_libdbusmenu_glib_gtkdoc:
	@echo "#!/bin/bash" > $@
	@echo cd $(abs_top_builddir)/docs/libdbusmenu-glib/reference >> $@
	@echo gtester --verbose -k $(GTKDOC_CHECK) >> $@
	@chmod +x $@

test_libdbusmenu_gtk_gtkdoc:
	@echo "#!/bin/bash" > $@
	@echo cd $(abs_top_builddir)/docs/libdbusmenu-gtk/reference >> $@
	@echo gtester --verbose -k $(GTKDOC_CHECK) >> $@
	@chmod +x $@

examplesdir = $(docdir)/examples/

examples_DATA = \
	$(glib_server_nomenu_SOURCES)

jsondir = $(datadir)/${PACKAGE}/json/

json_DATA = \
	test-gtk-label.json

EXTRA_DIST += \
	$(examples_DATA) \
	run-xvfb.sh \
	$(json_DATA) \
	test-gtk-objects.jpg \
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
	dbusmenu-gtk/mago_tests/data/test-gtk-label.json \
	test-json-01.json

CLEANFILES += \
	dbusmenu-gtk/mago_tests/dbusmenu.pyc

distclean-local:
	-rm -rf $(builddir)/mago.results

DISTCLEANFILES += \
	$(filter-out %.py, $(TESTS))

if USE_GTK3
VER=3
else
VER=
endif
