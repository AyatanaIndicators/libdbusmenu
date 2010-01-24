/*
A library to communicate a menu object set accross DBus and
track updates and maintain consistency.

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

#ifndef __DBUSMENU_SERVER_H__
#define __DBUSMENU_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_TYPE_SERVER            (dbusmenu_server_get_type ())
#define DBUSMENU_SERVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_SERVER, DbusmenuServer))
#define DBUSMENU_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_SERVER, DbusmenuServerClass))
#define DBUSMENU_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_SERVER))
#define DBUSMENU_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_SERVER))
#define DBUSMENU_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_SERVER, DbusmenuServerClass))

#define DBUSMENU_SERVER_SIGNAL_ID_PROP_UPDATE  "item-property-updated"
#define DBUSMENU_SERVER_SIGNAL_ID_UPDATE       "item-updated"
#define DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATED  "layout-updated"
#define DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATE   DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATED

#define DBUSMENU_SERVER_PROP_DBUS_OBJECT       "dbus-object"
#define DBUSMENU_SERVER_PROP_ROOT_NODE         "root-node"
#define DBUSMENU_SERVER_PROP_VERSION           "version"

/**
	DbusmenuServerClass:
	@parent_class: #GObjectClass
	@id_prop_update: Slot for #DbusmenuServer::id-prop-update.
	@id_update: Slot for #DbusmenuServer::id-update.
	@layout_updated: Slot for #DbusmenuServer::layout-update.
	@dbusmenu_server_reserved1: Reserved for future use.
	@dbusmenu_server_reserved2: Reserved for future use.
	@dbusmenu_server_reserved3: Reserved for future use.
	@dbusmenu_server_reserved4: Reserved for future use.

	The class implementing the virtual functions for #DbusmenuServer.
*/
typedef struct _DbusmenuServerClass DbusmenuServerClass;
struct _DbusmenuServerClass {
	GObjectClass parent_class;

	/* Signals */
	void (*id_prop_update)(guint id, gchar * property, gchar * value);
	void (*id_update)(guint id);
	void (*layout_updated)(gint revision);

	/* Reserved */
	void (*dbusmenu_server_reserved1)(void);
	void (*dbusmenu_server_reserved2)(void);
	void (*dbusmenu_server_reserved3)(void);
	void (*dbusmenu_server_reserved4)(void);
};

/**
	DbusmenuServer:
	@parent: #GObject

	A server which represents a sharing of a set of
	#DbusmenuMenuitems across DBus to a #DbusmenuClient.
*/
typedef struct _DbusmenuServer      DbusmenuServer;
struct _DbusmenuServer {
	GObject parent;
};

GType               dbusmenu_server_get_type   (void);
DbusmenuServer *    dbusmenu_server_new        (const gchar * object);
void                dbusmenu_server_set_root   (DbusmenuServer * server, DbusmenuMenuitem * root);

/**
	SECIONT:server
	@short_description: The server signals changed and
		updates on a tree of #DbusmenuMenuitem objecs.
	@stability: Unstable
	@include: libdbusmenu-glib/server.h

	A #DbusmenuServer is the object that represents the local
	tree of #DbusmenuMenuitem objects on DBus.  It watches the
	various signals that those objects emit and correctly
	represents them across DBus to a #DbusmenuClient so that
	the same tree can be maintained in another process.

	The server needs to have the root set of #DbusmenuMenuitem
	objects set via #dbusmenu_server_set_root but it will query
	all of the objects in that tree automatically.  After setting
	the root there should be no other maintence required by
	users of the server class.
*/
G_END_DECLS

#endif
