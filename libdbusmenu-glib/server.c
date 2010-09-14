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

#include "menuitem-private.h"
#include "server.h"
#include "server-marshal.h"

/* DBus Prototypes */
static gboolean _dbusmenu_server_get_layout (DbusmenuServer * server, gint parent, guint * revision, gchar ** layout, GError ** error);
static gboolean _dbusmenu_server_get_property (DbusmenuServer * server, gint id, gchar * property, gchar ** value, GError ** error);
static gboolean _dbusmenu_server_get_properties (DbusmenuServer * server, gint id, gchar ** properties, GHashTable ** dict, GError ** error);
static gboolean _dbusmenu_server_get_group_properties (DbusmenuServer * server, GArray * ids, gchar ** properties, GPtrArray ** values, GError ** error);
static gboolean _dbusmenu_server_event (DbusmenuServer * server, gint id, gchar * eventid, GValue * data, guint timestamp, GError ** error);
static gboolean _dbusmenu_server_get_children (DbusmenuServer * server, gint id, GPtrArray * properties, GPtrArray ** output, GError ** error);
static gboolean _dbusmenu_server_about_to_show (DbusmenuServer * server, gint id, gboolean * need_update, GError ** error);
/* DBus Helpers */
static void _gvalue_array_append_int(GValueArray *array, gint i);
static void _gvalue_array_append_hashtable(GValueArray *array, GHashTable * dict);

#include "dbusmenu-server.h"

static void layout_update_signal (DbusmenuServer * server);

#define DBUSMENU_VERSION_NUMBER  2

/* Privates, I'll show you mine... */
typedef struct _DbusmenuServerPrivate DbusmenuServerPrivate;

struct _DbusmenuServerPrivate
{
	DbusmenuMenuitem * root;
	gchar * dbusobject;
	gint layout_revision;
	guint layout_idle;
};

#define DBUSMENU_SERVER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_SERVER, DbusmenuServerPrivate))

