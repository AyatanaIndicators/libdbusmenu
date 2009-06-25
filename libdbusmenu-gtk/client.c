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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "client.h"

/* Prototypes */
static void dbusmenu_gtkclient_class_init (DbusmenuGtkClientClass *klass);
static void dbusmenu_gtkclient_init       (DbusmenuGtkClient *self);
static void dbusmenu_gtkclient_dispose    (GObject *object);
static void dbusmenu_gtkclient_finalize   (GObject *object);
static void new_menuitem (DbusmenuClient * client, DbusmenuMenuitem * mi, gpointer userdata);
static void new_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position, gpointer userdata);
static void delete_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, gpointer userdata);
static void move_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint new, guint old, gpointer userdata);

/* GObject Stuff */
G_DEFINE_TYPE (DbusmenuGtkClient, dbusmenu_gtkclient, DBUSMENU_TYPE_CLIENT);

static void
dbusmenu_gtkclient_class_init (DbusmenuGtkClientClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = dbusmenu_gtkclient_dispose;
	object_class->finalize = dbusmenu_gtkclient_finalize;

	return;
}

static void
dbusmenu_gtkclient_init (DbusmenuGtkClient *self)
{
	g_signal_connect(G_OBJECT(self), DBUSMENU_CLIENT_SIGNAL_NEW_MENUITEM, G_CALLBACK(new_menuitem), NULL);
	return;
}

static void
dbusmenu_gtkclient_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkclient_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtkclient_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkclient_parent_class)->finalize (object);
	return;
}

/* Internal Functions */

static const gchar * data_menuitem = "dbusmenugtk-data-gtkmenuitem";
static const gchar * data_menu =     "dbusmenugtk-data-gtkmenu";

/* This is the call back for the GTK widget for when it gets
   clicked on by the user to send it back across the bus. */
static gboolean
menu_pressed_cb (GtkMenuItem * gmi, DbusmenuMenuitem * mi)
{
	dbusmenu_menuitem_activate(mi);
	return TRUE;
}

/* Whenever we have a property change on a DbusmenuMenuitem
   we need to be responsive to that. */
static void
menu_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, gchar * value, GtkMenuItem * gmi)
{
	if (!g_strcmp0(prop, "label")) {
		gtk_menu_item_set_label(gmi, value);
		gtk_widget_show(GTK_WIDGET(gmi));
	}

	return;
}

/* Call back that happens when the DbusmenuMenuitem
   is destroyed.  We're making sure to clean up everything
   else down the pipe. */
static void
destoryed_dbusmenuitem_cb (gpointer udata, GObject * dbusmenuitem)
{
	/* g_debug("DbusmenuMenuitem was destroyed"); */
	gtk_widget_destroy(GTK_WIDGET(udata));
	return;
}

/* This takes a new DbusmenuMenuitem and attaches the
   various things that we need to make it work in a 
   GTK World.  */
static void
new_menuitem (DbusmenuClient * client, DbusmenuMenuitem * mi, gpointer userdata)
{
	gpointer ann_mi = g_object_get_data(G_OBJECT(mi), data_menuitem);
	GtkMenuItem * gmi = GTK_MENU_ITEM(ann_mi);

	if (gmi != NULL) {
		/* It's possible we've already been looked at, that's
		   okay, but we can just ignore this signal then. */
		return;
	}

	gmi = GTK_MENU_ITEM(gtk_menu_item_new());

	/* Attach these two */
	g_object_set_data(G_OBJECT(mi), data_menuitem, gmi);

	/* DbusmenuMenuitem signals */
	g_signal_connect(G_OBJECT(mi),  DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(menu_prop_change_cb), gmi);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED, G_CALLBACK(new_child), NULL);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(delete_child), NULL);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED, G_CALLBACK(move_child), NULL);

	/* GtkMenuitem signals */
	g_signal_connect(G_OBJECT(gmi), "activate", G_CALLBACK(menu_pressed_cb), mi);

	/* Life insurance */
	g_object_weak_ref(G_OBJECT(mi), destoryed_dbusmenuitem_cb, gmi);

	return;
}

static void
new_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position, gpointer userdata)
{
	gpointer ann_menu = g_object_get_data(G_OBJECT(mi), data_menu);
	GtkMenu * menu = GTK_MENU(ann_menu);
	if (menu == NULL) {
		/* Oh, we don't have a submenu, build one! */
		menu = GTK_MENU(gtk_menu_new());
		g_object_set_data(G_OBJECT(mi), data_menu, menu);

		GtkMenuItem * parent = dbusmenu_gtkclient_menuitem_get (mi);
		gtk_menu_item_set_submenu(parent, GTK_WIDGET(menu));
	} 

	GtkMenuItem * childmi  = dbusmenu_gtkclient_menuitem_get (child);
	gtk_menu_shell_insert(GTK_MENU_SHELL(menu), GTK_WIDGET(childmi), position);
	
	return;
}

static void
delete_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, gpointer userdata)
{
	if (g_list_length(dbusmenu_menuitem_get_children(mi)) == 0) {
		gpointer ann_menu = g_object_get_data(G_OBJECT(mi), data_menu);
		GtkMenu * menu = GTK_MENU(ann_menu);

		if (menu != NULL) {
			gtk_widget_destroy(GTK_WIDGET(menu));
			g_object_set_data(G_OBJECT(mi), data_menu, NULL);
		}
	}

	return;
}

static void
move_child (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint new, guint old, gpointer userdata)
{
	gpointer ann_menu = g_object_get_data(G_OBJECT(mi), data_menu);
	if (ann_menu == NULL) {
		g_warning("Moving a child when we don't have a submenu!");
		return;
	}

	GtkMenuItem * childmi  = dbusmenu_gtkclient_menuitem_get (child);
	gtk_menu_reorder_child(GTK_MENU(ann_menu), GTK_WIDGET(childmi), new);

	return;
}

/* Public API */

/**
	dbusmenu_gtkclient_new:
	@dbus_name: Name of the #DbusmenuServer on DBus
	@dbus_name: Name of the object on the #DbusmenuServer

	Creates a new #DbusmenuGtkClient object and creates a #DbusmenuClient
	that connects across DBus to a #DbusmenuServer.

	Return value: A new #DbusmenuGtkClient sync'd with a server
*/
DbusmenuGtkClient *
dbusmenu_gtkclient_new (gchar * dbus_name, gchar * dbus_object)
{
	return g_object_new(DBUSMENU_GTKCLIENT_TYPE,
	                    DBUSMENU_CLIENT_PROP_DBUS_OBJECT, dbus_object,
	                    DBUSMENU_CLIENT_PROP_DBUS_NAME, dbus_name,
	                    NULL);
}

/**
	dbusmenu_gtkclient_menuitem_get:
	@item: #DbusmenuMenuitem to get associated #GtkMenuItem on.

	This grabs the #GtkMenuItem that is associated with the
	#DbusmenuMenuitem.

	Return value: The #GtkMenuItem that can be played with.
*/
GtkMenuItem *
dbusmenu_gtkclient_menuitem_get (DbusmenuMenuitem * item)
{
	return GTK_MENU_ITEM(g_object_get_data(G_OBJECT(item), data_menuitem));
}

