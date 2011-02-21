#ifndef __DBUSMENU_DEFAULTS_H__
#define __DBUSMENU_DEFAULTS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define DBUSMENU_TYPE_DEFAULTS            (dbusmenu_defaults_get_type ())
#define DBUSMENU_DEFAULTS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_DEFAULTS, DbusmenuDefaults))
#define DBUSMENU_DEFAULTS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_DEFAULTS, DbusmenuDefaultsClass))
#define IS_DBUSMENU_DEFAULTS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_DEFAULTS))
#define IS_DBUSMENU_DEFAULTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_DEFAULTS))
#define DBUSMENU_DEFAULTS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_DEFAULTS, DbusmenuDefaultsClass))

typedef struct _DbusmenuDefaults      DbusmenuDefaults;
typedef struct _DbusmenuDefaultsClass DbusmenuDefaultsClass;

struct _DbusmenuDefaultsClass {
	GObjectClass parent_class;
};

struct _DbusmenuDefaults {
	GObject parent;
};

GType dbusmenu_defaults_get_type (void);

G_END_DECLS

#endif
