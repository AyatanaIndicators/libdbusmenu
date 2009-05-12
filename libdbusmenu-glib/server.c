/*
A library to communicate a menu object set accross DBus and
track updates and maintain consistency.

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server.h"
#include "server-marshal.h"

/* DBus Prototypes */
static gboolean _dbusmenu_server_get_property (void);
static gboolean _dbusmenu_server_get_properties (void);
static gboolean _dbusmenu_server_call (void);
static gboolean _dbusmenu_server_list_properties (void);

#include "dbusmenu-server.h"

/* Privates, I'll show you mine... */
typedef struct _DbusmenuServerPrivate DbusmenuServerPrivate;

struct _DbusmenuServerPrivate
{
	DbusmenuMenuitem * root;
	gchar * dbusobject;
};

#define DBUSMENU_SERVER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_SERVER, DbusmenuServerPrivate))

/* Signals */
enum {
	ID_PROP_UPDATE,
	ID_UPDATE,
	LAYOUT_UPDATE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Properties */
enum {
	PROP_0,
	PROP_DBUS_OBJECT,
	PROP_ROOT_NODE,
	PROP_LAYOUT
};

/* Prototype */
static void dbusmenu_server_class_init (DbusmenuServerClass *class);
static void dbusmenu_server_init       (DbusmenuServer *self);
static void dbusmenu_server_dispose    (GObject *object);
static void dbusmenu_server_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
static void menuitem_property_changed (DbusmenuMenuitem * mi, gchar * property, gchar * value, DbusmenuServer * server);
static void menuitem_child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, DbusmenuServer * server);
static void menuitem_child_removed (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, DbusmenuServer * server);
static void menuitem_signals_create (DbusmenuMenuitem * mi, gpointer data);
static void menuitem_signals_remove (DbusmenuMenuitem * mi, gpointer data);

G_DEFINE_TYPE (DbusmenuServer, dbusmenu_server, G_TYPE_OBJECT);

static void
dbusmenu_server_class_init (DbusmenuServerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	g_type_class_add_private (class, sizeof (DbusmenuServerPrivate));

	object_class->dispose = dbusmenu_server_dispose;
	object_class->finalize = dbusmenu_server_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	/**
		DbusmenuServer::id-prop-update:
		@arg0: The #DbusmenuServer emitting the signal.
		@arg1: The ID of the #DbusmenuMenuitem changing a property.
		@arg2: The property being changed.
		@arg3: The value of the property being changed.

		This signal is emitted when a menuitem updates or
		adds a property.
	*/
	signals[ID_PROP_UPDATE] =   g_signal_new(DBUSMENU_SERVER_SIGNAL_ID_PROP_UPDATE,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, id_prop_update),
	                                         NULL, NULL,
	                                         _dbusmenu_server_marshal_VOID__UINT_STRING_STRING,
	                                         G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	/**
		DbusmenuServer::id-update:
		@arg0: The #DbusmenuServer emitting the signal.
		@arg1: ID of the #DbusmenuMenuitem changing.

		The purpose of this signal is to show major change in
		a menuitem to the point that #DbusmenuServer::id-prop-update
		seems a little insubstantive.
	*/
	signals[ID_UPDATE] =        g_signal_new(DBUSMENU_SERVER_SIGNAL_ID_UPDATE,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, id_update),
	                                         NULL, NULL,
	                                         g_cclosure_marshal_VOID__UINT,
	                                         G_TYPE_NONE, 1, G_TYPE_UINT);
	/**
		DbusmenuServer::layout-update:
		@arg0: The #DbusmenuServer emitting the signal.

		This signal is emitted any time the layout of the
		menuitems under this server is changed.
	*/
	signals[LAYOUT_UPDATE] =    g_signal_new(DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATE,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, layout_update),
	                                         NULL, NULL,
	                                         g_cclosure_marshal_VOID__VOID,
	                                         G_TYPE_NONE, 0);


	g_object_class_install_property (object_class, PROP_DBUS_OBJECT,
	                                 g_param_spec_string(DBUSMENU_SERVER_PROP_DBUS_OBJECT, "DBus object path",
	                                              "The object that represents this set of menus on DBus",
	                                              "/org/freedesktop/dbusmenu",
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_ROOT_NODE,
	                                 g_param_spec_object(DBUSMENU_SERVER_PROP_ROOT_NODE, "Root menu node",
	                                              "The base object of the menus that are served",
	                                              DBUSMENU_TYPE_MENUITEM,
	                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_LAYOUT,
	                                 g_param_spec_string(DBUSMENU_SERVER_PROP_LAYOUT, "XML Layout of the menus",
	                                              "A simple XML string that describes the layout of the menus",
	                                              "<menu />",
	                                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	dbus_g_object_type_install_info(DBUSMENU_TYPE_SERVER, &dbus_glib__dbusmenu_server_object_info);

	return;
}

static void
dbusmenu_server_init (DbusmenuServer *self)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(self);

	priv->root = NULL;
	priv->dbusobject = NULL;

	return;
}

static void
dbusmenu_server_dispose (GObject *object)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(object);

	if (priv->root != NULL) {
		dbusmenu_menuitem_foreach(priv->root, menuitem_signals_remove, object);
		g_object_unref(priv->root);
	}

	G_OBJECT_CLASS (dbusmenu_server_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_server_finalize (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_server_parent_class)->finalize (object);
	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUS_OBJECT:
		g_return_if_fail(priv->dbusobject == NULL);
		priv->dbusobject = g_value_dup_string(value);
		DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
		dbus_g_connection_register_g_object(connection,
											priv->dbusobject,
											obj);
		break;
	case PROP_ROOT_NODE:
		if (priv->root != NULL) {
			dbusmenu_menuitem_foreach(priv->root, menuitem_signals_remove, obj);
			g_object_unref(G_OBJECT(priv->root));
			priv->root = NULL;
		}
		priv->root = DBUSMENU_MENUITEM(g_value_get_object(value));
		if (priv->root != NULL) {
			g_object_ref(G_OBJECT(priv->root));
			dbusmenu_menuitem_foreach(priv->root, menuitem_signals_create, obj);
		} else {
			g_debug("Setting root node to NULL");
		}
		g_signal_emit(obj, signals[LAYOUT_UPDATE], 0, TRUE);
		break;
	case PROP_LAYOUT:
		/* Can't set this, fall through to error */
		g_warning("Can not set property: layout");
	default:
		g_return_if_reached();
		break;
	}

	return;
}

static void
xmlarray_foreach_free (gpointer arrayentry, gpointer userdata)
{
	if (arrayentry != NULL) {
		/* g_debug("Freeing pointer: %s", (gchar *)arrayentry); */
		g_free(arrayentry);
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUS_OBJECT:
		g_value_set_string(value, priv->dbusobject);
		break;
	case PROP_ROOT_NODE:
		g_value_set_object(value, priv->root);
		break;
	case PROP_LAYOUT: {
		GPtrArray * xmlarray = g_ptr_array_new();
		if (priv->root == NULL) {
			g_debug("Getting layout without root node!");
			g_ptr_array_add(xmlarray, g_strdup("<menu />"));
		} else {
			dbusmenu_menuitem_buildxml(priv->root, xmlarray);
		}
		g_ptr_array_add(xmlarray, NULL);

		/* build string */
		gchar * finalstring = g_strjoinv("", (gchar **)xmlarray->pdata);
		g_value_take_string(value, finalstring);
		g_debug("Final string: %s", finalstring);

		g_ptr_array_foreach(xmlarray, xmlarray_foreach_free, NULL);
		g_ptr_array_free(xmlarray, TRUE);
		break;
	}
	default:
		g_return_if_reached();
		break;
	}

	return;
}

static void 
menuitem_property_changed (DbusmenuMenuitem * mi, gchar * property, gchar * value, DbusmenuServer * server)
{
	g_signal_emit(G_OBJECT(server), signals[ID_PROP_UPDATE], 0, dbusmenu_menuitem_get_id(mi), property, value, TRUE);
	return;
}

static void
menuitem_child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, DbusmenuServer * server)
{
	menuitem_signals_create(child, server);
	/* TODO: We probably need to group the layout update signals to make the number more reasonble. */
	g_signal_emit(G_OBJECT(server), signals[LAYOUT_UPDATE], 0, TRUE);
	return;
}

