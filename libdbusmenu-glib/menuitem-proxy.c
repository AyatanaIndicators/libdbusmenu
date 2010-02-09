/*
An object to ferry over properties and signals between two different
dbusmenu instances.  Useful for services.

Copyright 2010 Canonical Ltd.

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

#include "menuitem-proxy.h"

typedef struct _DbusmenuMenuitemProxyPrivate DbusmenuMenuitemProxyPrivate;
struct _DbusmenuMenuitemProxyPrivate {
	DbusmenuMenuitem * mi;
};

/* Properties */
enum {
	PROP_0,
	PROP_MENU_ITEM
};

#define PROP_MENU_ITEM_S   "menu-item"

#define DBUSMENU_MENUITEM_PROXY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_MENUITEM_PROXY, DbusmenuMenuitemProxyPrivate))

static void dbusmenu_menuitem_proxy_class_init (DbusmenuMenuitemProxyClass *klass);
static void dbusmenu_menuitem_proxy_init       (DbusmenuMenuitemProxy *self);
static void dbusmenu_menuitem_proxy_dispose    (GObject *object);
static void dbusmenu_menuitem_proxy_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
static void handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp);
static void add_menuitem (DbusmenuMenuitemProxy * pmi, DbusmenuMenuitem * mi);
static void remove_menuitem (DbusmenuMenuitemProxy * pmi);

G_DEFINE_TYPE (DbusmenuMenuitemProxy, dbusmenu_menuitem_proxy, DBUSMENU_TYPE_MENUITEM);

static void
dbusmenu_menuitem_proxy_class_init (DbusmenuMenuitemProxyClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuMenuitemProxyPrivate));

	object_class->dispose = dbusmenu_menuitem_proxy_dispose;
	object_class->finalize = dbusmenu_menuitem_proxy_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	DbusmenuMenuitemClass * miclass = DBUSMENU_MENUITEM_CLASS(klass);

	miclass->handle_event = handle_event;

	g_object_class_install_property (object_class, PROP_MENU_ITEM,
	                                 g_param_spec_object(PROP_MENU_ITEM_S, "The Menuitem we're proxying",
	                                                     "An instance of the DbusmenuMenuitem class that this menuitem will mimic.",
	                                                     DBUSMENU_TYPE_MENUITEM,
	                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	return;
}

static void
dbusmenu_menuitem_proxy_init (DbusmenuMenuitemProxy *self)
{
	DbusmenuMenuitemProxyPrivate * priv = DBUSMENU_MENUITEM_PROXY_GET_PRIVATE(self);

	priv->mi = NULL;

	return;
}

/* Remove references to objects */
static void
dbusmenu_menuitem_proxy_dispose (GObject *object)
{
	remove_menuitem(DBUSMENU_MENUITEM_PROXY(object));

	G_OBJECT_CLASS (dbusmenu_menuitem_proxy_parent_class)->dispose (object);
	return;
}

/* Free any memory that we've allocated */
static void
dbusmenu_menuitem_proxy_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_menuitem_proxy_parent_class)->finalize (object);
	return;
}

/* Set a property using the generic GObject interface */
static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	switch (id) {
	case PROP_MENU_ITEM: {
		GObject * lobj = g_value_get_object(value);
		add_menuitem(DBUSMENU_MENUITEM_PROXY(obj), DBUSMENU_MENUITEM(lobj));
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, pspec);
		break;
	}

	return;
}

/* Get a property using the generic GObject interface */
static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	DbusmenuMenuitemProxyPrivate * priv = DBUSMENU_MENUITEM_PROXY_GET_PRIVATE(obj);

	switch (id) {
	case PROP_MENU_ITEM:
		g_value_set_object(value, priv->mi);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, pspec);
		break;
	}

	return;
}

/* Takes the event and passes it along to the item that we're
   playing proxy for. */
static void
handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp)
{
	g_return_if_fail(DBUSMENU_IS_MENUITEM_PROXY(mi));
	DbusmenuMenuitemProxyPrivate * priv = DBUSMENU_MENUITEM_PROXY_GET_PRIVATE(mi);
	g_return_if_fail(priv->mi != NULL);
	return dbusmenu_menuitem_handle_event(priv->mi, name, value, timestamp);
}

/* References all of the things we need for talking to this menuitem
   including signals and other data.  If the menuitem already has
   properties we need to signal that they've changed for us.  */
static void
add_menuitem (DbusmenuMenuitemProxy * pmi, DbusmenuMenuitem * mi)
{

	return;
}

/* Removes the menuitem from being our proxy.  Typically this isn't
   done until this object is destroyed, but who knows?!? */
static void
remove_menuitem (DbusmenuMenuitemProxy * pmi)
{
	DbusmenuMenuitemProxyPrivate * priv = DBUSMENU_MENUITEM_PROXY_GET_PRIVATE(pmi);
	if (priv->mi == NULL) {
		return;
	}

	/* Remove signals */

	/* Unref */
	g_object_unref(G_OBJECT(priv->mi));
	priv->mi = NULL;

	return;
}
