#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "menu.h"
#include "libdbusmenu-glib/client.h"

/* Properties */
enum {
	PROP_0,
	PROP_DBUSOBJECT,
	PROP_DBUSNAME
};

/* Private */
typedef struct _DbusmenuGtkMenuPrivate DbusmenuGtkMenuPrivate;
struct _DbusmenuGtkMenuPrivate {
	DbusmenuClient * client;

	gchar * dbus_object;
	gchar * dbus_name;
};

#define DBUSMENU_GTKMENU_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_GTKMENU_TYPE, DbusmenuGtkMenuPrivate))

/* Prototypes */
static void dbusmenu_gtkmenu_class_init (DbusmenuGtkMenuClass *klass);
static void dbusmenu_gtkmenu_init       (DbusmenuGtkMenu *self);
static void dbusmenu_gtkmenu_dispose    (GObject *object);
static void dbusmenu_gtkmenu_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);
/* Internal */
static void build_client (DbusmenuGtkMenu * self);

/* GObject Stuff */
G_DEFINE_TYPE (DbusmenuGtkMenu, dbusmenu_gtkmenu, GTK_TYPE_MENU);

static void
dbusmenu_gtkmenu_class_init (DbusmenuGtkMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuGtkMenuPrivate));

	object_class->dispose = dbusmenu_gtkmenu_dispose;
	object_class->finalize = dbusmenu_gtkmenu_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_DBUSOBJECT,
	                                 g_param_spec_string(DBUSMENU_CLIENT_PROP_DBUS_OBJECT, "DBus Object we represent",
	                                              "The Object on the client that we're getting our data from.",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_DBUSNAME,
	                                 g_param_spec_string(DBUSMENU_CLIENT_PROP_DBUS_NAME, "DBus Client we connect to",
	                                              "Name of the DBus client we're connecting to.",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	return;
}

static void
dbusmenu_gtkmenu_init (DbusmenuGtkMenu *self)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(self);

	priv->client = NULL;

	priv->dbus_object = NULL;
	priv->dbus_name = NULL;

	return;
}

