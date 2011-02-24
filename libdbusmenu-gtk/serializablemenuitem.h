/*
An object to act as a base class for easy GTK widgets that can be
transfered over dbusmenu.

Copyright 2011 Canonical Ltd.

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

#ifndef DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_H__
#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_H__ 1

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/client.h>

G_BEGIN_DECLS

#define DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM            (dbusmenu_gtk_serializable_menu_item_get_type ())
#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM, DbusmenuGtkSerializableMenuItem))
#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM, DbusmenuGtkSerializableMenuItemClass))
#define DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM))
#define DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM))
#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM, DbusmenuGtkSerializableMenuItemClass))

/**
 * DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_PROP_MENUITEM:
 *
 * String to access property #DbusmenuGtkSerializableMenuItem:dbusmenu-menuitem
 */
#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_PROP_MENUITEM   "dbusmenu-menuitem"

typedef struct _DbusmenuGtkSerializableMenuItem        DbusmenuGtkSerializableMenuItem;
typedef struct _DbusmenuGtkSerializableMenuItemClass   DbusmenuGtkSerializableMenuItemClass;
typedef struct _DbusmenuGtkSerializableMenuItemPrivate DbusmenuGtkSerializableMenuItemPrivate;

/**
	DbusmenuGtkSerializableMenuItemClass:
	@parent_class: Inherit from GtkMenuItem
	@get_type_string: Static function to get a string describing this type
	@get_default_properties: Return a hashtable of defaults for the menu item type
	@build_dbusmenu_menuitem: Build a menuitem that can be sent over dbus
	@_dbusmenu_gtk_serializable_menu_item_reserved1: Reserved for future use.
	@_dbusmenu_gtk_serializable_menu_item_reserved2: Reserved for future use.
	@_dbusmenu_gtk_serializable_menu_item_reserved3: Reserved for future use.
	@_dbusmenu_gtk_serializable_menu_item_reserved4: Reserved for future use.
	@_dbusmenu_gtk_serializable_menu_item_reserved5: Reserved for future use.
	@_dbusmenu_gtk_serializable_menu_item_reserved6: Reserved for future use.

	Signals and functions for #DbusmenuGtkSerializableMenuItem.
*/
struct _DbusmenuGtkSerializableMenuItemClass {
	GtkMenuItemClass parent_class;

	/* Subclassable functions */
	const gchar *        (*get_type_string)          (void);
	GHashTable *         (*get_default_properties)   (void);

	DbusmenuMenuitem *   (*build_dbusmenu_menuitem)    (DbusmenuGtkSerializableMenuItem * smi);

	/* Signals */



	/* Empty Space */
	/*< Private >*/
	void (*_dbusmenu_gtk_serializable_menu_item_reserved1) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved2) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved3) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved4) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved5) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved6) (void);
};

/**
	DbusmenuGtkSerializableMenuItem:
	@parent: Inherit from GtkMenuItem
	@priv: Blind structure of private variables

	The Serializable Menuitem provides a way for menu items to be created
	that can easily be picked up by the Dbusmenu GTK Parser.  This way
	you can create custom items, and transport them across dbusmenu to
	your menus or the appmenu on the other side of the bus.  By providing
	these function the parser has enough information to both serialize, and
	deserialize on the other side, the menuitem you've so carefully created.
*/
struct _DbusmenuGtkSerializableMenuItem {
	GtkMenuItem parent;

	DbusmenuGtkSerializableMenuItemPrivate * priv;
};

GType dbusmenu_gtk_serializable_menu_item_get_type (void);

DbusmenuMenuitem *  dbusmenu_gtk_serializable_menu_item_build_menuitem (DbusmenuGtkSerializableMenuItem * smi);
void                dbusmenu_gtk_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type);
void                dbusmenu_gtk_serializable_menu_item_set_menuitem (DbusmenuGtkSerializableMenuItem * smi, DbusmenuMenuitem * mi);

/**
	SECTION:serializablemenuitem
	@short_description: A way to build #GtkMenuItems that can be sent over Dbusmenu
	@stability: Unstable
	@include: libdbusmenu-gtk/serializablemenuitem.h

	Menuitems can subclass from this instead of #GtkMenuItem and
	by providing the appropriate functions Dbusmenu will be able
	to parse them and send them over the bus.
*/

G_END_DECLS

#endif
