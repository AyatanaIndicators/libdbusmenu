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

#ifndef __DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_H__
#define __DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_H__

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
*/
struct _DbusmenuGtkSerializableMenuItem {
	GtkMenuItem parent;

	DbusmenuGtkSerializableMenuItemPrivate * priv;
};

GType dbusmenu_gtk_serializable_menu_item_get_type (void);

DbusmenuMenuitem *  dbusmenu_gtk_serializable_menu_item_build_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi);
void                dbusmenu_gtk_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type);
void                dbusmenu_gtk_serializable_menu_item_set_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi, DbusmenuMenuitem * mi);

G_END_DECLS

#endif
