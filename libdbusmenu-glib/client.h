#ifndef __DBUSMENU_CLIENT_H__
#define __DBUSMENU_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

#include "menuitem.h"

G_BEGIN_DECLS

#define DBUSMENU_TYPE_CLIENT            (dbusmenu_client_get_type ())
#define DBUSMENU_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_CLIENT, DbusmenuClient))
#define DBUSMENU_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_CLIENT, DbusmenuClientClass))
#define DBUSMENU_IS_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_CLIENT))
#define DBUSMENU_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_CLIENT))
#define DBUSMENU_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_CLIENT, DbusmenuClientClass))

#define DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED  "layout-updated"

#define DBUSMENU_CLIENT_PROP_DBUS_NAME     "dbus-name"
#define DBUSMENU_CLIENT_PROP_DBUS_OBJECT   "dbus-object"

/**
	DbusmenuClientClass:
	@parent_class: #GObjectClass
	@layout_updated: Slot for #DbusmenuClient::layout-updated.
	@reserved1: Reserved for future use.
	@reserved2: Reserved for future use.
	@reserved3: Reserved for future use.
	@reserved4: Reserved for future use.

	A simple class that takes all of the information from a
	#DbusmenuServer over DBus and makes the same set of 
	#DbusmenuMenuitem objects appear on the other side.
*/
typedef struct _DbusmenuClientClass DbusmenuClientClass;
struct _DbusmenuClientClass {
	GObjectClass parent_class;

	void (*layout_updated)(void);

	/* Reserved for future use */
	void (*reserved1) (void);
	void (*reserved2) (void);
	void (*reserved3) (void);
	void (*reserved4) (void);
};

/**
	DbusmenuClient:
	@parent: #GObject.

	The client for a #DbusmenuServer creating a shared
	object set of #DbusmenuMenuitem objects.
*/
typedef struct _DbusmenuClient      DbusmenuClient;
struct _DbusmenuClient {
	GObject parent;
};

GType                dbusmenu_client_get_type   (void);
DbusmenuClient *     dbusmenu_client_new        (const gchar * name, const gchar * object);
DbusmenuMenuitem *   dbusmenu_client_get_root   (DbusmenuClient * client);

/**
	SECTION:client
	@short_description: The catcher of all the server traffic
	@stability: Unstable
	@include: libdbusmenu-glib/client.h

	The client exists as a mirror to the server.  For most folks
	all they will do with a client is set it up to connect to
	a server and then watch as the menu items on their side
	of the bus change.  This is all they should need to know about
	the client, that it magically makes their menuitems dance.

	It does this by setting up signal watchers and adjusting
	the menuitems appropriately.  Most users should watch the
	menu items and the signal #DbusmenuClient::layout-changed for
	larger events so that they can be optimized.  It is possible
	with that signal that even the root node would change.  If
	that doesn't happen the normal signals on the individual
	nodes should be enough for most users.
*/

G_END_DECLS

#endif
