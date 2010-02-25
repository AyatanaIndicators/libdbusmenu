/*
A test for libdbusmenu to ensure its quality.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/server.h>

#include "test-glib-proxy.h"

static void
set_props (DbusmenuMenuitem * mi, gchar ** props)
{
	if (props == NULL) return;

	guint i;
	for (i = 0; props[i] != NULL; i += 2) {
		dbusmenu_menuitem_property_set(mi, props[i], props[i+1]);
	}

	return;
}

static DbusmenuMenuitem *
layout2menuitem (proplayout_t * layout)
{
	if (layout == NULL || layout->id == -1) return NULL;

	DbusmenuMenuitem * local = dbusmenu_menuitem_new();
	set_props(local, layout->properties);
	
	if (layout->submenu != NULL) {
		guint count;
		for (count = 0; layout->submenu[count].id != -1; count++) {
			DbusmenuMenuitem * child = layout2menuitem(&layout->submenu[count]);
			if (child != NULL) {
				dbusmenu_menuitem_child_append(local, child);
			}
		}
	}

	/* g_debug("Layout to menu return: 0x%X", (unsigned int)local); */
	return local;
}

static guint layouton = 0;
static DbusmenuServer * server = NULL;
static GMainLoop * mainloop = NULL;
static guint death_timer = 0;

static gboolean
timer_func (gpointer data)
{
	g_debug("Death timer.  Oops.  Got to: %d", layouton);
	g_main_loop_quit(mainloop);
	return FALSE;
}

static void
layout_change (DbusmenuMenuitem * oldroot, guint timestamp, gpointer data)
{
	if (layouts[layouton].id == -1) {
		g_main_loop_quit(mainloop);
		return;
	}
	g_debug("Updating to Layout %d", layouton);

	DbusmenuMenuitem * mi = layout2menuitem(&layouts[layouton]);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(layout_change), NULL);
	dbusmenu_menuitem_property_set_int(mi, LAYOUT_ON, layouton);
	dbusmenu_server_set_root(server, mi);
	g_object_unref(G_OBJECT(mi));
	layouton++;

	/* Extend our death */
	if (death_timer != 0) {
		g_source_remove(death_timer);
	}
	death_timer = g_timeout_add_seconds(4, timer_func, data);

	return;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	GError * error = NULL;
	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);

	g_debug("DBus ID: %s", dbus_connection_get_server_id(dbus_g_connection_get_connection(connection)));

	DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	guint nameret = 0;

	if (!org_freedesktop_DBus_request_name(bus_proxy, "test.proxy.server", 0, &nameret, &error)) {
		g_error("Unable to call to request name");
		return 1;
	}

	if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error("Unable to get name");
		return 1;
	}

	server = dbusmenu_server_new("/org/test");
	layout_change(NULL, 0, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(server));
	g_debug("Quiting");

	return 0;
}

