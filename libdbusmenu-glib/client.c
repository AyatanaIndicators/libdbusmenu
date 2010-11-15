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

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "client.h"
#include "menuitem.h"
#include "menuitem-private.h"
#include "client-menuitem.h"
#include "server-marshal.h"
#include "client-marshal.h"
#include "dbus-menu.xml.h"

/* Properties */
enum {
	PROP_0,
	PROP_DBUSOBJECT,
	PROP_DBUSNAME
};

/* Signals */
enum {
	LAYOUT_UPDATED,
	ROOT_CHANGED,
	NEW_MENUITEM,
	ITEM_ACTIVATE,
	EVENT_RESULT,
	LAST_SIGNAL
};

typedef void (*properties_func) (GVariant * properties, GError * error, gpointer user_data);

static guint signals[LAST_SIGNAL] = { 0 };

struct _DbusmenuClientPrivate
{
	DbusmenuMenuitem * root;
	
	gchar * dbus_object;
	gchar * dbus_name;

	GDBusConnection * session_bus;
	GCancellable * session_bus_cancel;

	GDBusProxy * menuproxy;
	GCancellable * menuproxy_cancel;

	GCancellable * layoutcall;

	gint current_revision;
	gint my_revision;

	GDBusProxy * dbusproxy;

	GHashTable * type_handlers;

	GArray * delayed_property_list;
	GArray * delayed_property_listeners;
	gint delayed_idle;
};

typedef struct _newItemPropData newItemPropData;
struct _newItemPropData
{
	DbusmenuClient * client;
	DbusmenuMenuitem * item;
	DbusmenuMenuitem * parent;
};

typedef struct _properties_listener_t properties_listener_t;
struct _properties_listener_t {
	gint id;
	properties_func callback;
	gpointer user_data;
	gboolean replied;
};

typedef struct _event_data_t event_data_t;
struct _event_data_t {
	DbusmenuClient * client;
	DbusmenuMenuitem * menuitem;
	gchar * event;
	GValue data;
	guint timestamp;
};


#define DBUSMENU_CLIENT_GET_PRIVATE(o) (DBUSMENU_CLIENT(o)->priv)
#define DBUSMENU_INTERFACE  "org.ayatana.dbusmenu"

/* GObject Stuff */
static void dbusmenu_client_class_init (DbusmenuClientClass *klass);
static void dbusmenu_client_init       (DbusmenuClient *self);
static void dbusmenu_client_dispose    (GObject *object);
static void dbusmenu_client_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
/* Private Funcs */
static void layout_update (GDBusProxy * proxy, guint revision, gint parent, DbusmenuClient * client);
static void id_prop_update (GDBusProxy * proxy, gint id, gchar * property, GValue * value, DbusmenuClient * client);
static void id_update (GDBusProxy * proxy, gint id, DbusmenuClient * client);
static void build_proxies (DbusmenuClient * client);
static gint parse_node_get_id (xmlNodePtr node);
static DbusmenuMenuitem * parse_layout_xml(DbusmenuClient * client, xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent, GDBusProxy * proxy);
static gint parse_layout (DbusmenuClient * client, const gchar * layout);
static void update_layout_cb (GDBusProxy * proxy, guint rev, gchar * xml, GError * in_error, void * data);
static void update_layout (DbusmenuClient * client);
static void menuitem_get_properties_cb (GVariant * properties, GError * error, gpointer data);
static void get_properties_globber (DbusmenuClient * client, gint id, const gchar ** properties, properties_func callback, gpointer user_data);
static GQuark error_domain (void);
static void item_activated (GDBusProxy * proxy, gint id, guint timestamp, DbusmenuClient * client);
static void menuproxy_build_cb (GObject * object, GAsyncResult * res, gpointer user_data);
static void menuproxy_name_changed_cb (GObject * object, GParamSpec * pspec, gpointer user_data);
static void menuproxy_signal_cb (GDBusProxy * proxy, gchar * sender, gchar * signal, GVariant * params, gpointer user_data);

/* Globals */
static GDBusNodeInfo *            dbusmenu_node_info = NULL;
static GDBusInterfaceInfo *       dbusmenu_interface_info = NULL;

/* Build a type */
G_DEFINE_TYPE (DbusmenuClient, dbusmenu_client, G_TYPE_OBJECT);