/* Signals */
enum {
	ID_PROP_UPDATE,
	ID_UPDATE,
	LAYOUT_UPDATED,
	ITEM_ACTIVATION,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Properties */
enum {
	PROP_0,
	PROP_DBUS_OBJECT,
	PROP_ROOT_NODE,
	PROP_VERSION
};

/* Errors */
enum {
	INVALID_MENUITEM_ID,
	INVALID_PROPERTY_NAME,
	UNKNOWN_DBUS_ERROR,
	NOT_IMPLEMENTED,
	LAST_ERROR
};

/* Prototype */
static void dbusmenu_server_class_init (DbusmenuServerClass *class);
static void dbusmenu_server_init       (DbusmenuServer *self);
static void dbusmenu_server_dispose    (GObject *object);
static void dbusmenu_server_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
static void menuitem_property_changed (DbusmenuMenuitem * mi, gchar * property, GValue * value, DbusmenuServer * server);
static void menuitem_child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint pos, DbusmenuServer * server);
static void menuitem_child_removed (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, DbusmenuServer * server);
static void menuitem_signals_create (DbusmenuMenuitem * mi, gpointer data);
static void menuitem_signals_remove (DbusmenuMenuitem * mi, gpointer data);
static GQuark error_quark (void);

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
	                                         _dbusmenu_server_marshal_VOID__INT_STRING_POINTER,
	                                         G_TYPE_NONE, 3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_VALUE);
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
	                                         g_cclosure_marshal_VOID__INT,
	                                         G_TYPE_NONE, 1, G_TYPE_INT);
	/**
		DbusmenuServer::layout-updated:
		@arg0: The #DbusmenuServer emitting the signal.
		@arg1: A revision number representing which revision the update
		       represents itself as.
		@arg2: The ID of the parent for this update.

		This signal is emitted any time the layout of the
		menuitems under this server is changed.
	*/
	signals[LAYOUT_UPDATED] =   g_signal_new(DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATED,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, layout_updated),
	                                         NULL, NULL,
	                                         _dbusmenu_server_marshal_VOID__UINT_INT,
	                                         G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_INT);
	/**
		DbusmenuServer::item-activation-requested:
		@arg0: The #DbusmenuServer emitting the signal.
		@arg1: The ID of the parent for this update.
		@arg2: The timestamp of when the event happened

		This is signaled when a menuitem under this server
		sends it's activate signal.
	*/
	signals[ITEM_ACTIVATION] =  g_signal_new(DBUSMENU_SERVER_SIGNAL_ITEM_ACTIVATION,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, item_activation),
	                                         NULL, NULL,
	                                         _dbusmenu_server_marshal_VOID__INT_UINT,
	                                         G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_UINT);


	g_object_class_install_property (object_class, PROP_DBUS_OBJECT,
	                                 g_param_spec_string(DBUSMENU_SERVER_PROP_DBUS_OBJECT, "DBus object path",
	                                              "The object that represents this set of menus on DBus",
	                                              "/org/ayatana/dbusmenu",
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_ROOT_NODE,
	                                 g_param_spec_object(DBUSMENU_SERVER_PROP_ROOT_NODE, "Root menu node",
	                                              "The base object of the menus that are served",
	                                              DBUSMENU_TYPE_MENUITEM,
	                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_VERSION,
	                                 g_param_spec_uint(DBUSMENU_SERVER_PROP_VERSION, "Dbusmenu API version",
	                                              "The version of the DBusmenu API that we're implementing.",
	                                              DBUSMENU_VERSION_NUMBER, DBUSMENU_VERSION_NUMBER, DBUSMENU_VERSION_NUMBER,
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
	priv->layout_revision = 1;
	priv->layout_idle = 0;

	return;
}

static void
dbusmenu_server_dispose (GObject *object)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(object);

	if (priv->layout_idle != 0) {
		g_source_remove(priv->layout_idle);
	}

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
	GError * error = NULL;

	switch (id) {
	case PROP_DBUS_OBJECT:
		g_return_if_fail(priv->dbusobject == NULL);
		priv->dbusobject = g_value_dup_string(value);
		DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

		if (connection == NULL || error != NULL) {
			g_warning("Unable to get session bus: %s", error == NULL ? "No message" : error->message);
			if (error != NULL) { g_error_free(error); }
		} else {
			dbus_g_connection_register_g_object(connection,
			                                    priv->dbusobject,
			                                    obj);
		}
		break;
	case PROP_ROOT_NODE:
		if (priv->root != NULL) {
			dbusmenu_menuitem_foreach(priv->root, menuitem_signals_remove, obj);
			dbusmenu_menuitem_set_root(priv->root, FALSE);
			g_object_unref(G_OBJECT(priv->root));
			priv->root = NULL;
		}
		priv->root = DBUSMENU_MENUITEM(g_value_get_object(value));
		if (priv->root != NULL) {
			g_object_ref(G_OBJECT(priv->root));
			dbusmenu_menuitem_set_root(priv->root, TRUE);
			dbusmenu_menuitem_foreach(priv->root, menuitem_signals_create, obj);
		} else {
			g_debug("Setting root node to NULL");
		}
		layout_update_signal(DBUSMENU_SERVER(obj));
		break;
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
	case PROP_VERSION:
		g_value_set_uint(value, DBUSMENU_VERSION_NUMBER);
		break;
	default:
		g_return_if_reached();
		break;
	}

	return;
}

/* Handle actually signalling in the idle loop.  This way we collect all
   the updates. */
static gboolean
layout_update_idle (gpointer user_data)
{
	DbusmenuServer * server = DBUSMENU_SERVER(user_data);
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	g_signal_emit(G_OBJECT(server), signals[LAYOUT_UPDATED], 0, priv->layout_revision, 0, TRUE);

	priv->layout_idle = 0;

	return FALSE;
}

/* Signals that the layout has been updated */
static void
layout_update_signal (DbusmenuServer * server)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	priv->layout_revision++;

	if (priv->layout_idle == 0) {
		priv->layout_idle = g_idle_add(layout_update_idle, server);
	}

	return;
}

static void 
menuitem_property_changed (DbusmenuMenuitem * mi, gchar * property, GValue * value, DbusmenuServer * server)
{
	g_signal_emit(G_OBJECT(server), signals[ID_PROP_UPDATE], 0, dbusmenu_menuitem_get_id(mi), property, value, TRUE);
	return;
}

/* Adds the signals for this entry to the list and looks at
   the children of this entry to add the signals we need
   as well.  We like signals. */
static void
added_check_children (gpointer data, gpointer user_data)
{
	DbusmenuMenuitem * mi = (DbusmenuMenuitem *)data;
	DbusmenuServer * server = (DbusmenuServer *)user_data;

	menuitem_signals_create(mi, server);
	g_list_foreach(dbusmenu_menuitem_get_children(mi), added_check_children, server);

	return;
}

/* Callback for when a child is added.  We need to connect everything
   up and signal that the layout has changed. */
static void
menuitem_child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint pos, DbusmenuServer * server)
{
	menuitem_signals_create(child, server);
	g_list_foreach(dbusmenu_menuitem_get_children(child), added_check_children, server);

	layout_update_signal(server);
	return;
}

static void 
menuitem_child_removed (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, DbusmenuServer * server)
{
	menuitem_signals_remove(child, server);
	layout_update_signal(server);
	return;
}

