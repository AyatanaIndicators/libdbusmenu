#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "menuitem.h"

/* Private */
typedef struct _DbusmenuMenuitemPrivate DbusmenuMenuitemPrivate;
struct _DbusmenuMenuitemPrivate
{
	guint id;
	GList * children;
};

/* Properties */
enum {
	PROP_0,
	PROP_ID,
};

#define DBUSMENU_MENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemPrivate))

/* Prototypes */
static void dbusmenu_menuitem_class_init (DbusmenuMenuitemClass *klass);
static void dbusmenu_menuitem_init       (DbusmenuMenuitem *self);
static void dbusmenu_menuitem_dispose    (GObject *object);
static void dbusmenu_menuitem_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);

/* GObject stuff */
G_DEFINE_TYPE (DbusmenuMenuitem, dbusmenu_menuitem, G_TYPE_OBJECT);

static void
dbusmenu_menuitem_class_init (DbusmenuMenuitemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuMenuitemPrivate));

	object_class->dispose = dbusmenu_menuitem_dispose;
	object_class->finalize = dbusmenu_menuitem_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_ID,
	                                 g_param_spec_uint("id", "ID for the menu item",
	                                              "This is a unique indentifier for the menu item.",
												  0, 30000, 0,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	return;
}

static guint menuitem_next_id = 1;

static void
dbusmenu_menuitem_init (DbusmenuMenuitem *self)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(self);

	priv->id = 0; 
	priv->children = NULL;
	
	return;
}

static void
dbusmenu_menuitem_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_menuitem_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_menuitem_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_menuitem_parent_class)->finalize (object);
	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(obj);

	switch (id) {
	case PROP_ID:
		priv->id = g_value_get_uint(value);
		if (priv->id > menuitem_next_id) {
			menuitem_next_id = priv->id;
		}
		break;
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(obj);

	switch (id) {
	case PROP_ID:
		if (priv->id == 0) {
			priv->id = menuitem_next_id++;
		}
		g_value_set_uint(value, priv->id);
		break;
	}

	return;
}


/* Public interface */
DbusmenuMenuitem *
dbusmenu_menuitem_new (void)
{
	return g_object_new(DBUSMENU_TYPE_MENUITEM, NULL);
}

DbusmenuMenuitem *
dbusmenu_menuitem_new_with_id (guint id)
{
	DbusmenuMenuitem * mi = g_object_new(DBUSMENU_TYPE_MENUITEM, "id", id, NULL);
	g_debug("New Menuitem id %d goal id %d", dbusmenu_menuitem_get_id(mi), id);
	return mi;
}

guint
dbusmenu_menuitem_get_id (DbusmenuMenuitem * mi)
{
	GValue retval = {0};
	g_value_init(&retval, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(mi), "id", &retval);
	return g_value_get_uint(&retval);
}

GList *
dbusmenu_menuitem_get_children (DbusmenuMenuitem * mi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	return priv->children;
}

/**
	dbusmenu_menuitem_take_children:
	@mi: The #DbusmenMenuitem to take the children from.

	While the name sounds devious that's exactly what this function
	does.  It takes the list of children from the @mi and clears the
	internal list.  The calling function is no in charge of the ref's
	on the children it has taken.  A lot of responsibility involved
	in taking children.
*/
GList *
dbusmenu_menuitem_take_children (DbusmenuMenuitem * mi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	GList * children = priv->children;
	priv->children = NULL;
	return children;
}

guint
dbusmenu_menuitem_get_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent)
{
	/* TODO: I'm not too happy returning zeros here.  But that's all I've got */
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), 0);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(parent), 0);

	GList * childs = dbusmenu_menuitem_get_children(parent);
	guint count = 0;
	for ( ; childs != NULL; childs = childs->next) {
		if (childs->data == mi) break;
	}

	if (childs == NULL) return 0;

	return count;
}

gboolean
dbusmenu_menuitem_child_append (DbusmenuMenuitem * mi, DbusmenuMenuitem * child)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_append(priv->children, child);
	return TRUE;
}

gboolean
dbusmenu_menuitem_child_delete (DbusmenuMenuitem * mi, DbusmenuMenuitem * child)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_remove(priv->children, child);
	return TRUE;
}

gboolean
dbusmenu_menuitem_child_add_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_insert(priv->children, child, position);
	return TRUE;
}

DbusmenuMenuitem *
dbusmenu_menuitem_child_find (DbusmenuMenuitem * mi, guint id)
{
	GList * childs = dbusmenu_menuitem_get_children(mi);
	if (childs == NULL) return NULL;

	for ( ; childs == NULL; childs = g_list_next(childs)) {
		DbusmenuMenuitem * lmi = DBUSMENU_MENUITEM(childs->data);
		if (id == dbusmenu_menuitem_get_id(lmi)) {
			return lmi;
		}
	}

	return NULL;
}

gboolean
dbusmenu_menuitem_property_set (DbusmenuMenuitem * mi, const gchar * property, const gchar * value)
{

	return FALSE;
}

const gchar *
dbusmenu_menuitem_property_get (DbusmenuMenuitem * mi, const gchar * property)
{

	return NULL;
}

gboolean
dbusmenu_menuitem_property_exist (DbusmenuMenuitem * mi, const gchar * property)
{

	return FALSE;
}

void
dbusmenu_menuitem_buildxml (DbusmenuMenuitem * mi, GPtrArray * array)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));

	GList * children = dbusmenu_menuitem_get_children(mi);
	if (children == NULL) {
		g_ptr_array_add(array, g_strdup_printf("<menu id=\"%d\" />", dbusmenu_menuitem_get_id(mi)));
	} else {
		g_ptr_array_add(array, g_strdup_printf("<menu id=\"%d\">", dbusmenu_menuitem_get_id(mi)));
		for ( ; children != NULL; children = children->next) {
			dbusmenu_menuitem_buildxml(DBUSMENU_MENUITEM(children->data), array);
		}
		g_ptr_array_add(array, g_strdup("</menu>"));
	}

	return;
}