static void
dbusmenu_client_class_init (DbusmenuClientClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuClientPrivate));

	object_class->dispose = dbusmenu_client_dispose;
	object_class->finalize = dbusmenu_client_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	/**
		DbusmenuClient::layout-update:
		@arg0: The #DbusmenuClient object

		Tells that the layout has been updated and parsed by
		this object and is ready for grabbing by the calling
		application.
	*/
	signals[LAYOUT_UPDATED]  = g_signal_new(DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED,
	                                        G_TYPE_FROM_CLASS (klass),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (DbusmenuClientClass, layout_updated),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__VOID,
	                                        G_TYPE_NONE, 0, G_TYPE_NONE);
	/**
		DbusmenuClient::root-changed:
		@arg0: The #DbusmenuClient object
		@arg1: The new root #DbusmenuMenuitem

		The layout has changed in a way that can not be
		represented by the individual items changing as the
		root of this client has changed.
	*/
	signals[ROOT_CHANGED]    = g_signal_new(DBUSMENU_CLIENT_SIGNAL_ROOT_CHANGED,
	                                        G_TYPE_FROM_CLASS (klass),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (DbusmenuClientClass, root_changed),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__OBJECT,
	                                        G_TYPE_NONE, 1, G_TYPE_OBJECT);
	/**
		DbusmenuClient::new-menuitem:
		@arg0: The #DbusmenuClient object
		@arg1: The new #DbusmenuMenuitem created

		Signaled when the client creates a new menuitem.  This
		doesn't mean that it's placed anywhere.  The parent that
		it's applied to will signal #DbusmenuMenuitem::child-added
		when it gets parented.
	*/
	signals[NEW_MENUITEM]    = g_signal_new(DBUSMENU_CLIENT_SIGNAL_NEW_MENUITEM,
	                                        G_TYPE_FROM_CLASS (klass),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (DbusmenuClientClass, new_menuitem),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__OBJECT,
	                                        G_TYPE_NONE, 1, G_TYPE_OBJECT);
	/**
		DbusmenuClient::item-activate:
		@arg0: The #DbusmenuClient object
		@arg1: The #DbusmenuMenuitem activated
		@arg2: A timestamp that the event happened at

		Signaled when the server wants to activate an item in
		order to display the menu.
	*/
	signals[ITEM_ACTIVATE]   = g_signal_new(DBUSMENU_CLIENT_SIGNAL_ITEM_ACTIVATE,
	                                        G_TYPE_FROM_CLASS (klass),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (DbusmenuClientClass, item_activate),
	                                        NULL, NULL,
	                                        _dbusmenu_client_marshal_VOID__OBJECT_UINT,
	                                        G_TYPE_NONE, 2, G_TYPE_OBJECT, G_TYPE_UINT);
	/**
		DbusmenuClient::event-error:
		@arg0: The #DbusmenuClient object
		@arg1: The #DbusmenuMenuitem sent an event
		@arg2: The ID of the event sent
		@arg3: The data sent along with the event
		@arg4: A timestamp that the event happened at
		@arg5: Possibly the error in sending the event (or NULL)

		Signal sent to show that there was an error in sending the event
		to the server.
	*/
	signals[EVENT_RESULT]    = g_signal_new(DBUSMENU_CLIENT_SIGNAL_EVENT_RESULT,
	                                        G_TYPE_FROM_CLASS (klass),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (DbusmenuClientClass, event_result),
	                                        NULL, NULL,
	                                        _dbusmenu_client_marshal_VOID__OBJECT_STRING_POINTER_UINT_POINTER,
	                                        G_TYPE_NONE, 5, G_TYPE_OBJECT, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_UINT, G_TYPE_POINTER);

	g_object_class_install_property (object_class, PROP_DBUSOBJECT,
	                                 g_param_spec_string(DBUSMENU_CLIENT_PROP_DBUS_OBJECT, "DBus Object we represent",
	                                              "The Object on the client that we're getting our data from.",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_DBUSNAME,
	                                 g_param_spec_string(DBUSMENU_CLIENT_PROP_DBUS_NAME, "DBus Client we connect to",
	                                              "Name of the DBus client we're connecting to.",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	if (dbusmenu_node_info == NULL) {
		GError * error = NULL;

		dbusmenu_node_info = g_dbus_node_info_new_for_xml(dbus_menu_xml, &error);
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

	return;
}

static void
dbusmenu_client_init (DbusmenuClient *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE ((self), DBUSMENU_TYPE_CLIENT, DbusmenuClientPrivate);

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(self);

	priv->root = NULL;

	priv->dbus_object = NULL;
	priv->dbus_name = NULL;

	priv->session_bus = NULL;
	priv->session_bus_cancel = NULL;

	priv->menuproxy = NULL;
	priv->menuproxy_cancel = NULL;

	priv->layoutcall = NULL;

	priv->current_revision = 0;
	priv->my_revision = 0;

	priv->dbusproxy = NULL;

	priv->type_handlers = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                            g_free, NULL);

	priv->delayed_idle = 0;
	priv->delayed_property_list = g_array_new(TRUE, FALSE, sizeof(gchar *));
	priv->delayed_property_listeners = g_array_new(FALSE, FALSE, sizeof(properties_listener_t));

	return;
}

static void
dbusmenu_client_dispose (GObject *object)
{
	DbusmenuClient * client = DBUSMENU_CLIENT(object);
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(object);

	if (priv->delayed_idle != 0) {
		g_source_remove(priv->delayed_idle);
		priv->delayed_idle = 0;
	}

	/* Only used for queueing up a new command, so we can
	   just drop this array. */
	if (priv->delayed_property_list == NULL) {
		gchar ** dataregion = (gchar **)g_array_free(priv->delayed_property_list, FALSE);
		if (dataregion != NULL) {
			g_strfreev(dataregion);
		}
		priv->delayed_property_list = NULL;
	}

	if (priv->delayed_property_listeners == NULL) {
		gint i;
		GError * localerror = NULL;

		/* Making sure all the callbacks get called so that if they had
		   memory in their user_data that needs to be free'd that happens. */
		for (i = 0; i < priv->delayed_property_listeners->len; i++) {
			properties_listener_t * listener = &g_array_index(priv->delayed_property_listeners, properties_listener_t, i);
			if (!listener->replied) {
				if (localerror == NULL) {
					g_set_error_literal(&localerror, error_domain(), 0, "DbusmenuClient Shutdown");
				}
				listener->callback(NULL, localerror, listener->user_data);
			}
		}
		if (localerror != NULL) {
			g_error_free(localerror);
		}

		g_array_free(priv->delayed_property_listeners, TRUE);
		priv->delayed_property_listeners = NULL;
	}

	if (priv->layoutcall != NULL) {
		dbus_g_proxy_cancel_call(priv->menuproxy, priv->layoutcall);
		priv->layoutcall = NULL;
	}

	/* Bring down the menu proxy, ensure we're not
	   looking for one at the same time. */
	if (priv->menuproxy_cancel != NULL) {
		g_cancellable_cancel(priv->menuproxy_cancel);
		g_object_unref(priv->menuproxy_cancel);
		priv->menuproxy_cancel = NULL;
	}
	if (priv->menuproxy != NULL) {
		g_object_unref(G_OBJECT(priv->menuproxy));
		priv->menuproxy = NULL;
	}

	if (priv->dbusproxy != NULL) {
		g_object_unref(G_OBJECT(priv->dbusproxy));
		priv->dbusproxy = NULL;
	}

	/* Bring down the session bus, ensure we're not
	   looking for one at the same time. */
	if (priv->session_bus_cancel != NULL) {
		g_cancellable_cancel(priv->session_bus_cancel);
		g_object_unref(priv->session_bus_cancel);
		priv->session_bus_cancel = NULL;
	}
	if (priv->session_bus != NULL) {
		g_object_unref(priv->session_bus);
		priv->session_bus = NULL;
	}

	if (priv->root != NULL) {
		g_object_unref(G_OBJECT(priv->root));
		priv->root = NULL;
	}

	G_OBJECT_CLASS (dbusmenu_client_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_client_finalize (GObject *object)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(object);

	g_free(priv->dbus_name);
	g_free(priv->dbus_object);

	if (priv->type_handlers != NULL) {
		g_hash_table_destroy(priv->type_handlers);
	}

	G_OBJECT_CLASS (dbusmenu_client_parent_class)->finalize (object);
	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUSNAME:
		priv->dbus_name = g_value_dup_string(value);
		if (priv->dbus_name != NULL && priv->dbus_object != NULL) {
			build_proxies(DBUSMENU_CLIENT(obj));
		}
		break;
	case PROP_DBUSOBJECT:
		priv->dbus_object = g_value_dup_string(value);
		if (priv->dbus_name != NULL && priv->dbus_object != NULL) {
			build_proxies(DBUSMENU_CLIENT(obj));
		}
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUSNAME:
		g_value_set_string(value, priv->dbus_name);
		break;
	case PROP_DBUSOBJECT:
		g_value_set_string(value, priv->dbus_object);
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

/* Internal funcs */

static GQuark
error_domain (void)
{
	static GQuark error = 0;
	if (error == 0) {
		error = g_quark_from_static_string(G_LOG_DOMAIN "-CLIENT");
	}
	return error;
}

/* Quick little function to search through the listeners and find
   one that matches an ID */
static properties_listener_t *
find_listener (GArray * listeners, guint index, gint id)
{
	if (index >= listeners->len) {
		return NULL;
	}

	properties_listener_t * retval = &g_array_index(listeners, properties_listener_t, index);
	if (retval->id == id) {
		return retval;
	}

	return find_listener(listeners, index + 1, id);
}

/* Call back from getting the group properties, now we need
   to unwind and call the various functions. */
static void 
get_properties_callback (GObject *obj, GAsyncResult * res, gpointer user_data)
{
	GArray * listeners = (GArray *)user_data;
	int i;
	GError * error = NULL;
	GVariant * params = NULL;

	params = g_dbus_proxy_call_finish(G_DBUS_PROXY(obj), res, &error);

	if (error != NULL) {
		/* If we get an error, all our callbacks need to hear about it. */
		g_warning("Group Properties error: %s", error->message);
		for (i = 0; i < listeners->len; i++) {
			properties_listener_t * listener = &g_array_index(listeners, properties_listener_t, i);
			listener->callback(NULL, error, listener->user_data);
		}
		g_array_free(listeners, TRUE);
		return;
	}

	/* Callback all the folks we can find */
	GVariantIter * iter = g_variant_iter_new(params);
	GVariant * child;
	while ((child = g_variant_iter_next_value(iter)) != NULL) {
		if (g_strcmp0(g_variant_get_type_string(child), "ia(sv)") != 0) {
			g_warning("Properties return signature is not 'ia(sv)' it is '%s'", g_variant_get_type_string(child));
			continue;
		}

		gint id = g_variant_get_int32(g_variant_get_child_value(child, 0));
		GVariant * properties = g_variant_get_child_value(child, 1);

		properties_listener_t * listener = find_listener(listeners, 0, id);
		if (listener == NULL) {
			g_warning("Unable to find listener for ID %d", id);
			continue;
		}

		if (!listener->replied) {
			listener->callback(properties, NULL, listener->user_data);
			listener->replied = TRUE;
		} else {
			g_warning("Odd, we've already replied to the listener on ID %d", id);
		}
	}
	g_variant_iter_free(iter);
	g_variant_unref(params);

	/* Provide errors for those who we can't */
	GError * localerror = NULL;
	for (i = 0; i < listeners->len; i++) {
		properties_listener_t * listener = &g_array_index(listeners, properties_listener_t, i);
		if (!listener->replied) {
			if (localerror == NULL) {
				g_set_error_literal(&localerror, error_domain(), 0, "Error getting properties for ID");
			}
			listener->callback(NULL, localerror, listener->user_data);
		}
	}
	if (localerror != NULL) {
		g_error_free(localerror);
	}

	/* Clean up */
	g_array_free(listeners, TRUE);

	return;
}

/* Idle handler to send out all of our property requests as one big
   lovely property request. */
static gboolean
get_properties_idle (gpointer user_data)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(user_data);
	g_return_val_if_fail(priv->menuproxy != NULL, TRUE);

	if (priv->delayed_property_listeners->len == 0) {
		g_warning("Odd, idle func got no listeners.");
		return FALSE;
	}

	/* Build up an ID list to pass */
	GArray * idlist = g_array_new(FALSE, FALSE, sizeof(gint));
	gint i;
	for (i = 0; i < priv->delayed_property_listeners->len; i++) {
		g_array_append_val(idlist, g_array_index(priv->delayed_property_listeners, properties_listener_t, i).id);
	}

	GVariant * variant_params = g_variant_new("a(s)", (const gchar **)priv->delayed_property_list->data);
	g_dbus_proxy_call(priv->menuproxy,
	                  "GetGroupProperties",
	                  variant_params,
	                  G_DBUS_CALL_FLAGS_NONE,
	                  -1,   /* timeout */
	                  NULL, /* cancellable */
	                  get_properties_callback,
	                  priv->delayed_property_listeners);

	/* Free ID List */
	g_array_free(idlist, TRUE);

	/* Free properties */
	gchar ** dataregion = (gchar **)g_array_free(priv->delayed_property_list, FALSE);
	if (dataregion != NULL) {
		g_strfreev(dataregion);
	}
	priv->delayed_property_list = g_array_new(TRUE, FALSE, sizeof(gchar *));

	/* Rebuild the listeners */
	priv->delayed_property_listeners = g_array_new(FALSE, FALSE, sizeof(properties_listener_t));

	/* Make sure we set for a new idle */
	priv->delayed_idle = 0;

	return FALSE;
}

/* Report and error if we're unable to flush the connection, likely
   to be a cause of some other issues. */
static void
connection_flush_cb (GObject * object, GAsyncResult * result, gpointer user_data)
{
	GError * error = NULL;

	if (!g_dbus_connection_flush_finish(G_DBUS_CONNECTION(object), result, &error)) {
		g_warning("Unable to flush DBus connection: %s", error->message);
		g_error_free(error);
	}

	return;
}

/* Forces a call out to start getting properties with the menu items
   that we have queued up already. */
static void
get_properties_flush (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->delayed_idle == 0) {
		return;
	}

	g_source_remove(priv->delayed_idle);
	priv->delayed_idle = 0;

	get_properties_idle(client);

	/* I'm not sure this flush is necissary with GDBus running the
	   DBus connection in another thread.  But, I don't think that
	   it'll hurt anything either, so I'm leaving it in. */
	g_return_if_fail(priv->session_bus != NULL);
	g_dbus_connection_flush(priv->session_bus, NULL, connection_flush_cb, NULL);

	return;
}

/* A function to group all the get_properties commands to make them
   more efficient over dbus. */
static void
get_properties_globber (DbusmenuClient * client, gint id, const gchar ** properties, properties_func callback, gpointer user_data)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	if (find_listener(priv->delayed_property_listeners, 0, id) != NULL) {
		g_warning("Asking for properties from same ID twice: %d", id);
		GError * localerror = NULL;
		g_set_error_literal(&localerror, error_domain(), 0, "ID already queued");
		callback(priv->menuproxy, NULL, localerror, user_data);
		g_error_free(localerror);
		return;
	}

	if (properties == NULL || properties[0] == NULL) {
		/* get all case */
		if (priv->delayed_property_list->len != 0) {
			/* If there are entries in the list, then we'll need to
			   remove them all, and start over */
			gchar ** dataregion = (gchar **)g_array_free(priv->delayed_property_list, FALSE);
			if (dataregion != NULL) {
				g_strfreev(dataregion);
			}
			priv->delayed_property_list = g_array_new(TRUE, FALSE, sizeof(gchar *));
		}
	} else {
		/* there could be a list we care about */
		/* TODO: No one uses this today */
		/* TODO: Copy them into the list */
	}

	properties_listener_t listener = {0};
	listener.id = id;
	listener.callback = callback;
	listener.user_data = user_data;
	listener.replied = FALSE;

	g_array_append_val(priv->delayed_property_listeners, listener);

	if (priv->delayed_idle == 0) {
		priv->delayed_idle = g_idle_add(get_properties_idle, client);
	}

	return;
}

/* Called when a server item wants to activate the menu */
static void
item_activated (GDBusProxy * proxy, gint id, guint timestamp, DbusmenuClient * client)
{
	g_return_if_fail(DBUSMENU_IS_CLIENT(client));

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->root == NULL) {
		g_warning("Asked to activate item %d when we don't have a menu structure.", id);
		return;
	}

	DbusmenuMenuitem * menuitem = dbusmenu_menuitem_find_id(priv->root, id);
	if (menuitem == NULL) {
		g_warning("Unable to find menu item %d to activate.", id);
		return;
	}

	g_signal_emit(G_OBJECT(client), signals[ITEM_ACTIVATE], 0, menuitem, timestamp, TRUE);

	return;
}