static void 
menuitem_child_moved (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint newpos, guint oldpos, DbusmenuServer * server)
{
	layout_update_signal(server);
	return;
}

/* Called when a menu item emits its activated signal so it
   gets passed across the bus. */
static void 
menuitem_shown (DbusmenuMenuitem * mi, guint timestamp, DbusmenuServer * server)
{
	g_signal_emit(G_OBJECT(server), signals[ITEM_ACTIVATION], 0, dbusmenu_menuitem_get_id(mi), timestamp, TRUE);
	return;
}

/* Connects all the signals that we're interested in
   coming from a menuitem */
static void
menuitem_signals_create (DbusmenuMenuitem * mi, gpointer data)
{
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED, G_CALLBACK(menuitem_child_added), data);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(menuitem_child_removed), data);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED, G_CALLBACK(menuitem_child_moved), data);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(menuitem_property_changed), data);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_SHOW_TO_USER, G_CALLBACK(menuitem_shown), data);
	return;
}

/* Removes all the signals that we're interested in
   coming from a menuitem */
static void
menuitem_signals_remove (DbusmenuMenuitem * mi, gpointer data)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_child_added), data);
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_child_removed), data);
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_child_moved), data);
	g_signal_handlers_disconnect_by_func(G_OBJECT(mi), G_CALLBACK(menuitem_property_changed), data);
	return;
}

static GQuark
error_quark (void)
{
	static GQuark quark = 0;
	if (quark == 0) {
		quark = g_quark_from_static_string (G_LOG_DOMAIN);
	}
	return quark;
}

/* DBus interface */
static gboolean
_dbusmenu_server_get_layout (DbusmenuServer * server, gint parent, guint * revision, gchar ** layout, GError ** error)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	*revision = priv->layout_revision;
	GPtrArray * xmlarray = g_ptr_array_new();

	if (parent == 0) {
		if (priv->root == NULL) {
			/* g_debug("Getting layout without root node!"); */
			g_ptr_array_add(xmlarray, g_strdup("<menu id=\"0\"/>"));
		} else {
			dbusmenu_menuitem_buildxml(priv->root, xmlarray);
		}
	} else {
		DbusmenuMenuitem * item = dbusmenu_menuitem_find_id(priv->root, parent);
		if (item == NULL) {
			if (error != NULL) {
				g_set_error(error,
				            error_quark(),
				            INVALID_MENUITEM_ID,
				            "The ID supplied %d does not refer to a menu item we have",
				            parent);
			}
			return FALSE;
		}
		dbusmenu_menuitem_buildxml(item, xmlarray);
	}
	g_ptr_array_add(xmlarray, NULL);

	/* build string */
	*layout = g_strjoinv("", (gchar **)xmlarray->pdata);

	g_ptr_array_foreach(xmlarray, xmlarray_foreach_free, NULL);
	g_ptr_array_free(xmlarray, TRUE);

	return TRUE;
}

static gboolean 
_dbusmenu_server_get_property (DbusmenuServer * server, gint id, gchar * property, gchar ** value, GError ** error)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		}
		return FALSE;
	}

	const gchar * prop = dbusmenu_menuitem_property_get(mi, property);
	if (prop == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            INVALID_PROPERTY_NAME,
			            "The property '%s' does not exist on menuitem with ID of %d",
			            property,
			            id);
		}
		return FALSE;
	}

	if (value == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            UNKNOWN_DBUS_ERROR,
			            "Uhm, yeah.  We didn't get anywhere to put the value, that's really weird.  Seems impossible really.");
		}
		return FALSE;
	}

	*value = g_strdup(prop);

	return TRUE;
}

static gboolean
_dbusmenu_server_get_properties (DbusmenuServer * server, gint id, gchar ** properties, GHashTable ** dict, GError ** error)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		}
		return FALSE;
	}

	*dict = dbusmenu_menuitem_properties_copy(mi);

	return TRUE;
}

/* Handles getting a bunch of properties from a variety of menu items
   to make one mega dbus message */
static gboolean
_dbusmenu_server_get_group_properties (DbusmenuServer * server, GArray * ids, gchar ** properties, GPtrArray ** values, GError ** error)
{
	/* Build an initial pointer array */
	*values = g_ptr_array_new();

	/* Go through each ID to get that ID's properties */
	int idcnt;
	for (idcnt = 0; idcnt < ids->len; idcnt++) {
		GHashTable * idprops = NULL;
		GError * error = NULL;
		gint id = g_array_index(ids, int, idcnt);

		/* Get the properties for this ID the old fashioned way. */
		if (!_dbusmenu_server_get_properties(server, id, properties, &idprops, &error)) {
			g_warning("Error getting the properties from ID %d: %s", id, error->message);
			g_error_free(error);
			error = NULL;
			continue;
		}

		GValueArray * valarray = g_value_array_new(2);

		_gvalue_array_append_int(valarray, id);
		_gvalue_array_append_hashtable(valarray, idprops);

		g_ptr_array_add(*values, valarray);
	}

	return TRUE;
}

