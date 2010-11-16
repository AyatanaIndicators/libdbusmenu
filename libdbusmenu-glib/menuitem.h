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
#define DBUSMENU_MENUITEM_SIGNAL_SHOW_TO_USER        "show-to-user"

#define DBUSMENU_MENUITEM_PROP_TYPE                  "type"
#define DBUSMENU_MENUITEM_PROP_VISIBLE               "visible"
#define DBUSMENU_MENUITEM_PROP_ENABLED               "enabled"
#define DBUSMENU_MENUITEM_PROP_LABEL                 "label"
#define DBUSMENU_MENUITEM_PROP_ICON_NAME             "icon-name"
#define DBUSMENU_MENUITEM_PROP_ICON_DATA             "icon-data"
#define DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE           "toggle-type"
#define DBUSMENU_MENUITEM_PROP_TOGGLE_STATE          "toggle-state"
#define DBUSMENU_MENUITEM_PROP_SHORTCUT              "shortcut"
#define DBUSMENU_MENUITEM_PROP_CHILD_DISPLAY         "children-display"

#define DBUSMENU_MENUITEM_TOGGLE_CHECK               "checkmark"
#define DBUSMENU_MENUITEM_TOGGLE_RADIO               "radio"

#define DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED     0
#define DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED       1
#define DBUSMENU_MENUITEM_TOGGLE_STATE_UNKNOWN       -1

#define DBUSMENU_MENUITEM_ICON_NAME_BLANK            "blank-icon"

#define DBUSMENU_MENUITEM_SHORTCUT_CONTROL           "Control"
#define DBUSMENU_MENUITEM_SHORTCUT_ALT               "Alt"
#define DBUSMENU_MENUITEM_SHORTCUT_SHIFT             "Shift"
#define DBUSMENU_MENUITEM_SHORTCUT_SUPER             "Super"

#define DBUSMENU_MENUITEM_CHILD_DISPLAY_SUBMENU      "submenu"

typedef struct _DbusmenuMenuitemPrivate DbusmenuMenuitemPrivate;

/**
 * DbusmenuMenuitem:
 * 
 * This is the #GObject based object that represents a menu
 * item.  It gets created the same on both the client and
 * the server side and libdbusmenu-glib does the work of making
 * this object model appear on both sides of DBus.  Simple
 * really, though through updates and people coming on and off
 * the bus it can lead to lots of fun complex scenarios.
 */
typedef struct _DbusmenuMenuitem      DbusmenuMenuitem;
struct _DbusmenuMenuitem
{
	GObject parent;

	/*< Private >*/
	DbusmenuMenuitemPrivate * priv;
};

/**
 * dbusmenu_menuitem_about_to_show_cb:
 * @mi: Menu item that should be shown
 * @user_data: (closure): Extra user data sent with the function
 * 
 * Callback prototype for a callback that is called when the
 * menu should be shown.
 */
typedef void (*dbusmenu_menuitem_about_to_show_cb) (DbusmenuMenuitem * mi, gpointer user_data);

/**
 * dbusmenu_menuitem_buildxml_slot_t:
 * @mi: (in): Menu item that should be built from
 * @stringarray: (inout) (transfer none) (array) (element-type utf8): An array of strings that can be combined into an XML file.
 * 
 * This is the function that is called to represent this menu item
 * as an XML fragment.  Should call it's own children.
 */
typedef void (*dbusmenu_menuitem_buildxml_slot_t) (DbusmenuMenuitem * mi, GPtrArray* stringarray);

/**
 * DbusmenuMenuitemClass:
 * @property_changed: Slot for #DbusmenuMenuitem::property-changed.
 * @item_activated: Slot for #DbusmenuMenuitem::item-activated.
 * @child_added: Slot for #DbusmenuMenuitem::child-added.
 * @child_removed: Slot for #DbusmenuMenuitem::child-removed.
 * @child_moved: Slot for #DbusmenuMenuitem::child-moved.
 * @realized: Slot for #DbusmenuMenuitem::realized.
 * @buildxml: Virtual function that appends the strings required to represent this menu item in the menu XML file.
 * @handle_event: This function is to override how events are handled by subclasses.  Look at #dbusmenu_menuitem_handle_event for lots of good information.
 * @send_about_to_show: Virtual function that notifies server that the client is about to show a menu.
 * @show_to_user: Slot for #DbusmenuMenuitem::show-to-user.
 *
 * @reserved1: Reserved for future use.
 * @reserved2: Reserved for future use.
 * @reserved3: Reserved for future use.
 * @reserved4: Reserved for future use.
 * @reserved5: Reserved for future use.
 * @reserved6: Reserved for future use.
 */
typedef struct _DbusmenuMenuitemClass DbusmenuMenuitemClass;
struct _DbusmenuMenuitemClass
{
	GObjectClass parent_class;

	/* Signals */
	void (*property_changed) (gchar * property, GValue * value);
	void (*item_activated) (guint timestamp);
	void (*child_added) (DbusmenuMenuitem * child, guint position);
	void (*child_removed) (DbusmenuMenuitem * child);
	void (*child_moved) (DbusmenuMenuitem * child, guint newpos, guint oldpos);
	void (*realized) (void);