/* Annoying little wrapper to make the right function update */
static void
layout_update (GDBusProxy * proxy, guint revision, gint parent, DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	priv->current_revision = revision;
	if (priv->current_revision > priv->my_revision) {
		update_layout(client);
	}
	return;
}

/* Signal from the server that a property has changed
   on one of our menuitems */
static void
id_prop_update (GDBusProxy * proxy, gint id, gchar * property, GValue * value, DbusmenuClient * client)
{
	#ifdef MASSIVEDEBUGGING
	GValue valstr = {0};
	g_value_init(&valstr, G_TYPE_STRING);
	g_value_transform(value, &valstr);
	g_debug("Property change sent to client for item %d property %s value %s", id, property, g_utf8_strlen(g_value_get_string(&valstr), 50) < 25 ? g_value_get_string(&valstr) : "<too long>");
	g_value_unset(&valstr);
	#endif

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	g_return_if_fail(priv->root != NULL);

	DbusmenuMenuitem * menuitem = dbusmenu_menuitem_find_id(priv->root, id);
	if (menuitem == NULL) {
		#ifdef MASSIVEDEBUGGING
		g_debug("Property update '%s' on id %d which couldn't be found", property, id);
		#endif
		return;
	}

	dbusmenu_menuitem_property_set_value(menuitem, property, value);

	return;
}

