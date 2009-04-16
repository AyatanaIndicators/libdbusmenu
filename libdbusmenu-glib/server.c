#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server.h"

/* DBus Prototypes */
static gboolean _dbusmenu_server_get_property (void);
static gboolean _dbusmenu_server_get_properties (void);
static gboolean _dbusmenu_server_call (void);
static gboolean _dbusmenu_server_list_properties (void);

#include "dbusmenu-server.h"

typedef struct _DbusmenuServerPrivate DbusmenuServerPrivate;

struct _DbusmenuServerPrivate
{
	DbusmenuMenuitem * root;
};

#define DBUSMENU_SERVER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_SERVER, DbusmenuServerPrivate))

static void dbusmenu_server_class_init (DbusmenuServerClass *klass);
static void dbusmenu_server_init       (DbusmenuServer *self);
static void dbusmenu_server_dispose    (GObject *object);
static void dbusmenu_server_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuServer, dbusmenu_server, G_TYPE_OBJECT);

static void
dbusmenu_server_class_init (DbusmenuServerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuServerPrivate));

	object_class->dispose = dbusmenu_server_dispose;
	object_class->finalize = dbusmenu_server_finalize;

	dbus_g_object_type_install_info(DBUSMENU_TYPE_SERVER, &dbus_glib__dbusmenu_server_object_info);

	return;
}

static void
dbusmenu_server_init (DbusmenuServer *self)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(self);

	priv->root = NULL;

	return;
}

static void
dbusmenu_server_dispose (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_server_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_server_finalize (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_server_parent_class)->finalize (object);
	return;
}

/* DBus Prototypes */
static gboolean 
_dbusmenu_server_get_property (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_get_properties (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_call (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_list_properties (void)
{

	return TRUE;
}