	/* Virtual functions */
	dbusmenu_menuitem_buildxml_slot_t buildxml;
	void (*handle_event) (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp);
	void (*send_about_to_show) (DbusmenuMenuitem * mi, void (*cb) (DbusmenuMenuitem * mi, gpointer user_data), gpointer cb_data);

	void (*show_to_user) (DbusmenuMenuitem * mi, guint timestamp, gpointer cb_data);

	/*< Private >*/
	void (*reserved1) (void);
	void (*reserved2) (void);
	void (*reserved3) (void);
	void (*reserved4) (void);
	void (*reserved5) (void);
	void (*reserved6) (void);
};

GType dbusmenu_menuitem_get_type (void);

DbusmenuMenuitem * dbusmenu_menuitem_new (void) G_GNUC_WARN_UNUSED_RESULT;
DbusmenuMenuitem * dbusmenu_menuitem_new_with_id (gint id) G_GNUC_WARN_UNUSED_RESULT;
gint dbusmenu_menuitem_get_id (DbusmenuMenuitem * mi);

GList * dbusmenu_menuitem_get_children (DbusmenuMenuitem * mi);
GList * dbusmenu_menuitem_take_children (DbusmenuMenuitem * mi) G_GNUC_WARN_UNUSED_RESULT;
guint dbusmenu_menuitem_get_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent);
guint dbusmenu_menuitem_get_position_realized (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent);

gboolean dbusmenu_menuitem_child_append (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_prepend (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_delete (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_add_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position);
gboolean dbusmenu_menuitem_child_reorder (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position);
DbusmenuMenuitem * dbusmenu_menuitem_child_find (DbusmenuMenuitem * mi, gint id);
DbusmenuMenuitem * dbusmenu_menuitem_find_id (DbusmenuMenuitem * mi, gint id);

gboolean dbusmenu_menuitem_property_set (DbusmenuMenuitem * mi, const gchar * property, const gchar * value);
gboolean dbusmenu_menuitem_property_set_value (DbusmenuMenuitem * mi, const gchar * property, const GValue * value);
gboolean dbusmenu_menuitem_property_set_variant (DbusmenuMenuitem * mi, const gchar * property, const GVariant * value);
gboolean dbusmenu_menuitem_property_set_bool (DbusmenuMenuitem * mi, const gchar * property, const gboolean value);
gboolean dbusmenu_menuitem_property_set_int (DbusmenuMenuitem * mi, const gchar * property, const gint value);
const gchar * dbusmenu_menuitem_property_get (DbusmenuMenuitem * mi, const gchar * property);
const GValue * dbusmenu_menuitem_property_get_value (DbusmenuMenuitem * mi, const gchar * property);
GVariant * dbusmenu_menuitem_property_get_variant (DbusmenuMenuitem * mi, const gchar * property);
gboolean dbusmenu_menuitem_property_get_bool (DbusmenuMenuitem * mi, const gchar * property);
gint dbusmenu_menuitem_property_get_int (DbusmenuMenuitem * mi, const gchar * property);
gboolean dbusmenu_menuitem_property_exist (DbusmenuMenuitem * mi, const gchar * property);
GList * dbusmenu_menuitem_properties_list (DbusmenuMenuitem * mi) G_GNUC_WARN_UNUSED_RESULT;
GHashTable * dbusmenu_menuitem_properties_copy (DbusmenuMenuitem * mi);
void dbusmenu_menuitem_property_remove (DbusmenuMenuitem * mi, const gchar * property);

void dbusmenu_menuitem_set_root (DbusmenuMenuitem * mi, gboolean root);
gboolean dbusmenu_menuitem_get_root (DbusmenuMenuitem * mi);

void dbusmenu_menuitem_foreach (DbusmenuMenuitem * mi, void (*func) (DbusmenuMenuitem * mi, gpointer data), gpointer data);
void dbusmenu_menuitem_handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp);
void dbusmenu_menuitem_send_about_to_show (DbusmenuMenuitem * mi, void (*cb) (DbusmenuMenuitem * mi, gpointer user_data), gpointer cb_data);

void dbusmenu_menuitem_show_to_user (DbusmenuMenuitem * mi, guint timestamp);

/**
 * SECTION:menuitem
 * @short_description: A lowlevel represenation of a menuitem
 * @stability: Unstable
 * @include: libdbusmenu-glib/menuitem.h
 * 
 * A #DbusmenuMenuitem is the lowest level of represenation of a
 * single item in a menu.  It gets created on the server side
 * and copied over to the client side where it gets rendered.  As
 * the server starts to change it, and grow it, and do all kinds
 * of fun stuff that information is transfered over DBus and the
 * client updates it's understanding of the object model.
 * 
 * Most people using either the client or the server should be
 * able to deal mostly with #DbusmenuMenuitem objects.  These
 * are simple, but then they can be attached to more complex
 * objects and handled appropriately.
 */

G_END_DECLS

#endif