/* Allocate a value on the stack for the int and append
   it to the array. */
static void
_gvalue_array_append_int(GValueArray *array, gint i)
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_INT);
	g_value_set_int(&value, i);
	g_value_array_append(array, &value);
	g_value_unset(&value);
}

/* Allocate a value on the stack for the hashtable and append
   it to the array. */
static void
_gvalue_array_append_hashtable(GValueArray *array, GHashTable * dict)
{
	GValue value = {0};

	g_value_init(&value, dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE));
	g_value_set_boxed(&value, dict);
	g_value_array_append(array, &value);
	g_value_unset(&value);
}

static void
serialize_menuitem(gpointer data, gpointer user_data)
{
	DbusmenuMenuitem * mi = DBUSMENU_MENUITEM(data);
	GPtrArray * output = (GPtrArray *)(user_data);

	gint id = dbusmenu_menuitem_get_id(mi);
	GHashTable * dict = dbusmenu_menuitem_properties_copy(mi);

	GValueArray * item = g_value_array_new(2);
	_gvalue_array_append_int(item, id);
	_gvalue_array_append_hashtable(item, dict);

	g_ptr_array_add(output, item);
}

static gboolean
_dbusmenu_server_get_children (DbusmenuServer * server, gint id, GPtrArray * properties, GPtrArray ** output, GError ** error)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		}
		return FALSE;
	}

	*output = g_ptr_array_new();
	GList * children = dbusmenu_menuitem_get_children(mi);
	g_list_foreach(children, serialize_menuitem, *output);

	return TRUE;
}

/* Structure for holding the event data for the idle function
   to pick it up. */
typedef struct _idle_event_t idle_event_t;
struct _idle_event_t {
	DbusmenuMenuitem * mi;
	gchar * eventid;
	GValue data;
	guint timestamp;
};

/* A handler for else where in the main loop so that the dbusmenu
   event response doesn't get blocked */
static gboolean
event_local_handler (gpointer user_data)
{
	idle_event_t * data = (idle_event_t *)user_data;

	dbusmenu_menuitem_handle_event(data->mi, data->eventid, &data->data, data->timestamp);

	g_object_unref(data->mi);
	g_free(data->eventid);
	g_value_unset(&data->data);
	g_free(data);
	return FALSE;
}

/* Handles the even coming off of DBus */
static gboolean
_dbusmenu_server_event (DbusmenuServer * server, gint id, gchar * eventid, GValue * data, guint timestamp, GError ** error)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		}
		return FALSE;
	}

	idle_event_t * event_data = g_new0(idle_event_t, 0);
	event_data->mi = mi;
	g_object_ref(event_data->mi);
	event_data->eventid = g_strdup(eventid);
	event_data->timestamp = timestamp;
	g_value_init(&(event_data->data), G_VALUE_TYPE(data));
	g_value_copy(data, &(event_data->data));

	g_timeout_add(0, event_local_handler, event_data);
	return TRUE;
}

/* Recieve the About To Show function.  Pass it to our menu item. */
static gboolean
_dbusmenu_server_about_to_show (DbusmenuServer * server, gint id, gboolean * need_update, GError ** error)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		if (error != NULL) {
			g_set_error(error,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		}
		return FALSE;
	}

	/* GTK+ does not support about-to-show concept for now */
	*need_update = FALSE;
	return TRUE;
}

/* Public Interface */
/**
	dbusmenu_server_new:
	@object: The object name to show for this menu structure
		on DBus.  May be NULL.

	Creates a new #DbusmenuServer object with a specific object
	path on DBus.  If @object is set to NULL the default object
	name of "/org/ayatana/dbusmenu" will be used.

	Return value: A brand new #DbusmenuServer
*/
DbusmenuServer *
dbusmenu_server_new (const gchar * object)
{
	if (object == NULL) {
		object = "/org/ayatana/dbusmenu";
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

	/* g_debug("Setting root object: 0x%X", (unsigned int)root); */
	GValue rootvalue = {0};
	g_value_init(&rootvalue, G_TYPE_OBJECT);
	g_value_set_object(&rootvalue, root);
	g_object_set_property(G_OBJECT(self), DBUSMENU_SERVER_PROP_ROOT_NODE, &rootvalue);
	g_object_unref(G_OBJECT(root));
	return;
}



