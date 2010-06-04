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

#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "menuitem.h"
#include "menuitem-marshal.h"
#include "menuitem-private.h"

#ifdef MASSIVEDEBUGGING
#define LABEL(x)  dbusmenu_menuitem_property_get(DBUSMENU_MENUITEM(x), DBUSMENU_MENUITEM_PROP_LABEL)
#define ID(x)     dbusmenu_menuitem_get_id(DBUSMENU_MENUITEM(x))
#endif

/* Private */
/**
	DbusmenuMenuitemPrivate:
	@id: The ID of this menu item
	@children: A list of #DbusmenuMenuitem objects that are
	      children to this one.
	@properties: All of the properties on this menu item.
	@root: Whether this node is the root node

	These are the little secrets that we don't want getting
	out of data that we have.  They can still be gotten using
	accessor functions, but are protected appropriately.
*/
typedef struct _DbusmenuMenuitemPrivate DbusmenuMenuitemPrivate;
struct _DbusmenuMenuitemPrivate
{
	gint id;
	GList * children;
	GHashTable * properties;
	gboolean root;
	gboolean realized;
};

/* Signals */
enum {
	PROPERTY_CHANGED,
	ITEM_ACTIVATED,
	CHILD_ADDED,
	CHILD_REMOVED,
	CHILD_MOVED,
	REALIZED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Properties */
enum {
	PROP_0,
	PROP_ID,
};

#define PROP_ID_S  "id"

#define DBUSMENU_MENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemPrivate))

/* Prototypes */
static void dbusmenu_menuitem_class_init (DbusmenuMenuitemClass *klass);
static void dbusmenu_menuitem_init       (DbusmenuMenuitem *self);
static void dbusmenu_menuitem_dispose    (GObject *object);
static void dbusmenu_menuitem_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
static void g_value_transform_STRING_BOOLEAN (const GValue * in, GValue * out);
static void g_value_transform_STRING_INT (const GValue * in, GValue * out);
static void handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp);

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

	klass->handle_event = handle_event;

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
	                                           _dbusmenu_menuitem_marshal_VOID__STRING_POINTER,
	                                           G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_POINTER);
	/**
		DbusmenuMenuitem::item-activated:
		@arg0: The #DbusmenuMenuitem object.
		@arg1: The timestamp of when it was activated

		Emitted on the objects on the server side when
		they are signaled on the client side.
	*/
	signals[ITEM_ACTIVATED] =   g_signal_new(DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, item_activated),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__UINT,
	                                           G_TYPE_NONE, 1, G_TYPE_UINT, G_TYPE_NONE);
	/**
		DbusmenuMenuitem::child-added:
		@arg0: The #DbusmenuMenuitem which is the parent.
		@arg1: The #DbusmenuMenuitem which is the child.
		@arg2: The position that the child is being added in.

		Signaled when the child menuitem has been added to
		the parent.
	*/
	signals[CHILD_ADDED] =        g_signal_new(DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, child_added),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__OBJECT_UINT,
	                                           G_TYPE_NONE, 2, G_TYPE_OBJECT, G_TYPE_UINT);
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
	                                           G_TYPE_NONE, 1, G_TYPE_OBJECT);
	/**
		DbusmenuMenuitem::child-moved:
		@arg0: The #DbusmenuMenuitem which is the parent.
		@arg1: The #DbusmenuMenuitem which is the child.
		@arg2: The position that the child is being moved to.
		@arg3: The position that the child is was in.

		Signaled when the child menuitem has had it's location
		in the list change.
	*/
	signals[CHILD_MOVED] =        g_signal_new(DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, child_moved),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__OBJECT_UINT_UINT,
	                                           G_TYPE_NONE, 3, G_TYPE_OBJECT, G_TYPE_UINT, G_TYPE_UINT);
	/**
		DbusmenuMenuitem::realized:
		@arg0: The #DbusmenuMenuitem object.

		Emitted when the initial request for properties
		is complete on the item.  If there is a type
		handler configured for the "type" parameter
		that will be executed before this is signaled.
	*/
	signals[REALIZED] =           g_signal_new(DBUSMENU_MENUITEM_SIGNAL_REALIZED,
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET(DbusmenuMenuitemClass, realized),
	                                           NULL, NULL,
	                                           _dbusmenu_menuitem_marshal_VOID__VOID,
	                                           G_TYPE_NONE, 0, G_TYPE_NONE);

	g_object_class_install_property (object_class, PROP_ID,
	                                 g_param_spec_int(PROP_ID_S, "ID for the menu item",
	                                              "This is a unique indentifier for the menu item.",
												  -1, 30000, -1,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	/* Check transfer functions for GValue */
	if (!g_value_type_transformable(G_TYPE_STRING, G_TYPE_BOOLEAN)) {
		g_value_register_transform_func(G_TYPE_STRING, G_TYPE_BOOLEAN, g_value_transform_STRING_BOOLEAN);
	}
	if (!g_value_type_transformable(G_TYPE_STRING, G_TYPE_INT)) {
		g_value_register_transform_func(G_TYPE_STRING, G_TYPE_INT, g_value_transform_STRING_INT);
	}

	return;
}

/* A little helper function to translate a string into
   a boolean value */
static void
g_value_transform_STRING_BOOLEAN (const GValue * in, GValue * out)
{
	const gchar * string = g_value_get_string(in);
	if (!g_strcmp0(string, "TRUE") || !g_strcmp0(string, "true") || !g_strcmp0(string, "True")) {
		g_value_set_boolean(out, TRUE);
	} else {
		g_value_set_boolean(out, FALSE);
	}
	return;
}

/* A little helper function to translate a string into
   a integer value */
static void
g_value_transform_STRING_INT (const GValue * in, GValue * out)
{
	g_value_set_int(out, atoi(g_value_get_string(in)));
	return;
}

static gint menuitem_next_id = 1;

/* A small little function to both clear the insides of a 
   value as well as the memory it itself uses. */
static void
_g_value_free (gpointer data)
{
	if (data == NULL) return;
	GValue * value = (GValue*)data;
	g_value_unset(value);
	g_free(data);
	return;
}

/* Initialize the values of the in the object, and build the
   properties hash table. */
static void
dbusmenu_menuitem_init (DbusmenuMenuitem *self)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(self);

	priv->id = -1; 
	priv->children = NULL;

	priv->properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _g_value_free);

	priv->root = FALSE;
	priv->realized = FALSE;
	
	return;
}

