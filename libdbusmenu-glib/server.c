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

#include <gio/gio.h>

#include "menuitem-private.h"
#include "server.h"
#include "server-marshal.h"

#include "dbus-menu-clean.xml.h"

static void layout_update_signal (DbusmenuServer * server);

#define DBUSMENU_VERSION_NUMBER    2
#define DBUSMENU_INTERFACE         "com.canonical.dbusmenu"

/* Privates, I'll show you mine... */
struct _DbusmenuServerPrivate
{
	DbusmenuMenuitem * root;
	gchar * dbusobject;
	gint layout_revision;
	guint layout_idle;

	GDBusConnection * bus;
	GCancellable * bus_lookup;
	guint dbus_registration;
};

#define DBUSMENU_SERVER_GET_PRIVATE(o) (DBUSMENU_SERVER(o)->priv)

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
	NO_VALID_LAYOUT,
	LAST_ERROR
};

/* Method Table */
typedef void (*MethodTableFunc) (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation);

typedef struct _method_table_t method_table_t;
struct _method_table_t {
	const gchar * interned_name;
	MethodTableFunc func;
};

enum {
	METHOD_GET_LAYOUT = 0,
	METHOD_GET_GROUP_PROPERTIES,
	METHOD_GET_CHILDREN,
	METHOD_GET_PROPERTY,
	METHOD_GET_PROPERTIES,
	METHOD_EVENT,
	METHOD_ABOUT_TO_SHOW,
	/* Counter, do not remove! */
	METHOD_COUNT
};

/* Prototype */
static void       dbusmenu_server_class_init  (DbusmenuServerClass *class);
static void       dbusmenu_server_init        (DbusmenuServer *self);
static void       dbusmenu_server_dispose     (GObject *object);
static void       dbusmenu_server_finalize    (GObject *object);
static void       set_property                (GObject * obj,
                                               guint id,
                                               const GValue * value,
                                               GParamSpec * pspec);
static void       get_property                (GObject * obj,
                                               guint id,
                                               GValue * value,
                                               GParamSpec * pspec);
static void       register_object             (DbusmenuServer * server);
static void       bus_got_cb                  (GObject * obj,
                                               GAsyncResult * result,
                                               gpointer user_data);
static void       bus_method_call             (GDBusConnection * connection,
                                               const gchar * sender,
                                               const gchar * path,
                                               const gchar * interface,
                                               const gchar * method,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation,
                                               gpointer user_data);
static GVariant * bus_get_prop                (GDBusConnection * connection,
                                               const gchar * sender,
                                               const gchar * path,
                                               const gchar * interface,
                                               const gchar * property,
                                               GError ** error,
                                               gpointer user_data);
static void       menuitem_property_changed   (DbusmenuMenuitem * mi,
                                               gchar * property,
                                               GVariant * variant,
                                               DbusmenuServer * server);
static void       menuitem_child_added        (DbusmenuMenuitem * parent,
                                               DbusmenuMenuitem * child,
                                               guint pos,
                                               DbusmenuServer * server);
static void       menuitem_child_removed      (DbusmenuMenuitem * parent,
                                               DbusmenuMenuitem * child,
                                               DbusmenuServer * server);
static void       menuitem_signals_create     (DbusmenuMenuitem * mi,
                                               gpointer data);
static void       menuitem_signals_remove     (DbusmenuMenuitem * mi,
                                               gpointer data);
static GQuark     error_quark                 (void);
static void       bus_get_layout              (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);
static void       bus_get_group_properties    (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);
static void       bus_get_children            (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);
static void       bus_get_property            (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);
static void       bus_get_properties          (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);
static void       bus_event                   (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);
static void       bus_about_to_show           (DbusmenuServer * server,
                                               GVariant * params,
                                               GDBusMethodInvocation * invocation);

