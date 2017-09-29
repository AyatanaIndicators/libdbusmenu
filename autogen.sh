#!/bin/sh

PKG_NAME="libdbusmenu"

which gnome-autogen.sh || {
	echo "You need gnome-common from GNOME SVN"
	exit 1
}

gtkdocize || exit 1

gnome-autogen.sh --enable-gtk-doc $@