static void
dbusmenu_menuitem_dispose (GObject *object)
{
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(object);

	GList * child = NULL;
	for (child = priv->children; child != NULL; child = g_list_next(child)) {
		g_object_unref(G_OBJECT(child->data));
	}
	g_list_free(priv->children);
	priv->children = NULL;

	G_OBJECT_CLASS (dbusmenu_menuitem_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_menuitem_finalize (GObject *object)
{
	/* g_debug("Menuitem dying"); */
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
		priv->id = g_value_get_int(value);
		if (priv->id > menuitem_next_id) {
			menuitem_next_id = priv->id + 1;
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, pspec);
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
		if (priv->id == -1) {
			priv->id = menuitem_next_id++;
		}
		if (dbusmenu_menuitem_get_root(DBUSMENU_MENUITEM(obj))) {
			g_value_set_int(value, 0);
		} else {
			g_value_set_int(value, priv->id);
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, pspec);
		break;
	}

	return;
}

/* Handles the activate event if it is sent. */
static void
handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp)
{
	if (g_strcmp0(name, "clicked") == 0) {
		g_signal_emit(G_OBJECT(mi), signals[ITEM_ACTIVATED], 0, timestamp, TRUE);
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
dbusmenu_menuitem_new_with_id (gint id)
{
	DbusmenuMenuitem * mi = g_object_new(DBUSMENU_TYPE_MENUITEM, PROP_ID_S, id, NULL);
	/* g_debug("New Menuitem id %d goal id %d", dbusmenu_menuitem_get_id(mi), id); */
	return mi;
}

/**
	dbusmenu_menuitem_get_id:
	@mi: The #DbusmenuMenuitem to query.

	Gets the unique ID for @mi.

	Return value: The ID of the @mi.
*/
gint
dbusmenu_menuitem_get_id (DbusmenuMenuitem * mi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), -1);

	GValue retval = {0};
	g_value_init(&retval, G_TYPE_INT);
	g_object_get_property(G_OBJECT(mi), PROP_ID_S, &retval);
	gint ret = g_value_get_int(&retval);
	#ifdef MASSIVEDEBUGGING
	g_debug("Getting menuitem ID: %d", ret);
	#endif
	return ret;
}