/* Globals */
static GDBusNodeInfo *            dbusmenu_node_info = NULL;
static GDBusInterfaceInfo *       dbusmenu_interface_info = NULL;
static const GDBusInterfaceVTable dbusmenu_interface_table = {
	method_call:    bus_method_call,
	get_property:   bus_get_prop,
	set_property:   NULL /* No properties that can be set */
};
static method_table_t             dbusmenu_method_table[METHOD_COUNT];

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
	                                         _dbusmenu_server_marshal_VOID__INT_STRING_VARIANT,
	                                         G_TYPE_NONE, 3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_VARIANT);
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
	                                              "/com/canonical/dbusmenu",
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

	if (dbusmenu_node_info == NULL) {
		GError * error = NULL;

		dbusmenu_node_info = g_dbus_node_info_new_for_xml(dbus_menu_clean_xml, &error);
		if (error != NULL) {
			g_error("Unable to parse DBusmenu Interface description: %s", error->message);
			g_error_free(error);
		}
	}

	if (dbusmenu_interface_info == NULL) {
		dbusmenu_interface_info = g_dbus_node_info_lookup_interface(dbusmenu_node_info, DBUSMENU_INTERFACE);

		if (dbusmenu_interface_info == NULL) {
			g_error("Unable to find interface '" DBUSMENU_INTERFACE "'");
		}
	}

	/* Building our Method table :( */
	dbusmenu_method_table[METHOD_GET_LAYOUT].interned_name = g_intern_static_string("GetLayout");
	dbusmenu_method_table[METHOD_GET_LAYOUT].func          = bus_get_layout;

	dbusmenu_method_table[METHOD_GET_GROUP_PROPERTIES].interned_name = g_intern_static_string("GetGroupProperties");
	dbusmenu_method_table[METHOD_GET_GROUP_PROPERTIES].func          = bus_get_group_properties;

	dbusmenu_method_table[METHOD_GET_CHILDREN].interned_name = g_intern_static_string("GetChildren");
	dbusmenu_method_table[METHOD_GET_CHILDREN].func          = bus_get_children;

	dbusmenu_method_table[METHOD_GET_PROPERTY].interned_name = g_intern_static_string("GetProperty");
	dbusmenu_method_table[METHOD_GET_PROPERTY].func          = bus_get_property;

	dbusmenu_method_table[METHOD_GET_PROPERTIES].interned_name = g_intern_static_string("GetProperties");
	dbusmenu_method_table[METHOD_GET_PROPERTIES].func          = bus_get_properties;

	dbusmenu_method_table[METHOD_EVENT].interned_name = g_intern_static_string("Event");
	dbusmenu_method_table[METHOD_EVENT].func          = bus_event;

	dbusmenu_method_table[METHOD_ABOUT_TO_SHOW].interned_name = g_intern_static_string("AboutToShow");
	dbusmenu_method_table[METHOD_ABOUT_TO_SHOW].func          = bus_about_to_show;

	return;
}

