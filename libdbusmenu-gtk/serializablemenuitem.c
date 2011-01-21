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

/**
	DbusmenuGtkSerializableMenuItemPrivate:
	@mi: Menuitem to watch the property changes from
*/
struct _DbusmenuGtkSerializableMenuItemPrivate {
	DbusmenuMenuitem * mi;
};

/* Properties */
enum {
	PROP_0,
	PROP_MENUITEM
};

/* Private macro, only used in object init */
#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM, DbusmenuGtkSerializableMenuItemPrivate))

/* Function prototypes */
static void dbusmenu_gtk_serializable_menu_item_class_init (DbusmenuGtkSerializableMenuItemClass *klass);
static void dbusmenu_gtk_serializable_menu_item_init       (DbusmenuGtkSerializableMenuItem *self);
static void dbusmenu_gtk_serializable_menu_item_dispose    (GObject *object);
static void dbusmenu_gtk_serializable_menu_item_finalize   (GObject *object);
static void set_property                                   (GObject * obj,
                                                            guint id,
                                                            const GValue * value,
                                                            GParamSpec * pspec);
static void get_property                                   (GObject * obj,
                                                            guint id,
                                                            GValue * value,
                                                            GParamSpec * pspec);

/* GObject boiler plate */
G_DEFINE_TYPE (DbusmenuGtkSerializableMenuItem, dbusmenu_gtk_serializable_menu_item, GTK_TYPE_MENU_ITEM);

/* Initialize the stuff in the class structure */
static void
dbusmenu_gtk_serializable_menu_item_class_init (DbusmenuGtkSerializableMenuItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuGtkSerializableMenuItemPrivate));

	object_class->dispose = dbusmenu_gtk_serializable_menu_item_dispose;
	object_class->finalize = dbusmenu_gtk_serializable_menu_item_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_MENUITEM,
	                                 g_param_spec_object(DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_PROP_MENUITEM, "DBusmenu Menuitem attached to item",
	                                              "A menuitem who's properties are being watched and where changes should be watched for updates.  It is the responsibility of subclasses to set up the signal handlers for those property changes.",
	                                              DBUSMENU_TYPE_MENUITEM,
	                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	return;
}

/* Initialize the object structures and private structure */
static void
dbusmenu_gtk_serializable_menu_item_init (DbusmenuGtkSerializableMenuItem *self)
{
	self->priv = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(self);

	self->priv->mi = NULL;

	return;
}

/* Free all references to objects */
static void
dbusmenu_gtk_serializable_menu_item_dispose (GObject *object)
{
	DbusmenuGtkSerializableMenuItem * smi = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM(object);
	g_return_if_fail(smi != NULL);

	if (smi->priv->mi != NULL) {
		g_object_unref(G_OBJECT(smi->priv->mi));
		smi->priv->mi = NULL;
	}


	G_OBJECT_CLASS (dbusmenu_gtk_serializable_menu_item_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
dbusmenu_gtk_serializable_menu_item_finalize (GObject *object)
{



	G_OBJECT_CLASS (dbusmenu_gtk_serializable_menu_item_parent_class)->finalize (object);
	return;
}

/* Set an object property */
static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	DbusmenuGtkSerializableMenuItem * smi = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM(obj);

	switch (id) {
	case PROP_MENUITEM:
		smi->priv->mi = g_value_get_object(value);
		break;
	default:
		g_return_if_reached();
		break;
	}

	return;
}

/* Get an object property */
static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	DbusmenuGtkSerializableMenuItem * smi = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM(obj);

	switch (id) {
	case PROP_MENUITEM:
		g_value_set_object(value, smi->priv->mi);
		break;
	default:
		g_return_if_reached();
		break;
	}

	return;
}

/**
	dbusmenu_gtk_serializable_menu_item_build_dbusmenu_menuitem:
	@smi: #DbusmenuGtkSerializableMenuItem to build a #DbusmenuMenuitem mirroring

	This function is for menu items that are instanciated from
	GTK and have their properites set using GTK functions.  This
	builds a #DbusmenuMenuitem that then has the properties that
	should be sent over the bus to create a new item of this
	type on the other side.

	Return value: A #DbusmenuMenuitem who's values will be set by
		this object.
*/
DbusmenuMenuitem *
dbusmenu_gtk_serializable_menu_item_build_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi)
{
	g_return_val_if_fail(DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM(smi), NULL);

	DbusmenuGtkSerializableMenuItemClass * klass = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_CLASS(smi);
	if (klass->build_dbusmenu_menuitem != NULL) {
		return klass->build_dbusmenu_menuitem(smi);
	}

	return NULL;
}

/* Callback to the generic type handler */
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

/**
	dbusmenu_gtk_serializable_menu_item_register_to_client:
	@client: #DbusmenuClient that we should register a type at.
	@item_type: The #GType of a class that is a subclass of #DbusmenuGtkSerializableMenuItem

	Registers a generic handler for dealing with all subclasses of
	#DbusmenuGtkSerializableMenuItem.  This handler responds to the callback,
	creates a new object and attaches it to the appropriate #DbusmenuMenuitem
	object.
*/
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

/**
	dbusmenu_gtk_serializable_menu_item_set_dbusmenu_menuitem:
	@smi: #DbusmenuGtkSerializableMenuItem to set the @DbusmenuGtkSerializableMenuItem::dbusmenu-menuitem of
	@mi: Menuitem to get the properties from

	This function is used on the server side to signal to the object
	that it should get its' property change events from @mi instead
	of expecting calls to its' API.  A call to this function sets the
	property and subclasses should listen to the notify signal to
	pick up this property being set.
*/
void
dbusmenu_gtk_serializable_menu_item_set_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi, DbusmenuMenuitem * mi)
{
	g_return_if_fail(DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM(smi));
	g_return_if_fail(mi != NULL);

	smi->priv->mi = mi;
	g_object_notify(G_OBJECT(smi), DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_PROP_MENUITEM);

	return;
}
