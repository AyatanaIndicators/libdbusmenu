/*
A small tool to grab the dbusmenu structure that a program is
exporting.

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

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-glib/menuitem.h>

static GMainLoop * mainloop = NULL;

static void
print_menuitem (DbusmenuMenuitem * item, int depth)
{
	gchar * space = g_strnfill(depth, ' ');
	g_print("%s\"id\": %d", space, dbusmenu_menuitem_get_id(item));

	GList * properties = dbusmenu_menuitem_properties_list(item);
	GList * property;
	for (property = properties; property != NULL; property = g_list_next(property)) {
		g_print("\n%s\"%s\": %s", space, (gchar *)property->data, dbusmenu_menuitem_property_get(item, (gchar *)property->data));
	}
	g_list_free(properties);

	GList * children = dbusmenu_menuitem_get_children(item);
	if (children != NULL) {
		gchar * childspace = g_strnfill(depth + 4, ' ');
		g_print("\n%s\"submenu\": [\n%s{\n", space, childspace);
		GList * child;
		for (child = children; child != NULL; child = g_list_next(child)) {
			print_menuitem(DBUSMENU_MENUITEM(child->data), depth + 4 + 2);
			if (child->next != NULL) {
				g_print("\n%s},\n%s{\n", childspace, childspace);
			}
		}
		g_print("\n%s}\n%s]", childspace, space);
		g_free(childspace);
	}

	g_free(space);

	return;
}

static void
new_root_cb (DbusmenuClient * client, DbusmenuMenuitem * newroot)
{
	if (newroot == NULL) {
		g_printerr("ERROR: Unable to create Dbusmenu Root\n");
		g_main_loop_quit(mainloop);
		return;
	}

	g_print("{\n");
	print_menuitem(newroot, 2);
	g_print("\n}\n");

	g_main_quit(mainloop);
	return;
}


static gchar * dbusname = NULL;
static gchar * dbusobject = NULL;

static gboolean
option_dbusname (const gchar * arg, const gchar * value, gpointer data, GError ** error)
{
	if (dbusname != NULL) {
		g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "DBus name already set to '%s' can't reset it to '%s'.", dbusname, value);
		return FALSE;
	}

	dbusname = g_strdup(value);
	return TRUE;
}

static gboolean
option_dbusobject (const gchar * arg, const gchar * value, gpointer data, GError ** error)
{
	if (dbusobject != NULL) {
		g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "DBus name already set to '%s' can't reset it to '%s'.", dbusobject, value);
		return FALSE;
	}

	dbusobject = g_strdup(value);
	return TRUE;
}

void
usage (void)
{
	g_printerr("dbusmenu-dumper --dbus-name=<name> --dbus-object=<object>\n");
	return;
}

static GOptionEntry general_options[] = {
	{"dbus-name",     'd',  0,                        G_OPTION_ARG_CALLBACK,  option_dbusname, "The name of the program to connect to (i.e. org.test.bob", "dbusname"},
	{"dbus-object",   'o',  0,                        G_OPTION_ARG_CALLBACK,  option_dbusobject, "The path to the Dbus object (i.e /org/test/bob/alvin)", "dbusobject"}
};

int
main (int argc, char ** argv)
{
	g_type_init();
	GError * error = NULL;
	GOptionContext * context;

	context = g_option_context_new("- Grab the entires in a DBus Menu");

	g_option_context_add_main_entries(context, general_options, "dbusmenu-dumper");

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_printerr("option parsing failed: %s\n", error->message);
		g_error_free(error);
		return 1;
	}

	if (dbusname == NULL) {
		g_printerr("ERROR: dbus-name not specified\n");
		usage();
		return 1;
	}

	if (dbusobject == NULL) {
		g_printerr("ERROR: dbus-object not specified\n");
		usage();
		return 1;
	}

	DbusmenuClient * client = dbusmenu_client_new (dbusname, dbusobject);
	if (client == NULL) {
		g_printerr("ERROR: Unable to create Dbusmenu Client\n");
		return 1;
	}

	g_signal_connect(G_OBJECT(client), DBUSMENU_CLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(new_root_cb), NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}

