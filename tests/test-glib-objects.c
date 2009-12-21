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
test_object_menuitem_props_string (void)
{
	/* Build a menu item */
	DbusmenuMenuitem * item = dbusmenu_menuitem_new();
	const GValue * out = NULL;

	/* Test to make sure it's a happy object */
	g_assert(item != NULL);

	/* Setting a string */
	dbusmenu_menuitem_property_set(item, "string", "value");
	out = dbusmenu_menuitem_property_get_value(item, "string");
	g_assert(out != NULL);
	g_assert(G_VALUE_TYPE(out) == G_TYPE_STRING);
	g_assert(!g_strcmp0(g_value_get_string(out), "value"));
	g_assert(!g_strcmp0(dbusmenu_menuitem_property_get(item, "string"), "value"));

	g_object_unref(item);

	return;
}

void
test_object_menuitem_props_int (void)
{
	/* Build a menu item */
	DbusmenuMenuitem * item = dbusmenu_menuitem_new();
	const GValue * out = NULL;

	/* Test to make sure it's a happy object */
	g_assert(item != NULL);

	/* Setting a string */
	dbusmenu_menuitem_property_set_int(item, "int", 12345);
	out = dbusmenu_menuitem_property_get_value(item, "int");
	g_assert(out != NULL);
	g_assert(G_VALUE_TYPE(out) == G_TYPE_INT);
	g_assert(g_value_get_int(out) == 12345);
	g_assert(dbusmenu_menuitem_property_get_int(item, "int") == 12345);

	g_object_unref(item);

	return;
}

void
test_object_menuitem_props_bool (void)
{
	/* Build a menu item */
	DbusmenuMenuitem * item = dbusmenu_menuitem_new();
	const GValue * out = NULL;

	/* Test to make sure it's a happy object */
	g_assert(item != NULL);

	/* Setting a string */
	dbusmenu_menuitem_property_set_bool(item, "boolean", TRUE);
	out = dbusmenu_menuitem_property_get_value(item, "boolean");
	g_assert(out != NULL);
	g_assert(G_VALUE_TYPE(out) == G_TYPE_BOOLEAN);
	g_assert(g_value_get_boolean(out));
	g_assert(dbusmenu_menuitem_property_get_int(item, "boolean"));

	g_object_unref(item);

	return;
}

void
test_object_menuitem_props_swap (void)
{
	/* Build a menu item */
	DbusmenuMenuitem * item = dbusmenu_menuitem_new();

	/* Test to make sure it's a happy object */
	g_assert(item != NULL);

	/* Setting a boolean */
	dbusmenu_menuitem_property_set_bool(item, "swapper", TRUE);
	g_assert(dbusmenu_menuitem_property_get_bool(item, "swapper"));

	/* Setting a int */
	dbusmenu_menuitem_property_set_int(item, "swapper", 5432);
	g_assert(dbusmenu_menuitem_property_get_int(item, "swapper") == 5432);

	/* Setting a string */
	dbusmenu_menuitem_property_set(item, "swapper", "mystring");
	g_assert(!g_strcmp0(dbusmenu_menuitem_property_get(item, "swapper"), "mystring"));

	/* Setting a boolean */
	dbusmenu_menuitem_property_set_bool(item, "swapper", FALSE);
	g_assert(!dbusmenu_menuitem_property_get_bool(item, "swapper"));

	g_object_unref(item);

	return;
}

void
test_glib_objects_suite (void)
{
	g_test_add_func ("/dbusmenu/glib/objects/menuitem/base",          test_object_menuitem);
	g_test_add_func ("/dbusmenu/glib/objects/menuitem/props_string",  test_object_menuitem_props_string);
	g_test_add_func ("/dbusmenu/glib/objects/menuitem/props_int",     test_object_menuitem_props_int);
	g_test_add_func ("/dbusmenu/glib/objects/menuitem/props_bool",    test_object_menuitem_props_bool);
	g_test_add_func ("/dbusmenu/glib/objects/menuitem/props_swap",    test_object_menuitem_props_swap);
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
