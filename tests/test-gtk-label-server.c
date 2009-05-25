#include <glib.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/server.h>

#include "test-gtk-label.h"

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
	if (layout == NULL || layout->id == 0) return NULL;

	DbusmenuMenuitem * local = dbusmenu_menuitem_new_with_id(layout->id);
	set_props(local, layout->properties);
	
	if (layout->submenu != NULL) {
		guint count;
		for (count = 0; layout->submenu[count].id != 0; count++) {
			DbusmenuMenuitem * child = layout2menuitem(&layout->submenu[count]);
			if (child != NULL) {
				dbusmenu_menuitem_child_append(local, child);
			}
		}
	}

	g_debug("Layout to menu return: 0x%X", (unsigned int)local);
	return local;
}

static guint layouton = 0;
static DbusmenuServer * server = NULL;
static GMainLoop * mainloop = NULL;

static gboolean
timer_func (gpointer data)
{
	if (layouts[layouton].id == 0) {
		g_main_loop_quit(mainloop);
		return FALSE;
	}
	g_debug("Updating to Layout %d", layouton);

	dbusmenu_server_set_root(server, layout2menuitem(&layouts[layouton]));
	layouton++;

	return TRUE;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	g_debug("DBus ID: %s", dbus_connection_get_server_id(dbus_g_connection_get_connection(dbus_g_bus_get(DBUS_BUS_SESSION, NULL))));

	DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	GError * error = NULL;
	guint nameret = 0;

	if (!org_freedesktop_DBus_request_name(bus_proxy, "glib.label.test", 0, &nameret, &error)) {
		g_error("Unable to call to request name");
		return 1;
	}

	if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error("Unable to get name");
		return 1;
	}

	server = dbusmenu_server_new("/org/test");

	timer_func(NULL);
	g_timeout_add(2500, timer_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_debug("Quiting");

	return 0;
}

