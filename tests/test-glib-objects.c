/*
Testing for the various objects just by themselves.

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
#include <glib-object.h>

#include <libdbusmenu-glib/menuitem.h>

void
test_object_menuitem (void)
{
	/* Build a menu item */
	DbusmenuMenuitem * item = dbusmenu_menuitem_new();

	/* Test to make sure it's a happy object */
	g_assert(item != NULL);
	g_assert(G_IS_OBJECT(item));
	g_assert(DBUSMENU_IS_MENUITEM(item));

	/* Set up a check to make sure it gets destroyed on unref */
	g_object_add_weak_pointer(G_OBJECT(item), (gpointer *)&item);
	g_object_unref(item);

	/* Did it go away? */
	g_assert(item == NULL);

	return;
}

void
test_object_menuitem_props (void)
{


}

void
test_glib_objects_suite (void)
{
	g_test_add_func ("/dbusmenu/glib/objects/menuitem",          test_object_menuitem);
	g_test_add_func ("/dbusmenu/glib/objects/menuitem-props",    test_object_menuitem_props);
	return;
}

gint
main (gint argc, gchar * argv[])
{
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_glib_objects_suite();


	return g_test_run ();
}
