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

/* Properties */
enum {
	PROP_0,
	PROP_DBUSOBJECT,
	PROP_DBUSNAME
};

/* Signals */
enum {
	LAYOUT_UPDATED,
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
static DbusmenuMenuitem * parse_layout_xml(xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent);
static void parse_layout (DbusmenuClient * client, const gchar * layout);
static void update_layout_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data);
static void update_layout (DbusmenuClient * client);

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
	priv->session_bus = NULL;

	G_OBJECT_CLASS (dbusmenu_client_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_client_finalize (GObject *object)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(object);

	g_free(priv->dbus_name);
	g_free(priv->dbus_object);

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

static void
id_prop_update (DBusGProxy * proxy, guint id, gchar * property, gchar * value, DbusmenuClient * client)
{

	return;
}

static void
id_update (DBusGProxy * proxy, guint id, DbusmenuClient * client)
{

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
		return;
	}

	priv->propproxy = dbus_g_proxy_new_for_name_owner(priv->session_bus,
	                                                  priv->dbus_name,
	                                                  priv->dbus_object,
	                                                  DBUS_INTERFACE_PROPERTIES,
	                                                  &error);
	if (error != NULL) {
		g_error("Unable to get property proxy for %s on %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		g_error_free(error);
		return;
	}

	priv->menuproxy = dbus_g_proxy_new_for_name_owner(priv->session_bus,
	                                                  priv->dbus_name,
	                                                  priv->dbus_object,
	                                                  "org.freedesktop.dbusmenu",
	                                                  &error);
	if (error != NULL) {
		g_error("Unable to get dbusmenu proxy for %s on %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		g_error_free(error);
		return;
	}

	dbus_g_proxy_add_signal(priv->menuproxy, "LayoutUpdate", G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->menuproxy, "LayoutUpdate", G_CALLBACK(layout_update), client, NULL);

	dbus_g_proxy_add_signal(priv->menuproxy, "IdPropUpdate", G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->menuproxy, "IdPropUpdate", G_CALLBACK(id_prop_update), client, NULL);

	dbus_g_proxy_add_signal(priv->menuproxy, "IdUpdate", G_TYPE_UINT, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(priv->menuproxy, "IdUpdate", G_CALLBACK(id_update), client, NULL);

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
				g_debug ("Found ID: %d", id);
				return id;
			}
			break;
		}
	}

	g_warning("Unable to find an ID on the node");
	return 0;
}

/* Parse recursively through the XML and make it into
   objects as need be */
static DbusmenuMenuitem *
parse_layout_xml(xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent)
{
	guint id = parse_node_get_id(node);
	g_debug("Looking at node with id: %d", id);
	if (item == NULL || dbusmenu_menuitem_get_id(item) != id || id == 0) {
		if (item != NULL) {
			if (parent != NULL) {
				dbusmenu_menuitem_child_delete(parent, item);
			}
			g_object_unref(G_OBJECT(item));
		}

		if (id == 0) {
			g_warning("ID from XML file is zero");
			return NULL;
		}

		/* Build a new item */
		item = dbusmenu_menuitem_new_with_id(id);
	} 

	xmlNodePtr children;
	guint position;
	GList * oldchildren = dbusmenu_menuitem_take_children(item);

	for (children = node->children, position = 0; children != NULL; children = children->next, position++) {
		g_debug("Looking at child: %d", position);
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

		childmi = parse_layout_xml(children, childmi, item);
		dbusmenu_menuitem_child_add_position(item, childmi, position);
	}

	GList * oldchildleft = NULL;
	for (oldchildleft = oldchildren; oldchildleft != NULL; oldchildleft = g_list_next(oldchildleft)) {
		DbusmenuMenuitem * oldmi = DBUSMENU_MENUITEM(oldchildleft->data);
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

	priv->root = parse_layout_xml(root, priv->root, NULL);
	if (priv->root == NULL) {
		g_warning("Unable to parse layout on client %s object %s: %s", priv->dbus_name, priv->dbus_object, layout);
	}

	xmlFreeDoc(xmldoc);
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

	if (!dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &value, G_TYPE_INVALID)) {
		g_warning("Getting layout failed on client %s object %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		priv->layoutcall = NULL;
		g_error_free(error);
		return;
	}

	const gchar * xml = g_value_get_string(&value);
	g_debug("Got layout string: %s", xml);
	parse_layout(client, xml);

	priv->layoutcall = NULL;
	g_debug("Root is now: 0x%X", (unsigned int)priv->root);
	g_signal_emit(G_OBJECT(client), signals[LAYOUT_UPDATED], 0, TRUE);

	return;
}

/* Call the property on the server we're connected to and set it up to
   be async back to _update_layout_cb */
static void
update_layout (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

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
	update_layout(self);

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
		menu on the server.
*/
DbusmenuMenuitem *
dbusmenu_client_get_root (DbusmenuClient * client)
{
	g_return_val_if_fail(DBUSMENU_IS_CLIENT(client), NULL);

	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->layoutcall != NULL) {
		/* Will end the current call and block on it's completion */
		update_layout_cb(priv->propproxy, priv->layoutcall, client);
	}

	return priv->root;
}
