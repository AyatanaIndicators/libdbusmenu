/*
A library to take the object model made consistent by libdbusmenu-glib
and visualize it in GTK.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the 
Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by 
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the applicable version of the GNU Lesser General Public 
License for more details.

You should have received a copy of both the GNU Lesser General Public 
License version 3 and version 2.1 along with this program.  If not, see 
<http://www.gnu.org/licenses/>
*/

#include "menuitem.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>

/**
	dbusmenu_menuitem_property_set_image:
	@menuitem: The #DbusmenuMenuitem to set the property on.
	@property: Name of the property to set.
	@data: The image to place on the property.

	This function takes the pixbuf that is stored in @data and
	turns it into a base64 encoded PNG so that it can be placed
	onto a standard #DbusmenuMenuitem property.

	Return value: Whether the function was able to set the property
		or not.
*/
gboolean
dbusmenu_menuitem_property_set_image (DbusmenuMenuitem * menuitem, const gchar * property, const GdkPixbuf * data)
{
	g_return_val_if_fail(GDK_IS_PIXBUF(data), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(menuitem), FALSE);
	g_return_val_if_fail(property != NULL && property[0] != '\0', FALSE);

	GError * error = NULL;
	gchar * png_data;
	gsize png_data_len;

	if (!gdk_pixbuf_save_to_buffer((GdkPixbuf *)data, &png_data, &png_data_len, "png", &error, NULL)) {
		if (error == NULL) {
			g_warning("Unable to create pixbuf data stream: %d", (gint)png_data_len);
		} else {
			g_warning("Unable to create pixbuf data stream: %s", error->message);
			g_error_free(error);
			error = NULL;
		}

		return FALSE;
	}

	gchar * prop_str = g_base64_encode((guchar *)png_data, png_data_len);
	gboolean propreturn = FALSE;
	propreturn = dbusmenu_menuitem_property_set(menuitem, property, prop_str);

	g_free(prop_str);
	g_free(png_data);

	return propreturn;
}

/**
	dbusmenu_menuitem_property_get_image:
	@menuitem: The #DbusmenuMenuite to look for the property on
	@property: The name of the property to look for.

	This function looks on the menu item for a property by the
	name of @property.  If one exists it tries to turn it into
	a #GdkPixbuf.  It assumes that the property is a base64 encoded
	PNG file like the one created by #dbusmenu_menuite_property_set_image.

	Return value: A pixbuf or #NULL to signal error.
*/
GdkPixbuf *
dbusmenu_menuitem_property_get_image (DbusmenuMenuitem * menuitem, const gchar * property)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(menuitem), NULL);
	g_return_val_if_fail(property != NULL && property[0] != '\0', NULL);

	const gchar * value = dbusmenu_menuitem_property_get(menuitem, property);

	/* There is no icon */
	if (value == NULL || value[0] == '\0') {
		return NULL;
	}

	gsize length = 0;
	guchar * icondata = g_base64_decode(value, &length);
	
	GInputStream * input = g_memory_input_stream_new_from_data(icondata, length, NULL);
	if (input == NULL) {
		g_warning("Cound not create input stream from icon property data");
		g_free(icondata);
		return NULL;
	}

	GError * error = NULL;
	GdkPixbuf * icon = gdk_pixbuf_new_from_stream(input, NULL, &error);

	if (error != NULL) {
		g_warning("Unable to build Pixbuf from icon data: %s", error->message);
		g_error_free(error);
	}

	error = NULL;
	g_input_stream_close(input, NULL, &error);
	if (error != NULL) {
		g_warning("Unable to close input stream: %s", error->message);
		g_error_free(error);
	}
	g_free(icondata);

	return icon;
}

/**
	dbusmenu_menuitem_property_set_shortcut_string:
	@menuitem: The #DbusmenuMenuitem to set the shortcut on
	@shortcut: String describing the shortcut

	This function takes a GTK shortcut string as defined in
	#gtk_accelerator_parse and turns that into the information
	required to send it over DBusmenu.

	Return value: Whether it was successful at setting the property.
*/
gboolean
dbusmenu_menuitem_property_set_shortcut_string (DbusmenuMenuitem * menuitem, const gchar * shortcut)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(menuitem), FALSE);
	g_return_val_if_fail(shortcut != NULL, FALSE);

	guint key = 0;
	GdkModifierType modifier = 0;

	gtk_accelerator_parse(shortcut, &key, &modifier);

	if (key == 0) {
		g_warning("Unable to parse shortcut string '%s'", shortcut);
		return FALSE;
	}

	return dbusmenu_menuitem_property_set_shortcut(menuitem, key, modifier);
}

