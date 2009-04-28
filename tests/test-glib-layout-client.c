#include <glib.h>

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-glib/menuitem.h>

#include "test-glib-layout.h"

static guint layouton = 0;
static GMainLoop * mainloop = NULL;
static gboolean passed = TRUE;

static gboolean
timer_func (gpointer data)
{
	g_debug("Death timer.  Oops.  Got to: %d", layouton);
	passed = FALSE;
	g_main_loop_quit(mainloop);
	return FALSE;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	DbusmenuClient * client = dbusmenu_client_new(":1", "/org/test");
	dbusmenu_client_get_root(client);

	g_timeout_add_seconds(2, timer_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	if (passed) {
		g_debug("Quiting");
		return 0;
	} else {
		g_debug("Quiting as we're a failure");
		return 0;
	}
}