/**
	dbusmenu_menuitem_realized:
	@mi: #DbusmenuMenuitem to check on

	This function returns whether the menuitem has been realized or
	not.  This is significant mostly in client implementations that
	can use this additional state to see if the second layers of
	the implementation have been built yet.

	Return value: Returns whether or not the menu item has been realized
		yet or not.
*/
gboolean
dbusmenu_menuitem_realized (DbusmenuMenuitem * mi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	return priv->realized;
}

/**
	dbusmenu_menuitem_set_realized:
	@mi: #DbusmenuMenuitem to realize

	Sets the internal variable tracking whether it's been realized and
	signals the DbusmenuMenuitem::realized event.
*/
void
dbusmenu_menuitem_set_realized (DbusmenuMenuitem * mi)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	if (priv->realized) {
		g_warning("Realized entry realized again?  ID: %d", dbusmenu_menuitem_get_id(mi));
	}
	priv->realized = TRUE;
	g_signal_emit(G_OBJECT(mi), signals[REALIZED], 0, TRUE);
	return;
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
	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) signalling child removed %d (%s)", ID(user_data), LABEL(user_data), ID(data), LABEL(data));
	#endif
	g_signal_emit(G_OBJECT(user_data), signals[CHILD_REMOVED], 0, DBUSMENU_MENUITEM(data), TRUE);
	g_object_unref(G_OBJECT(data));
	return;
}

/**
	dbusmenu_menuitem_take_children:
	@mi: The #DbusmenMenuitem to take the children from.

	While the name sounds devious that's exactly what this function
	does.  It takes the list of children from the @mi and clears the
	internal list.  The calling function is now in charge of the ref's
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
	#ifdef MASSIVEDEBUGGING
	if (!DBUSMENU_IS_MENUITEM(mi))     g_warning("Getting position of %d (%s), it's at: %d (mi fail)", ID(mi), LABEL(mi), 0);
	if (!DBUSMENU_IS_MENUITEM(parent)) g_warning("Getting position of %d (%s), it's at: %d (parent fail)", ID(mi), LABEL(mi), 0);
	#endif

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

	#ifdef MASSIVEDEBUGGING
	g_debug("Getting position of %d (%s), it's at: %d", ID(mi), LABEL(mi), count);
	#endif

	return count;
}

/**
	dbusmenu_menuitem_get_position_realized:
	@mi: The #DbusmenuMenuitem to find the position of
	@parent: The #DbusmenuMenuitem who's children contain @mi

	This function is very similar to #dbusmenu_menuitem_get_position
	except that it only counts in the children that have been realized.

	Return value: The position of @mi in the realized children of @parent.
*/
guint
dbusmenu_menuitem_get_position_realized (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent)
{
	#ifdef MASSIVEDEBUGGING
	if (!DBUSMENU_IS_MENUITEM(mi))     g_warning("Getting position of %d (%s), it's at: %d (mi fail)", ID(mi), LABEL(mi), 0);
	if (!DBUSMENU_IS_MENUITEM(parent)) g_warning("Getting position of %d (%s), it's at: %d (parent fail)", ID(mi), LABEL(mi), 0);
	#endif

	/* TODO: I'm not too happy returning zeros here.  But that's all I've got */
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), 0);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(parent), 0);

	GList * childs = dbusmenu_menuitem_get_children(parent);
	if (childs == NULL) return 0;
	guint count = 0;
	for ( ; childs != NULL; childs = childs->next, count++) {
		if (!dbusmenu_menuitem_realized(DBUSMENU_MENUITEM(childs->data))) {
			count--;
			continue;
		}
		if (childs->data == mi) {
			break;
		}
	}

	if (childs == NULL) return 0;

	#ifdef MASSIVEDEBUGGING
	g_debug("Getting position of %d (%s), it's at: %d", ID(mi), LABEL(mi), count);
	#endif

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
	g_return_val_if_fail(g_list_find(priv->children, child) == NULL, FALSE);

	priv->children = g_list_append(priv->children, child);
	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) signalling child added %d (%s) at %d", ID(mi), LABEL(mi), ID(child), LABEL(child), g_list_length(priv->children) - 1);
	#endif
	g_object_ref(G_OBJECT(child));
	g_signal_emit(G_OBJECT(mi), signals[CHILD_ADDED], 0, child, g_list_length(priv->children) - 1, TRUE);
	return TRUE;
}

