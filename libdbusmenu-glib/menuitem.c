#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "menuitem.h"

/* Private */
/**
	DbusmenuMenuitemPrivate:
	@id: The ID of this menu item
	@children: A list of #DbusmenuMenuitem objects that are
	      children to this one.

	These are the little secrets that we don't want getting
	out of data that we have.  They can still be gotten using
	accessor functions, but are protected appropriately.
*/
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

/**
	dbusmenu_menuitem_new:

	Create a new #DbusmenuMenuitem with all default values.

	Return value: A newly allocated #DbusmenuMenuitem.
*/
DbusmenuMenuitem *
dbusmenu_menuitem_new (void)
{
	return g_object_new(DBUSMENU_TYPE_MENUITEM, NULL);
}

/**
	dbusmenu_menuitem_new_with_id:
	@id: ID to use for this menuitem

	This creates a blank #DbusmenuMenuitem with a specific ID.

	Return value: A newly allocated #DbusmenuMenuitem.
*/
DbusmenuMenuitem *
dbusmenu_menuitem_new_with_id (guint id)
{
	DbusmenuMenuitem * mi = g_object_new(DBUSMENU_TYPE_MENUITEM, "id", id, NULL);
	g_debug("New Menuitem id %d goal id %d", dbusmenu_menuitem_get_id(mi), id);
	return mi;
}

/**
	dbusmenu_menuitem_get_id:
	@mi: The #DbusmenuMenuitem to query.

	Gets the unique ID for @mi.

	Return value: The ID of the @mi.
*/
guint
dbusmenu_menuitem_get_id (DbusmenuMenuitem * mi)
{
	GValue retval = {0};
	g_value_init(&retval, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(mi), "id", &retval);
	return g_value_get_uint(&retval);
}

/**
	dbusmenu_menuitem_get_children:
	@mi: The #DbusmenuMenuitem to query.

	Returns simply the list of children that this menu item
	has.  The list is valid until another child related function
	is called, where it might be changed.

	Return value: A #GList of pointers to #DbusmenuMenuitem objects.
*/
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

	Return value: A #GList of pointers to #DbusmenuMenuitem objects.
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

/**
	dbusmenu_menuitem_get_position:
	@mi: The #DbusmenuMenuitem to find the position of
	@parent: The #DbusmenuMenuitem who's children contain @mi

	This function returns the position of the menu item @mi
	in the children of @parent.  It will return zero if the
	menu item can't be found.

	Return value: The position of @mi in the children of @parent.
*/
guint
dbusmenu_menuitem_get_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent)
{
	/* TODO: I'm not too happy returning zeros here.  But that's all I've got */
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), 0);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(parent), 0);

	GList * childs = dbusmenu_menuitem_get_children(parent);
	if (childs == NULL) return 0;
	guint count = 0;
	for ( ; childs != NULL; childs = childs->next, count++) {
		if (childs->data == mi) break;
	}

	if (childs == NULL) return 0;

	return count;
}

/**
	dbusmenu_menuitem_child_append:
	@mi: The #DbusmenuMenuitem which will become a new parent
	@child: The #DbusmenMenuitem that will be a child

	This function adds @child to the list of children on @mi at
	the end of that list.

	Return value: Whether the child has been added successfully.
*/
gboolean
dbusmenu_menuitem_child_append (DbusmenuMenuitem * mi, DbusmenuMenuitem * child)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_append(priv->children, child);
	return TRUE;
}

/**
	dbusmenu_menuitem_child_delete:
	@mi: The #DbusmenuMenuitem which has @child as a child
	@child: The child #DbusmenuMenuitem that you want to no longer
	    be a child of @mi.
	
	This function removes @child from the children list of @mi.  It does
	not call #g_object_unref on @child.

	Return value: If we were able to delete @child.
*/
gboolean
dbusmenu_menuitem_child_delete (DbusmenuMenuitem * mi, DbusmenuMenuitem * child)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_remove(priv->children, child);
	return TRUE;
}

/**
	dbusmenu_menuitem_child_add_position:
	@mi: The #DbusmenuMenuitem that we're adding the child @child to.
	@child: The #DbusmenuMenuitem to make a child of @mi.
	@position: Where in @mi object's list of chidren @child should be placed.

	Puts @child in the list of children for @mi at the location
	specified in @position.  If there is not enough entires available
	then @child will be placed at the end of the list.

	Return value: Whether @child was added successfully.
*/
gboolean
dbusmenu_menuitem_child_add_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_insert(priv->children, child, position);
	return TRUE;
}

/**
	dbusmenu_menuitem_child_find:
	@mi: The #DbusmenuMenuitem who's children to look on
	@id: The ID of the child that we're looking for.

	Search the children of @mi to find one with the ID of @id.
	If it doesn't exist then we return #NULL.

	Return value: The menu item with the ID @id or #NULL if it
	   can't be found.
*/
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

/**
	dbusmenu_menuitem_buildxml:
	@mi: #DbusmenuMenuitem to represent in XML
	@array: A list of string that will be turned into an XML file

	This function will add strings to the array @array.  It will put
	at least one entry if this menu item has no children.  If it has
	children it will put two for this entry, one representing the
	start tag and one that is a closing tag.  It will allow it's
	children to place their own tags in the array in between those two.
*/
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

