#ifndef __DBUSMENU_MENUITEM_H__
#define __DBUSMENU_MENUITEM_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define DBUSMENU_TYPE_MENUITEM            (dbusmenu_menuitem_get_type ())
#define DBUSMENU_MENUITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitem))
#define DBUSMENU_MENUITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemClass))
#define DBUSMENU_IS_MENUITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_MENUITEM))
#define DBUSMENU_IS_MENUITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_MENUITEM))
#define DBUSMENU_MENUITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_MENUITEM, DbusmenuMenuitemClass))

typedef struct _DbusmenuMenuitem      DbusmenuMenuitem;
typedef struct _DbusmenuMenuitemClass DbusmenuMenuitemClass;

#define DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED    "property-changed"
#define DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED      "item-activated"

struct _DbusmenuMenuitemClass
{
	GObjectClass parent_class;

	/* Signals */
	void (*property_changed) (gchar * property);
	void (*item_activated) (void);

	/* Virtual functions */
	void (*buildxml) (GPtrArray ** stringarray);

	void (*reserved1) (void);
	void (*reserved2) (void);
	void (*reserved3) (void);
	void (*reserved4) (void);
};

struct _DbusmenuMenuitem
{
	GObject parent;
};

GType dbusmenu_menuitem_get_type (void);

GList * dbusmenu_menuitem_get_children (DbusmenuMenuitem * mi);
guint dbusmenu_menuitem_get_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * parent);

gboolean dbusmenu_menuitem_child_append (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_delete (DbusmenuMenuitem * mi, DbusmenuMenuitem * child);
gboolean dbusmenu_menuitem_child_add_position (DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position);

gboolean dbusmenu_menuitem_property_set (DbusmenuMenuitem * mi, const gchar * property, const gchar * value);
const gchar * dbusmenu_menuitem_property_get (DbusmenuMenuitem * mi, const gchar * property);
gboolean dbusmenu_menuitem_property_exist (DbusmenuMenuitem * mi, const gchar * property);

void dbusmenu_menuitem_buildxml (DbusmenuMenuitem * mi, GPtrArray ** array);

G_END_DECLS

#endif