/**
	dbusmenu_menuitem_child_prepend:
	@mi: The #DbusmenuMenuitem which will become a new parent
	@child: The #DbusmenMenuitem that will be a child

	This function adds @child to the list of children on @mi at
	the beginning of that list.

	Return value: Whether the child has been added successfully.
*/
gboolean
dbusmenu_menuitem_child_prepend (DbusmenuMenuitem * mi, DbusmenuMenuitem * child)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(child), FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	g_return_val_if_fail(g_list_find(priv->children, child) == NULL, FALSE);

	priv->children = g_list_prepend(priv->children, child);
	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) signalling child added %d (%s) at %d", ID(mi), LABEL(mi), ID(child), LABEL(child), 0);
	#endif
	g_object_ref(G_OBJECT(child));
	g_signal_emit(G_OBJECT(mi), signals[CHILD_ADDED], 0, child, 0, TRUE);
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
	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) signalling child removed %d (%s)", ID(mi), LABEL(mi), ID(child), LABEL(child));
	#endif
	g_signal_emit(G_OBJECT(mi), signals[CHILD_REMOVED], 0, child, TRUE);
	g_object_unref(G_OBJECT(child));
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
	g_return_val_if_fail(g_list_find(priv->children, child) == NULL, FALSE);

	priv->children = g_list_insert(priv->children, child, position);
	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) signalling child added %d (%s) at %d", ID(mi), LABEL(mi), ID(child), LABEL(child), position);
	#endif
	g_object_ref(G_OBJECT(child));
	g_signal_emit(G_OBJECT(mi), signals[CHILD_ADDED], 0, child, position, TRUE);
	return TRUE;
}

/**
	dbusmenu_menuitem_child_reorder:
	@base: The #DbusmenuMenuitem that has children needing realignment
	@child: The #DbusmenuMenuitem that is a child needing to be moved
	@position: The position in the list to place it in

	This function moves a child on the list of children.  It is
	for a child that is already in the list, but simply needs a 
	new location.

	Return value: Whether the move was successful.
*/
gboolean
dbusmenu_menuitem_child_reorder(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(child), FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	gint oldpos = g_list_index(priv->children, child);

	if (oldpos == -1) {
		g_warning("Can not reorder child that isn't actually a child.");
		return FALSE;
	}
	if (oldpos == position) {
		return TRUE;
	}

	priv->children = g_list_remove(priv->children, child);
	priv->children = g_list_insert(priv->children, child, position);

	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) signalling child %d (%s) moved from %d to %d", ID(mi), LABEL(mi), ID(child), LABEL(child), oldpos, position);
	#endif
	g_signal_emit(G_OBJECT(mi), signals[CHILD_MOVED], 0, child, position, oldpos, TRUE);

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
dbusmenu_menuitem_child_find (DbusmenuMenuitem * mi, gint id)
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
	gint id;
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
dbusmenu_menuitem_find_id (DbusmenuMenuitem * mi, gint id)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);
	if (id == 0) {
		if (dbusmenu_menuitem_get_root(mi) == FALSE) {
			g_warning("Getting a menuitem with id zero, but it's not set as root.");
		}
		return mi;
	}
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
	GValue val = {0};
	g_value_init(&val, G_TYPE_STRING);
	g_value_set_static_string(&val, value);
	return dbusmenu_menuitem_property_set_value(mi, property, &val);
}

/**
	dbusmenu_menuitem_property_set_bool:
	@mi: The #DbusmenuMenuitem to set the property on.
	@property: Name of the property to set.
	@value: The value of the property.

	Takes a boolean @value and sets it on @property as a
	property on @mi.  If a property already exists by that name,
	then the value is set to the new value.  If not, the property
	is added.  If the value is changed or the property was previously
	unset then the signal #DbusmenuMenuitem::prop-changed will be
	emitted by this function.

	Return value:  A boolean representing if the property value was set.
*/
gboolean
dbusmenu_menuitem_property_set_bool (DbusmenuMenuitem * mi, const gchar * property, const gboolean value)
{
	GValue val = {0};
	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, value);
	return dbusmenu_menuitem_property_set_value(mi, property, &val);
}

