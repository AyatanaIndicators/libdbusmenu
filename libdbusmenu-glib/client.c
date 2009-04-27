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
static void build_proxies (DbusmenuClient * client);

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

	g_object_class_install_property (object_class, PROP_DBUSOBJECT,
	                                 g_param_spec_string("dbus-object", "DBus Object we represent",
	                                              "The Object on the client that we're getting our data from.",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_DBUSNAME,
	                                 g_param_spec_string("dbus-name", "DBus Client we connect to",
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
		return 0;
	}

	xmlAttrPtr attrib;
	for (attrib = node->properties; node != NULL; node = node->next) {
		if (g_strcmp0((gchar *)node->name, "id") == 0) {
			if (node->children != NULL) {
				return (guint)g_ascii_strtoull((gchar *)attrib->children->content, NULL, 10);
			}
			break;
		}
	}

	return 0;
}

/* Parse recursively through the XML and make it into
   objects as need be */
static DbusmenuMenuitem *
parse_layout_xml(xmlNodePtr node, DbusmenuMenuitem * item, DbusmenuMenuitem * parent)
{
	guint id = parse_node_get_id(node);
	if (item == NULL || dbusmenu_menuitem_get_id(item) != id) {
		if (item != NULL) {
			if (parent != NULL) {
				dbusmenu_menuitem_child_delete(parent, item);
			}
			g_object_unref(G_OBJECT(item));
		}

		/* Build a new item */
		item = dbusmenu_menuitem_new_with_id(id);
	} 

	xmlNodePtr children;
	guint position;
	for (children = node->children, position = 0; children != NULL; children = children->next, position++) {
		guint childid = parse_node_get_id(children);
		DbusmenuMenuitem * childmi = dbusmenu_menuitem_child_find(item, childid);
		childmi = parse_layout_xml(children, childmi, item);
		dbusmenu_menuitem_child_add_position(item, childmi, position);
	}

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
		g_warning("Unable to parse layout on client %s object %s", priv->dbus_name, priv->dbus_object);
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

	g_value_init(&value, G_TYPE_STRING);

	if (!dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &value, G_TYPE_INVALID)) {
		g_warning("Getting layout failed on client %s object %s: %s", priv->dbus_name, priv->dbus_object, error->message);
		g_error_free(error);
		return;
	}

	const gchar * xml = g_value_get_string(&value);
	parse_layout(client, xml);

	priv->layoutcall = NULL;

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
DbusmenuClient *
dbusmenu_client_new (const gchar * name, const gchar * object)
{
	DbusmenuClient * self = g_object_new(DBUSMENU_TYPE_CLIENT, "name", name, "object", object, NULL);
	update_layout(self);

	return self;
}

DbusmenuMenuitem *
dbusmenu_client_get_root (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->layoutcall != NULL) {
		/* Will end the current call and block on it's completion */
		update_layout_cb(priv->propproxy, priv->layoutcall, client);
	}

	return priv->root;
}
