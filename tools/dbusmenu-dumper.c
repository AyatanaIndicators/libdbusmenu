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
#include <dbus/dbus-glib.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-glib/menuitem.h>

#include <dbus/dbus-gtype-specialized.h>
#include <X11/Xlib.h>

static GMainLoop * mainloop = NULL;

static gchar * value2string (const GValue * value, int depth);

static gchar *
strv_dumper(const GValue * value)
{
	gchar ** strv = (gchar **)g_value_get_boxed(value);

	gchar * joined = g_strjoinv("\", \"", strv);
	gchar * retval = g_strdup_printf("[\"%s\"]", joined);
	g_free(joined);
	return retval;
}

typedef struct _collection_iterator_t collection_iterator_t;
struct _collection_iterator_t {
	gchar * space;
	GPtrArray * array;
	gboolean first;
	int depth;
};

static void
collection_iterate (const GValue * value, gpointer user_data)
{
	collection_iterator_t * iter = (collection_iterator_t *)user_data;

	gchar * str = value2string(value, iter->depth);
	gchar * retval = NULL;

	if (iter->first) {
		iter->first = FALSE;
		retval = g_strdup_printf("\n%s%s", iter->space, str);
	} else {
		retval = g_strdup_printf(",\n%s%s", iter->space, str);
	}

	g_ptr_array_add(iter->array, retval);
	g_free(str);

	return;
}

static gchar *
collection_dumper (const GValue * value, int depth)
{
	gchar * space = g_strnfill(depth, ' ');
	GPtrArray * array = g_ptr_array_new_with_free_func(g_free);

	g_ptr_array_add(array, g_strdup("["));

	collection_iterator_t iter;
	iter.space = space;
	iter.array = array;
	iter.first = TRUE;
	iter.depth = depth + 2;

	dbus_g_type_collection_value_iterate(value, collection_iterate, &iter);

	g_ptr_array_add(array, g_strdup_printf("\n%s]", space));

	g_free(space);

	gchar * retstr = NULL;
	if (array->len == 3) {
		retstr = g_strdup_printf("[%s]", ((gchar *)array->pdata[1]) + depth + 1/*for newline*/);
	} else {
		retstr = g_strjoinv(NULL, (gchar **)array->pdata);
	}

	g_ptr_array_free(array, TRUE);

	return retstr;
}

static gchar *
value2string (const GValue * value, int depth)
{
	gchar * str = NULL;

	if (value == NULL) {
		return g_strdup("(null)");
	}

	if (dbus_g_type_is_collection(G_VALUE_TYPE(value))) {
		str = collection_dumper(value, depth);
	} else if (G_VALUE_TYPE(value) == G_TYPE_STRV) {
		str = strv_dumper(value);
	} else if (G_VALUE_TYPE(value) == G_TYPE_BOOLEAN) {
		if (g_value_get_boolean(value)) {
			str = g_strdup("true");
		} else {
			str = g_strdup("false");
		}
	} else {
		str = g_strdup_value_contents(value);
	}

	return str;
}

