#ifndef __DBUSMENU_CLIENT_H__
#define __DBUSMENU_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_TYPE_CLIENT            (dbusmenu_client_get_type ())
#define DBUSMENU_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_CLIENT, DbusmenuClient))
#define DBUSMENU_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_CLIENT, DbusmenuClientClass))
#define DBUSMENU_IS_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_CLIENT))
#define DBUSMENU_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_CLIENT))
#define DBUSMENU_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_CLIENT, DbusmenuClientClass))

#define DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED  "layout-updated"

#define DBUSMENU_CLIENT_PROP_DBUS_NAME     "dbus-name"
#define DBUSMENU_CLIENT_PROP_DBUS_OBJECT   "dbus-object"

typedef struct _DbusmenuClient      DbusmenuClient;
typedef struct _DbusmenuClientClass DbusmenuClientClass;

struct _DbusmenuClientClass {
	GObjectClass parent_class;

	void (*layout_updated)(void);

	/* Reserved for future use */
	void (*reserved1) (void);
	void (*reserved2) (void);
	void (*reserved3) (void);
	void (*reserved4) (void);
};

struct _DbusmenuClient {
	GObject parent;
};

GType                dbusmenu_client_get_type   (void);
DbusmenuClient *     dbusmenu_client_new        (const gchar * name, const gchar * object);
DbusmenuMenuitem *   dbusmenu_client_get_root   (DbusmenuClient * client);

G_END_DECLS

#endif
