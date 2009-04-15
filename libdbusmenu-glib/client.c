#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client.h"
#include "dbusmenu-client.h"

typedef struct _DbusmenuClientPrivate DbusmenuClientPrivate;

struct _DbusmenuClientPrivate
{
};

#define DBUSMENU_CLIENT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_CLIENT_TYPE, DbusmenuClientPrivate))

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