/* Oh, lots of updates now.  That silly server, they want
   to change all kinds of stuff! */
static void
id_update (GDBusProxy * proxy, gint id, DbusmenuClient * client)
{
	#ifdef MASSIVEDEBUGGING
	g_debug("Client side ID update: %d", id);
	#endif 

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	g_return_if_fail(priv->root != NULL);

	DbusmenuMenuitem * menuitem = dbusmenu_menuitem_find_id(priv->root, id);
	g_return_if_fail(menuitem != NULL);

	g_debug("Getting properties");
	g_object_ref(menuitem);
	get_properties_globber(client, id, NULL, menuitem_get_properties_cb, menuitem);
	return;
}

/* Watches to see if our DBus savior comes onto the bus */
static void
dbus_owner_change (GDBusProxy * proxy, const gchar * name, const gchar * prev, const gchar * new, DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	/* g_debug("Owner change: %s %s %s", name, prev, new); */

	if (!(new[0] != '\0' && prev[0] == '\0')) {
		/* If it's not someone new getting on the bus, sorry we
		   simply just don't care.  It's not that your service isn't
		   important to someone, just not us.  You'll find the right
		   process someday, there's lots of processes out there. */
		return;
	}

	if (g_strcmp0(name, priv->dbus_name)) {
		/* Again, someone else's service. */
		return;
	}

	/* Woot!  A service for us to love and to hold for ever
	   and ever and ever! */
	return build_proxies(client);
}

/* This is the response to see if the name has an owner.  If
   it does, then we should build the proxies here.  Race condition
   check. */
static void
name_owner_check (GDBusProxy *proxy, gboolean has_owner, GError *error, gpointer userdata)
{
	if (error != NULL) {
		return;
	}

	if (!has_owner) {
		return;
	}

	DbusmenuClient * client = DBUSMENU_CLIENT(userdata);
	build_proxies(client);
	return;
}

/* This function builds the DBus proxy which will look out for
   the service coming up. */
static void
build_dbus_proxy (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	GError * error = NULL;

	if (priv->dbusproxy != NULL) {
		return;
	}

	priv->dbusproxy = dbus_g_proxy_new_for_name_owner (priv->session_bus,
	                                                   DBUS_SERVICE_DBUS,
	                                                   DBUS_PATH_DBUS,
	                                                   DBUS_INTERFACE_DBUS,
	                                                   &error);
	if (error != NULL) {
		g_debug("Oh, that's bad.  That's really bad.  We can't get a proxy to DBus itself?  Seriously?  Here's all I know: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_proxy_add_signal(priv->dbusproxy, "NameOwnerChanged",
	                        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
	                        G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->dbusproxy, "NameOwnerChanged",
	                            G_CALLBACK(dbus_owner_change), client, NULL);

	/* Now let's check to make sure we're not in some race
	   condition case. */
	org_freedesktop_DBus_name_has_owner_async(priv->dbusproxy,
	                                          priv->dbus_name,
	                                          name_owner_check,
	                                          client);

	return;
}

/* A signal handler that gets called when a proxy is destoryed a
   so it needs to clean up a little.  Make sure we don't think we
   have a layout and setup the dbus watcher. */
static void
proxy_destroyed (GObject * gobj_proxy, gpointer userdata)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(userdata);

	if (priv->root != NULL) {
		g_object_unref(G_OBJECT(priv->root));
		priv->root = NULL;
		#ifdef MASSIVEDEBUGGING
		g_debug("Proxies destroyed, signaling a root change and a layout update.");
		#endif
		g_signal_emit(G_OBJECT(userdata), signals[ROOT_CHANGED], 0, NULL, TRUE);
		g_signal_emit(G_OBJECT(userdata), signals[LAYOUT_UPDATED], 0, TRUE);
	}

	if ((gpointer)priv->menuproxy == (gpointer)gobj_proxy) {
		priv->layoutcall = NULL;
	}

	priv->current_revision = 0;
	priv->my_revision = 0;

	build_dbus_proxy(DBUSMENU_CLIENT(userdata));
	return;
}

