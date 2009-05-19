#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "menu.h"

typedef struct _MenuPrivate MenuPrivate;

struct _MenuPrivate
{
};

#define MENU_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), MENU_TYPE, MenuPrivate))

static void menu_class_init (MenuClass *klass);
static void menu_init       (Menu *self);
static void menu_dispose    (GObject *object);
static void menu_finalize   (GObject *object);

G_DEFINE_TYPE (Menu, menu, GTK_TYPE_MENU);

static void
menu_class_init (MenuClass *klass)
{
GObjectClass *object_class = G_OBJECT_CLASS (klass);

g_type_class_add_private (klass, sizeof (MenuPrivate));

object_class->dispose = menu_dispose;
object_class->finalize = menu_finalize;
}

static void
menu_init (Menu *self)
{
}

static void
menu_dispose (GObject *object)
{
G_OBJECT_CLASS (menu_parent_class)->dispose (object);
}

static void
menu_finalize (GObject *object)
{
G_OBJECT_CLASS (menu_parent_class)->finalize (object);
}