/**
	dbusmenu_menuitem_property_set_int:
	@mi: The #DbusmenuMenuitem to set the property on.
	@property: Name of the property to set.
	@value: The value of the property.

	Takes a boolean @value and sets it on @property as a
	property on @mi.  If a property already exists by that name,
	then the value is set to the new value.  If not, the property
	is added.  If the value is changed or the property was previously
	unset then the signal #DbusmenuMenuitem::prop-changed will be
	emitted by this function.

	Return value:  A boolean representing if the property value was set.
*/
gboolean
dbusmenu_menuitem_property_set_int (DbusmenuMenuitem * mi, const gchar * property, const gint value)
{
	GValue val = {0};
	g_value_init(&val, G_TYPE_INT);
	g_value_set_int(&val, value);
	return dbusmenu_menuitem_property_set_value(mi, property, &val);
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
dbusmenu_menuitem_property_set_value (DbusmenuMenuitem * mi, const gchar * property, const GValue * value)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	g_return_val_if_fail(property != NULL, FALSE);
	g_return_val_if_fail(G_IS_VALUE(value), FALSE);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	/* g_debug("Setting a property.  ID: %d  Prop: %s  Value: %s", priv->id, property, value); */

	#if 0
	gpointer lookup = g_hash_table_lookup(priv->properties, property);
	if (g_strcmp0((gchar *)lookup, value) == 0) {
		/* The value is the same as the value currently in the
		   table so we don't really care.  Just say everything's okay */
		return TRUE;
	}
	#endif

	gchar * lprop = g_strdup(property);
	GValue * lval = g_new0(GValue, 1);
	g_value_init(lval, G_VALUE_TYPE(value));
	g_value_copy(value, lval);

	g_hash_table_replace(priv->properties, lprop, lval);
	#ifdef MASSIVEDEBUGGING
	gchar * valstr = g_strdup_value_contents(lval);
	g_debug("Menuitem %d (%s) signalling property '%s' changed to '%s'", ID(mi), LABEL(mi), property, g_utf8_strlen(valstr, 50) < 25 ? valstr : "<too long>");
	g_free(valstr);
	#endif

	g_signal_emit(G_OBJECT(mi), signals[PROPERTY_CHANGED], 0, lprop, lval, TRUE);

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
		is not set or is not a string.
*/
const gchar *
dbusmenu_menuitem_property_get (DbusmenuMenuitem * mi, const gchar * property)
{
	const GValue * value = dbusmenu_menuitem_property_get_value(mi, property);
	if (value == NULL) return NULL;
	if (G_VALUE_TYPE(value) != G_TYPE_STRING) return NULL;
	return g_value_get_string(value);
}

/**
	dbusmenu_menuitem_property_get_value:
	@mi: The #DbusmenuMenuitem to look for the property on.
	@property: The property to grab.

	Look up a property on @mi and return the value of it if
	it exits.  #NULL will be returned if the property doesn't
	exist.

	Return value: A GValue for the property.
*/
const GValue *
dbusmenu_menuitem_property_get_value (DbusmenuMenuitem * mi, const gchar * property)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), NULL);
	g_return_val_if_fail(property != NULL, NULL);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);

	return (const GValue *)g_hash_table_lookup(priv->properties, property);
}

/**
	dbusmenu_menuitem_property_get_bool:
	@mi: The #DbusmenuMenuitem to look for the property on.
	@property: The property to grab.

	Look up a property on @mi and return the value of it if
	it exits.  Returns #FALSE if the property doesn't exist.

	Return value: The value of the property or #FALSE.
*/
gboolean
dbusmenu_menuitem_property_get_bool (DbusmenuMenuitem * mi, const gchar * property)
{
	const GValue * value = dbusmenu_menuitem_property_get_value(mi, property);
	if (value == NULL) return FALSE;
	if (G_VALUE_TYPE(value) != G_TYPE_BOOLEAN) {
		if (g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_BOOLEAN)) {
			GValue boolval = {0};
			g_value_init(&boolval, G_TYPE_BOOLEAN);
			g_value_transform(value, &boolval);
			return g_value_get_boolean(&boolval);
		} else {
			return FALSE;
		}
	}
	return g_value_get_boolean(value);
}

/**
	dbusmenu_menuitem_property_get_int:
	@mi: The #DbusmenuMenuitem to look for the property on.
	@property: The property to grab.

	Look up a property on @mi and return the value of it if
	it exits.  Returns zero if the property doesn't exist.

	Return value: The value of the property or zero.
*/
gint
dbusmenu_menuitem_property_get_int (DbusmenuMenuitem * mi, const gchar * property)
{
	const GValue * value = dbusmenu_menuitem_property_get_value(mi, property);
	if (value == NULL) return 0;
	if (G_VALUE_TYPE(value) != G_TYPE_INT) {
		if (g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_INT)) {
			GValue intval = {0};
			g_value_init(&intval, G_TYPE_INT);
			g_value_transform(value, &intval);
			return g_value_get_int(&intval);
		} else {
			return 0;
		}
	}
	return g_value_get_int(value);
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
	dbusmenu_menuitem_property_remove:
	@mi: The #DbusmenuMenuitem to remove the property on.
	@property: The property to look for.

	Removes a property from the menuitem.
