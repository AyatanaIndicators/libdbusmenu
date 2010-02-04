#ifndef __DBUSMENU_CLIENT_MENUITEM_H__
#define __DBUSMENU_CLIENT_MENUITEM_H__

#include <glib.h>
#include <glib-object.h>
#include "menuitem.h"
#include "client.h"

G_BEGIN_DECLS

#define DBUSMENU_CLIENT_MENUITEM_TYPE            (dbusmenu_client_menuitem_get_type ())
#define DBUSMENU_CLIENT_MENUITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_CLIENT_MENUITEM_TYPE, DbusmenuClientMenuitem))
#define DBUSMENU_CLIENT_MENUITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_CLIENT_MENUITEM_TYPE, DbusmenuClientMenuitemClass))
#define DBUSMENU_IS_CLIENT_MENUITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_CLIENT_MENUITEM_TYPE))
#define DBUSMENU_IS_CLIENT_MENUITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_CLIENT_MENUITEM_TYPE))
#define DBUSMENU_CLIENT_MENUITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_CLIENT_MENUITEM_TYPE, DbusmenuClientMenuitemClass))

typedef struct _DbusmenuClientMenuitem      DbusmenuClientMenuitem;
typedef struct _DbusmenuClientMenuitemClass DbusmenuClientMenuitemClass;

struct _DbusmenuClientMenuitemClass {
	DbusmenuMenuitemClass parent_class;
};

struct _DbusmenuClientMenuitem {
	DbusmenuMenuitem parent;
};

GType dbusmenu_client_menuitem_get_type (void);
DbusmenuClientMenuitem * dbusmenu_client_menuitem_new (gint id, DbusmenuClient * client);

G_END_DECLS

#endif
