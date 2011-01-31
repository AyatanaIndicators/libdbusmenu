/*
Parse to take a set of GTK Menus and turn them into something that can
be sent over the wire.

Copyright 2011 Canonical Ltd.

Authors:
	Numerous (check Bazaar)

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

#include "parser.h"
#include "menuitem.h"
#include "serializablemenuitem.h"

#define CACHED_MENUITEM  "dbusmenu-gtk-parser-cached-item"

typedef struct _RecurseContext
{
  GtkWidget * toplevel;
  DbusmenuMenuitem * parent;
} RecurseContext;

static void parse_menu_structure_helper (GtkWidget * widget, RecurseContext * recurse);
static DbusmenuMenuitem * construct_dbusmenu_for_widget (GtkWidget * widget);
static void           accel_changed            (GtkWidget *         widget,
                                                gpointer            data);
static gboolean       update_stock_item        (DbusmenuMenuitem *  menuitem,
                                                GtkWidget *         widget);
static void           checkbox_toggled         (GtkWidget *         widget,
                                                DbusmenuMenuitem *  mi);
static void           update_icon_name         (DbusmenuMenuitem *  menuitem,
                                                GtkWidget *         widget);
static GtkWidget *    find_menu_label          (GtkWidget *         widget);
static void           label_notify_cb          (GtkWidget *         widget,
                                                GParamSpec *        pspec,
                                                gpointer            data);
static void           action_notify_cb         (GtkAction *         action,
                                                GParamSpec *        pspec,
                                                gpointer            data);
static void           item_activated           (DbusmenuMenuitem *  item,
                                                guint               timestamp,
                                                gpointer            user_data);
static gboolean       item_about_to_show       (DbusmenuMenuitem *  item,
                                                gpointer            user_data);
static void           widget_notify_cb         (GtkWidget  *        widget,
                                                GParamSpec *        pspec,
                                                gpointer            data);
static gboolean       should_show_image        (GtkImage *          image);
static void           menuitem_notify_cb       (GtkWidget *         widget,
                                                GParamSpec *        pspec,
                                                gpointer            data);

/**
	dbusmenu_gtk_parse_menu_structure:
	@widget: A #GtkMenuItem or #GtkMenuShell to turn into a #DbusmenuMenuitem

	Goes through the GTK structures and turns them into the appropraite
	Dbusmenu structures along with setting up all the relationships
	between the objects.  It also stores the dbusmenu items as a cache
	on the GTK items so that they'll be reused if necissary.

	Return value: A dbusmenu item representing the menu structure
*/
DbusmenuMenuitem *
dbusmenu_gtk_parse_menu_structure (GtkWidget * widget)
{
  g_return_val_if_fail(GTK_IS_MENU_ITEM(widget) || GTK_IS_MENU_SHELL(widget), NULL);

  RecurseContext recurse = {0};

  recurse.toplevel = gtk_widget_get_toplevel(widget);

  parse_menu_structure_helper(widget, &recurse);

  return recurse.parent;
}

/* Called when the dbusmenu item that we're keeping around
   is finalized */
static void
dbusmenu_cache_freed (gpointer data, GObject * obj)
{
	/* If the dbusmenu item is killed we don't need to remove
	   the weak ref as well. */
	g_object_steal_data(G_OBJECT(data), CACHED_MENUITEM);
	g_signal_handlers_disconnect_by_func(data, G_CALLBACK(widget_notify_cb), obj);
	return;
}

/* Called if we replace the cache on the object with a new
   dbusmenu menuitem */
static void
object_cache_freed (gpointer data)
{
	if (!G_IS_OBJECT(data)) return;
	g_object_weak_unref(G_OBJECT(data), dbusmenu_cache_freed, data);
	return;
}

