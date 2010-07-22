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

static GMainLoop *mainloop = NULL;

static gboolean
timer_func (gpointer data)
{
  g_main_loop_quit (mainloop);

  return FALSE;
}

DbusmenuMenuitem *
add_item(DbusmenuMenuitem * parent, const char * label)
{
	DbusmenuMenuitem * item = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(item, "label", label);
	dbusmenu_menuitem_child_append(parent, item);
	return item;
}

int
main (int argc, char ** argv)
{
	GError * error = NULL;

	g_type_init();

	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	g_debug("DBus ID: %s", dbus_connection_get_server_id(dbus_g_connection_get_connection(dbus_g_bus_get(DBUS_BUS_SESSION, NULL))));

	DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	guint nameret = 0;

	if (!org_freedesktop_DBus_request_name(bus_proxy, "glib.label.test", 0, &nameret, &error)) {
		g_error("Unable to call to request name");
		return 1;
	}

	if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error("Unable to get name");
		return 1;
	}

	DbusmenuServer * server = dbusmenu_server_new("/org/test");
	DbusmenuMenuitem * root = dbusmenu_menuitem_new();
	dbusmenu_server_set_root(server, root);

	DbusmenuMenuitem * item;
	item = add_item(root, "Folder 1");
	add_item(item, "1.1");
	add_item(item, "1.2");
	add_item(item, "1.3");

	item = add_item(root, "Folder 2");
	add_item(item, "2.1");
	add_item(item, "2.2");
	add_item(item, "2.3");

        g_timeout_add_seconds(3, timer_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_debug("Quiting");

	return 0;
}

