#ifndef __MENU_H__
#define __MENU_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MENU_TYPE            (menu_get_type ())
#define MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MENU_TYPE, Menu))
#define MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MENU_TYPE, MenuClass))
#define IS_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MENU_TYPE))
#define IS_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MENU_TYPE))
#define MENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MENU_TYPE, MenuClass))

typedef struct _Menu      Menu;
typedef struct _MenuClass MenuClass;

struct _MenuClass
{
GtkMenuClass parent_class;
};

struct _Menu
{
GtkMenu parent;
};

GType menu_get_type (void);

G_END_DECLS

#endif