/* Respond to us getting the session bus (hopefully) or handle
   the error if not */
void
session_bus_cb (GObject * object, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;

	/* NOTE: We're not using any other variables before checking
	   the result because they could be destroyed and thus invalid */
	GDBusConnection * bus = g_bus_get_finish(res, &error);
	if (error != NULL) {
		g_warning("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	/* If this wasn't cancelled, we should be good */
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	priv->session_bus = bus;

	if (priv->session_bus_cancel != NULL) {
		g_object_unref(priv->session_bus_cancel);
		priv->session_bus_cancel = NULL;
	}

	/* Retry to build the proxies now that we have a bus */
	build_proxies(DBUSMENU_CLIENT(user_data));

	return;
}

/* When we have a name and an object, build the two proxies and get the
   first version of the layout */
static void
build_proxies (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	GError * error = NULL;

	g_return_if_fail(priv->dbus_object != NULL);
	g_return_if_fail(priv->dbus_name != NULL);

	if (priv->session_bus == NULL) {
		/* We don't have the session bus yet, that's okay, but
		   we need to handle that. */

		/* If we're already running we don't need to look again. */
		if (priv->session_bus_cancel == NULL) {
			priv->session_bus_cancel = g_cancellable_new();

			/* Async get the session bus */
			g_bus_get(G_BUS_SESSION, priv->session_bus_cancel, session_bus_cb, client);
		}

		/* This function exists, it'll be called again when we get
		   the session bus so this condition will be ignored */
		return;
	}

	/* Build us a menu proxy */
	if (priv->menuproxy == NULL) {

		/* Check to see if we're already building one */
		if (priv->menuproxy_cancel == NULL) {
			priv->menuproxy_cancel = g_cancellable_new();

			g_dbus_proxy_new(priv->session_bus,
			                 G_DBUS_PROXY_FLAGS_NONE,
			                 dbusmenu_interface_info,
			                 priv->dbus_name,
			                 priv->dbus_object,
			                 DBUSMENU_INTERFACE,
			                 priv->menuproxy_cancel,
			                 menuproxy_build_cb,
			                 client);
		}
	}

	return;
}

/* Callback when we know if the menu proxy can be created or
   not and do something with it! */
static void
menuproxy_build_cb (GObject * object, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;

	/* NOTE: We're not using any other variables before checking
	   the result because they could be destroyed and thus invalid */
	GDBusProxy * proxy = g_dbus_proxy_new_finish(res, &error);
	if (error != NULL) {
		g_warning("Unable to get menu proxy: %s", error->message);
		g_error_free(error);
		return;
	}

	/* If this wasn't cancelled, we should be good */
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(user_data);
	priv->menuproxy = proxy;

	if (priv->menuproxy_cancel != NULL) {
		g_object_unref(priv->menuproxy_cancel);
		priv->menuproxy_cancel = NULL;
	}

	/* If we get here, we don't need the DBus proxy */
	if (priv->dbusproxy != NULL) {
		g_object_unref(G_OBJECT(priv->dbusproxy));
		priv->dbusproxy = NULL;
	}

	g_signal_connect(priv->menuproxy, "g-signal",             G_CALLBACK(menuproxy_signal_cb),       client);
	g_signal_connect(priv->menuproxy, "notify::g-name-owner", G_CALLBACK(menuproxy_name_changed_cb), client);

	update_layout(client);

	return;
}

/* Handle the case where we change owners */
static void
menuproxy_name_changed_cb (GObject * object, GParamSpec * pspec, gpointer user_data)
{
	GDBusProxy * proxy = G_DBUS_PROXY(object);

	gchar * owner = g_dbus_proxy_get_name_owner(proxy);

	if (owner == NULL) {
		/* Oh, no!  We lost our owner! */
		proxy_destroyed(G_OBJECT(proxy), user_data);
	} else {
		g_free(owner);
	}

	return;
}

/* Handle the signals out of the proxy */
static void
menuproxy_signal_cb (GDBusProxy * proxy, gchar * sender, gchar * signal, GVariant * params, gpointer user_data)
{
	g_return_if_fail(DBUSMENU_IS_CLIENT(user_data));
	DbusmenuClient * client = DBUSMENU_CLIENT(user_data);

	if (g_strcmp0(signal, "LayoutUpdated") == 0) {
		guint revision; gint parent;
		g_variant_get(params, "(ui)", &revision, &parent);
		layout_update(proxy, revision, parent, client);
	} else if (g_strcmp0(signal, "ItemPropertyUpdated") == 0) {
		gint id; gchar * property; GVariant * value;
		g_variant_get(params, "(isv)", &id, &property, &value);
		id_prop_update(proxy, id, property, value, client);
	} else if (g_strcmp0(signal, "ItemUpdated") == 0) {
		gint id;
		g_variant_get(params, "(i)", &id);
		id_update(proxy, id, client);
	} else if (g_strcmp0(signal, "ItemActivationRequested") == 0) {
		gint id; guint timestamp;
		g_variant_get(params, "(iu)", &id, &timestamp);
		item_activated(proxy, id, timestamp, client);
	} else {
		g_warning("Received signal '%s' from menu proxy that is unknown", signal);
	}

	return;
}

/* Get the ID attribute of the node, parse it and
   return it.  Also we're checking to ensure the node
   is a 'menu' here. */
static gint
parse_node_get_id (xmlNodePtr node)
{
	if (node == NULL) {
		return -1;
	}
	if (node->type != XML_ELEMENT_NODE) {
		return -1;
	}
	if (g_strcmp0((gchar *)node->name, "menu") != 0) {
		/* This kills some nodes early */
		g_warning("XML Node is not 'menu' it is '%s'", node->name);
		return -1;
	}

	xmlAttrPtr attrib;
	for (attrib = node->properties; attrib != NULL; attrib = attrib->next) {
		if (g_strcmp0((gchar *)attrib->name, "id") == 0) {
			if (attrib->children != NULL) {
				gint id = (guint)g_ascii_strtoll((gchar *)attrib->children->content, NULL, 10);
				/* g_debug ("Found ID: %d", id); */
				return id;
			}
			break;
		}
	}

	g_warning("Unable to find an ID on the node");
	return -1;
}

/* A small helper that calls _property_set on each hash table
   entry in the properties hash. */
static void
get_properties_helper (gpointer key, gpointer value, gpointer data)
{
	dbusmenu_menuitem_property_set_value((DbusmenuMenuitem *)data, (gchar *)key, (GValue *)value);
	return;
}

/* This is the callback for the properties on a menu item.  There
   should be all of them in the Hash, and they we use foreach to
   copy them into the menuitem.
   This isn't the most efficient way.  We can optimize this by
   somehow removing the foreach.  But that is for later.  */
static void
menuitem_get_properties_cb (GVariant * properties, GError * error, gpointer data)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(data));
	DbusmenuMenuitem * item = DBUSMENU_MENUITEM(data);

	if (error != NULL) {
		g_warning("Error getting properties on a menuitem: %s", error->message);
		g_object_unref(data);
		return;
	}

	GVariantIter * iter = g_variant_iter_new(properties);
	gchar * key;
	GVariant * value;

	while (g_variant_iter_next(iter, "{sv}", &key, &value)) {
		dbusmenu_menuitem_property_set_variant(item, key, value);

		g_variant_unref(value);
		g_free(key);
	}

	g_variant_iter_free(iter);

	g_object_unref(data);

	return;
}