static void
parse_menu_structure_helper (GtkWidget * widget, RecurseContext * recurse)
{

	/* If this is a shell, then let's handle the items in it. */
	if (GTK_IS_MENU_SHELL (widget)) {
		/* Okay, this is a little janky and all.. but some applications update some
		 * menuitem properties such as sensitivity on the activate callback.  This
		 * seems a little weird, but it's not our place to judge when all this code
		 * is so crazy.  So we're going to get ever crazier and activate all the
		 * menus that are directly below the menubar and force the applications to
		 * update their sensitivity.  The menus won't actually popup in the app
		 * window due to our gtk+ patches.
		 *
		 * Note that this will not force menuitems in submenus to be updated as well.
		 */
		if (recurse->parent == NULL && GTK_IS_MENU_BAR(widget)) {
			GList *children = gtk_container_get_children (GTK_CONTAINER (widget));

			for (; children != NULL; children = children->next) {
				gtk_menu_shell_activate_item (GTK_MENU_SHELL (widget),
				                              children->data,
				                              TRUE);
			}

			g_list_free (children);
		}

		if (recurse->parent == NULL) {
			recurse->parent = dbusmenu_menuitem_new();
		}

		gtk_container_foreach (GTK_CONTAINER (widget),
		                       (GtkCallback)parse_menu_structure_helper,
		                       recurse);
		return;
	}

	if (GTK_IS_MENU_ITEM(widget)) {
		DbusmenuMenuitem * thisitem = NULL;

		/* Check to see if we're cached already */
		gpointer pmi = g_object_get_data(G_OBJECT(widget), CACHED_MENUITEM);
		if (pmi != NULL) {
			thisitem = DBUSMENU_MENUITEM(pmi);
			g_object_ref(G_OBJECT(thisitem));
		}

		/* We don't have one, so we'll need to build it */
		if (thisitem == NULL) {
			thisitem = construct_dbusmenu_for_widget (widget);
			g_object_set_data_full(G_OBJECT(widget), CACHED_MENUITEM, thisitem, object_cache_freed);
			g_object_weak_ref(G_OBJECT(thisitem), dbusmenu_cache_freed, widget);

			if (!gtk_widget_get_visible (widget)) {
				g_signal_connect (G_OBJECT (widget),
				                  "notify::visible",
				                  G_CALLBACK (menuitem_notify_cb),
				                  recurse->toplevel);
            }

			if (GTK_IS_TEAROFF_MENU_ITEM (widget)) {
				dbusmenu_menuitem_property_set_bool (thisitem,
				                                     DBUSMENU_MENUITEM_PROP_VISIBLE,
				                                     FALSE);
            }
		}

		/* Check to see if we're in our parents list of children, if we have
		   a parent. */
		if (recurse->parent != NULL) {
			GList * children = dbusmenu_menuitem_get_children (recurse->parent);
			GList * peek = NULL;

			if (children != NULL) {
				peek = g_list_find (children, thisitem);
			}

			/* Oops, let's tell our parents about us */
			if (peek == NULL) {
				/* TODO: Should we set a weak ref on the parent? */
				g_object_set_data (G_OBJECT (thisitem),
				                   "dbusmenu-parent",
				                   recurse->parent);
				dbusmenu_menuitem_child_append (recurse->parent,
				                                thisitem);
			}
		}

		GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
		if (menu != NULL) {
			DbusmenuMenuitem * parent_save = recurse->parent;
			recurse->parent = thisitem;
			parse_menu_structure_helper (menu, recurse);
			recurse->parent = parent_save;
		}

		if (recurse->parent == NULL) {
			recurse->parent = thisitem;
		} else {
			g_object_unref(thisitem);
		}
	}

	return;
}

/* Turn a widget into a dbusmenu item depending on the type of GTK
   object that it is. */