static void
print_menuitem (DbusmenuMenuitem * item, int depth)
{
	gchar * space = g_strnfill(depth, ' ');
	g_print("%s\"id\": %d", space, dbusmenu_menuitem_get_id(item));

	GList * properties = dbusmenu_menuitem_properties_list(item);
	GList * property;
	for (property = properties; property != NULL; property = g_list_next(property)) {
		const GValue * value = dbusmenu_menuitem_property_get_value(item, (gchar *)property->data);
		gchar * str = value2string(value, depth + g_utf8_strlen((gchar *)property->data, -1) + 2 /*quotes*/ + 2 /*: */);
		g_print(",\n%s\"%s\": %s", space, (gchar *)property->data, str);
		g_free(str);
	}
	g_list_free(properties);

	GList * children = dbusmenu_menuitem_get_children(item);
	if (children != NULL) {
		gchar * childspace = g_strnfill(depth + 4, ' ');
		g_print(",\n%s\"submenu\": [\n%s{\n", space, childspace);
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

static gboolean
root_timeout (gpointer data)
{
	DbusmenuMenuitem * newroot = DBUSMENU_MENUITEM(data);

	g_print("{\n");
	print_menuitem(newroot, 2);
	g_print("\n}\n");

	g_main_quit(mainloop);
	return FALSE;
}

static void
new_root_cb (DbusmenuClient * client, DbusmenuMenuitem * newroot)
{
	if (newroot == NULL) {
		g_printerr("ERROR: Unable to create Dbusmenu Root\n");
		g_main_loop_quit(mainloop);
		return;
	}

	g_timeout_add_seconds(2, root_timeout, newroot);
	return;
}

/* Window clicking ***************************************************/
static GdkFilterReturn
click_filter (GdkXEvent *gdk_xevent,
              GdkEvent  *event,
              gpointer   data);

static Window
find_real_window (Window w, int depth)
{
	if (depth > 5) {
		return None;
	}
	/*static*/ Atom wm_state = XInternAtom(gdk_display, "WM_STATE", False);
	Atom type;
	int format;
	unsigned long nitems, after;
	unsigned char* prop;
	if (XGetWindowProperty(gdk_display, w, wm_state, 0, 0, False, AnyPropertyType,
				&type, &format, &nitems, &after, &prop) == Success) {
		if (prop != NULL) {
			XFree(prop);
		}
		if (type != None) {
			return w;
		}
	}
	Window root, parent;
	Window* children;
	unsigned int nchildren;
	Window ret = None;
	if (XQueryTree(gdk_display, w, &root, &parent, &children, &nchildren) != 0) {
		unsigned int i;
		for(i = 0; i < nchildren && ret == None; ++i) {
			ret = find_real_window(children[ i ], depth + 1);
		}
		if (children != NULL) {
			XFree(children);
		}
	}
	return ret;
}

static Window
get_window_under_cursor (void)
{
	Window root;
	Window child;
	uint mask;
	int rootX, rootY, winX, winY;
	XQueryPointer(gdk_display, gdk_x11_get_default_root_xwindow(), &root, &child, &rootX, &rootY, &winX, &winY, &mask);
	if (child == None) {
		return None;
	}
	return find_real_window(child, 0);
}

static void
uninstall_click_filter (void)
{
	GdkWindow *root;

	root = gdk_get_default_root_window ();
	gdk_window_remove_filter (root, (GdkFilterFunc) click_filter, NULL);

	gdk_pointer_ungrab (GDK_CURRENT_TIME);
	gdk_keyboard_ungrab (GDK_CURRENT_TIME);

	gtk_main_quit ();
}

static GdkFilterReturn
click_filter (GdkXEvent *gdk_xevent,
              GdkEvent  *event,
              gpointer   data)

{
	XEvent *xevent = (XEvent *) gdk_xevent;
	gboolean *success = (gboolean *)data;

	switch (xevent->type) {
	case ButtonPress:
		uninstall_click_filter();
		*success = TRUE;
		return GDK_FILTER_REMOVE;
	case KeyPress:
		if (xevent->xkey.keycode == XKeysymToKeycode(gdk_display, XK_Escape)) {
			uninstall_click_filter();
			*success = FALSE;
			return GDK_FILTER_REMOVE;
		}
		break;
	default:
		break;
	}

	return GDK_FILTER_CONTINUE;
}

static gboolean
install_click_filter (gpointer data)
{
	GdkGrabStatus  status;
	GdkCursor     *cross;
	GdkWindow     *root;

	root = gdk_get_default_root_window();

	gdk_window_add_filter(root, (GdkFilterFunc) click_filter, data);

	cross = gdk_cursor_new(GDK_CROSS);
	status = gdk_pointer_grab(root, FALSE, GDK_BUTTON_PRESS_MASK,
	                          NULL, cross, GDK_CURRENT_TIME);
	gdk_cursor_unref(cross);

	if (status != GDK_GRAB_SUCCESS) {
		g_warning("Pointer grab failed.\n");
		uninstall_click_filter();
		return FALSE;
	}

	status = gdk_keyboard_grab(root, FALSE, GDK_CURRENT_TIME);
	if (status != GDK_GRAB_SUCCESS) {
		g_warning("Keyboard grab failed.\n");
		uninstall_click_filter();
		return FALSE;
	}

	gdk_flush();
	return FALSE;
}

static gboolean
wait_for_click (void)
{
	gboolean success;
	g_idle_add (install_click_filter, (gpointer)(&success));
	gtk_main ();
	return success;
}

static gchar * dbusname = NULL;
static gchar * dbusobject = NULL;

static gboolean
init_dbus_vars_from_window(Window window)
{
	DBusGConnection *connection;
	GError *error;
	DBusGProxy *proxy;

	error = NULL;
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		g_printerr("Failed to open connection to bus: %s\n", error->message);
		g_error_free(error);
		return FALSE;
	}

	proxy = dbus_g_proxy_new_for_name (connection,
			"org.ayatana.AppMenu.Registrar",
			"/org/ayatana/AppMenu/Registrar",
			"org.ayatana.AppMenu.Registrar");

	error = NULL;
	if (!dbus_g_proxy_call (proxy, "GetMenuForWindow", &error,
		G_TYPE_UINT, window, G_TYPE_INVALID, 
		G_TYPE_STRING, &dbusname, DBUS_TYPE_G_OBJECT_PATH, &dbusobject, G_TYPE_INVALID))
	{
		g_printerr("ERROR: %s\n", error->message);
		g_error_free(error);
        g_object_unref(proxy);
		return FALSE;
	}

	if (!g_strcmp0(dbusobject, "/")) {
		return FALSE;
	}

	g_object_unref (proxy);

	return TRUE;
}

/* Option parser *****************************************************/
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
	{"dbus-object",   'o',  0,                        G_OPTION_ARG_CALLBACK,  option_dbusobject, "The path to the Dbus object (i.e /org/test/bob/alvin)", "dbusobject"},
	{NULL}
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

	if (dbusname == NULL && dbusobject == NULL) {
		gtk_init(&argc, &argv);
		if (!wait_for_click()) {
			return 1;
		}
		Window window = get_window_under_cursor();
		if (window == None) {
			g_printerr("ERROR: could not get the id for the pointed window\n");
			return 1;
		}
		g_debug("window: %u", (unsigned int)window);
		if (!init_dbus_vars_from_window(window)) {
			g_printerr("ERROR: could not find a menu for the pointed window\n");
			return 1;
		}
		g_debug("dbusname: %s, dbusobject: %s", dbusname, dbusobject);
	} else {
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

