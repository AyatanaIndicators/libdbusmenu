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

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-glib/menuitem.h>

#include "test-glib-submenu.h"

static GMainLoop * mainloop = NULL;
static gboolean passed = TRUE;
static gboolean first = TRUE;

static void
event_status (DbusmenuClient * client, DbusmenuMenuitem * item, gchar * name, GValue * data, guint timestamp, GError * error, gpointer user_data)
{
	g_debug("Event status: %s", error == NULL ? "Sent" : "Error");

	if (first && error != NULL) {
		passed = FALSE;
		g_debug("First signal back failed.");
		g_main_loop_quit(mainloop);
		return;
	}

	if (!first && error == NULL) {
		passed = FALSE;
		g_debug("Second signal didn't fail.");
		g_main_loop_quit(mainloop);
		return;
	}

	if (!first && error != NULL) {
		g_debug("Second signal failed: pass.");
		g_main_loop_quit(mainloop);
		return;
	}

	first = FALSE;
	dbusmenu_menuitem_handle_event(item, "clicked", NULL, 0);
	return;
}

static void
layout_updated (DbusmenuClient * client, gpointer data)
{
	g_debug("Layout Updated");

	DbusmenuMenuitem * menuroot = dbusmenu_client_get_root(client);
	if (menuroot == NULL) {
		g_debug("Root is NULL?");
		return;
	}

	dbusmenu_menuitem_handle_event(menuroot, "clicked", NULL, 0);

	return;
}

static gboolean
timer_func (gpointer data)
{
	g_debug("Death timer.  Oops.");
	passed = FALSE;
	g_main_loop_quit(mainloop);
	return FALSE;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	DbusmenuClient * client = dbusmenu_client_new("org.dbusmenu.test", "/org/test");
	g_signal_connect(G_OBJECT(client), DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED, G_CALLBACK(layout_updated), NULL);
	g_signal_connect(G_OBJECT(client), DBUSMENU_CLIENT_SIGNAL_EVENT_RESULT, G_CALLBACK(event_status), NULL);

	g_timeout_add_seconds(5, timer_func, client);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(client));

	if (passed) {
		g_debug("Quiting");
		return 0;
	} else {
		g_debug("Quiting as we're a failure");
		return 1;
	}
}