static void
dbusmenu_server_init (DbusmenuServer *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE ((self), DBUSMENU_TYPE_SERVER, DbusmenuServerPrivate);

	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(self);

	priv->root = NULL;
	priv->dbusobject = NULL;
	priv->layout_revision = 1;
	priv->layout_idle = 0;
	priv->bus = NULL;
	priv->bus_lookup = NULL;
	priv->dbus_registration = 0;

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

	if (priv->dbus_registration != 0) {
		g_dbus_connection_unregister_object(priv->bus, priv->dbus_registration);
		priv->dbus_registration = 0;
	}

	if (priv->bus != NULL) {
		g_object_unref(priv->bus);
		priv->bus = NULL;
	}

	if (priv->bus_lookup != NULL) {
		if (!g_cancellable_is_cancelled(priv->bus_lookup)) {
			/* Note, this may case the async function to run at
			   some point in the future.  That's okay, it'll get an
			   error, but just FYI */
			g_cancellable_cancel(priv->bus_lookup);
		}
		g_object_unref(priv->bus_lookup);
		priv->bus_lookup = NULL;
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

		if (priv->bus == NULL) {
			if (priv->bus_lookup == NULL) {
				priv->bus_lookup = g_cancellable_new();
				g_return_if_fail(priv->bus_lookup != NULL);
			}

			g_bus_get(G_BUS_TYPE_SESSION, priv->bus_lookup, bus_got_cb, obj);
		} else {
			register_object(DBUSMENU_SERVER(obj));
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

/* Register the object on the dbus bus */
static void
register_object (DbusmenuServer * server)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	/* Object info */
	g_return_if_fail(priv->bus != NULL);
	g_return_if_fail(priv->dbusobject != NULL);

	/* Class info */
	g_return_if_fail(dbusmenu_node_info != NULL);
	g_return_if_fail(dbusmenu_interface_info != NULL);

	/* We might block on this in the future, but it'd be nice if
	   we could change the object path.  Thinking about it... */
	if (priv->dbus_registration != 0) {
		g_dbus_connection_unregister_object(priv->bus, priv->dbus_registration);
		priv->dbus_registration = 0;
	}

	GError * error = NULL;
	priv->dbus_registration = g_dbus_connection_register_object(priv->bus,
	                                                            priv->dbusobject,
	                                                            dbusmenu_interface_info,
	                                                            &dbusmenu_interface_table,
	                                                            server,
	                                                            NULL,
	                                                            &error);

	if (error != NULL) {
		g_warning("Unable to register object on bus: %s", error->message);
		g_error_free(error);
		return;
	}

	/* If we've got it registered let's tell everyone about it */
	g_signal_emit(G_OBJECT(server), signals[LAYOUT_UPDATED], 0, priv->layout_revision, 0, TRUE);
	if (priv->dbusobject != NULL && priv->bus != NULL) {
		g_dbus_connection_emit_signal(priv->bus,
		                              NULL,
		                              priv->dbusobject,
		                              DBUSMENU_INTERFACE,
		                              "LayoutUpdated",
		                              g_variant_new("(ui)", priv->layout_revision, 0),
		                              NULL);
	}

	return;
}

/* Callback from asking GIO to get us the session bus */
static void
bus_got_cb (GObject * obj, GAsyncResult * result, gpointer user_data)
{
	GError * error = NULL;

	GDBusConnection * bus = g_bus_get_finish(result, &error);

	if (error != NULL) {
		g_warning("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	/* Note: We're not using the user_data before we check for
	   the error so that in the cancelled case at destruction of
	   the object we don't end up with an invalid object. */

	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(user_data);
	priv->bus = bus;

	register_object(DBUSMENU_SERVER(user_data));

	return;
}

/* Function for the GDBus vtable to handle all method calls and dish
   them out the appropriate functions */
static void
bus_method_call (GDBusConnection * connection, const gchar * sender, const gchar * path, const gchar * interface, const gchar * method, GVariant * params, GDBusMethodInvocation * invocation, gpointer user_data)
{
	int i;
	const gchar * interned_method = g_intern_string(method);

	for (i = 0; i < METHOD_COUNT; i++) {
		if (dbusmenu_method_table[i].interned_name == interned_method) {
			if (dbusmenu_method_table[i].func != NULL) {
				return dbusmenu_method_table[i].func(DBUSMENU_SERVER(user_data), params, invocation);
			} else {
				/* If we have a null function we're responding but nothing else. */
				g_warning("Invalid function call for '%s' with parameters: %s", method, g_variant_print(params, TRUE));
				g_dbus_method_invocation_return_value(invocation, NULL);
				return;
			}
		}
	}

	/* We're here because there's an error */
	g_dbus_method_invocation_return_error(invocation,
	                                      error_quark(),
	                                      NOT_IMPLEMENTED,
	                                      "Unable to find method '%s'",
	                                      method);
	return;
}

/* For the GDBus vtable but we only have one property so it's pretty
   simple. */
static GVariant *
bus_get_prop (GDBusConnection * connection, const gchar * sender, const gchar * path, const gchar * interface, const gchar * property, GError ** error, gpointer user_data)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(user_data);

	/* None of these should happen */
	g_return_val_if_fail(g_strcmp0(interface, DBUSMENU_INTERFACE) == 0, NULL);
	g_return_val_if_fail(g_strcmp0(path, priv->dbusobject) == 0, NULL);
	g_return_val_if_fail(g_strcmp0(property, "version") == 0, NULL);

	return g_variant_new_uint32(DBUSMENU_VERSION_NUMBER);
}

/* Handle actually signalling in the idle loop.  This way we collect all
   the updates. */
static gboolean
layout_update_idle (gpointer user_data)
{
	DbusmenuServer * server = DBUSMENU_SERVER(user_data);
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	g_signal_emit(G_OBJECT(server), signals[LAYOUT_UPDATED], 0, priv->layout_revision, 0, TRUE);
	if (priv->dbusobject != NULL && priv->bus != NULL) {
		g_dbus_connection_emit_signal(priv->bus,
		                              NULL,
		                              priv->dbusobject,
		                              DBUSMENU_INTERFACE,
		                              "LayoutUpdated",
		                              g_variant_new("(ui)", priv->layout_revision, 0),
		                              NULL);
	}

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
menuitem_property_changed (DbusmenuMenuitem * mi, gchar * property, GVariant * variant, DbusmenuServer * server)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	g_signal_emit(G_OBJECT(server), signals[ID_PROP_UPDATE], 0, dbusmenu_menuitem_get_id(mi), property, variant, TRUE);

	if (priv->dbusobject != NULL && priv->bus != NULL) {
		g_dbus_connection_emit_signal(priv->bus,
		                              NULL,
		                              priv->dbusobject,
		                              DBUSMENU_INTERFACE,
		                              "ItemPropertyUpdated",
		                              g_variant_new("(isv)", dbusmenu_menuitem_get_id(mi), property, variant),
		                              NULL);
	}
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
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	g_signal_emit(G_OBJECT(server), signals[ITEM_ACTIVATION], 0, dbusmenu_menuitem_get_id(mi), timestamp, TRUE);

	if (priv->dbusobject != NULL && priv->bus != NULL) {
		g_dbus_connection_emit_signal(priv->bus,
		                              NULL,
		                              priv->dbusobject,
		                              DBUSMENU_INTERFACE,
		                              "ItemActivationRequested",
		                              g_variant_new("(iu)", dbusmenu_menuitem_get_id(mi), timestamp),
		                              NULL);
	}

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
static void
bus_get_layout (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	gint parent = 0;
	g_variant_get(params, "(i)", &parent);

	guint revision = priv->layout_revision;
	GPtrArray * xmlarray = g_ptr_array_new();

	if (parent == 0) {
		if (priv->root == NULL) {
			/* g_debug("Getting layout without root node!"); */
			g_ptr_array_add(xmlarray, g_strdup("<menu id=\"0\"/>"));
		} else {
			dbusmenu_menuitem_buildxml(priv->root, xmlarray);
		}
	} else {
		DbusmenuMenuitem * item = NULL;
		if (priv->root != NULL) {
			item = dbusmenu_menuitem_find_id(priv->root, parent);
		}

		if (item == NULL) {
			g_dbus_method_invocation_return_error(invocation,
				                                  error_quark(),
				                                  INVALID_MENUITEM_ID,
				                                  "The ID supplied %d does not refer to a menu item we have",
				                                  parent);
			return;
		}
		dbusmenu_menuitem_buildxml(item, xmlarray);
	}
	g_ptr_array_add(xmlarray, NULL);

	/* build string */
	gchar * layout = g_strjoinv("", (gchar **)xmlarray->pdata);

	g_ptr_array_foreach(xmlarray, xmlarray_foreach_free, NULL);
	g_ptr_array_free(xmlarray, TRUE);

	g_dbus_method_invocation_return_value(invocation,
	                                      g_variant_new("(us)",
	                                                    revision,
	                                                    layout));

	g_free(layout);

	return;
}

/* Get a single property off of a single menuitem */
static void
bus_get_property (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	if (priv->root == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            NO_VALID_LAYOUT,
			            "There currently isn't a layout in this server");
		return;
	}
	
	gint id = g_variant_get_int32(g_variant_get_child_value(params, 0));
	const gchar * property = g_variant_get_string(g_variant_get_child_value(params, 1), NULL);

	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		return;
	}

	GVariant * variant = dbusmenu_menuitem_property_get_variant(mi, property);
	if (variant == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            INVALID_PROPERTY_NAME,
			            "The property '%s' does not exist on menuitem with ID of %d",
			            property,
			            id);
		return;
	}

	g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", variant));
	return;
}

/* Get some properties off of a single menuitem */
static void
bus_get_properties (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	
	if (priv->root == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            NO_VALID_LAYOUT,
			            "There currently isn't a layout in this server");
		return;
	}

	gint id = g_variant_get_int32(g_variant_get_child_value(params, 0));

	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            INVALID_MENUITEM_ID,
			            "The ID supplied %d does not refer to a menu item we have",
			            id);
		return;
	}

	GVariant * dict = dbusmenu_menuitem_properties_variant(mi);

	g_dbus_method_invocation_return_value(invocation, g_variant_new("(a{sv})", dict));

	return;
}

