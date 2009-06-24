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

	return;
}

static void
dbusmenu_gtkclient_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkclient_parent_class)->dispose (object);
	return;
}

static void process_layout_change (DbusmenuClient * client, DbusmenuGtkClient * gtkmenu);

static void
dbusmenu_gtkclient_finalize (GObject *object)
{
	process_layout_change(NULL, NULL);
	G_OBJECT_CLASS (dbusmenu_gtkclient_parent_class)->finalize (object);
	return;
}

/* Internal Functions */

static const gchar * data_menuitem = "dbusmenugtk-data-gtkmenuitem";
static const gchar * data_menu =     "dbusmenugtk-data-gtkmenu";

static gboolean
menu_pressed_cb (GtkMenuItem * gmi, DbusmenuMenuitem * mi)
{
	dbusmenu_menuitem_activate(mi);
	return TRUE;
}

static void
menu_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, gchar * value, GtkMenuItem * gmi)
{
	if (!g_strcmp0(prop, "label")) {
		gtk_menu_item_set_label(gmi, value);
		gtk_widget_show(GTK_WIDGET(gmi));
	}

	return;
}

static void
destoryed_dbusmenuitem_cb (gpointer udata, GObject * dbusmenuitem)
{
	/* g_debug("DbusmenuMenuitem was destroyed"); */
	gtk_widget_destroy(GTK_WIDGET(udata));
	return;
}

static void
connect_menuitem (DbusmenuMenuitem * mi, GtkMenuItem * gmi)
{
	g_object_set_data(G_OBJECT(mi), data_menuitem, gmi);

	g_signal_connect(G_OBJECT(mi),  DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(menu_prop_change_cb), gmi);
	g_signal_connect(G_OBJECT(gmi), "activate", G_CALLBACK(menu_pressed_cb), mi);

	g_object_weak_ref(G_OBJECT(mi), destoryed_dbusmenuitem_cb, gmi);

	return;
}

static void
process_dbusmenu_menuitem (DbusmenuMenuitem * mi, GtkMenu * parentmenu)
{
	gpointer unknown_menuitem = g_object_get_data(G_OBJECT(mi), data_menuitem);
	if (unknown_menuitem == NULL) {
		/* Oh, a virgin DbusmenuMenuitem, let's fix that. */
		GtkWidget * menuitem = gtk_menu_item_new();
		connect_menuitem(mi, GTK_MENU_ITEM(menuitem));
		unknown_menuitem = menuitem;
		gtk_menu_shell_append(GTK_MENU_SHELL(parentmenu), menuitem);
	}

	GList * children = dbusmenu_menuitem_get_children(mi);
	if (children == NULL) {
		/* If there are no children to process we are
		   done and we can move along */
		return;
	}

	/* Phase 0: Make a submenu if we don't have one */
	gpointer unknown_menu = g_object_get_data(G_OBJECT(mi), data_menu);
	if (unknown_menu == NULL) {
		GtkWidget * gtkmenu = gtk_menu_new();
		g_object_ref(gtkmenu);
		g_object_set_data_full(G_OBJECT(mi), data_menu, gtkmenu, g_object_unref);
		unknown_menu = gtkmenu;
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(unknown_menuitem), gtkmenu);
		gtk_widget_show(gtkmenu);
	}

	/* Phase 1: Add missing children */
	GList * child = NULL;
	for (child = children; child != NULL; child = g_list_next(child)) {
		process_dbusmenu_menuitem(DBUSMENU_MENUITEM(child->data), GTK_MENU(unknown_menu));	
	}

	/* Phase 2: Delete removed children */
	/* Actually, we don't need to do this because of the weak
	   reference that we've added above.  When the DbusmenuMenuitem
	   gets destroyed it takes its GtkMenuItem with it.  Bye bye. */

	/* Phase 3: Profit! */
	return;
}

/* Processing the layout being updated and handling
   that and making it into a menu */
static void
process_layout_change (DbusmenuClient * client, DbusmenuGtkClient * gtkmenu)
{
	DbusmenuMenuitem * root = dbusmenu_client_get_root(client);

	GList * children = dbusmenu_menuitem_get_children(root);
	if (children == NULL) {
		return;
	}

	GList * child = NULL;
	for (child = children; child != NULL; child = g_list_next(child)) {
		process_dbusmenu_menuitem(DBUSMENU_MENUITEM(child->data), GTK_MENU(gtkmenu));
	}

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

