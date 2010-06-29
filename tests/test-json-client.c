#include <glib.h>
#include <gio/gio.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

GMainLoop * mainloop = NULL;

int
main (int argv, char ** argc)
{
	g_type_init();
	g_debug("Wait for friends");

	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		return 1;
	}   

	DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	gboolean has_owner = FALSE;
	gint owner_count = 0;
	while (!has_owner && owner_count < 10000) {
		org_freedesktop_DBus_name_has_owner(bus_proxy, "org.dbusmenu.test", &has_owner, NULL);
		owner_count++;
	}

	if (owner_count == 10000) {
		g_error("Unable to get name owner after 10000 tries");
		return 1;
	}

	g_usleep(500000);

	g_debug("Initing");

	gchar * command = g_strdup_printf("%s --dbus-name=org.dbusmenu.test --dbus-object=/org/test", argc[1]);
	g_debug("Executing: %s", command);

	gchar * output;
	g_spawn_command_line_sync(command, &output, NULL, NULL, NULL);

	GFile * ofile = g_file_new_for_commandline_arg(argc[2]);
	if (ofile != NULL) {
		g_file_replace_contents(ofile, output, g_utf8_strlen(output, -1), NULL, FALSE, 0, NULL, NULL, NULL);
	}

	g_debug("Exiting");

	return 0;
}
