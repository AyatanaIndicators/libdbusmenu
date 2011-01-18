#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "idoserializablemenuitem.h"

struct _IdoSerializableMenuItemPrivate {
	int dummy;
};

#define IDO_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), IDO_TYPE_SERIALIZABLE_MENU_ITEM, IdoSerializableMenuItemPrivate))

static void ido_serializable_menu_item_class_init (IdoSerializableMenuItemClass *klass);
static void ido_serializable_menu_item_init       (IdoSerializableMenuItem *self);
static void ido_serializable_menu_item_dispose    (GObject *object);
static void ido_serializable_menu_item_finalize   (GObject *object);

G_DEFINE_TYPE (IdoSerializableMenuItem, ido_serializable_menu_item, GTK_TYPE_MENU_ITEM);

static void
ido_serializable_menu_item_class_init (IdoSerializableMenuItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IdoSerializableMenuItemPrivate));

	object_class->dispose = ido_serializable_menu_item_dispose;
	object_class->finalize = ido_serializable_menu_item_finalize;

	return;
}

static void
ido_serializable_menu_item_init (IdoSerializableMenuItem *self)
{
	self->priv = IDO_SERIALIZABLE_MENU_ITEM_GET_PRIVATE(self);

	self->priv->dummy = 5;

	return;
}

static void
ido_serializable_menu_item_dispose (GObject *object)
{


	G_OBJECT_CLASS (ido_serializable_menu_item_parent_class)->dispose (object);
	return;
}

static void
ido_serializable_menu_item_finalize (GObject *object)
{



	G_OBJECT_CLASS (ido_serializable_menu_item_parent_class)->finalize (object);
	return;
}

DbusmenuMenuitem *
ido_serializable_menu_item_get_dbusmenu_menuitem (IdoSerializableMenuItem * smi)
{
	g_return_val_if_fail(IDO_IS_SERIALIZABLE_MENU_ITEM(smi), NULL);

	IdoSerializableMenuItemClass * klass = IDO_SERIALIZABLE_MENU_ITEM_GET_CLASS(smi);
	if (klass->get_dbusmenu_menuitem != NULL) {
		return klass->get_dbusmenu_menuitem(smi);
	}

	return NULL;
}

void
ido_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type)
{
	g_return_if_fail(g_type_is_a(item_type, IDO_TYPE_SERIALIZABLE_MENU_ITEM));

	gpointer type_class = g_type_class_ref(item_type);
	g_return_if_fail(type_class != NULL);

	IdoSerializableMenuItemClass * class = IDO_SERIALIZABLE_MENU_ITEM_CLASS(type_class);

	if (class->get_type_string == NULL) {
		g_type_class_unref(type_class);
		g_error("No 'get_type_string' in subclass of IdoSerializableMenuItem");
		return;
	}

	/* Register type */


	/* Register defaults */


	return;
}
