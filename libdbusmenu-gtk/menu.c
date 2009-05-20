#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "menu.h"
#include "libdbusmenu-glib/client.h"

/* Properties */
enum {
	PROP_0,
	PROP_DBUSOBJECT,
	PROP_DBUSNAME
};

/* Private */
typedef struct _DbusmenuGtkMenuPrivate DbusmenuGtkMenuPrivate;
struct _DbusmenuGtkMenuPrivate {
	DbusmenuClient * client;

	gchar * dbus_object;
	gchar * dbus_name;
};

#define DBUSMENU_GTKMENU_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_GTKMENU_TYPE, DbusmenuGtkMenuPrivate))

/* Prototypes */
static void dbusmenu_gtkmenu_class_init (DbusmenuGtkMenuClass *klass);
static void dbusmenu_gtkmenu_init       (DbusmenuGtkMenu *self);
static void dbusmenu_gtkmenu_dispose    (GObject *object);
static void dbusmenu_gtkmenu_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
/* Internal */
static void build_client (DbusmenuGtkMenu * self);

/* GObject Stuff */
G_DEFINE_TYPE (DbusmenuGtkMenu, dbusmenu_gtkmenu, GTK_TYPE_MENU);

static void
dbusmenu_gtkmenu_class_init (DbusmenuGtkMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuGtkMenuPrivate));

	object_class->dispose = dbusmenu_gtkmenu_dispose;
	object_class->finalize = dbusmenu_gtkmenu_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

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
dbusmenu_gtkmenu_init (DbusmenuGtkMenu *self)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(self);

	priv->client = NULL;

	priv->dbus_object = NULL;
	priv->dbus_name = NULL;

	return;
}

static void
dbusmenu_gtkmenu_dispose (GObject *object)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(object);

	if (priv->client != NULL) {
		g_object_unref(G_OBJECT(priv->client));
		priv->client = NULL;
	}

	G_OBJECT_CLASS (dbusmenu_gtkmenu_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtkmenu_finalize (GObject *object)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(object);

	g_free(priv->dbus_object);
	priv->dbus_object = NULL;

	g_free(priv->dbus_name);
	priv->dbus_name = NULL;

	G_OBJECT_CLASS (dbusmenu_gtkmenu_parent_class)->finalize (object);
	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUSNAME:
		priv->dbus_name = g_value_dup_string(value);
		if (priv->dbus_name != NULL && priv->dbus_object != NULL) {
			build_client(DBUSMENU_GTKMENU(obj));
		}
		break;
	case PROP_DBUSOBJECT:
		priv->dbus_object = g_value_dup_string(value);
		if (priv->dbus_name != NULL && priv->dbus_object != NULL) {
			build_client(DBUSMENU_GTKMENU(obj));
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
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(obj);

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

/* Internal Functions */

static void
build_client (DbusmenuGtkMenu * self)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(self);

	if (priv->client == NULL) {
		priv->client = dbusmenu_client_new(priv->dbus_name, priv->dbus_object);
	}

	return;
}

