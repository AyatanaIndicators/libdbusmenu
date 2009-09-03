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

#ifndef __DBUSMENU_MENUITEM_H__
#define __DBUSMENU_MENUITEM_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define DBUSMENU_TYPE_MENUITEM            (dbusmenu_menuitem_get_type ())
#define DBUSMENU_MENUITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitem))
#define DBUSMENU_MENUITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemClass))
#define DBUSMENU_IS_MENUITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_MENUITEM))
#define DBUSMENU_IS_MENUITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_MENUITEM))
#define DBUSMENU_MENUITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemClass))


#define DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED    "property-changed"
#define DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED      "item-activated"
#define DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED         "child-added"
#define DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED       "child-removed"
#define DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED         "child-moved"
#define DBUSMENU_MENUITEM_SIGNAL_REALIZED            "realized"
#define DBUSMENU_MENUITEM_SIGNAL_REALIZED_ID         (g_signal_lookup(DBUSMENU_MENUITEM_SIGNAL_REALIZED, DBUSMENU_TYPE_MENUITEM))

#define DBUSMENU_MENUITEM_PROP_VISIBLE               "visible"
#define DBUSMENU_MENUITEM_PROP_SENSITIVE             "sensitive"
#define DBUSMENU_MENUITEM_PROP_LABEL                 "label"
#define DBUSMENU_MENUITEM_PROP_ICON                  "icon"
#define DBUSMENU_MENUITEM_PROP_ICON_DATA             "icon-data"

/**
	DbusmenuMenuitem:

	This is the #GObject based object that represents a menu
	item.  It gets created the same on both the client and
	the server side and libdbusmenu-glib does the work of making
	this object model appear on both sides of DBus.  Simple
	really, though through updates and people coming on and off
	the bus it can lead to lots of fun complex scenarios.
*/
typedef struct _DbusmenuMenuitem      DbusmenuMenuitem;
struct _DbusmenuMenuitem
{
	GObject parent;
};

/**
	DbusmenuMenuitemClass:
	@property_changed: Slot for #DbusmenuMenuitem::property-changed.
	@item_activated: Slot for #DbusmenuMenuitem::item-activated.
	@child_added: Slot for #DbusmenuMenuitem::child-added.
	@child_removed: Slot for #DbusmenuMenuitem::child-removed.
	@child_moved: Slot for #DbusmenuMenuitem::child-moved.
	@realized: Slot for #DbusmenuMenuitem::realized.
	@buildxml: Virtual function that appends the strings required
	           to represent this menu item in the menu XML file.
	@reserved1: Reserved for future use.
	@reserved2: Reserved for future use.
	@reserved3: Reserved for future use.
	@reserved4: Reserved for future use.
*/
typedef struct _DbusmenuMenuitemClass DbusmenuMenuitemClass;
struct _DbusmenuMenuitemClass
{
	GObjectClass parent_class;

	/* Signals */
	void (*property_changed) (gchar * property, gchar * value);
	void (*item_activated) (void);
	void (*child_added) (DbusmenuMenuitem * child, guint position);
	void (*child_removed) (DbusmenuMenuitem * child);
	void (*child_moved) (DbusmenuMenuitem * child, guint newpos, guint oldpos);
	void (*realized) (void);

	/* Virtual functions */
	void (*buildxml) (GPtrArray * stringarray);

	void (*reserved1) (void);
	void (*reserved2) (void);
	void (*reserved3) (void);
	/* void (*reserved4) (void); -- realized, realloc when bumping lib version */
};

GType dbusmenu_menuitem_get_type (void);

DbusmenuMenuitem * dbusmenu_menuitem_new (void) G_GNUC_WARN_UNUSED_RESULT;
DbusmenuMenuitem * dbusmenu_menuitem_new_with_id (guint id) G_GNUC_WARN_UNUSED_RESULT;
guint dbusmenu_menuitem_get_id (DbusmenuMenuitem * mi);

GList * dbusmenu_menuitem_get_children (DbusmenuMenuitem * mi);
GList * dbusmenu_menuitem_take_children (DbusmenuMenuitem * mi) G_GNUC_WARN_UNUSED_RESULT;
guint dbusmenu_menuitem_get_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent);

gboolean dbusmenu_menuitem_child_append (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_prepend (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_delete (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_add_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position);
gboolean dbusmenu_menuitem_child_reorder (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position);
DbusmenuMenuitem * dbusmenu_menuitem_child_find (DbusmenuMenuitem * mi, guint id);
DbusmenuMenuitem * dbusmenu_menuitem_find_id (DbusmenuMenuitem * mi, guint id);

gboolean dbusmenu_menuitem_property_set (DbusmenuMenuitem * mi, const gchar * property, const gchar * value);
const gchar * dbusmenu_menuitem_property_get (DbusmenuMenuitem * mi, const gchar * property);
gboolean dbusmenu_menuitem_property_exist (DbusmenuMenuitem * mi, const gchar * property);
GList * dbusmenu_menuitem_properties_list (DbusmenuMenuitem * mi) G_GNUC_WARN_UNUSED_RESULT;
GHashTable * dbusmenu_menuitem_properties_copy (DbusmenuMenuitem * mi);

void dbusmenu_menuitem_set_root (DbusmenuMenuitem * mi, gboolean root);
gboolean dbusmenu_menuitem_get_root (DbusmenuMenuitem * mi);

void dbusmenu_menuitem_buildxml (DbusmenuMenuitem * mi, GPtrArray * array);
void dbusmenu_menuitem_foreach (DbusmenuMenuitem * mi, void (*func) (DbusmenuMenuitem * mi, gpointer data), gpointer data);
void dbusmenu_menuitem_activate (DbusmenuMenuitem * mi);

/**
	SECTION:menuitem
	@short_description: A lowlevel represenation of a menuitem
	@stability: Unstable
	@include: libdbusmenu-glib/menuitem.h

	A #DbusmenuMenuitem is the lowest level of represenation of a
	single item in a menu.  It gets created on the server side
	and copied over to the client side where it gets rendered.  As
	the server starts to change it, and grow it, and do all kinds
	of fun stuff that information is transfered over DBus and the
	client updates it's understanding of the object model.

	Most people using either the client or the server should be
	able to deal mostly with #DbusmenuMenuitem objects.  These
	are simple, but then they can be attached to more complex
	objects and handled appropriately.
*/

G_END_DECLS

#endif
