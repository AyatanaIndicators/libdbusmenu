/*
Testing for the various objects just by themselves.

Copyright 2011 Canonical Ltd.

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

#include <libdbusmenu-gtk/parser.h>

/* Just makes sure we can connect here people */
static void
test_parser_runs (void)
{
	GtkWidget * gmi = gtk_menu_item_new_with_label("Test Item");
	g_assert(gmi != NULL);
	DbusmenuMenuitem * mi = dbusmenu_gtk_parse_menu_structure(gmi);
	g_assert(mi != NULL);

	g_object_unref(gmi);
	g_object_unref(mi);

	return;
}

/* Build the test suite */
static void
test_gtk_parser_suite (void)
{
	g_test_add_func ("/dbusmenu/gtk/parser/base",          test_parser_runs);
	return;
}

gint
main (gint argc, gchar * argv[])
{
	gtk_init(&argc, &argv);
	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_gtk_parser_suite();


	return g_test_run ();
}