/**
	dbusmenu_menuitem_property_set_shortcut:
	@menuitem: The #DbusmenuMenuitem to set the shortcut on
	@key: The keycode of the key to send
	@modifier: A bitmask of modifiers used to activate the item

	Takes the modifer described by @key and @modifier and places that into
	the format sending across Dbus for shortcuts.

	Return value: Whether it was successful at setting the property.
*/
gboolean
dbusmenu_menuitem_property_set_shortcut (DbusmenuMenuitem * menuitem, guint key, GdkModifierType modifier)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(menuitem), FALSE);

	GPtrArray * array = g_ptr_array_new();

	if (modifier & GDK_CONTROL_MASK) {
		g_ptr_array_add(array, g_strdup(DBUSMENU_MENUITEM_SHORTCUT_CONTROL));
	}
	if (modifier & GDK_MOD1_MASK) {
		g_ptr_array_add(array, g_strdup(DBUSMENU_MENUITEM_SHORTCUT_ALT));
	}
	if (modifier & GDK_SHIFT_MASK) {
		g_ptr_array_add(array, g_strdup(DBUSMENU_MENUITEM_SHORTCUT_SHIFT));
	}
	if (modifier & GDK_SUPER_MASK) {
		g_ptr_array_add(array, g_strdup(DBUSMENU_MENUITEM_SHORTCUT_SUPER));
	}

	g_ptr_array_add(array, g_strdup(gdk_keyval_name(key)));

	GPtrArray * wrapper = g_ptr_array_new();
	g_ptr_array_add(wrapper, array);

	GValue value = {0};
	g_value_init(&value, G_TYPE_BOXED);
	g_value_set_boxed(&value, wrapper);

	dbusmenu_menuitem_property_set_value(menuitem, DBUSMENU_MENUITEM_PROP_SHORTCUT, &value);

	return TRUE;
}

/* Look at the closures in an accel group and find
   the one that matches the one we've been passed */
static gboolean
find_closure (GtkAccelKey * key, GClosure * closure, gpointer user_data)
{
	return closure == user_data;
}

/**
	dbusmenu_menuitem_property_set_shortcut_menuitem:
	@menuitem: The #DbusmenuMenuitem to set the shortcut on
	@gmi: A menu item to steal the shortcut off of

	Takes the shortcut that is installed on a menu item and calls
	#dbusmenu_menuitem_property_set_shortcut with it.  It also sets
	up listeners to watch it change.

	Return value: Whether it was successful at setting the property.
*/
gboolean
dbusmenu_menuitem_property_set_shortcut_menuitem (DbusmenuMenuitem * menuitem, const GtkMenuItem * gmi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(menuitem), FALSE);
	g_return_val_if_fail(GTK_IS_MENU_ITEM(gmi), FALSE);

	GClosure * closure = NULL;
	GList * clist;

	clist = gtk_widget_list_accel_closures(GTK_WIDGET(gmi));
	if (clist == NULL) {
		g_warning("Menuitem does not have any closures.");
		return FALSE;
	}

	closure = (GClosure *)clist->data;
	g_list_free(clist);

	GtkAccelGroup * group = gtk_accel_group_from_accel_closure(closure);
	
	/* Seriously, if this returns NULL something is seriously
	   wrong in GTK. */
	g_return_val_if_fail(group != NULL, FALSE);

	GtkAccelKey * key = gtk_accel_group_find(group, find_closure, closure);
	/* Again, not much we can do except complain loudly. */
	g_return_val_if_fail(key != NULL, FALSE);

	return dbusmenu_menuitem_property_set_shortcut(menuitem, key->accel_key, key->accel_mods);
}

/**
	dbusmenu_menuitem_property_get_shortcut:
	@menuitem: The #DbusmenuMenuitem to get the shortcut off
	@key: Location to put the key value
	@modifier: Location to put the modifier mask

	This function gets a GTK shortcut as a key and a mask
	for use to set the accelerators.
*/
void
dbusmenu_menuitem_property_get_shortcut (DbusmenuMenuitem * menuitem, guint * key, GdkModifierType * modifier)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(menuitem));

	return;
}