static DbusmenuMenuitem *
construct_dbusmenu_for_widget (GtkWidget * widget)
{
	/* If it's a subclass of our serializable menu item then we can
	   use its own build function */
	if (DBUSMENU_IS_GTK_SERIALIZABLE_MENU_ITEM(widget)) {
		DbusmenuGtkSerializableMenuItem * smi = DBUSMENU_GTK_SERIALIZABLE_MENU_ITEM(widget);
		return dbusmenu_gtk_serializable_menu_item_build_menuitem(smi);
	}

  /* If it's a standard GTK Menu Item we need to do some of our own work */
  if (GTK_IS_MENU_ITEM (widget))
    {
      DbusmenuMenuitem *mi = dbusmenu_menuitem_new ();

      gboolean visible = FALSE;
      gboolean sensitive = FALSE;
      if (GTK_IS_SEPARATOR_MENU_ITEM (widget))
        {
          dbusmenu_menuitem_property_set (mi,
                                          "type",
                                          "separator");

          visible = gtk_widget_get_visible (widget);
          sensitive = gtk_widget_get_sensitive (widget);
        }
      else
        {
          gboolean label_set = FALSE;

          g_signal_connect (widget,
                            "accel-closures-changed",
                            G_CALLBACK (accel_changed),
                            mi);

          if (GTK_IS_CHECK_MENU_ITEM (widget))
            {
              dbusmenu_menuitem_property_set (mi,
                                              DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,
                                              gtk_check_menu_item_get_draw_as_radio (GTK_CHECK_MENU_ITEM (widget)) ? DBUSMENU_MENUITEM_TOGGLE_RADIO : DBUSMENU_MENUITEM_TOGGLE_CHECK);

              dbusmenu_menuitem_property_set_int (mi,
                                                  DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                                  gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);

              g_signal_connect (widget,
                                "activate",
                                G_CALLBACK (checkbox_toggled),
                                mi);
            }

          if (GTK_IS_IMAGE_MENU_ITEM (widget))
            {
              GtkWidget *image;
              GtkImageType image_type;

              image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (widget));

              if (GTK_IS_IMAGE (image))
                {
                  image_type = gtk_image_get_storage_type (GTK_IMAGE (image));

                  if (image_type == GTK_IMAGE_STOCK)
                    {
                      label_set = update_stock_item (mi, image);
                    }
                  else if (image_type == GTK_IMAGE_ICON_NAME)
                    {
                      update_icon_name (mi, image);
                    }
                  else if (image_type == GTK_IMAGE_PIXBUF)
                    {
                      dbusmenu_menuitem_property_set_image (mi,
                                                            DBUSMENU_MENUITEM_PROP_ICON_DATA,
                                                            gtk_image_get_pixbuf (GTK_IMAGE (image)));
                    }
                }
            }

          GtkWidget *label = find_menu_label (widget);

          dbusmenu_menuitem_property_set (mi,
                                          "label",
                                          label ? gtk_label_get_text (GTK_LABEL (label)) : NULL);

          if (label)
            {
              // Sometimes, an app will directly find and modify the label
              // (like empathy), so watch the label especially for that.
              g_signal_connect (G_OBJECT (label),
                                "notify",
                                G_CALLBACK (label_notify_cb),
                                mi);
            }

          if (GTK_IS_ACTIVATABLE (widget))
            {
              GtkActivatable *activatable = GTK_ACTIVATABLE (widget);

              if (gtk_activatable_get_use_action_appearance (activatable))
                {
                  GtkAction *action = gtk_activatable_get_related_action (activatable);

                  if (action)
                    {
                      visible = gtk_action_is_visible (action);
                      sensitive = gtk_action_is_sensitive (action);

                      g_signal_connect_object (action, "notify",
                                               G_CALLBACK (action_notify_cb),
                                               mi,
                                               G_CONNECT_AFTER);
                    }
                }
            }

          if (!g_object_get_data (G_OBJECT (widget), "gtk-empty-menu-item") && !GTK_IS_TEAROFF_MENU_ITEM (widget))
            {
              visible = gtk_widget_get_visible (widget);
              sensitive = gtk_widget_get_sensitive (widget);
            }

          dbusmenu_menuitem_property_set_shortcut_menuitem (mi, GTK_MENU_ITEM (widget));

          g_signal_connect (G_OBJECT (mi),
                            DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (item_activated),
                            widget);

          g_signal_connect (G_OBJECT (mi),
                            DBUSMENU_MENUITEM_SIGNAL_ABOUT_TO_SHOW,
                            G_CALLBACK (item_about_to_show),
                            widget);
        }

      dbusmenu_menuitem_property_set_bool (mi,
                                           DBUSMENU_MENUITEM_PROP_VISIBLE,
                                           visible);

      dbusmenu_menuitem_property_set_bool (mi,
                                           DBUSMENU_MENUITEM_PROP_ENABLED,
                                           sensitive);

      g_signal_connect (widget,
                        "notify",
                        G_CALLBACK (widget_notify_cb),
                        mi);
      return mi;
    }

	/* If it's none of those we're going to just create a
	   generic menuitem as a place holder for it. */
	return dbusmenu_menuitem_new();
}