/* This function is called to refresh the properites on an item that
   is getting recycled with the update, but we think might have prop
   changes. */
static void
menuitem_get_properties_replace_cb (GVariant * properties, GError * error, gpointer data)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(data));
	gboolean have_error = FALSE;

	if (error != NULL) {
		g_warning("Unable to replace properties on %d: %s", dbusmenu_menuitem_get_id(DBUSMENU_MENUITEM(data)), error->message);
		have_error = TRUE;
	}

	GList * current_props = NULL;

	for (current_props = dbusmenu_menuitem_properties_list(DBUSMENU_MENUITEM(data));
			current_props != NULL ; current_props = g_list_next(current_props)) {
		if (have_error || g_hash_table_lookup(properties, current_props->data) == NULL) {
			dbusmenu_menuitem_property_remove(DBUSMENU_MENUITEM(data), (const gchar *)current_props->data);
		}
	}

	if (!have_error) {
		menuitem_get_properties_cb(properties, error, data);
	} else {
		g_object_unref(data);
	}

	return;
}

/* This is a different get properites call back that also sends
   new signals.  It basically is a small wrapper around the original. */
static void
menuitem_get_properties_new_cb (GVariant * properties, GError * error, gpointer data)
{
	g_return_if_fail(data != NULL);
	newItemPropData * propdata = (newItemPropData *)data;

	if (error != NULL) {
		g_warning("Error getting properties on a new menuitem: %s", error->message);
		g_object_unref(propdata->item);
		g_free(data);
		return;
	}

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(propdata->client);

	/* Extra ref as get_properties will unref once itself */
	g_object_ref(propdata->item);
	menuitem_get_properties_cb (properties, error, propdata->item);

	gboolean handled = FALSE;

	const gchar * type;
	DbusmenuClientTypeHandler newfunc = NULL;
	
	type = dbusmenu_menuitem_property_get(propdata->item, DBUSMENU_MENUITEM_PROP_TYPE);
	if (type != NULL) {
		newfunc = g_hash_table_lookup(priv->type_handlers, type);
	} else {
		newfunc = g_hash_table_lookup(priv->type_handlers, DBUSMENU_CLIENT_TYPES_DEFAULT);
	}

	if (newfunc != NULL) {
		handled = newfunc(propdata->item, propdata->parent, propdata->client);
	}

	#ifdef MASSIVEDEBUGGING
	g_debug("Client has realized a menuitem: %d", dbusmenu_menuitem_get_id(propdata->item));
	#endif
	dbusmenu_menuitem_set_realized(propdata->item);

	if (!handled) {
		g_signal_emit(G_OBJECT(propdata->client), signals[NEW_MENUITEM], 0, propdata->item, TRUE);
	}

	g_object_unref(propdata->item);
	g_free(propdata);

	return;
}

/* Respond to the call function to make sure that the other side
   got it, or print a warning. */
static void
menuitem_call_cb (GDBusProxy * proxy, GError * error, gpointer userdata)
{
	event_data_t * edata = (event_data_t *)userdata;

	if (error != NULL) {
		g_warning("Unable to call event '%s' on menu item %d: %s", edata->event, dbusmenu_menuitem_get_id(edata->menuitem), error->message);
	}

	g_signal_emit(edata->client, signals[EVENT_RESULT], 0, edata->menuitem, edata->event, &edata->data, edata->timestamp, error, TRUE);

	g_value_unset(&edata->data);
	g_free(edata->event);
	g_object_unref(edata->menuitem);
	g_free(edata);

	return;
}

/* Sends the event over DBus to the server on the other side
   of the bus. */
void
dbusmenu_client_send_event (DbusmenuClient * client, gint id, const gchar * name, const GValue * value, guint timestamp)
{
	g_return_if_fail(DBUSMENU_IS_CLIENT(client));
	g_return_if_fail(id >= 0);
	g_return_if_fail(name != NULL);

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	DbusmenuMenuitem * mi = dbusmenu_menuitem_find_id(priv->root, id);
	if (mi == NULL) {
		g_warning("Asked to activate a menuitem %d that we don't know about", id);
		return;
	}

	if (value == NULL) {
		GValue internalval = {0};
		g_value_init(&internalval, G_TYPE_INT);
		g_value_set_int(&internalval, 0);
		value = &internalval;
	}

	event_data_t * edata = g_new0(event_data_t, 1);
	edata->client = client;
	edata->menuitem = mi;
	g_object_ref(edata->menuitem);
	edata->event = g_strdup(name);
	g_value_init(&edata->data, G_VALUE_TYPE(value));
	g_value_copy(value, &edata->data);
	edata->timestamp = timestamp;

	DBusGAsyncData *stuff;
	stuff = g_slice_new (DBusGAsyncData);
	stuff->cb = G_CALLBACK (menuitem_call_cb);
	stuff->userdata = edata;
	dbus_g_proxy_begin_call_with_timeout (priv->menuproxy, "Event", org_ayatana_dbusmenu_event_async_callback, stuff, _dbus_glib_async_data_free, 1000, G_TYPE_INT, id, G_TYPE_STRING, name, G_TYPE_VALUE, value, G_TYPE_UINT, timestamp, G_TYPE_INVALID);

	return;
}

typedef struct _about_to_show_t about_to_show_t;
struct _about_to_show_t {
	DbusmenuClient * client;
	void (*cb) (gpointer data);
	gpointer cb_data;
};

/* Reports errors and responds to update request that were a result
   of sending the about to show signal. */
