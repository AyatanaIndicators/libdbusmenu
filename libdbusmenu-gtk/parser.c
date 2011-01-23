
#include "parser.h"
#include "menuitem.h"

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

DbusmenuMenuitem *
dbusmenu_gtk_parse_menu_structure (GtkWidget * widget)
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

  gtk_stock_lookup (image->data.stock.stock_id, &stock);

  if (should_show_image (image))
    dbusmenu_menuitem_property_set (menuitem,
                                    DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                    image->data.stock.stock_id);
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

  if (should_show_image (image))
    dbusmenu_menuitem_property_set (menuitem,
                                    DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                    image->data.name.icon_name);
  else
    dbusmenu_menuitem_property_remove (menuitem,
                                       DBUSMENU_MENUITEM_PROP_ICON_NAME);
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

