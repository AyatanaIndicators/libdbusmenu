#ifndef __DBUSMENU_CLIENT_H__
#define __DBUSMENU_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_CLIENT_TYPE            (dbusmenu_client_get_type ())
#define DBUSMENU_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_CLIENT_TYPE, DbusmenuClient))
#define DBUSMENU_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_CLIENT_TYPE, DbusmenuClientClass))
#define DBUSMENU_IS_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_CLIENT_TYPE))
#define DBUSMENU_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_CLIENT_TYPE))
#define DBUSMENU_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_CLIENT_TYPE, DbusmenuClientClass))

typedef struct _DbusmenuClient      DbusmenuClient;
typedef struct _DbusmenuClientClass DbusmenuClientClass;

struct _DbusmenuClientClass {
	GObjectClass parent_class;
};

struct _DbusmenuClient {
	GObject parent;
};

GType                dbusmenu_client_get_type   (void);
DbusmenuClient *     dbusmenu_client_new        (const gchar * name, const gchar * object);
DbusmenuMenuitem *   dbusmenu_client_get_root   (DbusmenuClient * client);

G_END_DECLS

#endif
