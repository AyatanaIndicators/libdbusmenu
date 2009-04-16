#include <glib.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

int
main (int argc, char ** argv)
{
	g_type_init();

	DbusmenuServer * server = dbusmenu_server_new("/org/test");
	DbusmenuMenuitem * menuitem = dbusmenu_menuitem_new();
	dbusmenu_server_set_root(server, menuitem);

	g_main_loop_run(g_main_loop_new(NULL, FALSE));

	return 0;
}
