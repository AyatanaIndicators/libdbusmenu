#ifndef __IDO_SERIALIZABLE_MENU_ITEM_H__
#define __IDO_SERIALIZABLE_MENU_ITEM_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/client.h>

G_BEGIN_DECLS

#define IDO_TYPE_SERIALIZABLE_MENU_ITEM            (ido_serializable_menu_item_get_type ())
#define IDO_SERIALIZABLE_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDO_TYPE_SERIALIZABLE_MENU_ITEM, IdoSerializableMenuItem))
#define IDO_SERIALIZABLE_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), IDO_TYPE_SERIALIZABLE_MENU_ITEM, IdoSerializableMenuItemClass))
#define IDO_IS_SERIALIZABLE_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDO_TYPE_SERIALIZABLE_MENU_ITEM))
#define IDO_IS_SERIALIZABLE_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDO_TYPE_SERIALIZABLE_MENU_ITEM))
#define IDO_SERIALIZABLE_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), IDO_TYPE_SERIALIZABLE_MENU_ITEM, IdoSerializableMenuItemClass))

typedef struct _IdoSerializableMenuItem        IdoSerializableMenuItem;
typedef struct _IdoSerializableMenuItemClass   IdoSerializableMenuItemClass;
typedef struct _IdoSerializableMenuItemPrivate IdoSerializableMenuItemPrivate;

struct _IdoSerializableMenuItemClass {
	GtkMenuItemClass parent_class;

	/* Subclassable functions */
	const gchar *        (*get_type_string)          (void);
	GHashTable *         (*get_default_properties)   (void);

	DbusmenuMenuitem *   (*get_dbusmenu_menuitem)    (IdoSerializableMenuItem * smi);

	/* Signals */



	/* Empty Space */
	void (*_ido_serializable_menu_item_reserved1) (void);
	void (*_ido_serializable_menu_item_reserved2) (void);
	void (*_ido_serializable_menu_item_reserved3) (void);
	void (*_ido_serializable_menu_item_reserved4) (void);
	void (*_ido_serializable_menu_item_reserved5) (void);
	void (*_ido_serializable_menu_item_reserved6) (void);
};

struct _IdoSerializableMenuItem {
	GtkMenuItem parent;

	IdoSerializableMenuItemPrivate * priv;
};

GType ido_serializable_menu_item_get_type (void);

DbusmenuMenuitem *  ido_serializable_menu_item_get_dbusmenu_menuitem (IdoSerializableMenuItem * smi);
void                ido_serializable_menu_item_register_to_client (DbusmenuClient * client, GType item_type);

G_END_DECLS

#endif
