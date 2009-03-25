
EXTRA_DIST = \
	dbusmenu-qt.pc.in

lib_LTLIBRARIES = \
	libdbusmenu-qt.la

libdbusmenu_qtincludedir=$(includedir)/libdbusmenu-0.1/libdbusmenu-qt/

libdbusmenu_qtinclude_HEADERS = \
	test.h

libdbusmenu_qt_la_SOURCES = \
	test.c

libdbusmenu_qt_la_LDFLAGS = \
	-version-info $(LIBDBUSMENU_CURRENT):$(LIBDBUSMENU_REVISION):$(LIBDBUSMENU_AGE) \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

libdbusmenu_qt_la_CFLAGS = \
	$(LIBDBUSMENU_QT_CFLAGS)

libdbusmenu_qt_la_LIBADD = \
	$(LIBDBUSMENU_QT_LIBS)

pkgconfig_DATA = dbusmenu-qt.pc
pkgconfigdir = $(libdir)/pkgconfig