static void
menuitem_notify_cb (GtkWidget  *widget,
                    GParamSpec *pspec,
                    gpointer    data)
{
  if (pspec->name == g_intern_static_string ("visible"))
    {
      GtkWidget * new_toplevel = gtk_widget_get_toplevel (widget);
	  GtkWidget * old_toplevel = GTK_WIDGET(data);

      if (new_toplevel == old_toplevel) {
          /* TODO: Figure this out -> rebuild (context->bridge, window); */
      }

      /* We only care about this once, so let's disconnect now. */
      g_signal_handlers_disconnect_by_func (widget,
                                            G_CALLBACK (menuitem_notify_cb),
                                            data);
    }
}

static void
accel_changed (GtkWidget *widget,
               gpointer   data)
{
  DbusmenuMenuitem *mi = (DbusmenuMenuitem *)data;
  dbusmenu_menuitem_property_set_shortcut_menuitem (mi, GTK_MENU_ITEM (widget));
}

static gboolean
update_stock_item (DbusmenuMenuitem *menuitem,
                   GtkWidget        *widget)
{
  GtkStockItem stock;
  GtkImage *image;

  g_return_val_if_fail (GTK_IS_IMAGE (widget), FALSE);

  image = GTK_IMAGE (widget);

  if (gtk_image_get_storage_type (image) != GTK_IMAGE_STOCK)
    return FALSE;

  gchar * stock_id = NULL;
  gtk_image_get_stock(image, &stock_id, NULL);

  gtk_stock_lookup (stock_id, &stock);

  if (should_show_image (image))
    dbusmenu_menuitem_property_set (menuitem,
                                    DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                    stock_id);
  else
    dbusmenu_menuitem_property_remove (menuitem,
                                       DBUSMENU_MENUITEM_PROP_ICON_NAME);

  const gchar *label = dbusmenu_menuitem_property_get (menuitem,
                                                       DBUSMENU_MENUITEM_PROP_LABEL);

  if (stock.label != NULL && label != NULL)
    {
      dbusmenu_menuitem_property_set (menuitem,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      stock.label);

      return TRUE;
    }

  return FALSE;
}

static void
checkbox_toggled (GtkWidget *widget, DbusmenuMenuitem *mi)
{
  dbusmenu_menuitem_property_set_int (mi,
                                      DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                      gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
}

static void
update_icon_name (DbusmenuMenuitem *menuitem,
                  GtkWidget        *widget)
{
  GtkImage *image;

  g_return_if_fail (GTK_IS_IMAGE (widget));

  image = GTK_IMAGE (widget);

  if (gtk_image_get_storage_type (image) != GTK_IMAGE_ICON_NAME)
    return;

  if (should_show_image (image)) {
    const gchar * icon_name = NULL;
	gtk_image_get_icon_name(image, &icon_name, NULL);
    dbusmenu_menuitem_property_set (menuitem,
                                    DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                    icon_name);
  } else {
    dbusmenu_menuitem_property_remove (menuitem,
                                       DBUSMENU_MENUITEM_PROP_ICON_NAME);
  }
}

static GtkWidget *
find_menu_label (GtkWidget *widget)
{
  GtkWidget *label = NULL;

  if (GTK_IS_LABEL (widget))
    return widget;

  if (GTK_IS_CONTAINER (widget))
    {
      GList *children;
      GList *l;

      children = gtk_container_get_children (GTK_CONTAINER (widget));

      for (l = children; l; l = l->next)
        {
          label = find_menu_label (l->data);

          if (label)
            break;
        }

      g_list_free (children);
    }

  return label;
}

static void
label_notify_cb (GtkWidget  *widget,
                 GParamSpec *pspec,
                 gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

  if (pspec->name == g_intern_static_string ("label"))
    {
      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_label_get_text (GTK_LABEL (widget)));
    }
}