/* Handles getting a bunch of properties from a variety of menu items
   to make one mega dbus message */
static void
bus_get_group_properties (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	if (priv->root == NULL) {
		GVariant * idlist = g_variant_get_child_value(params, 0);
		if (g_variant_n_children(idlist) == 1 && g_variant_get_int32(g_variant_get_child_value(idlist, 0)) == 0) {
			GVariant * final = g_variant_parse(g_variant_type_new("(a(ia{sv}))"), "([(0, {})],)", NULL, NULL, NULL);
			g_dbus_method_invocation_return_value(invocation, final);
			return;
		}

		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            NO_VALID_LAYOUT,
			            "There currently isn't a layout in this server");
		return;
	}

	GVariantIter ids;
	g_variant_iter_init(&ids, g_variant_get_child_value(params, 0));

	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

	gint id;
	while (g_variant_iter_next(&ids, "i", &id)) {
		DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);
		if (mi == NULL) continue;

		GVariantBuilder wbuilder;
		g_variant_builder_init(&wbuilder, G_VARIANT_TYPE_TUPLE);
		g_variant_builder_add(&wbuilder, "i", id);
		GVariant * props = dbusmenu_menuitem_properties_variant(mi);

		if (props == NULL) {
			props = g_variant_parse(g_variant_type_new("a{sv}"), "{}", NULL, NULL, NULL);
		}

		g_variant_builder_add_value(&wbuilder, props);
		GVariant * mi_data = g_variant_builder_end(&wbuilder);

		g_variant_builder_add_value(&builder, mi_data);
	}

	GVariant * ret = g_variant_builder_end(&builder);

	g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add_value(&builder, ret);
	GVariant * final = g_variant_builder_end(&builder);

	g_dbus_method_invocation_return_value(invocation, final);

	return;
}

