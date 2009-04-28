
#include <glib.h>

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

	dbusmenu_server_set_root(server, layout2menuitem(&layouts[layouton++]));

	return TRUE;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	server = dbusmenu_server_new("/org/test");

	timer_func(NULL);
	g_timeout_add(100, timer_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
