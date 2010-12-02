
AC_INIT(libdbusmenu, 0.3.90, ted@canonical.com)
AC_COPYRIGHT([Copyright 2009,2010 Canonical])

AC_PREREQ(2.62)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(libdbusmenu, 0.3.90, [-Wno-portability])

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

AC_ARG_ENABLE([deprecations],
  [AS_HELP_STRING([--enable-deprecations],
    [allow deprecated API usage @<:@default=yes@:>@])],
  [],
  [enable_deprecations=yes])
AS_IF([test "x$enable_deprecations" = xno],
  [CFLAGS="$CFLAGS -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE -DGTK_DISABLE_SINGLE_INCLUDES"]
)

###########################
# GTK Doc 
###########################

GTK_DOC_CHECK([1.4])
GNOME_DOC_INIT

###########################
# Dependencies - GLib
###########################

GLIB_REQUIRED_VERSION=2.26
XML_REQUIRED_VERSION=2.6

PKG_CHECK_MODULES(DBUSMENUGLIB, glib-2.0 >= $GLIB_REQUIRED_VERSION
                                gio-2.0 >= $GLIB_REQUIRED_VERSION
                                libxml-2.0 >= $XML_REQUIRED_VERSION)

AC_SUBST(DBUSMENUGLIB_CFLAGS)
AC_SUBST(DBUSMENUGLIB_LIBS)

###########################
# Dependencies - GTK
###########################

GTK_REQUIRED_VERSION=2.16
GTK3_REQUIRED_VERSION=2.91

AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk],
    [Which version of gtk to use @<:@default=2@:>@])],
  [],
  [with_gtk=2])
AS_IF([test "x$with_gtk" = x3],
        [PKG_CHECK_MODULES(DBUSMENUGTK,  gtk+-3.0 >= $GTK3_REQUIRED_VERSION
                                         glib-2.0 >= $GLIB_REQUIRED_VERSION
                                         libxml-2.0 >= $XML_REQUIRED_VERSION)
         AC_SUBST(DBUSMENUGTK_CFLAGS)
         AC_SUBST(DBUSMENUGTK_LIBS)
         AC_DEFINE(HAVE_GTK3, 1, [whether gtk3 is available])
        ],
      [test "x$with_gtk" = x2],
        [PKG_CHECK_MODULES(DBUSMENUGTK,  gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                         glib-2.0 >= $GLIB_REQUIRED_VERSION
                                         libxml-2.0 >= $XML_REQUIRED_VERSION)
         AC_SUBST(DBUSMENUGTK_CFLAGS)
         AC_SUBST(DBUSMENUGTK_LIBS)
        ],
      [AC_MSG_FAILURE([Value for --with-gtk was neither 2 nor 3])]
)
AM_CONDITIONAL(USE_GTK3, [test "x$with_gtk" = x3])

###########################
# Dependencies - dumper
###########################

X11_REQUIRED_VERSION=1.3

PKG_CHECK_MODULES(DBUSMENUDUMPER, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                  x11 >= $X11_REQUIRED_VERSION)

AC_SUBST(DBUSMENUDUMPER_CFLAGS)
AC_SUBST(DBUSMENUDUMPER_LIBS)

###########################
# Dependencies - Testing
###########################

JSON_GLIB_REQUIRED_VERSION=0.6.0
GIO_UNIX_REQUIRED_VERSION=2.24

PKG_CHECK_MODULES(DBUSMENUTESTS,  json-glib-1.0 >= $JSON_GLIB_REQUIRED_VERSION
                                  gio-unix-2.0  >= $GIO_UNIX_REQUIRED_VERSION)

AC_SUBST(DBUSMENUTESTS_CFLAGS)
AC_SUBST(DBUSMENUTESTS_LIBS)

###########################
# GObject Introspection
###########################

GOBJECT_INTROSPECTION_CHECK([0.6.7])

###########################
# Vala API Generation
###########################

AC_PATH_PROG([VALA_API_GEN], [vapigen])

###########################
# XSLT Processor
###########################

AC_PATH_PROG([XSLT_PROC], [xsltproc])

###########################
# Lib versioning 
###########################

LIBDBUSMENU_CURRENT=2
LIBDBUSMENU_REVISION=0
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
# Massive Debugging
###########################

with_massivedebugging="no"
AC_ARG_ENABLE(massivedebugging, AS_HELP_STRING([--enable-massivedebugging], [Print out so much information your brain will hurt]), with_massivedebugging=$enableval, with_massivedebugging=no)
if test "x$with_massivedebugging" = "xyes"; then
	AC_DEFINE([MASSIVEDEBUGGING], [1], [Print everyting])
fi

###########################
# Files
###########################

AC_OUTPUT([
Makefile
po/Makefile.in
libdbusmenu-glib/Makefile
libdbusmenu-glib/dbusmenu-glib-0.4.pc
libdbusmenu-gtk/Makefile
libdbusmenu-gtk/dbusmenu-gtk-0.4.pc
libdbusmenu-gtk/dbusmenu-gtk3-0.4.pc
tools/Makefile
tools/testapp/Makefile
tests/Makefile
tests/dbusmenu-jsonloader-0.4.pc
docs/Makefile
docs/libdbusmenu-glib/Makefile
docs/libdbusmenu-glib/reference/Makefile
docs/libdbusmenu-glib/reference/version.xml
docs/libdbusmenu-gtk/Makefile
docs/libdbusmenu-gtk/reference/Makefile
docs/libdbusmenu-gtk/reference/version.xml
])

###########################
# Results
###########################

AC_MSG_NOTICE([

libdbusmenu Configuration:

	Prefix:                 $prefix
	Massive Debugging:      $with_massivedebugging
	GTK+ Version:           $with_gtk
])

