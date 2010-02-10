/*
An object to ferry over properties and signals between two different
dbusmenu instances.  Useful for services.

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
DbusmenuMenuitemProxy * dbusmenu_menuitem_proxy_new (DbusmenuMenuitem * mi);

G_END_DECLS

#endif