static void
dbusmenu_gtkmenu_dispose (GObject *object)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(object);

	if (priv->client != NULL) {
		g_object_unref(G_OBJECT(priv->client));
		priv->client = NULL;
	}

	G_OBJECT_CLASS (dbusmenu_gtkmenu_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_gtkmenu_finalize (GObject *object)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(object);

	g_free(priv->dbus_object);
	priv->dbus_object = NULL;

	g_free(priv->dbus_name);
	priv->dbus_name = NULL;

	G_OBJECT_CLASS (dbusmenu_gtkmenu_parent_class)->finalize (object);
	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUSNAME:
		priv->dbus_name = g_value_dup_string(value);
		if (priv->dbus_name != NULL && priv->dbus_object != NULL) {
			build_client(DBUSMENU_GTKMENU(obj));
		}
		break;
	case PROP_DBUSOBJECT:
		priv->dbus_object = g_value_dup_string(value);
		if (priv->dbus_name != NULL && priv->dbus_object != NULL) {
			build_client(DBUSMENU_GTKMENU(obj));
		}
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(obj);

	switch (id) {
	case PROP_DBUSNAME:
		g_value_set_string(value, priv->dbus_name);
		break;
	case PROP_DBUSOBJECT:
		g_value_set_string(value, priv->dbus_object);
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

/* Internal Functions */

static const gchar * data_menuitem = "dbusmenugtk-data-gtkmenuitem";
static const gchar * data_menu =     "dbusmenugtk-data-gtkmenu";

static gboolean
menu_pressed_cb (GtkMenuItem * gmi, DbusmenuMenuitem * mi)
{
	dbusmenu_menuitem_activate(mi);
	return TRUE;
}

static void
menu_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, gchar * value, GtkMenuItem * gmi)
{
	if (!g_strcmp0(prop, "label")) {
		gtk_menu_item_set_label(gmi, value);
		gtk_widget_show(GTK_WIDGET(gmi));
	}

	return;
}

static void
destoryed_dbusmenuitem_cb (gpointer udata, GObject * dbusmenuitem)
{
	g_debug("DbusmenuMenuitem was destroyed");
	gtk_widget_destroy(GTK_WIDGET(udata));
	return;
}

static void
connect_menuitem (DbusmenuMenuitem * mi, GtkMenuItem * gmi)
{
	g_object_set_data(G_OBJECT(mi), data_menuitem, gmi);

	g_signal_connect(G_OBJECT(mi),  DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(menu_prop_change_cb), gmi);
	g_signal_connect(G_OBJECT(gmi), "activate", G_CALLBACK(menu_pressed_cb), mi);

	g_object_weak_ref(G_OBJECT(mi), destoryed_dbusmenuitem_cb, gmi);

	return;
}

static void
process_dbusmenu_menuitem (DbusmenuMenuitem * mi, GtkMenu * parentmenu)
{
	gpointer unknown_menuitem = g_object_get_data(G_OBJECT(mi), data_menuitem);
	if (unknown_menuitem == NULL) {
		/* Oh, a virgin DbusmenuMenuitem, let's fix that. */
		GtkWidget * menuitem = gtk_menu_item_new();
		connect_menuitem(mi, GTK_MENU_ITEM(menuitem));
		unknown_menuitem = menuitem;
		gtk_menu_shell_append(GTK_MENU_SHELL(parentmenu), menuitem);
	}

	GList * children = dbusmenu_menuitem_get_children(mi);
	if (children == NULL) {
		/* If there are no children to process we are
		   done and we can move along */
		return;
	}

	/* Phase 0: Make a submenu if we don't have one */
	gpointer unknown_menu = g_object_get_data(G_OBJECT(mi), data_menu);
	if (unknown_menu == NULL) {
		GtkWidget * gtkmenu = gtk_menu_new();
		g_object_ref(gtkmenu);
		g_object_set_data_full(G_OBJECT(mi), data_menu, gtkmenu, g_object_unref);
		unknown_menu = gtkmenu;
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(unknown_menuitem), gtkmenu);
		gtk_widget_show(gtkmenu);
	}

	/* Phase 1: Add missing children */
	GList * child = NULL;
	for (child = children; child != NULL; child = g_list_next(child)) {
		process_dbusmenu_menuitem(DBUSMENU_MENUITEM(child->data), GTK_MENU(unknown_menu));	
	}

	/* Phase 2: Delete removed children */
	/* Actually, we don't need to do this because of the weak
	   reference that we've added above.  When the DbusmenuMenuitem
	   gets destroyed it takes its GtkMenuItem with it.  Bye bye. */

	/* Phase 3: Profit! */
	return;
}

/* Processing the layout being updated and handling
   that and making it into a menu */
static void
process_layout_change (DbusmenuClient * client, DbusmenuGtkMenu * gtkmenu)
{
	DbusmenuMenuitem * root = dbusmenu_client_get_root(client);

	GList * children = dbusmenu_menuitem_get_children(root);
	if (children == NULL) {
		return;
	}

	GList * child = NULL;
	for (child = children; child != NULL; child = g_list_next(child)) {
		process_dbusmenu_menuitem(DBUSMENU_MENUITEM(child->data), GTK_MENU(gtkmenu));
	}

	return;
}


/* Builds the client and connects all of the signals
   up for it so that it's happy-happy */
static void
build_client (DbusmenuGtkMenu * self)
{
	DbusmenuGtkMenuPrivate * priv = DBUSMENU_GTKMENU_GET_PRIVATE(self);

	if (priv->client == NULL) {
		priv->client = dbusmenu_client_new(priv->dbus_name, priv->dbus_object);

		/* Register for layout changes, this should come after the
		   creation of the client pulls it from DBus */
		g_signal_connect(G_OBJECT(priv->client), DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED, G_CALLBACK(process_layout_change), self);
	}

	return;
}

/* Public API */

/**
	dbusmenu_gtkmenu_new:
	@dbus_name: Name of the #DbusmenuServer on DBus
	@dbus_name: Name of the object on the #DbusmenuServer

	Creates a new #DbusmenuGtkMenu object and creates a #DbusmenuClient
	that connects across DBus to a #DbusmenuServer.

	Return value: A new #DbusmenuGtkMenu sync'd with a server
*/
DbusmenuGtkMenu *
dbusmenu_gtkmenu_new (gchar * dbus_name, gchar * dbus_object)
{
	return g_object_new(DBUSMENU_GTKMENU_TYPE,
	                    DBUSMENU_CLIENT_PROP_DBUS_OBJECT, dbus_object,
	                    DBUSMENU_CLIENT_PROP_DBUS_NAME, dbus_name,
	                    NULL);
}

