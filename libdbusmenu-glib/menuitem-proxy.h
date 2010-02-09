#ifndef __DBUSMENU_MENUITEM_PROXY_H__
#define __DBUSMENU_MENUITEM_PROXY_H__

#include <glib.h>
#include <glib-object.h>
#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_TYPE_MENUITEM_PROXY            (dbusmenu_menuitem_proxy_get_type ())
#define DBUSMENU_MENUITEM_PROXY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_MENUITEM_PROXY, DbusmenuMenuitemProxy))
#define DBUSMENU_MENUITEM_PROXY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_MENUITEM_PROXY, DbusmenuMenuitemProxyClass))
#define DBUSMENU_IS_MENUITEM_PROXY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_MENUITEM_PROXY))
#define DBUSMENU_IS_MENUITEM_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_MENUITEM_PROXY))
#define DBUSMENU_MENUITEM_PROXY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_MENUITEM_PROXY, DbusmenuMenuitemProxyClass))

typedef struct _DbusmenuMenuitemProxy      DbusmenuMenuitemProxy;
typedef struct _DbusmenuMenuitemProxyClass DbusmenuMenuitemProxyClass;

/**
	DbusmenuMenuitemProxyClass:
	@parent_class: The Class of #DbusmeneMenuitem

	Functions and signal slots for #DbusmenuMenuitemProxy.
*/
struct _DbusmenuMenuitemProxyClass {
	DbusmenuMenuitemClass parent_class;
};

/**
	DbusmeneMenuitemProxy:
	@parent: The instance of #DbusmenuMenuitem

	Public instance data for a #DbusmenuMenuitemProxy.
*/
struct _DbusmenuMenuitemProxy {
	DbusmenuMenuitem parent;
};

GType dbusmenu_menuitem_proxy_get_type (void);

G_END_DECLS

#endif