/* Turn a menuitem into an variant and attach it to the
   VariantBuilder we passed in */
static void
serialize_menuitem(gpointer data, gpointer user_data)
{
	DbusmenuMenuitem * mi = DBUSMENU_MENUITEM(data);
	GVariantBuilder * builder = (GVariantBuilder *)(user_data);

	gint id = dbusmenu_menuitem_get_id(mi);
	GVariant * props = dbusmenu_menuitem_properties_variant(mi);

	g_variant_builder_add(builder, "ia{sv}", id, props);

	return;
}

/* Gets the children and their properties of the ID that is
   passed into the function */
static void
bus_get_children (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);
	gint id = g_variant_get_int32(g_variant_get_child_value(params, 0));

	if (priv->root == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            NO_VALID_LAYOUT,
			            "There currently isn't a layout in this server");
		return;
	}

	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			                                  error_quark(),
			                                  INVALID_MENUITEM_ID,
			                                  "The ID supplied %d does not refer to a menu item we have",
			                                  id);
		return;
	}

	GList * children = dbusmenu_menuitem_get_children(mi);
	GVariant * ret = NULL;

	if (children != NULL) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY); 

		g_list_foreach(children, serialize_menuitem, &builder);

		ret = g_variant_new("(a(ia{svg}))", g_variant_builder_end(&builder));
	} else {
		ret = g_variant_parse(g_variant_type_new("(a(ia{sv}))"), "([(0, {})],)", NULL, NULL, NULL);
	}

	g_dbus_method_invocation_return_value(invocation, ret);
	return;
}

