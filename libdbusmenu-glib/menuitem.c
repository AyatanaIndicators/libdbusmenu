/*
A library to communicate a menu object set accross DBus and
track updates and maintain consistency.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the 
Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by 
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the applicable version of the GNU Lesser General Public 
License for more details.

You should have received a copy of both the GNU Lesser General Public 
License version 3 and version 2.1 along with this program.  If not, see 
<http://www.gnu.org/licenses/>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "menuitem.h"
#include "menuitem-marshal.h"

/* Private */
/**
	DbusmenuMenuitemPrivate:
	@id: The ID of this menu item
	@children: A list of #DbusmenuMenuitem objects that are
	      children to this one.
	@properties: All of the properties on this menu item.

	These are the little secrets that we don't want getting
	out of data that we have.  They can still be gotten using
	accessor functions, but are protected appropriately.
*/
typedef struct _DbusmenuMenuitemPrivate DbusmenuMenuitemPrivate;
struct _DbusmenuMenuitemPrivate
{
	guint id;
	GList * children;
	GHashTable * properties;
};

/* Signals */
enum {
	PROPERTY_CHANGED,
	ITEM_ACTIVATED,
	CHILD_ADDED,
	CHILD_REMOVED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

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

	/**
		DbusmenuMenuitem::property-changed:
		@arg0: The #DbusmenuMenuitem object.
		@arg1: The name of the property that changed
		@arg2: The new value of the property

		Emitted everytime a property on a menuitem is either
		updated or added.
	*/
	signals[PROPERTY_CHANGED] =   g_signal_new(DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, property_changed),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__STRING_STRING,
	                                           G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);
	/**
		DbusmenuMenuitem::item-activated:
		@arg0: The #DbusmenuMenuitem object.

		Emitted on the objects on the server side when
		they are signaled on the client side.
	*/
	signals[ITEM_ACTIVATED] =   g_signal_new(DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, item_activated),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__VOID,
	                                           G_TYPE_NONE, 0, G_TYPE_NONE);
	/**
		DbusmenuMenuitem::child-added:
		@arg0: The #DbusmenuMenuitem which is the parent.
		@arg1: The #DbusmenuMenuitem which is the child.

		Signaled when the child menuitem has been added to
		the parent.
	*/
	signals[CHILD_ADDED] =        g_signal_new(DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, child_added),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__OBJECT,
	                                           G_TYPE_NONE, 2, G_TYPE_OBJECT);
	/**
		DbusmenuMenuitem::child-removed:
		@arg0: The #DbusmenuMenuitem which was the parent.
		@arg1: The #DbusmenuMenuitem which was the child.

		Signaled when the child menuitem has been requested to
		be removed from the parent.  This signal is called when
		it has been removed from the list but not yet had
		#g_object_unref called on it.
	*/
	signals[CHILD_REMOVED] =      g_signal_new(DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, child_removed),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__OBJECT,
	                                           G_TYPE_NONE, 2, G_TYPE_OBJECT);

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

	priv->properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	
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
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(object);

	if (priv->properties != NULL) {
		g_hash_table_destroy(priv->properties);
		priv->properties = NULL;
	}

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
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), 0);

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

/* For all the taken children we need to signal
   that they were removed */
static void
take_children_signal (gpointer data, gpointer user_data)
{
	g_signal_emit(G_OBJECT(user_data), signals[CHILD_REMOVED], 0, DBUSMENU_MENUITEM(data), TRUE);
	return;
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
	g_list_foreach(children, take_children_signal, mi);
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
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(child), FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_append(priv->children, child);
	g_signal_emit(G_OBJECT(mi), signals[CHILD_ADDED], 0, child, TRUE);
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
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(child), FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_remove(priv->children, child);
	g_signal_emit(G_OBJECT(mi), signals[CHILD_REMOVED], 0, child, TRUE);
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
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(child), FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->children = g_list_insert(priv->children, child, position);
	g_signal_emit(G_OBJECT(mi), signals[CHILD_ADDED], 0, child, TRUE);
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
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);

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

typedef struct {
	DbusmenuMenuitem * mi;
	guint id;
} find_id_t;

/* Basically the heart of the find_id that matches the
   API of GFunc.  Unfortunately, this goes through all the
   children, but it rejects them quickly. */
static void
find_id_helper (gpointer in_mi, gpointer in_find_id)
{
	DbusmenuMenuitem * mi = (DbusmenuMenuitem *)in_mi;
	find_id_t * find_id = (find_id_t *)in_find_id;

	if (find_id->mi != NULL) return;
	if (find_id->id == dbusmenu_menuitem_get_id(mi)) {
		find_id->mi = mi;
		return;
	}

	g_list_foreach(dbusmenu_menuitem_get_children(mi), find_id_helper, in_find_id);
	return;
}

/**
	dbusmenu_menuitem_find_id:
	@mi: #DbusmenuMenuitem at the top of the tree to search
	@id: ID of the #DbusmenuMenuitem to search for

	This function searchs the whole tree of children that
	are attached to @mi.  This could be quite a few nodes, all
	the way down the tree.  It is a depth first search.

	Return value: The #DbusmenuMenuitem with the ID of @id
		or #NULL if there isn't such a menu item in the tree
		represented by @mi.
*/
DbusmenuMenuitem *
dbusmenu_menuitem_find_id (DbusmenuMenuitem * mi, guint id)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);
	find_id_t find_id = {mi: NULL, id: id};
	find_id_helper(mi, &find_id);
	return find_id.mi;
}