static void
action_notify_cb (GtkAction  *action,
                  GParamSpec *pspec,
                  gpointer    data)
{
  DbusmenuMenuitem *mi = (DbusmenuMenuitem *)data;

  if (pspec->name == g_intern_static_string ("sensitive"))
    {
      dbusmenu_menuitem_property_set_bool (mi,
                                           DBUSMENU_MENUITEM_PROP_ENABLED,
                                           gtk_action_is_sensitive (action));
    }
  else if (pspec->name == g_intern_static_string ("visible"))
    {
      dbusmenu_menuitem_property_set_bool (mi,
                                           DBUSMENU_MENUITEM_PROP_VISIBLE,
                                           gtk_action_is_visible (action));
    }
  else if (pspec->name == g_intern_static_string ("active"))
    {
      dbusmenu_menuitem_property_set_int (mi,
                                          DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                          gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
    }
  else if (pspec->name == g_intern_static_string ("label"))
    {
      dbusmenu_menuitem_property_set (mi,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_action_get_label (action));
    }
}

static void
item_activated (DbusmenuMenuitem *item, guint timestamp, gpointer user_data)
{
  GtkWidget *child;

  if (user_data != NULL)
    {
      child = (GtkWidget *)user_data;

      if (GTK_IS_MENU_ITEM (child))
        {
          gtk_menu_item_activate (GTK_MENU_ITEM (child));
        }
    }
}

static gboolean
item_about_to_show (DbusmenuMenuitem *item, gpointer user_data)
{
  GtkWidget *child;

  if (user_data != NULL)
    {
      child = (GtkWidget *)user_data;

      if (GTK_IS_MENU_ITEM (child))
        {
          // Only called for items with submens.  So we activate it here in
          // case the program dynamically creates menus (like empathy does)
          gtk_menu_item_activate (GTK_MENU_ITEM (child));
        }
    }

  return TRUE;
}

static void
widget_notify_cb (GtkWidget  *widget,
                  GParamSpec *pspec,
                  gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

  if (pspec->name == g_intern_static_string ("sensitive"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                           DBUSMENU_MENUITEM_PROP_ENABLED,
                                           gtk_widget_get_sensitive (widget));
    }
  else if (pspec->name == g_intern_static_string ("label"))
    {
      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_menu_item_get_label (GTK_MENU_ITEM (widget)));
    }
  else if (pspec->name == g_intern_static_string ("visible"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                           DBUSMENU_MENUITEM_PROP_VISIBLE,
                                           gtk_widget_get_visible (widget));
    }
  else if (pspec->name == g_intern_static_string ("stock"))
    {
      update_stock_item (child, widget);
    }
  else if (pspec->name == g_intern_static_string ("icon-name"))
    {
      update_icon_name (child, widget);
    }
  else if (pspec->name == g_intern_static_string ("parent"))
    {
      /*
        * We probably should have added a 'remove' method to the
        * UbuntuMenuProxy early on, but it's late in the cycle now.
        */
      if (gtk_widget_get_parent (widget) == NULL)
        {
          g_signal_handlers_disconnect_by_func (widget,
                                                G_CALLBACK (widget_notify_cb),
                                                child);

          DbusmenuMenuitem *parent = g_object_get_data (G_OBJECT (child), "dbusmenu-parent");

          if (DBUSMENU_IS_MENUITEM (parent) && DBUSMENU_IS_MENUITEM (child))
            {
              dbusmenu_menuitem_child_delete (parent, child);
            }
        }
    }
  else if (pspec->name == g_intern_static_string ("submenu"))
    {
      /* The underlying submenu got swapped out.  Let's see what it is now. */
      /* First, delete any children that may exist currently. */
      DbusmenuMenuitem * item = DBUSMENU_MENUITEM(g_object_get_data(G_OBJECT(widget), CACHED_MENUITEM));
      if (item != NULL)
        {
          GList * children = dbusmenu_menuitem_take_children (item);
          GList * child = children;
          while (child != NULL) {
            g_object_unref (G_OBJECT(child->data));
            child = child->next;
          }
          g_list_free(children);
        }

      /* Now parse new submenu. */
      GtkWidget * menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
      RecurseContext recurse = {0};
      recurse.toplevel = gtk_widget_get_toplevel(widget);
      recurse.parent = item;
      parse_menu_structure_helper(menu, &recurse);
    }
}

static gboolean
should_show_image (GtkImage *image)
{
  GtkWidget *item;

  item = gtk_widget_get_ancestor (GTK_WIDGET (image),
                                  GTK_TYPE_IMAGE_MENU_ITEM);

  if (item)
    {
      GtkSettings *settings;
      gboolean gtk_menu_images;

      settings = gtk_widget_get_settings (item);

      g_object_get (settings, "gtk-menu-images", &gtk_menu_images, NULL);

      if (gtk_menu_images)
        return TRUE;

      return gtk_image_menu_item_get_always_show_image (GTK_IMAGE_MENU_ITEM (item));
    }

  return FALSE;
}

