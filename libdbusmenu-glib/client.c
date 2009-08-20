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

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "client.h"
#include "dbusmenu-client.h"
#include "server-marshal.h"

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
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

typedef struct _DbusmenuClientPrivate DbusmenuClientPrivate;
struct _DbusmenuClientPrivate
{
	DbusmenuMenuitem * root;
	
	gchar * dbus_object;
	gchar * dbus_name;

	DBusGConnection * session_bus;
	DBusGProxy * menuproxy;
	DBusGProxy * propproxy;
	DBusGProxyCall * layoutcall;

	DBusGProxy * dbusproxy;

	GHashTable * type_handlers;
};

typedef struct _newItemPropData newItemPropData;
struct _newItemPropData
{
	DbusmenuClient * client;
	DbusmenuMenuitem * item;
	DbusmenuMenuitem * parent;
};

#define DBUSMENU_CLIENT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_CLIENT, DbusmenuClientPrivate))

/* GObject Stuff */
static void dbusmenu_client_class_init (DbusmenuClientClass *klass);
static void dbusmenu_client_init       (DbusmenuClient *self);
static void dbusmenu_client_dispose    (GObject *object);
static void dbusmenu_client_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
/* Private Funcs */
static void layout_update (DBusGProxy * proxy, DbusmenuClient * client);
static void id_prop_update (DBusGProxy * proxy, guint id, gchar * property, gchar * value, DbusmenuClient * client);
static void id_update (DBusGProxy * proxy, guint id, DbusmenuClient * client);
static void build_proxies (DbusmenuClient * client);
static guint parse_node_get_id (xmlNodePtr node);
static DbusmenuMenuitem * parse_layout_xml(DbusmenuClient * client, xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent, DBusGProxy * proxy);
static void parse_layout (DbusmenuClient * client, const gchar * layout);
static void update_layout_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data);
static void update_layout (DbusmenuClient * client);
static void menuitem_get_properties_cb (DBusGProxy * proxy, GHashTable * properties, GError * error, gpointer data);

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

	return;
}

static void
dbusmenu_client_init (DbusmenuClient *self)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(self);

	priv->root = NULL;

	priv->dbus_object = NULL;
	priv->dbus_name = NULL;

	priv->session_bus = NULL;
	priv->menuproxy = NULL;
	priv->propproxy = NULL;
	priv->layoutcall = NULL;

	priv->dbusproxy = NULL;

	priv->type_handlers = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                            g_free, NULL);

	return;
}

static void
dbusmenu_client_dispose (GObject *object)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(object);

	if (priv->layoutcall != NULL) {
		dbus_g_proxy_cancel_call(priv->propproxy, priv->layoutcall);
		priv->layoutcall = NULL;
	}
	if (priv->menuproxy != NULL) {
		g_object_unref(G_OBJECT(priv->menuproxy));
		priv->menuproxy = NULL;
	}
	if (priv->propproxy != NULL) {
		g_object_unref(G_OBJECT(priv->propproxy));
		priv->propproxy = NULL;
	}
	if (priv->dbusproxy != NULL) {
		g_object_unref(G_OBJECT(priv->dbusproxy));
		priv->dbusproxy = NULL;
	}
	priv->session_bus = NULL;

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

/* Annoying little wrapper to make the right function update */
static void
layout_update (DBusGProxy * proxy, DbusmenuClient * client)
{
	update_layout(client);
	return;
}

/* Signal from the server that a property has changed
   on one of our menuitems */
static void
id_prop_update (DBusGProxy * proxy, guint id, gchar * property, gchar * value, DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	g_return_if_fail(priv->root != NULL);

	DbusmenuMenuitem * menuitem = dbusmenu_menuitem_find_id(priv->root, id);
	g_return_if_fail(menuitem != NULL);

	dbusmenu_menuitem_property_set(menuitem, property, value);
	return;
}

/* Oh, lots of updates now.  That silly server, they want
   to change all kinds of stuff! */
static void
id_update (DBusGProxy * proxy, guint id, DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	g_return_if_fail(priv->root != NULL);

	DbusmenuMenuitem * menuitem = dbusmenu_menuitem_find_id(priv->root, id);
	g_return_if_fail(menuitem != NULL);

	org_freedesktop_dbusmenu_get_properties_async(proxy, id, menuitem_get_properties_cb, menuitem);
	return;
}

