#ifndef __DBUSMENU_SERVER_H__
#define __DBUSMENU_SERVER_H__

#include <glib.h>
#include <glib-object.h>

#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_SERVER_TYPE            (dbusmenu_server_get_type ())
#define DBUSMENU_SERVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_SERVER_TYPE, DbusmenuServer))
#define DBUSMENU_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_SERVER_TYPE, DbusmenuServerClass))
#define DBUSMENU_IS_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_SERVER_TYPE))
#define DBUSMENU_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_SERVER_TYPE))
#define DBUSMENU_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_SERVER_TYPE, DbusmenuServerClass))

typedef struct _DbusmenuServer      DbusmenuServer;
typedef struct _DbusmenuServerClass DbusmenuServerClass;

struct _DbusmenuServerClass {
	GObjectClass parent_class;
};

struct _DbusmenuServer {
	GObject parent;
};

GType               dbusmenu_server_get_type   (void);
DbusmenuServer *    dbusmenu_server_new        (const gchar * object);
void                dbusmenu_server_set_root   (DbusmenuServer * server, DbusmenuMenuitem * root);

G_END_DECLS

#endif