/**
	dbusmenu_menuitem_property_set:
	@mi: The #DbusmenuMenuitem to set the property on.
	@property: Name of the property to set.
	@value: The value of the property.

	Takes the pair of @property and @value and places them as a
	property on @mi.  If a property already exists by that name,
	then the value is set to the new value.  If not, the property
	is added.  If the value is changed or the property was previously
	unset then the signal #DbusmenuMenuitem::prop-changed will be
	emitted by this function.

	Return value:  A boolean representing if the property value was set.
*/
gboolean
dbusmenu_menuitem_property_set (DbusmenuMenuitem * mi, const gchar * property, const gchar * value)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(property != NULL, FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);

	gpointer lookup = g_hash_table_lookup(priv->properties, property);
	if (g_strcmp0((gchar *)lookup, value) == 0) {
		/* The value is the same as the value currently in the
		   table so we don't really care.  Just say everything's okay */
		return TRUE;
	}

	gchar * lprop = g_strdup(property);
	gchar * lval = g_strdup(value);

	g_hash_table_insert(priv->properties, lprop, lval);
	g_signal_emit(G_OBJECT(mi), signals[PROPERTY_CHANGED], 0, property, value, TRUE);

	return TRUE;
}

/**
	dbusmenu_menuitem_property_get:
	@mi: The #DbusmenuMenuitem to look for the property on.
	@property: The property to grab.

	Look up a property on @mi and return the value of it if
	it exits.  #NULL will be returned if the property doesn't
	exist.

	Return value: A string with the value of the property
		that shouldn't be free'd.  Or #NULL if the property
		is not set.
*/
const gchar *
dbusmenu_menuitem_property_get (DbusmenuMenuitem * mi, const gchar * property)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);
	g_return_val_if_fail(property != NULL, NULL);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);

	return (const gchar *)g_hash_table_lookup(priv->properties, property);
}

/**
	dbusmenu_menuitem_property_exit:
	@mi: The #DbusmenuMenuitem to look for the property on.
	@property: The property to look for.

	Checkes to see if a particular property exists on @mi and 
	returns #TRUE if so.

	Return value: A boolean checking to see if the property is available
*/
gboolean
dbusmenu_menuitem_property_exist (DbusmenuMenuitem * mi, const gchar * property)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(property != NULL, FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);

	gpointer value = g_hash_table_lookup(priv->properties, property);

	return value != NULL;
}

/**
	dbusmenu_menuitem_properties_list:
	@mi: #DbusmenuMenuitem to list the properties on

	This functiong gets a list of the names of all the properties
	that are set on this menu item.  This data on the list is owned
	by the menuitem but the list is not and should be freed using
	g_list_free() when the calling function is done with it.

	Return value: A list of strings or NULL if there are none.
*/
GList *
dbusmenu_menuitem_properties_list (DbusmenuMenuitem * mi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	return g_hash_table_get_keys(priv->properties);
}

static void
copy_helper (gpointer in_key, gpointer in_value, gpointer in_data)
{
	GHashTable * table = (GHashTable *)in_data;
	g_hash_table_insert(table, in_key, in_value);
	return;
}

/**
	dbusmenu_menuitem_properties_copy:
	@mi: #DbusmenuMenuitem that we're interested in the properties of

	This function takes the properties of a #DbusmenuMenuitem
	and puts them into a #GHashTable that is referenced by the
	key of a string and has the value of a string.  The hash
	table may not have any entries if there aren't any or there
	is an error in processing.  It is the caller's responsibility
	to destroy the created #GHashTable.

	Return value: A brand new #GHashTable that contains all of the
		properties that are on this #DbusmenuMenuitem @mi.
*/
GHashTable *
dbusmenu_menuitem_properties_copy (DbusmenuMenuitem * mi)
{
	GHashTable * ret = g_hash_table_new(g_str_hash, g_str_equal);

	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), ret);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	g_hash_table_foreach(priv->properties, copy_helper, ret);

	return ret;
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

typedef struct {
	void (*func) (DbusmenuMenuitem * mi, gpointer data);
	gpointer data;
} foreach_struct_t;

static void
foreach_helper (gpointer data, gpointer user_data)
{
	dbusmenu_menuitem_foreach(DBUSMENU_MENUITEM(data), ((foreach_struct_t *)user_data)->func, ((foreach_struct_t *)user_data)->data);
	return;
}

/**
	dbusmenu_menuitem_foreach:
	@mi: The #DbusmenItem to start from
	@func: Function to call on every node in the tree
	@data: User data to pass to the function

	This calls the function @func on this menu item and all
	of the children of this item.  And their children.  And
	their children.  And... you get the point.  It will get
	called on the whole tree.
*/
void
dbusmenu_menuitem_foreach (DbusmenuMenuitem * mi, void (*func) (DbusmenuMenuitem * mi, gpointer data), gpointer data)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	g_return_if_fail(func != NULL);

	func(mi, data);
	GList * children = dbusmenu_menuitem_get_children(mi);
	foreach_struct_t foreach_data = {func: func, data: data};
	g_list_foreach(children, foreach_helper, &foreach_data);
	return;
}

/**
	dbusmenu_menuitem_activate:
	@mi: The #DbusmenuMenuitem to send the signal on.

	Emits the #DbusmenuMenuitem::item-activate signal on this
	menu item.  Called by server objects when they get the
	appropriate DBus signals from the client.
*/
void
dbusmenu_menuitem_activate (DbusmenuMenuitem * mi)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	g_signal_emit(G_OBJECT(mi), signals[ITEM_ACTIVATED], 0, TRUE);
	return;
}
