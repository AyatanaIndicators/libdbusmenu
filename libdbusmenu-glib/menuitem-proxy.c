#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "menuitem-proxy.h"

typedef struct _DbusmenuMenuitemProxyPrivate DbusmenuMenuitemProxyPrivate;
struct _DbusmenuMenuitemProxyPrivate {
	gint placeholder;
};

#define DBUSMENU_MENUITEM_PROXY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_MENUITEM_PROXY, DbusmenuMenuitemProxyPrivate))

static void dbusmenu_menuitem_proxy_class_init (DbusmenuMenuitemProxyClass *klass);
static void dbusmenu_menuitem_proxy_init       (DbusmenuMenuitemProxy *self);
static void dbusmenu_menuitem_proxy_dispose    (GObject *object);
static void dbusmenu_menuitem_proxy_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuMenuitemProxy, dbusmenu_menuitem_proxy, DBUSMENU_TYPE_MENUITEM);

static void
dbusmenu_menuitem_proxy_class_init (DbusmenuMenuitemProxyClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuMenuitemProxyPrivate));

	object_class->dispose = dbusmenu_menuitem_proxy_dispose;
	object_class->finalize = dbusmenu_menuitem_proxy_finalize;

	return;
}

static void
dbusmenu_menuitem_proxy_init (DbusmenuMenuitemProxy *self)
{

	return;
}

static void
dbusmenu_menuitem_proxy_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_menuitem_proxy_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_menuitem_proxy_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_menuitem_proxy_parent_class)->finalize (object);
	return;
}
