#ifndef __GENERICMENUITEM_H__
#define __GENERICMENUITEM_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GENERICMENUITEM_TYPE            (genericmenuitem_get_type ())
#define GENERICMENUITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GENERICMENUITEM_TYPE, Genericmenuitem))
#define GENERICMENUITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GENERICMENUITEM_TYPE, GenericmenuitemClass))
#define IS_GENERICMENUITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GENERICMENUITEM_TYPE))
#define IS_GENERICMENUITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GENERICMENUITEM_TYPE))
#define GENERICMENUITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GENERICMENUITEM_TYPE, GenericmenuitemClass))

typedef struct _Genericmenuitem        Genericmenuitem;
typedef struct _GenericmenuitemClass   GenericmenuitemClass;
typedef struct _GenericmenuitemPrivate GenericmenuitemPrivate;

/**
	GenericmenuitemClass:
	@parent_class: Our parent #GtkCheckMenuItemClass
*/
struct _GenericmenuitemClass {
	GtkCheckMenuItemClass parent_class;
};

/**
	Genericmenuitem:
	@parent: Our parent #GtkCheckMenuItem
*/
struct _Genericmenuitem {
	GtkCheckMenuItem parent;
	GenericmenuitemPrivate * priv;
};

GType genericmenuitem_get_type (void);

G_END_DECLS

#endif
