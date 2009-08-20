
AC_INIT(libdbusmenu, 0.0.1, ted@canonical.com)
AC_COPYRIGHT([Copyright 2009 Canonical])

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(libdbusmenu, 0.0.1)

AM_MAINTAINER_MODE

IT_PROG_INTLTOOL([0.35.0])
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

###########################
# Dependencies - GLib
###########################

GLIB_REQUIRED_VERSION=2.18
DBUS_REQUIRED_VERSION=0.76
XML_REQUIRED_VERSION=2.6

PKG_CHECK_MODULES(DBUSMENUGLIB, glib-2.0 >= $GLIB_REQUIRED_VERSION
                                dbus-glib-1 >= $DBUS_REQUIRED_VERSION
                                libxml-2.0 >= $XML_REQUIRED_VERSION)

AC_SUBST(DBUSMENUGLIB_CFLAGS)
AC_SUBST(DBUSMENUGLIB_LIBS)

###########################
# Dependencies - GTK
###########################

GTK_REQUIRED_VERSION=2.16

PKG_CHECK_MODULES(DBUSMENUGTK,  gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                glib-2.0 >= $GLIB_REQUIRED_VERSION
                                dbus-glib-1 >= $DBUS_REQUIRED_VERSION
                                libxml-2.0 >= $XML_REQUIRED_VERSION)

AC_SUBST(DBUSMENUGTK_CFLAGS)
AC_SUBST(DBUSMENUGTK_LIBS)

###########################
# Dependencies - Testing
###########################

JSON_GLIB_REQUIRED_VERSION=0.6.0

PKG_CHECK_MODULES(DBUSMENUTESTS,  json-glib-1.0 >= $JSON_GLIB_REQUIRED_VERSION)

AC_SUBST(DBUSMENUTESTS_CFLAGS)
AC_SUBST(DBUSMENUTESTS_LIBS)

###########################
# Lib versioning 
###########################

LIBDBUSMENU_CURRENT=0
LIBDBUSMENU_REVISION=1
LIBDBUSMENU_AGE=0

AC_SUBST(LIBDBUSMENU_CURRENT)
AC_SUBST(LIBDBUSMENU_REVISION)
AC_SUBST(LIBDBUSMENU_AGE)

###########################
# Internationalization
###########################

GETTEXT_PACKAGE=libdbusmenu
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the default gettext domain])

AM_GLIB_GNU_GETTEXT

###########################
# Files
###########################

AC_OUTPUT([
Makefile
po/Makefile.in
libdbusmenu-glib/Makefile
libdbusmenu-glib/dbusmenu-glib.pc
libdbusmenu-gtk/Makefile
libdbusmenu-gtk/dbusmenu-gtk.pc
libdbusmenu-qt/Makefile
libdbusmenu-qt/dbusmenu-qt.pc
tests/Makefile
tests/dbusmenu-gtk/Makefile
])

###########################
# Results
###########################

AC_MSG_NOTICE([

libdbusmenu Configuration:

	Prefix:                 $prefix
])

