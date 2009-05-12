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

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "test-glib-layout.h"


static DbusmenuMenuitem *
layout2menuitem (layout_t * layout)
{
	if (layout == NULL || layout->id == 0) return NULL;

	DbusmenuMenuitem * local = dbusmenu_menuitem_new_with_id(layout->id);
	
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

	g_debug("DBus ID: %s", dbus_connection_get_server_id(dbus_g_connection_get_connection(dbus_g_bus_get(DBUS_BUS_SESSION, NULL))));

	server = dbusmenu_server_new("/org/test");

	timer_func(NULL);
	g_timeout_add(2500, timer_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_debug("Quiting");

	return 0;
}
