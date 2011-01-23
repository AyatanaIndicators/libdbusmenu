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

#define CACHED_MENUITEM  "dbusmenu-gtk-parser-cached-item"

typedef struct _RecurseContext
{
  GtkWidget * toplevel;
  gint count;
  DbusmenuMenuitem *stack[30];
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

DbusmenuMenuitem *
dbusmenu_gtk_parse_menu_structure (GtkWidget * widget)
{
  RecurseContext recurse = {0};

  recurse.count = -1;
  recurse.toplevel = gtk_widget_get_toplevel(widget);

  parse_menu_structure_helper(widget, &recurse);

  if (recurse.stack[0] != NULL && DBUSMENU_IS_MENUITEM(recurse.stack[0])) {
  	return recurse.stack[0];
  }

  return NULL;
}

static void
dbusmenu_cache_freed (gpointer data, GObject * obj)
{
	g_object_set_data(G_OBJECT(data), CACHED_MENUITEM, NULL);
	return;
}

static void
object_cache_freed (gpointer data)
{
	g_object_weak_unref(G_OBJECT(data), dbusmenu_cache_freed, data);
	return;
}

static void
parse_menu_structure_helper (GtkWidget * widget, RecurseContext * recurse)
{
  if (GTK_IS_CONTAINER (widget))
    {
      gboolean increment = GTK_IS_MENU_BAR (widget) || GTK_IS_MENU_ITEM (widget);

      if (increment)
        recurse->count++;

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
      if (recurse->count == 0 && GTK_IS_MENU_BAR (widget))
        {
          GList *children = gtk_container_get_children (GTK_CONTAINER (widget));

          for (; children != NULL; children = children->next)
            {
              gtk_menu_shell_activate_item (GTK_MENU_SHELL (widget),
                                            children->data,
                                            TRUE);
            }

          g_list_free (children);
        }

      if (recurse->count > -1 && increment)
        {
		  gpointer pmi = g_object_get_data(G_OBJECT(widget), CACHED_MENUITEM);
          DbusmenuMenuitem *dmi = NULL;
		  if (pmi != NULL) dmi = DBUSMENU_MENUITEM(pmi);

          if (dmi != NULL)
            {
              if (increment)
                recurse->count--;

              return;
            }
          else
            {
              recurse->stack[recurse->count] = construct_dbusmenu_for_widget (widget);
			  g_object_set_data_full(G_OBJECT(widget), CACHED_MENUITEM, recurse->stack[recurse->count], object_cache_freed);
			  g_object_weak_ref(G_OBJECT(recurse->stack[recurse->count]), dbusmenu_cache_freed, widget);
            }

          if (!gtk_widget_get_visible (widget))
            {
              g_signal_connect (G_OBJECT (widget),
                                "notify::visible",
                                G_CALLBACK (menuitem_notify_cb),
                                recurse->toplevel);
            }

          if (GTK_IS_TEAROFF_MENU_ITEM (widget))
            {
              dbusmenu_menuitem_property_set_bool (recurse->stack[recurse->count],
                                                   DBUSMENU_MENUITEM_PROP_VISIBLE,
                                                   FALSE);
            }

          if (recurse->count > 0)
            {
              GList *children = NULL;
              GList *peek = NULL;

              if (recurse->stack[recurse->count - 1])
                {
                  children = dbusmenu_menuitem_get_children (recurse->stack[recurse->count - 1]);

                  if (children)
                    {
                      peek = g_list_find (children, recurse->stack[recurse->count]);
                    }

                  if (!peek)
                    {
                      /* Should we set a weak ref on the parent? */
                      g_object_set_data (G_OBJECT (recurse->stack[recurse->count]),
                                         "dbusmenu-parent",
                                         recurse->stack[recurse->count - 1]);
                      dbusmenu_menuitem_child_append (recurse->stack[recurse->count - 1],
                                                      recurse->stack[recurse->count]);
                    }
                }
              else
                {
                  DbusmenuMenuitem *item = NULL; /* g_hash_table_lookup (recurse->context->lookup,
                                                                gtk_widget_get_parent (widget)); */

                  if (item)
                    {
                      children = dbusmenu_menuitem_get_children (item);

                      if (children)
                        {
                          peek = g_list_find (children, recurse->stack[recurse->count]);
                        }

                      if (!peek)
                        {
                          g_object_set_data (G_OBJECT (recurse->stack[recurse->count]),
                                             "dbusmenu-parent",
                                             recurse->stack[recurse->count - 1]);

                          dbusmenu_menuitem_child_append (item, recurse->stack[recurse->count]);
                        }
                    }
                }
            }
        }

      gtk_container_foreach (GTK_CONTAINER (widget),
                             (GtkCallback)parse_menu_structure_helper,
                             recurse);

      if (GTK_IS_MENU_ITEM (widget))
        {
          GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));

          if (menu != NULL)
            {
              parse_menu_structure_helper (menu, recurse);
            }
        }

      if (increment)
        recurse->count--;
    }
}

static DbusmenuMenuitem *
construct_dbusmenu_for_widget (GtkWidget * widget)
{
  DbusmenuMenuitem *mi = dbusmenu_menuitem_new ();

  if (GTK_IS_MENU_ITEM (widget))
    {
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
    }

  return mi;

	return NULL;
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
      dbusmenu_menuitem_property_set_bool (mi,
                                           DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                           gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
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

