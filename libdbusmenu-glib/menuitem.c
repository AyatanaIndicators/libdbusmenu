#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "menuitem.h"

typedef struct _DbusmenuMenuitemPrivate DbusmenuMenuitemPrivate;

struct _DbusmenuMenuitemPrivate
{
};

#define DBUSMENU_MENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemPrivate))

static void dbusmenu_menuitem_class_init (DbusmenuMenuitemClass *klass);
static void dbusmenu_menuitem_init       (DbusmenuMenuitem *self);
static void dbusmenu_menuitem_dispose    (GObject *object);
static void dbusmenu_menuitem_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuMenuitem, dbusmenu_menuitem, G_TYPE_OBJECT);

static void
dbusmenu_menuitem_class_init (DbusmenuMenuitemClass *klass)
{
GObjectClass *object_class = G_OBJECT_CLASS (klass);

g_type_class_add_private (klass, sizeof (DbusmenuMenuitemPrivate));

object_class->dispose = dbusmenu_menuitem_dispose;
object_class->finalize = dbusmenu_menuitem_finalize;
}

static void
dbusmenu_menuitem_init (DbusmenuMenuitem *self)
{
}

static void
dbusmenu_menuitem_dispose (GObject *object)
{
G_OBJECT_CLASS (dbusmenu_menuitem_parent_class)->dispose (object);
}

static void
dbusmenu_menuitem_finalize (GObject *object)
{
G_OBJECT_CLASS (dbusmenu_menuitem_parent_class)->finalize (object);
}