static void
about_to_show_cb (GDBusProxy * proxy, gboolean need_update, GError * error, gpointer userdata)
{
	about_to_show_t * data = (about_to_show_t *)userdata;

	if (error != NULL) {
		g_warning("Unable to send about_to_show: %s", error->message);
		/* Note: we're just ensuring only the callback gets called */
		need_update = FALSE;
	}

	/* If we need to update, do that first. */
	if (need_update) {
		update_layout(data->client);
	}

	if (data->cb != NULL) {
		data->cb(data->cb_data);
	}

	g_object_unref(data->client);
	g_free(data);

	return;
}

/* Sends the about to show signal for a given id to the
   server on the other side of DBus */
void
dbusmenu_client_send_about_to_show(DbusmenuClient * client, gint id, void (*cb)(gpointer data), gpointer cb_data)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	about_to_show_t * data = g_new0(about_to_show_t, 1);
	data->client = client;
	data->cb = cb;
	data->cb_data = cb_data;
	g_object_ref(client);

	org_ayatana_dbusmenu_about_to_show_async (priv->menuproxy, id, about_to_show_cb, data);
	return;
}

/* Builds a new child with property requests and everything
   else to clean up the code a bit */
static DbusmenuMenuitem *
parse_layout_new_child (gint id, DbusmenuClient * client, DbusmenuMenuitem * parent)
{
	DbusmenuMenuitem * item = NULL;

	/* Build a new item */
	item = DBUSMENU_MENUITEM(dbusmenu_client_menuitem_new(id, client));
	if (parent == NULL) {
		dbusmenu_menuitem_set_root(item, TRUE);
	}

	/* Get the properties queued up for this item */
	/* Not happy allocating about this, but I need these :( */
	newItemPropData * propdata = g_new0(newItemPropData, 1);
	if (propdata != NULL) {
		propdata->client  = client;
		propdata->item    = item;
		propdata->parent  = parent;

		g_object_ref(item);
		get_properties_globber(client, id, NULL, menuitem_get_properties_new_cb, propdata);
	} else {
		g_warning("Unable to allocate memory to get properties for menuitem.  This menuitem will never be realized.");
	}

	return item;
}

/* Refresh the properties on this item */
static void
parse_layout_update (DbusmenuMenuitem * item, DbusmenuClient * client)
{
	g_object_ref(item);
	get_properties_globber(client, dbusmenu_menuitem_get_id(item), NULL, menuitem_get_properties_replace_cb, item);
	return;
}

/* Parse recursively through the XML and make it into
   objects as need be */
static DbusmenuMenuitem *
parse_layout_xml(DbusmenuClient * client, xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent, GDBusProxy * proxy)
{
	/* First verify and figure out what we've got */
	gint id = parse_node_get_id(node);
	if (id < 0) {
		return NULL;
	}
	#ifdef MASSIVEDEBUGGING
	g_debug("Client looking at node with id: %d", id);
	#endif

	g_return_val_if_fail(item != NULL, NULL);
	g_return_val_if_fail(id == dbusmenu_menuitem_get_id(item), NULL);

	/* Some variables */
	xmlNodePtr children;
	guint position;
	GList * oldchildren = g_list_copy(dbusmenu_menuitem_get_children(item));
	/* g_debug("Starting old children: %d", g_list_length(oldchildren)); */

	/* Go through all the XML Nodes and make sure that we have menuitems
	   to cover those XML nodes. */
	for (children = node->children, position = 0; children != NULL; children = children->next, position++) {
		/* g_debug("Looking at child: %d", position); */
		gint childid = parse_node_get_id(children);
		if (childid < 0) {
			/* Don't increment the position when there isn't a valid
			   node in the XML tree.  It's probably a comment. */
			position--;
			continue;
		}
		DbusmenuMenuitem * childmi = NULL;

		/* First see if we can recycle a node that we've already built
		   on this menu item */
		GList * childsearch = NULL;
		for (childsearch = oldchildren; childsearch != NULL; childsearch = g_list_next(childsearch)) {
			DbusmenuMenuitem * cs_mi = DBUSMENU_MENUITEM(childsearch->data);
			if (childid == dbusmenu_menuitem_get_id(cs_mi)) {
				oldchildren = g_list_remove(oldchildren, cs_mi);
				childmi = cs_mi;
				break;
			}
		}

		if (childmi == NULL) {
			#ifdef MASSIVEDEBUGGING
			g_debug("Building new menu item %d at position %d", childid, position);
			#endif
			/* If we can't recycle, then we build a new one */
			childmi = parse_layout_new_child(childid, client, item);
			dbusmenu_menuitem_child_add_position(item, childmi, position);
			g_object_unref(childmi);
		} else {
			#ifdef MASSIVEDEBUGGING
			g_debug("Recycling menu item %d at position %d", childid, position);
			#endif
			/* If we can recycle, make sure it's in the right place */
			dbusmenu_menuitem_child_reorder(item, childmi, position);
			parse_layout_update(childmi, client);
		}
	}

	/* Remove any children that are no longer used by this version of
	   the layout. */
	GList * oldchildleft = NULL;
	for (oldchildleft = oldchildren; oldchildleft != NULL; oldchildleft = g_list_next(oldchildleft)) {
		DbusmenuMenuitem * oldmi = DBUSMENU_MENUITEM(oldchildleft->data);
		#ifdef MASSIVEDEBUGGING
		g_debug("Unref'ing menu item with layout update. ID: %d", dbusmenu_menuitem_get_id(oldmi));
		#endif
		dbusmenu_menuitem_child_delete(item, oldmi);
	}
	g_list_free(oldchildren);

	/* We've got everything built up at this node and reconcilled */

	/* Flush the properties requests */
	get_properties_flush(client);

	/* now it's time to recurse down the tree. */
	children = node->children;
	GList * childmis = dbusmenu_menuitem_get_children(item);
	while (children != NULL && childmis != NULL) {
		gint xmlid = parse_node_get_id(children);
		/* If this isn't a valid menu item we need to move on
		   until we have one.  This avoids things like comments. */
		if (xmlid < 0) {
			children = children->next;
			continue;
		}

		#ifdef MASSIVEDEBUGGING
		gint miid = dbusmenu_menuitem_get_id(DBUSMENU_MENUITEM(childmis->data));
		g_debug("Recursing parse_layout_xml.  XML ID: %d  MI ID: %d", xmlid, miid);
		#endif
		
		parse_layout_xml(client, children, DBUSMENU_MENUITEM(childmis->data), item, proxy);

		children = children->next;
		childmis = g_list_next(childmis);
	}
	if (children != NULL) {
		g_warning("Sync failed, now we've got extra XML nodes.");
	}
	if (childmis != NULL) {
		g_warning("Sync failed, now we've got extra menu items.");
	}

	return item;
}