static void 
menuitem_child_removed (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, DbusmenuServer * server)
{
	menuitem_signals_remove(child, server);
	/* TODO: We probably need to group the layout update signals to make the number more reasonble. */
	g_signal_emit(G_OBJECT(server), signals[LAYOUT_UPDATE], 0, TRUE);
	return;
}

/* Connects all the signals that we're interested in
   coming from a menuitem */
static void
menuitem_signals_create (DbusmenuMenuitem * mi, gpointer data)
{
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED, G_CALLBACK(menuitem_child_added), data);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(menuitem_child_removed), data);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(menuitem_property_changed), data);
	return;
}

/* Removes all the signals that we're interested in
   coming from a menuitem */
static void
menuitem_signals_remove (DbusmenuMenuitem * mi, gpointer data)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_child_added), data);
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_child_removed), data);
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_property_changed), data);
	return;
}

/* DBus interface */
static gboolean 
_dbusmenu_server_get_property (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_get_properties (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_call (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_list_properties (void)
{

	return TRUE;
}

/* Public Interface */
/**
	dbusmenu_server_new:
	@object: The object name to show for this menu structure
		on DBus.  May be NULL.

	Creates a new #DbusmenuServer object with a specific object
	path on DBus.  If @object is set to NULL the default object
	name of "/org/freedesktop/dbusmenu" will be used.

	Return value: A brand new #DbusmenuServer
*/
DbusmenuServer *
dbusmenu_server_new (const gchar * object)
{
	if (object == NULL) {
		object = "/org/freedesktop/dbusmenu";
	}

	DbusmenuServer * self = g_object_new(DBUSMENU_TYPE_SERVER,
	                                     DBUSMENU_SERVER_PROP_DBUS_OBJECT, object,
	                                     NULL);

	return self;
}

/**
	dbusmenu_server_set_root:
	@self: The #DbusmenuServer object to set the root on
	@root: The new root #DbusmenuMenuitem tree

	This function contains all of the #GValue wrapping
	required to set the property #DbusmenuServer:root-node
	on the server @self.
*/
void
dbusmenu_server_set_root (DbusmenuServer * self, DbusmenuMenuitem * root)
{
	g_return_if_fail(DBUSMENU_IS_SERVER(self));
	g_return_if_fail(DBUSMENU_IS_MENUITEM(root));

	g_debug("Setting root object: 0x%X", (unsigned int)root);
	GValue rootvalue = {0};
	g_value_init(&rootvalue, G_TYPE_OBJECT);
	g_value_set_object(&rootvalue, root);
	g_object_set_property(G_OBJECT(self), DBUSMENU_SERVER_PROP_ROOT_NODE, &rootvalue);
	return;
}