/* Structure for holding the event data for the idle function
   to pick it up. */
typedef struct _idle_event_t idle_event_t;
struct _idle_event_t {
	DbusmenuMenuitem * mi;
	gchar * eventid;
	GVariant * variant;
	guint timestamp;
};

/* A handler for else where in the main loop so that the dbusmenu
   event response doesn't get blocked */
static gboolean
event_local_handler (gpointer user_data)
{
	idle_event_t * data = (idle_event_t *)user_data;

	dbusmenu_menuitem_handle_event(data->mi, data->eventid, data->variant, data->timestamp);

	g_object_unref(data->mi);
	g_free(data->eventid);
	g_variant_unref(data->variant);
	g_free(data);
	return FALSE;
}

/* Handles the events coming off of DBus */
static void
bus_event (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	if (priv->root == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            NO_VALID_LAYOUT,
			            "There currently isn't a layout in this server");
		return;
	}

	gint id = g_variant_get_int32(g_variant_get_child_value(params, 0));
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			                                  error_quark(),
			                                  INVALID_MENUITEM_ID,
			                                  "The ID supplied %d does not refer to a menu item we have",
			                                  id);
		return;
	}

	idle_event_t * event_data = g_new0(idle_event_t, 1);
	event_data->mi = mi;
	g_object_ref(event_data->mi);
	event_data->eventid = g_strdup(g_variant_get_string(g_variant_get_child_value(params, 1), NULL));
	event_data->timestamp = g_variant_get_uint32(g_variant_get_child_value(params, 3));
	event_data->variant = g_variant_get_child_value(params, 2);

	if (g_variant_is_of_type(event_data->variant, G_VARIANT_TYPE_VARIANT)) {
		event_data->variant = g_variant_get_variant(event_data->variant);
	}

	g_variant_ref(event_data->variant);

	g_timeout_add(0, event_local_handler, event_data);

	g_dbus_method_invocation_return_value(invocation, NULL);
	return;
}

/* Recieve the About To Show function.  Pass it to our menu item. */
static void
bus_about_to_show (DbusmenuServer * server, GVariant * params, GDBusMethodInvocation * invocation)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(server);

	if (priv->root == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			            error_quark(),
			            NO_VALID_LAYOUT,
			            "There currently isn't a layout in this server");
		return;
	}

	gint id = g_variant_get_int32(g_variant_get_child_value(params, 0));
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);

	if (mi == NULL) {
		g_dbus_method_invocation_return_error(invocation,
			                                  error_quark(),
			                                  INVALID_MENUITEM_ID,
			                                  "The ID supplied %d does not refer to a menu item we have",
			                                  id);
		return;
	}

	dbusmenu_menuitem_send_about_to_show(mi, NULL, NULL);

	/* GTK+ does not support about-to-show concept for now */
	g_dbus_method_invocation_return_value(invocation,
	                                      g_variant_new("(b)", FALSE));
	return;
}

/* Public Interface */
/**
	dbusmenu_server_new:
	@object: The object name to show for this menu structure
		on DBus.  May be NULL.

	Creates a new #DbusmenuServer object with a specific object
	path on DBus.  If @object is set to NULL the default object
	name of "/com/canonical/dbusmenu" will be used.

	Return value: A brand new #DbusmenuServer
*/
DbusmenuServer *
dbusmenu_server_new (const gchar * object)
{
	if (object == NULL) {
		object = "/com/canonical/dbusmenu";
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



