#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "genericmenuitem.h"

typedef struct _GenericmenuitemPrivate GenericmenuitemPrivate;

struct _GenericmenuitemPrivate
{
};

#define GENERICMENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), GENERICMENUITEM_TYPE, GenericmenuitemPrivate))

static void genericmenuitem_class_init (GenericmenuitemClass *klass);
static void genericmenuitem_init       (Genericmenuitem *self);
static void genericmenuitem_dispose    (GObject *object);
static void genericmenuitem_finalize   (GObject *object);

G_DEFINE_TYPE (Genericmenuitem, genericmenuitem, GTK_TYPE_CHECK_MENU_ITEM);

static void
genericmenuitem_class_init (GenericmenuitemClass *klass)
{
GObjectClass *object_class = G_OBJECT_CLASS (klass);

g_type_class_add_private (klass, sizeof (GenericmenuitemPrivate));

object_class->dispose = genericmenuitem_dispose;
object_class->finalize = genericmenuitem_finalize;
}

static void
genericmenuitem_init (Genericmenuitem *self)
{
}

static void
genericmenuitem_dispose (GObject *object)
{
G_OBJECT_CLASS (genericmenuitem_parent_class)->dispose (object);
}

static void
genericmenuitem_finalize (GObject *object)
{
G_OBJECT_CLASS (genericmenuitem_parent_class)->finalize (object);
}
