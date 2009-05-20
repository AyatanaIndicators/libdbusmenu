#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "menu.h"

typedef struct _DbusmenuGtkMenuPrivate DbusmenuGtkMenuPrivate;

struct _DbusmenuGtkMenuPrivate
{
};

#define DBUSMENU_GTKMENU_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_GTKMENU_TYPE, DbusmenuGtkMenuPrivate))

static void dbusmenu_gtkmenu_class_init (DbusmenuGtkMenuClass *klass);
static void dbusmenu_gtkmenu_init       (DbusmenuGtkMenu *self);
static void dbusmenu_gtkmenu_dispose    (GObject *object);
static void dbusmenu_gtkmenu_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuGtkMenu, dbusmenu_gtkmenu, GTK_TYPE_MENU);

static void
dbusmenu_gtkmenu_class_init (DbusmenuGtkMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuGtkMenuPrivate));

	object_class->dispose = dbusmenu_gtkmenu_dispose;
	object_class->finalize = dbusmenu_gtkmenu_finalize;

	return;
}

static void
dbusmenu_gtkmenu_init (DbusmenuGtkMenu *self)
{

	return;
}

static void
dbusmenu_gtkmenu_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkmenu_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtkmenu_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_gtkmenu_parent_class)->finalize (object);
	return;
}
