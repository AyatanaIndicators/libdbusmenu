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

typedef struct _DbusmenuGtkSerializableMenuItem        DbusmenuGtkSerializableMenuItem;
typedef struct _DbusmenuGtkSerializableMenuItemClass   DbusmenuGtkSerializableMenuItemClass;
typedef struct _DbusmenuGtkSerializableMenuItemPrivate DbusmenuGtkSerializableMenuItemPrivate;

struct _DbusmenuGtkSerializableMenuItemClass {
	GtkMenuItemClass parent_class;

	/* Subclassable functions */
	const gchar *        (*get_type_string)          (void);
	GHashTable *         (*get_default_properties)   (void);

	DbusmenuMenuitem *   (*get_dbusmenu_menuitem)    (DbusmenuGtkSerializableMenuItem * smi);

	/* Signals */



	/* Empty Space */
	void (*_dbusmenu_gtk_serializable_menu_item_reserved1) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved2) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved3) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved4) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved5) (void);
	void (*_dbusmenu_gtk_serializable_menu_item_reserved6) (void);
};

struct _DbusmenuGtkSerializableMenuItem {
	GtkMenuItem parent;

	DbusmenuGtkSerializableMenuItemPrivate * priv;
};

GType dbusmenu_gtk_serializable_menu_item_get_type (void);

DbusmenuMenuitem *  dbusmenu_gtk_serializable_menu_item_get_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi);
void                dbusmenu_gtk_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type);

G_END_DECLS

#endif