/* Watches to see if our DBus savior comes onto the bus */
static void
dbus_owner_change (DBusGProxy * proxy, const gchar * name, const gchar * prev, const gchar * new, DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (!(new != NULL && prev == NULL)) {
		/* If it's not someone new getting on the bus, sorry we
		   simply just don't care.  It's not that your service isn't
		   important to someone, just not us.  You'll find the right
		   process someday, there's lots of processes out there. */
		return;
	}

	if (g_strcmp0(new, priv->dbus_name)) {
		/* Again, someone else's service. */
		return;
	}

	/* Woot!  A service for us to love and to hold for ever
	   and ever and ever! */
	return build_proxies(client);
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
		g_signal_emit(G_OBJECT(userdata), signals[ROOT_CHANGED], 0, NULL, TRUE);
		g_signal_emit(G_OBJECT(userdata), signals[LAYOUT_UPDATED], 0, TRUE);
	}

	if ((gpointer)priv->menuproxy == (gpointer)gobj_proxy) {
		priv->layoutcall = NULL;
	}

	build_dbus_proxy(DBUSMENU_CLIENT(userdata));
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

	priv->session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		g_error_free(error);
		build_dbus_proxy(client);
		return;
	}

	priv->propproxy = dbus_g_proxy_new_for_name_owner(priv->session_bus,
	                                                  priv->dbus_name,
	                                                  priv->dbus_object,
	                                                  DBUS_INTERFACE_PROPERTIES,
	                                                  &error);
	if (error != NULL) {
		g_warning("Unable to get property proxy for %s on %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		g_error_free(error);
		return;
	}
	g_object_add_weak_pointer(G_OBJECT(priv->propproxy), (gpointer *)&priv->propproxy);
	g_signal_connect(G_OBJECT(priv->propproxy), "destroy", G_CALLBACK(proxy_destroyed), client);

	priv->menuproxy = dbus_g_proxy_new_for_name_owner(priv->session_bus,
	                                                  priv->dbus_name,
	                                                  priv->dbus_object,
	                                                  "org.freedesktop.dbusmenu",
	                                                  &error);
	if (error != NULL) {
		g_warning("Unable to get dbusmenu proxy for %s on %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		g_error_free(error);
		return;
	}
	g_object_add_weak_pointer(G_OBJECT(priv->menuproxy), (gpointer *)&priv->menuproxy);
	g_signal_connect(G_OBJECT(priv->menuproxy), "destroy", G_CALLBACK(proxy_destroyed), client);

	/* If we get here, we don't need the DBus proxy */
	if (priv->dbusproxy != NULL) {
		g_object_unref(G_OBJECT(priv->dbusproxy));
		priv->dbusproxy = NULL;
	}

	dbus_g_proxy_add_signal(priv->menuproxy, "LayoutUpdate", G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->menuproxy, "LayoutUpdate", G_CALLBACK(layout_update), client, NULL);

	dbus_g_object_register_marshaller(_dbusmenu_server_marshal_VOID__UINT_STRING_STRING, G_TYPE_NONE, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_add_signal(priv->menuproxy, "IdPropUpdate", G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->menuproxy, "IdPropUpdate", G_CALLBACK(id_prop_update), client, NULL);

	dbus_g_proxy_add_signal(priv->menuproxy, "IdUpdate", G_TYPE_UINT, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->menuproxy, "IdUpdate", G_CALLBACK(id_update), client, NULL);

	update_layout(client);

	return;
}

/* Get the ID attribute of the node, parse it and
   return it.  Also we're checking to ensure the node
   is a 'menu' here. */
static guint
parse_node_get_id (xmlNodePtr node)
{
	if (g_strcmp0((gchar *)node->name, "menu") != 0) {
		/* This kills some nodes early */
		g_warning("XML Node is not 'menu' it is '%s'", node->name);
		return 0;
	}

	xmlAttrPtr attrib;
	for (attrib = node->properties; attrib != NULL; attrib = attrib->next) {
		if (g_strcmp0((gchar *)attrib->name, "id") == 0) {
			if (attrib->children != NULL) {
				guint id = (guint)g_ascii_strtoull((gchar *)attrib->children->content, NULL, 10);
				/* g_debug ("Found ID: %d", id); */
				return id;
			}
			break;
		}
	}

	g_warning("Unable to find an ID on the node");
	return 0;
}

/* A small helper that calls _property_set on each hash table
   entry in the properties hash. */
static void
get_properties_helper (gpointer key, gpointer value, gpointer data)
{
	dbusmenu_menuitem_property_set((DbusmenuMenuitem *)data, (gchar *)key, (gchar *)value);
	return;
}

/* This is the callback for the properties on a menu item.  There
   should be all of them in the Hash, and they we use foreach to
   copy them into the menuitem.
   This isn't the most efficient way.  We can optimize this by
   somehow removing the foreach.  But that is for later.  */
static void
menuitem_get_properties_cb (DBusGProxy * proxy, GHashTable * properties, GError * error, gpointer data)
{
	g_hash_table_foreach(properties, get_properties_helper, data);
	g_hash_table_destroy(properties);
	return;
}

/* This is a different get properites call back that also sends
   new signals.  It basically is a small wrapper around the original. */
static void
menuitem_get_properties_new_cb (DBusGProxy * proxy, GHashTable * properties, GError * error, gpointer data)
{
	newItemPropData * propdata = (newItemPropData *)data;
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(propdata->client);

	menuitem_get_properties_cb (proxy, properties, error, propdata->item);

	gboolean handled = FALSE;

	const gchar * type;
	DbusmenuClientTypeHandler newfunc = NULL;
	
	type = dbusmenu_menuitem_property_get(propdata->item, "type");
	if (type != NULL) {
		newfunc = g_hash_table_lookup(priv->type_handlers, type);
	}

	if (newfunc != NULL) {
		handled = newfunc(propdata->item, propdata->parent);
	}

	if (!handled) {
		g_signal_emit(G_OBJECT(propdata->client), signals[NEW_MENUITEM], 0, propdata->item, TRUE);
	}

	g_free(propdata);

	return;
}

static void
menuitem_call_cb (DBusGProxy * proxy, GError * error, gpointer userdata)
{
	DbusmenuMenuitem * mi = (DbusmenuMenuitem *)userdata;

	if (error != NULL) {
		g_warning("Unable to call menu item %d: %s", dbusmenu_menuitem_get_id(mi), error->message);
	}

	return;
}

static void
menuitem_activate (DbusmenuMenuitem * mi, DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);
	org_freedesktop_dbusmenu_call_async (priv->menuproxy, dbusmenu_menuitem_get_id(mi), menuitem_call_cb, mi);
	return;
}

/* Parse recursively through the XML and make it into
   objects as need be */
static DbusmenuMenuitem *
parse_layout_xml(DbusmenuClient * client, xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent, DBusGProxy * proxy)
{
	guint id = parse_node_get_id(node);
	/* g_debug("Looking at node with id: %d", id); */
	if (item == NULL || dbusmenu_menuitem_get_id(item) != id || id == 0) {
		if (item != NULL) {
			if (parent != NULL) {
				dbusmenu_menuitem_child_delete(parent, item);
			}
			g_object_unref(G_OBJECT(item));
			item = NULL;
		}

		if (id == 0) {
			g_warning("ID from XML file is zero");
			return NULL;
		}

		/* Build a new item */
		item = dbusmenu_menuitem_new_with_id(id);
		if (parent == NULL) {
			dbusmenu_menuitem_set_root(item, TRUE);
		}
		g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(menuitem_activate), client);

		/* Get the properties queued up for this item */
		/* Not happy about this, but I need these :( */
		newItemPropData * propdata = g_new0(newItemPropData, 1);
		propdata->client  = client;
		propdata->item    = item;
		propdata->parent  = parent;

		org_freedesktop_dbusmenu_get_properties_async(proxy, id, menuitem_get_properties_new_cb, propdata);
	} 

	xmlNodePtr children;
	guint position;
	GList * oldchildren = dbusmenu_menuitem_take_children(item);
	/* g_debug("Starting old children: %d", g_list_length(oldchildren)); */

	for (children = node->children, position = 0; children != NULL; children = children->next, position++) {
		/* g_debug("Looking at child: %d", position); */
		guint childid = parse_node_get_id(children);
		DbusmenuMenuitem * childmi = NULL;

		GList * childsearch = NULL;
		for (childsearch = oldchildren; childsearch != NULL; childsearch = g_list_next(childsearch)) {
			DbusmenuMenuitem * cs_mi = DBUSMENU_MENUITEM(childsearch->data);
			if (childid == dbusmenu_menuitem_get_id(cs_mi)) {
				oldchildren = g_list_remove(oldchildren, cs_mi);
				childmi = cs_mi;
				break;
			}
		}

		childmi = parse_layout_xml(client, children, childmi, item, proxy);
		dbusmenu_menuitem_child_add_position(item, childmi, position);
	}

	/* g_debug("Stopping old children: %d", g_list_length(oldchildren)); */
	GList * oldchildleft = NULL;
	for (oldchildleft = oldchildren; oldchildleft != NULL; oldchildleft = g_list_next(oldchildleft)) {
		DbusmenuMenuitem * oldmi = DBUSMENU_MENUITEM(oldchildleft->data);
		g_debug("Unref'ing menu item with layout update. ID: %d", dbusmenu_menuitem_get_id(oldmi));
		g_object_unref(G_OBJECT(oldmi));
	}
	g_list_free(oldchildren);

	return item;
}