*/
void
dbusmenu_menuitem_property_remove (DbusmenuMenuitem * mi, const gchar * property)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	g_return_if_fail(property != NULL);

	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);

	g_hash_table_remove(priv->properties, property);

	return;
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
	dbusmenu_menuitem_set_root:
	@mi: #DbusmenuMenuitem to set whether it's root
	@root: Whether @mi is a root node or not

	This function sets the internal value of whether this is a
	root node or not.

	Return value: None
*/
void
dbusmenu_menuitem_set_root (DbusmenuMenuitem * mi, gboolean root)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	priv->root = root;
	return;
}

/**
	dbusmenu_menuitem_get_root:
	@mi: #DbusmenuMenuitem to see whether it's root

	This function returns the internal value of whether this is a
	root node or not.

	Return value: #TRUE if this is a root node
*/
gboolean
dbusmenu_menuitem_get_root (DbusmenuMenuitem * mi)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(mi), FALSE);
	DbusmenuMenuitemPrivate * priv = DBUSMENU_MENUITEM_GET_PRIVATE(mi);
	return priv->root;
}


/**
	dbusmenu_menuitem_buildxml:
	@mi: #DbusmenuMenuitem to represent in XML
	@array: (element-type utf8): A list of string that will be turned into an XML file

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

	gint id = 0;
	if (!dbusmenu_menuitem_get_root(mi)) {
		id = dbusmenu_menuitem_get_id(mi);
	}

	GList * children = dbusmenu_menuitem_get_children(mi);
	if (children == NULL) {
		g_ptr_array_add(array, g_strdup_printf("<menu id=\"%d\"/>", id));
	} else {
		g_ptr_array_add(array, g_strdup_printf("<menu id=\"%d\">", id));
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
	@data: (closure): User data to pass to the function

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
	dbusmenu_menuitem_handle_event:
	@mi: The #DbusmenuMenuitem to send the signal on.
	@name: The name of the signal
	@value: A value that could be set for the event
	@timestamp: The timestamp of when the event happened

	This function is called to create an event.  It is likely
	to be overrided by subclasses.  The default menu item
	will respond to the activate signal and do:

	Emits the #DbusmenuMenuitem::item-activate signal on this
	menu item.  Called by server objects when they get the
	appropriate DBus signals from the client.

	If you subclass this function you should really think
	about calling the parent function unless you have a good
	reason not to.
*/
void
dbusmenu_menuitem_handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	#ifdef MASSIVEDEBUGGING
	g_debug("Menuitem %d (%s) is getting event '%s'", ID(mi), LABEL(mi), name);
	#endif
	DbusmenuMenuitemClass * class = DBUSMENU_MENUITEM_GET_CLASS(mi);

	if (class->handle_event != NULL) {
		return class->handle_event(mi, name, value, timestamp);
	}
	return;
}

/**
	dbusmenu_menuitem_send_about_to_show:
	@mi: The #DbusmenuMenuitem to send the signal on.
	@cb: Callback to call when the call has returned.
	@cb_data: (closure): Data to pass to the callback.

	This function is used to send the even that the submenu
	of this item is about to be shown.  Callers to this event
	should delay showing the menu until their callback is
	called if possible.
*/
void
dbusmenu_menuitem_send_about_to_show (DbusmenuMenuitem * mi, dbusmenu_menuitem_about_to_show_cb cb, gpointer cb_data)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM(mi));
	#ifdef MASSIVEDEBUGGING
	g_debug("Submenu for menuitem %d (%s) is about to be shown", ID(mi), LABEL(mi));
	#endif
	DbusmenuMenuitemClass * class = DBUSMENU_MENUITEM_GET_CLASS(mi);

	if (class->send_about_to_show != NULL) {
		return class->send_about_to_show(mi, cb, cb_data);
	} else if (cb != NULL) {
		cb(mi, cb_data);
	}

	return;
}
