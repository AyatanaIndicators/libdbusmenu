#ifndef __DBUSMENU_GTKMENU_H__
#define __DBUSMENU_GTKMENU_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define DBUSMENU_GTKMENU_TYPE            (dbusmenu_gtkmenu_get_type ())
#define DBUSMENU_GTKMENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_GTKMENU_TYPE, DbusmenuGtkMenu))
#define DBUSMENU_GTKMENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_GTKMENU_TYPE, DbusmenuGtkMenuClass))
#define DBUSMENU_IS_GTKMENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_GTKMENU_TYPE))
#define DBUSMENU_IS_GTKMENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_GTKMENU_TYPE))
#define DBUSMENU_GTKMENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_GTKMENU_TYPE, DbusmenuGtkMenuClass))

typedef struct _DbusmenuGtkMenuClass DbusmenuGtkMenuClass;
struct _DbusmenuGtkMenuClass {
	GtkMenuClass parent_class;
};

typedef struct _DbusmenuGtkMenu      DbusmenuGtkMenu;
struct _DbusmenuGtkMenu {
	GtkMenu parent;
};

GType dbusmenu_gtkmenu_get_type (void);

G_END_DECLS

#endif