/* Take the layout passed to us over DBus and turn it into
   a set of beautiful objects */
static void
parse_layout (DbusmenuClient * client, const gchar * layout)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	xmlDocPtr xmldoc;

	xmldoc = xmlReadMemory(layout, g_utf8_strlen(layout, 16*1024), "dbusmenu.xml", NULL, 0);

	xmlNodePtr root = xmlDocGetRootElement(xmldoc);

	DbusmenuMenuitem * oldroot = priv->root;
	priv->root = parse_layout_xml(client, root, priv->root, NULL, priv->menuproxy);
	xmlFreeDoc(xmldoc);

	if (priv->root == NULL) {
		g_warning("Unable to parse layout on client %s object %s: %s", priv->dbus_name, priv->dbus_object, layout);
	}

	if (priv->root != oldroot) {
		g_signal_emit(G_OBJECT(client), signals[ROOT_CHANGED], 0, priv->root, TRUE);
	}

	return;
}

/* When the layout property returns, here's where we take care of that. */
static void
update_layout_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	DbusmenuClient * client = DBUSMENU_CLIENT(data);
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	GError * error = NULL;
	GValue value = {0};

	priv->layoutcall = NULL;
	if (!dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &value, G_TYPE_INVALID)) {
		g_warning("Getting layout failed on client %s object %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		g_error_free(error);
		return;
	}

	const gchar * xml = g_value_get_string(&value);
	/* g_debug("Got layout string: %s", xml); */
	parse_layout(client, xml);

	/* g_debug("Root is now: 0x%X", (unsigned int)priv->root); */
	g_signal_emit(G_OBJECT(client), signals[LAYOUT_UPDATED], 0, TRUE);

	return;
}

/* Call the property on the server we're connected to and set it up to
   be async back to _update_layout_cb */
static void
update_layout (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->propproxy == NULL) {
		return;
	}

	if (priv->layoutcall != NULL) {
		return;
	}

	priv->layoutcall = dbus_g_proxy_begin_call (priv->propproxy,
	                                            "Get",
	                                            update_layout_cb,
	                                            client,
	                                            NULL,
	                                            G_TYPE_STRING, "org.freedesktop.dbusmenu",
	                                            G_TYPE_STRING, "layout",
	                                            G_TYPE_INVALID, G_TYPE_VALUE, G_TYPE_INVALID);

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

	if (priv->propproxy == NULL) {
		return NULL;
	}

	if (priv->layoutcall != NULL) {
		/* Will end the current call and block on it's completion */
		update_layout_cb(priv->propproxy, priv->layoutcall, client);
	}

	return priv->root;
}
