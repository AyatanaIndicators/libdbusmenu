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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client.h"
#include "serializablemenuitem.h"

struct _DbusmenuGtkSerializableMenuItemPrivate {
	int dummy;
};

#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM, DbusmenuGtkSerializableMenuItemPrivate))

static void dbusmenu_gtk_serializable_menu_item_class_init (DbusmenuGtkSerializableMenuItemClass *klass);
static void dbusmenu_gtk_serializable_menu_item_init       (DbusmenuGtkSerializableMenuItem *self);
static void dbusmenu_gtk_serializable_menu_item_dispose    (GObject *object);
static void dbusmenu_gtk_serializable_menu_item_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuGtkSerializableMenuItem, dbusmenu_gtk_serializable_menu_item, GTK_TYPE_MENU_ITEM);

static void
dbusmenu_gtk_serializable_menu_item_class_init (DbusmenuGtkSerializableMenuItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuGtkSerializableMenuItemPrivate));

	object_class->dispose = dbusmenu_gtk_serializable_menu_item_dispose;
	object_class->finalize = dbusmenu_gtk_serializable_menu_item_finalize;

	return;
}

static void
dbusmenu_gtk_serializable_menu_item_init (DbusmenuGtkSerializableMenuItem *self)
{
	self->priv = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(self);

	self->priv->dummy = 5;

	return;
}

static void
dbusmenu_gtk_serializable_menu_item_dispose (GObject *object)
{


	G_OBJECT_CLASS (dbusmenu_gtk_serializable_menu_item_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtk_serializable_menu_item_finalize (GObject *object)
{



	G_OBJECT_CLASS (dbusmenu_gtk_serializable_menu_item_parent_class)->finalize (object);
	return;
}

DbusmenuMenuitem *
dbusmenu_gtk_serializable_menu_item_get_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi)
{
	g_return_val_if_fail(DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM(smi), NULL);

	DbusmenuGtkSerializableMenuItemClass * klass = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_CLASS(smi);
	if (klass->get_dbusmenu_menuitem != NULL) {
		return klass->get_dbusmenu_menuitem(smi);
	}

	return NULL;
}

typedef struct _type_handler_t type_handler_t;
struct _type_handler_t {
	DbusmenuGtkSerializableMenuItemClass * class;
	GType type;
};

/* Handle the type with this item. */
static gboolean
type_handler (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client, gpointer user_data)
{
	type_handler_t * th = (type_handler_t *)user_data;

	DbusmenuGtkSerializableMenuItem * smi = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM(g_object_new(th->type, NULL));
	g_return_val_if_fail(smi != NULL, FALSE);

	dbusmenu_gtk_serializable_menu_item_set_dbusmenu_menuitem(smi, newitem);
	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, GTK_MENU_ITEM(smi), parent);

	return TRUE;
}

/* Destruction is inevitable */
static void
type_destroy_handler (DbusmenuClient * client, const gchar * type, gpointer user_data)
{
	g_return_if_fail(user_data != NULL);
	type_handler_t * th = (type_handler_t *)user_data;
	g_type_class_unref(th->class);
	g_free(user_data);
	return;
}

void
dbusmenu_gtk_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type)
{
	g_return_if_fail(g_type_is_a(item_type, DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM));

	gpointer type_class = g_type_class_ref(item_type);
	g_return_if_fail(type_class != NULL);

	DbusmenuGtkSerializableMenuItemClass * class = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_CLASS(type_class);

	if (class->get_type_string == NULL) {
		g_type_class_unref(type_class);
		g_error("No 'get_type_string' in subclass of DbusmenuGtkSerializableMenuItem");
		return;
	}

	/* Register type */
	type_handler_t * th = g_new0(type_handler_t, 1);
	th->class = class;
	th->type = item_type;
	if (!dbusmenu_client_add_type_handler_full(client, class->get_type_string(), type_handler, th, type_destroy_handler)) {
		type_destroy_handler(client, class->get_type_string(), th);
	}

	/* Register defaults */
	/* TODO: Need API on another branch */

	return;
}

void
dbusmenu_gtk_serializable_menu_item_set_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi, DbusmenuMenuitem * mi)
{

	return;
}
