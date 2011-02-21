#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "defaults.h"

typedef struct _DbusmenuDefaultsPrivate DbusmenuDefaultsPrivate;

struct _DbusmenuDefaultsPrivate
{
};

#define DBUSMENU_DEFAULTS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_DEFAULTS_TYPE, DbusmenuDefaultsPrivate))

static void dbusmenu_defaults_class_init (DbusmenuDefaultsClass *klass);
static void dbusmenu_defaults_init       (DbusmenuDefaults *self);
static void dbusmenu_defaults_dispose    (GObject *object);
static void dbusmenu_defaults_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuDefaults, dbusmenu_defaults, G_TYPE_OBJECT);

static void
dbusmenu_defaults_class_init (DbusmenuDefaultsClass *klass)
{
GObjectClass *object_class = G_OBJECT_CLASS (klass);

g_type_class_add_private (klass, sizeof (DbusmenuDefaultsPrivate));

object_class->dispose = dbusmenu_defaults_dispose;
object_class->finalize = dbusmenu_defaults_finalize;
}

static void
dbusmenu_defaults_init (DbusmenuDefaults *self)
{
}

static void
dbusmenu_defaults_dispose (GObject *object)
{
G_OBJECT_CLASS (dbusmenu_defaults_parent_class)->dispose (object);
}

static void
dbusmenu_defaults_finalize (GObject *object)
{
G_OBJECT_CLASS (dbusmenu_defaults_parent_class)->finalize (object);
}