/* Take the layout passed to us over DBus and turn it into
   a set of beautiful objects */
static gint
parse_layout (DbusmenuClient * client, const gchar * layout)
{
	#ifdef MASSIVEDEBUGGING
	g_debug("Client Parsing a new layout");
	#endif 

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	xmlDocPtr xmldoc;

	/* No one should need more characters than this! */
	xmldoc = xmlReadMemory(layout, g_utf8_strlen(layout, 1024*1024), "dbusmenu.xml", NULL, 0);

	xmlNodePtr root = xmlDocGetRootElement(xmldoc);

	if (root == NULL) {
		g_warning("Unable to get root node of menu XML");
	}

	DbusmenuMenuitem * oldroot = priv->root;

	if (priv->root == NULL) {
		priv->root = parse_layout_new_child(0, client, NULL);
	} else {
		parse_layout_update(priv->root, client);
	}

	priv->root = parse_layout_xml(client, root, priv->root, NULL, priv->menuproxy);
	xmlFreeDoc(xmldoc);

	if (priv->root == NULL) {
		g_warning("Unable to parse layout on client %s object %s: %s", priv->dbus_name, priv->dbus_object, layout);
	}

	if (priv->root != oldroot) {
		#ifdef MASSIVEDEBUGGING
		g_debug("Client signaling root changed.");
		#endif 

		/* If they are different, and there was an old root we must
		   clean up that old root */
		if (oldroot != NULL) {
			dbusmenu_menuitem_set_root(oldroot, FALSE);
			g_object_unref(oldroot);
			oldroot = NULL;
		}

		/* If the root changed we can signal that */
		g_signal_emit(G_OBJECT(client), signals[ROOT_CHANGED], 0, priv->root, TRUE);
	}

	return 1;
}

/* When the layout property returns, here's where we take care of that. */
static void
update_layout_cb (GDBusProxy * proxy, guint rev, gchar * xml, GError * error, void * data)
{
	DbusmenuClient * client = DBUSMENU_CLIENT(data);
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (error != NULL) {
		g_warning("Getting layout failed on client %s object %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		return;
	}

	if (!parse_layout(client, xml)) {
		g_warning("Unable to parse layout!");
		return;
	}

	priv->my_revision = rev;
	/* g_debug("Root is now: 0x%X", (unsigned int)priv->root); */
	priv->layoutcall = NULL;
	#ifdef MASSIVEDEBUGGING
	g_debug("Client signaling layout has changed.");
	#endif 
	g_signal_emit(G_OBJECT(client), signals[LAYOUT_UPDATED], 0, TRUE);

	/* Check to see if we got another update in the time this
	   one was issued. */
	if (priv->my_revision < priv->current_revision) {
		update_layout(client);
	}

	return;
}

/* Call the property on the server we're connected to and set it up to
   be async back to _update_layout_cb */
static void
update_layout (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->menuproxy == NULL) {
		return;
	}

	if (priv->layoutcall != NULL) {
		return;
	}

	priv->layoutcall = org_ayatana_dbusmenu_get_layout_async(priv->menuproxy,
	                                                         0, /* Parent is the root */
	                                                         update_layout_cb,
	                                                         client);

	return;
}

/* Public API */
/**
	dbusmenu_client_new:
	@name: The DBus name for the server to connect to
	@object: The object on the server to monitor

	This function creates a new client that connects to a specific
	server on DBus.  That server is at a specific location sharing
	a known object.  The interface is assumed by the code to be 
	the DBus menu interface.  The newly created client will start
	sending out events as it syncs up with the server.

	Return value: A brand new #DbusmenuClient
*/
DbusmenuClient *
dbusmenu_client_new (const gchar * name, const gchar * object)
{
	DbusmenuClient * self = g_object_new(DBUSMENU_TYPE_CLIENT,
	                                     DBUSMENU_CLIENT_PROP_DBUS_NAME, name,
	                                     DBUSMENU_CLIENT_PROP_DBUS_OBJECT, object,
	                                     NULL);

	return self;
}

/**
	dbusmenu_client_get_root:
	@client: The #DbusmenuClient to get the root node from

	Grabs the root node for the specified client @client.  This
	function may block.  It will block if there is currently a
	call to update the layout, it will block on that layout 
	updated and then return the newly updated layout.  Chances
	are that this update is in the queue for the mainloop as
	it would have been requested some time ago, but in theory
	it could block longer.

	Return value: A #DbusmenuMenuitem representing the root of
		menu on the server.  If there is no server or there is
		an error receiving its layout it'll return #NULL.
*/
DbusmenuMenuitem *
dbusmenu_client_get_root (DbusmenuClient * client)
{
	g_return_val_if_fail(DBUSMENU_IS_CLIENT(client), NULL);

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	#ifdef MASSIVEDEBUGGING
	g_debug("Client get root: %X", (guint)priv->root);
	#endif

	return priv->root;
}

/**
	dbusmenu_client_add_type_handler:
	@client: Client where we're getting types coming in
	@type: A text string that will be matched with the 'type'
	    property on incoming menu items
	@newfunc: The function that will be executed with those new
	    items when they come in.

	This function connects into the type handling of the #DbusmenuClient.
	Every new menuitem that comes in immediately gets asked for it's
	properties.  When we get those properties we check the 'type'
	property and look to see if it matches a handler that is known
	by the client.  If so, the @newfunc function is executed on that
	#DbusmenuMenuitem.  If not, then the DbusmenuClient::new-menuitem
	signal is sent.

	In the future the known types will be sent to the server so that it
	can make choices about the menu item types availble.

	Return value: If registering the new type was successful.
*/
gboolean
dbusmenu_client_add_type_handler (DbusmenuClient * client, const gchar * type, DbusmenuClientTypeHandler newfunc)
{
	g_return_val_if_fail(DBUSMENU_IS_CLIENT(client), FALSE);
	g_return_val_if_fail(type != NULL, FALSE);

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	#ifdef MASSIVEDEBUGGING
	g_debug("Adding a type handler for '%s'", type);
	#endif

	if (priv->type_handlers == NULL) {
		g_warning("Type handlers hashtable not built");
		return FALSE;
	}

	gpointer value = g_hash_table_lookup(priv->type_handlers, type);
	if (value != NULL) {
		g_warning("Type '%s' already had a registered handler.", type);
		return FALSE;
	}

	g_hash_table_insert(priv->type_handlers, g_strdup(type), newfunc);
	return TRUE;
}
