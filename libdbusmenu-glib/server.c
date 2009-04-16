#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "server.h"
#include "server-marshal.h"

/* DBus Prototypes */
static gboolean _dbusmenu_server_get_property (void);
static gboolean _dbusmenu_server_get_properties (void);
static gboolean _dbusmenu_server_call (void);
static gboolean _dbusmenu_server_list_properties (void);

#include "dbusmenu-server.h"

/* Privates, I'll show you mine... */
typedef struct _DbusmenuServerPrivate DbusmenuServerPrivate;

struct _DbusmenuServerPrivate
{
	DbusmenuMenuitem * root;
	gchar * dbusobject;
};

#define DBUSMENU_SERVER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_SERVER, DbusmenuServerPrivate))

/* Signals */
enum {
	ID_PROP_UPDATE,
	ID_UPDATE,
	LAYOUT_UPDATE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Properties */
enum {
	PROP_0,
	PROP_DBUS_OBJECT,
	PROP_ROOT_NODE,
	PROP_LAYOUT
};

/* Prototype */
static void dbusmenu_server_class_init (DbusmenuServerClass *class);
static void dbusmenu_server_init       (DbusmenuServer *self);
static void dbusmenu_server_dispose    (GObject *object);
static void dbusmenu_server_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);

G_DEFINE_TYPE (DbusmenuServer, dbusmenu_server, G_TYPE_OBJECT);

static void
dbusmenu_server_class_init (DbusmenuServerClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	g_type_class_add_private (class, sizeof (DbusmenuServerPrivate));

	object_class->dispose = dbusmenu_server_dispose;
	object_class->finalize = dbusmenu_server_finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	signals[ID_PROP_UPDATE] =   g_signal_new(DBUSMENU_SERVER_SIGNAL_ID_PROP_UPDATE,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, id_prop_update),
	                                         NULL, NULL,
	                                         _dbusmenu_server_marshal_VOID__UINT_STRING_STRING,
	                                         G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	signals[ID_UPDATE] =        g_signal_new(DBUSMENU_SERVER_SIGNAL_ID_UPDATE,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, id_update),
	                                         NULL, NULL,
	                                         g_cclosure_marshal_VOID__UINT,
	                                         G_TYPE_NONE, 1, G_TYPE_UINT);
	signals[LAYOUT_UPDATE] =    g_signal_new(DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATE,
	                                         G_TYPE_FROM_CLASS(class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET(DbusmenuServerClass, layout_update),
	                                         NULL, NULL,
	                                         g_cclosure_marshal_VOID__VOID,
	                                         G_TYPE_NONE, 0);


	g_object_class_install_property (object_class, PROP_DBUS_OBJECT,
	                                 g_param_spec_string(DBUSMENU_SERVER_PROP_DBUS_OBJECT, "DBus object path",
	                                              "The object that represents this set of menus on DBus",
	                                              "/org/freedesktop/dbusmenu",
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_ROOT_NODE,
	                                 g_param_spec_object(DBUSMENU_SERVER_PROP_ROOT_NODE, "Root menu node",
	                                              "The base object of the menus that are served",
	                                              DBUSMENU_TYPE_MENUITEM,
	                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_LAYOUT,
	                                 g_param_spec_string(DBUSMENU_SERVER_PROP_LAYOUT, "XML Layout of the menus",
	                                              "A simple XML string that describes the layout of the menus",
	                                              "<menu />",
	                                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	dbus_g_object_type_install_info(DBUSMENU_TYPE_SERVER, &dbus_glib__dbusmenu_server_object_info);

	return;
}

static void
dbusmenu_server_init (DbusmenuServer *self)
{
	DbusmenuServerPrivate * priv = DBUSMENU_SERVER_GET_PRIVATE(self);

	priv->root = NULL;

	return;
}

static void
dbusmenu_server_dispose (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_server_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_server_finalize (GObject *object)
{
	G_OBJECT_CLASS (dbusmenu_server_parent_class)->finalize (object);
	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	switch (id) {
	case PROP_DBUS_OBJECT:
		break;
	case PROP_ROOT_NODE:
		break;
	case PROP_LAYOUT:
		break;
	default:
		g_return_if_reached();
		break;
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	switch (id) {
	case PROP_DBUS_OBJECT:
		break;
	case PROP_ROOT_NODE:
		break;
	case PROP_LAYOUT:
		break;
	default:
		g_return_if_reached();
		break;
	}

	return;
}

/* DBus interface */
static gboolean 
_dbusmenu_server_get_property (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_get_properties (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_call (void)
{

	return TRUE;
}

static gboolean
_dbusmenu_server_list_properties (void)
{

	return TRUE;
}

/* Public Interface */
DbusmenuServer *
dbusmenu_server_new (const gchar * object)
{
	if (object == NULL) {
		object = "/org/freedesktop/dbusmenu";
	}

	DbusmenuServer * self = g_object_new(DBUSMENU_TYPE_SERVER,
	                                     "dbus-object-name", object,
	                                     NULL);

	return self;
}

void
dbusmenu_server_set_root (DbusmenuServer * self, DbusmenuMenuitem * root)
{
	GValue rootvalue = {0};
	g_value_init(&rootvalue, G_TYPE_POINTER);
	g_value_set_pointer(&rootvalue, root);
	g_object_set_property(G_OBJECT(self), "root-node", &rootvalue);
	return;
}



