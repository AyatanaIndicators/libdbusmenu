#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client-menuitem.h"

typedef struct _DbusmenuClientMenuitemPrivate DbusmenuClientMenuitemPrivate;

struct _DbusmenuClientMenuitemPrivate
{
	DbusmenuClient * client;
};

#define DBUSMENU_CLIENT_MENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_CLIENT_MENUITEM_TYPE, DbusmenuClientMenuitemPrivate))

static void dbusmenu_client_menuitem_class_init (DbusmenuClientMenuitemClass *klass);
static void dbusmenu_client_menuitem_init       (DbusmenuClientMenuitem *self);
static void dbusmenu_client_menuitem_dispose    (GObject *object);
static void dbusmenu_client_menuitem_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuClientMenuitem, dbusmenu_client_menuitem, DBUSMENU_TYPE_MENUITEM);

static void
dbusmenu_client_menuitem_class_init (DbusmenuClientMenuitemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuClientMenuitemPrivate));

	object_class->dispose = dbusmenu_client_menuitem_dispose;
	object_class->finalize = dbusmenu_client_menuitem_finalize;

	return;
}

static void
dbusmenu_client_menuitem_init (DbusmenuClientMenuitem *self)
{

	return;
}

static void
dbusmenu_client_menuitem_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_client_menuitem_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_client_menuitem_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_client_menuitem_parent_class)->finalize (object);
	return;
}

DbusmenuClientMenuitem *
dbusmenu_client_menuitem_new (gint id, DbusmenuClient * client)
{
	DbusmenuClientMenuitem * mi = g_object_new(DBUSMENU_CLIENT_MENUITEM_TYPE, "id", id, NULL);
	DbusmenuClientMenuitemPrivate * priv = DBUSMENU_CLIENT_MENUITEM_GET_PRIVATE(mi);
	priv->client = client;
	return mi;
}
