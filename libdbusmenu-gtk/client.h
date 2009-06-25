/*
A library to take the object model made consistent by libdbusmenu-glib
and visualize it in GTK.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the 
Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by 
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the applicable version of the GNU Lesser General Public 
License for more details.

You should have received a copy of both the GNU Lesser General Public 
License version 3 and version 2.1 along with this program.  If not, see 
<http://www.gnu.org/licenses/>
*/

#ifndef __DBUSMENU_GTKCLIENT_H__
#define __DBUSMENU_GTKCLIENT_H__

#include <glib.h>
#include <glib-object.h>
#include <libdbusmenu-glib/client.h>

G_BEGIN_DECLS

#define DBUSMENU_GTKCLIENT_TYPE            (dbusmenu_gtkclient_get_type ())
#define DBUSMENU_GTKCLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_GTKCLIENT_TYPE, DbusmenuGtkClient))
#define DBUSMENU_GTKCLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_GTKCLIENT_TYPE, DbusmenuGtkClientClass))
#define DBUSMENU_IS_GTKCLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_GTKCLIENT_TYPE))
#define DBUSMENU_IS_GTKCLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_GTKCLIENT_TYPE))
#define DBUSMENU_GTKCLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_GTKCLIENT_TYPE, DbusmenuGtkClientClass))

#define DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED  DBUSMENU_CLIENT_SIGNAL_ROOT_CHANGED

/**
	DbusmenuGtkClientClass:
	@parent_class: #GtkMenuClass
	@reserved1: Reserved for future use.
	@reserved2: Reserved for future use.
	@reserved3: Reserved for future use.
	@reserved4: Reserved for future use.
*/
typedef struct _DbusmenuGtkClientClass DbusmenuGtkClientClass;
struct _DbusmenuGtkClientClass {
	DbusmenuClientClass parent_class;

	/* Signals */
	void (*root_changed) (DbusmenuMenuitem * newroot);

	/* Reserved */
	void (*reserved1) (void);
	void (*reserved2) (void);
	void (*reserved3) (void);
	void (*reserved4) (void);
};

/**
	DbusmenuGtkClient:
	@parent: #GtkMenu
*/
typedef struct _DbusmenuGtkClient      DbusmenuGtkClient;
struct _DbusmenuGtkClient {
	DbusmenuClient parent;
};

GType dbusmenu_gtkclient_get_type (void);
DbusmenuGtkClient * dbusmenu_gtkclient_new (gchar * dbus_name, gchar * dbus_object);
GtkMenuItem * dbusmenu_gtkclient_menuitem_get (DbusmenuMenuitem * item);

/**
	SECTION:gtkmenu
	@short_description: A GTK Menu Object that syncronizes over DBus
	@stability: Unstable
	@include: libdbusmenu-gtk/menu.h

	In general, this is just a #GtkMenu, why else would you care?  Oh,
	because this menu is created by someone else on a server that exists
	on the other side of DBus.  You need a #DbusmenuServer to be able
	push the data into this menu.

	The first thing you need to know is how to find that #DbusmenuServer
	on DBus.  This involves both the DBus name and the DBus object that
	the menu interface can be found on.  Those two value should be set
	when creating the object using dbusmenu_gtkmenu_new().  They are then
	stored on two properties #DbusmenuGtkClient:dbus-name and #DbusmenuGtkClient:dbus-object.

	After creation the #DbusmenuGtkClient it will continue to keep in
	synchronization with the #DbusmenuServer object across Dbus.  If the
	number of entries change, the menus change, if they change thier
	properties change, they update in the items.  All of this should
	be handled transparently to the user of this object.

	TODO: Document properties.
*/
G_END_DECLS

#endif
