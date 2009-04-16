#ifndef __DBUSMENU_SERVER_H__
#define __DBUSMENU_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_TYPE_SERVER            (dbusmenu_server_get_type ())
#define DBUSMENU_SERVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_SERVER, DbusmenuServer))
#define DBUSMENU_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_SERVER, DbusmenuServerClass))
#define DBUSMENU_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_SERVER))
#define DBUSMENU_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_SERVER))
#define DBUSMENU_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_SERVER, DbusmenuServerClass))

#define DBUSMENU_SERVER_SIGNAL_ID_PROP_UPDATE  "id-prop-update"
#define DBUSMENU_SERVER_SIGNAL_ID_UPDATE       "id-update"
#define DBUSMENU_SERVER_SIGNAL_LAYOUT_UPDATE   "layout-update"

#define DBUSMENU_SERVER_PROP_DBUS_OBJECT       "dbus-object"
#define DBUSMENU_SERVER_PROP_ROOT_NODE         "root-node"

typedef struct _DbusmenuServer      DbusmenuServer;
typedef struct _DbusmenuServerClass DbusmenuServerClass;

struct _DbusmenuServerClass {
	GObjectClass parent_class;

	/* Signals */
	void (*id_prop_update)(guint id, gchar * property, gchar * value);
	void (*id_update)(guint id);
	void (*layout_update)(void);

	/* Reserved */
	void (*dbusmenu_server_reserved1)(void);
	void (*dbusmenu_server_reserved2)(void);
	void (*dbusmenu_server_reserved3)(void);
	void (*dbusmenu_server_reserved4)(void);
};

struct _DbusmenuServer {
	GObject parent;
};

GType               dbusmenu_server_get_type   (void);
DbusmenuServer *    dbusmenu_server_new        (const gchar * object);
void                dbusmenu_server_set_root   (DbusmenuServer * server, DbusmenuMenuitem * root);

G_END_DECLS

#endif
