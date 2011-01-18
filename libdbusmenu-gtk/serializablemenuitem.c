#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "serializablemenuitem.h"

struct _DbusmenuGtkSerializableMenuItemPrivate {
	int dummy;
};

#define DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM, DbusmenuGtkSerializableMenuItemPrivate))

static void dbusmenu_gtk_serializable_menu_item_class_init (DbusmenuGtkSerializableMenuItemClass *klass);
static void dbusmenu_gtk_serializable_menu_item_init       (DbusmenuGtkSerializableMenuItem *self);
static void dbusmenu_gtk_serializable_menu_item_dispose    (GObject *object);
static void dbusmenu_gtk_serializable_menu_item_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuGtkSerializableMenuItem, dbusmenu_gtk_serializable_menu_item, GTK_TYPE_MENU_ITEM);

static void
dbusmenu_gtk_serializable_menu_item_class_init (DbusmenuGtkSerializableMenuItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuGtkSerializableMenuItemPrivate));

	object_class->dispose = dbusmenu_gtk_serializable_menu_item_dispose;
	object_class->finalize = dbusmenu_gtk_serializable_menu_item_finalize;

	return;
}

static void
dbusmenu_gtk_serializable_menu_item_init (DbusmenuGtkSerializableMenuItem *self)
{
	self->priv = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(self);

	self->priv->dummy = 5;

	return;
}

static void
dbusmenu_gtk_serializable_menu_item_dispose (GObject *object)
{


	G_OBJECT_CLASS (dbusmenu_gtk_serializable_menu_item_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtk_serializable_menu_item_finalize (GObject *object)
{



	G_OBJECT_CLASS (dbusmenu_gtk_serializable_menu_item_parent_class)->finalize (object);
	return;
}

DbusmenuMenuitem *
dbusmenu_gtk_serializable_menu_item_get_dbusmenu_menuitem (DbusmenuGtkSerializableMenuItem * smi)
{
	g_return_val_if_fail(DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM(smi), NULL);

	DbusmenuGtkSerializableMenuItemClass * klass = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_GET_CLASS(smi);
	if (klass->get_dbusmenu_menuitem != NULL) {
		return klass->get_dbusmenu_menuitem(smi);
	}

	return NULL;
}

/* Handle the type with this item. */
static gboolean
type_handler (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{

	return TRUE;
}

void
dbusmenu_gtk_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type)
{
	g_return_if_fail(g_type_is_a(item_type, DBUSMENU_TYPE_GTK_SERIALIZABLE_MENU_ITEM));

	gpointer type_class = g_type_class_ref(item_type);
	g_return_if_fail(type_class != NULL);

	DbusmenuGtkSerializableMenuItemClass * class = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM_CLASS(type_class);

	if (class->get_type_string == NULL) {
		g_type_class_unref(type_class);
		g_error("No 'get_type_string' in subclass of DbusmenuGtkSerializableMenuItem");
		return;
	}

	/* Register type */
	dbusmenu_client_add_type_handler(client, class->get_type_string(), type_handler); /* need type */

	/* Register defaults */
	/* TODO: Need API on another branch */

	return;
}
