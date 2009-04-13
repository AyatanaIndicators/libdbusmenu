#ifndef __DBUSMENU_MENUITEM_H__
#define __DBUSMENU_MENUITEM_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define DBUSMENU_MENUITEM_TYPE            (dbusmenu_menuitem_get_type ())
#define DBUSMENU_MENUITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_MENUITEM_TYPE, DbusmenuMenuitem))
#define DBUSMENU_MENUITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_MENUITEM_TYPE, DbusmenuMenuitemClass))
#define DBUSMENU_IS_MENUITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_MENUITEM_TYPE))
#define DBUSMENU_IS_MENUITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_MENUITEM_TYPE))
#define DBUSMENU_MENUITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_MENUITEM_TYPE, DbusmenuMenuitemClass))

typedef struct _DbusmenuMenuitem      DbusmenuMenuitem;
typedef struct _DbusmenuMenuitemClass DbusmenuMenuitemClass;

struct _DbusmenuMenuitemClass
{
GObjectClass parent_class;
};

struct _DbusmenuMenuitem
{
GObject parent;
};

GType dbusmenu_menuitem_get_type (void);

G_END_DECLS

#endif
