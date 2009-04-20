#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client.h"
#include "dbusmenu-client.h"

typedef struct _DbusmenuClientPrivate DbusmenuClientPrivate;

struct _DbusmenuClientPrivate
{
	DbusmenuMenuitem * root;

	DBusGProxy * menuproxy;
	DBusGProxy * propproxy;
	DBusGProxyCall * layoutcall;
};

#define DBUSMENU_CLIENT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_CLIENT, DbusmenuClientPrivate))

static void dbusmenu_client_class_init (DbusmenuClientClass *klass);
static void dbusmenu_client_init       (DbusmenuClient *self);
static void dbusmenu_client_dispose    (GObject *object);
static void dbusmenu_client_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuClient, dbusmenu_client, G_TYPE_OBJECT);

static void
dbusmenu_client_class_init (DbusmenuClientClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuClientPrivate));

	object_class->dispose = dbusmenu_client_dispose;
	object_class->finalize = dbusmenu_client_finalize;

	return;
}

static void
dbusmenu_client_init (DbusmenuClient *self)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(self);

	priv->root = NULL;
	priv->menuproxy = NULL;
	priv->propproxy = NULL;
	priv->layoutcall = NULL;

	return;
}

static void
dbusmenu_client_dispose (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_client_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_client_finalize (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_client_parent_class)->finalize (object);
	return;
}

/* Internal funcs */

/* When we have a name and an object, build the two proxies and get the
   first version of the layout */
static void
build_proxies (DbusmenuClient * client)
{


}

/* When the layout property returns, here's where we take care of that. */
static void
update_layout_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{


}

/* Call the property on the server we're connected to and set it up to
   be async back to _update_layout_cb */
static void
update_layout (DbusmenuClient * client)
{


}

/* Public API */
DbusmenuClient *
dbusmenu_client_new (const gchar * name, const gchar * object)
{
	DbusmenuClient * self = g_object_new(DBUSMENU_TYPE_CLIENT, "name", name, "object", object, NULL);

	return self;
}

DbusmenuMenuitem *
dbusmenu_client_get_root (DbusmenuClient * client)
{
	DbusmenuClientPrivate * priv = DBUSMENU_CLIENT_GET_PRIVATE(client);

	if (priv->layoutcall != NULL) {
		/* Oh, we're in the middle of getting it */
		/* TODO: Wait here */
	}

	return priv->root;
}
