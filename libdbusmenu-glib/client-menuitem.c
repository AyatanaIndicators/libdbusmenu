/*
A small subclass of the menuitem for using clients.

Copyright 2010 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the 
Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by 
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the applicable version of the GNU Lesser General Public 
License for more details.

You should have received a copy of both the GNU Lesser General Public 
License version 3 and version 2.1 along with this program.  If not, see 
<http://www.gnu.org/licenses/>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "client-menuitem.h"

typedef struct _DbusmenuClientMenuitemPrivate DbusmenuClientMenuitemPrivate;

struct _DbusmenuClientMenuitemPrivate
{
	DbusmenuClient * client;
};

#define DBUSMENU_CLIENT_MENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_CLIENT_MENUITEM_TYPE, DbusmenuClientMenuitemPrivate))

static void dbusmenu_client_menuitem_class_init (DbusmenuClientMenuitemClass *klass);
static void dbusmenu_client_menuitem_init       (DbusmenuClientMenuitem *self);
static void dbusmenu_client_menuitem_dispose    (GObject *object);
static void dbusmenu_client_menuitem_finalize   (GObject *object);
static void handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp);
static void send_about_to_show (DbusmenuMenuitem * mi);

G_DEFINE_TYPE (DbusmenuClientMenuitem, dbusmenu_client_menuitem, DBUSMENU_TYPE_MENUITEM);

static void
dbusmenu_client_menuitem_class_init (DbusmenuClientMenuitemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuClientMenuitemPrivate));

	object_class->dispose = dbusmenu_client_menuitem_dispose;
	object_class->finalize = dbusmenu_client_menuitem_finalize;

	DbusmenuMenuitemClass * mclass = DBUSMENU_MENUITEM_CLASS(klass);
	mclass->handle_event = handle_event;
	mclass->send_about_to_show = send_about_to_show;

	return;
}

static void
dbusmenu_client_menuitem_init (DbusmenuClientMenuitem *self)
{

	return;
}

static void
dbusmenu_client_menuitem_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_client_menuitem_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_client_menuitem_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_client_menuitem_parent_class)->finalize (object);
	return;
}

DbusmenuClientMenuitem *
dbusmenu_client_menuitem_new (gint id, DbusmenuClient * client)
{
	DbusmenuClientMenuitem * mi = g_object_new(DBUSMENU_CLIENT_MENUITEM_TYPE, "id", id, NULL);
	DbusmenuClientMenuitemPrivate * priv = DBUSMENU_CLIENT_MENUITEM_GET_PRIVATE(mi);
	priv->client = client;
	return mi;
}

static void
handle_event (DbusmenuMenuitem * mi, const gchar * name, const GValue * value, guint timestamp)
{
	DbusmenuClientMenuitemPrivate * priv = DBUSMENU_CLIENT_MENUITEM_GET_PRIVATE(mi);
	dbusmenu_client_send_event(priv->client, dbusmenu_menuitem_get_id(mi), name, value, timestamp);
	return;
}

static void
send_about_to_show (DbusmenuMenuitem * mi)
{
	DbusmenuClientMenuitemPrivate * priv = DBUSMENU_CLIENT_MENUITEM_GET_PRIVATE(mi);
	dbusmenu_client_send_about_to_show(priv->client, dbusmenu_menuitem_get_id(mi));
	return;
}
