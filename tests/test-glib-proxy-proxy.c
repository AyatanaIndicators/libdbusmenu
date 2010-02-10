#include <glib.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/menuitem-proxy.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/client.h>

#include "test-glib-proxy.h"

static DbusmenuServer * server = NULL;
static DbusmenuClient * client = NULL;
static GMainLoop * mainloop = NULL;

void
root_changed (DbusmenuClient * client, DbusmenuMenuitem * newroot, gpointer user_data)
{
	g_debug("New root: %X", (guint)newroot);

	if (newroot == NULL) {
		g_debug("Root removed, exiting");
		g_main_loop_quit(mainloop);
		return;
	}

	DbusmenuMenuitemProxy * pmi = dbusmenu_menuitem_proxy_new(newroot);
	dbusmenu_server_set_root(server, DBUSMENU_MENUITEM(pmi));
	return;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	if (argc != 3) {
		g_error ("Need two params");
		return 1;
	}
	
	gchar * whoami = argv[1];
	gchar * myproxy = argv[2];

	g_debug("I am '%s' and I'm proxying '%s'", whoami, myproxy);

	GError * error = NULL;
	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);

	g_debug("DBus ID: %s", dbus_connection_get_server_id(dbus_g_connection_get_connection(connection)));

	DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	guint nameret = 0;

	if (!org_freedesktop_DBus_request_name(bus_proxy, whoami, 0, &nameret, &error)) {
		g_error("Unable to call to request name");
		return 1;
	}

	if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error("Unable to get name");
		return 1;
	}

	server = dbusmenu_server_new("/org/test");
	client = dbusmenu_client_new(myproxy, "/org/test");

	g_signal_connect(client, DBUSMENU_CLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(root_changed), server);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(server));
	g_debug("Quiting");

	return 0;
}
