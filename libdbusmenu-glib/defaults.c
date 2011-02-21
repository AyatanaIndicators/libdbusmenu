/*
A library to communicate a menu object set accross DBus and
track updates and maintain consistency.

Copyright 2011 Canonical Ltd.

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

#include "defaults.h"

struct _DbusmenuDefaultsPrivate {
	GHashTable * types;
};

#define DBUSMENU_DEFAULTS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_DEFAULTS_TYPE, DbusmenuDefaultsPrivate))

static void dbusmenu_defaults_class_init (DbusmenuDefaultsClass *klass);
static void dbusmenu_defaults_init       (DbusmenuDefaults *self);
static void dbusmenu_defaults_dispose    (GObject *object);
static void dbusmenu_defaults_finalize   (GObject *object);

G_DEFINE_TYPE (DbusmenuDefaults, dbusmenu_defaults, G_TYPE_OBJECT);

static void
dbusmenu_defaults_class_init (DbusmenuDefaultsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (DbusmenuDefaultsPrivate));

	object_class->dispose = dbusmenu_defaults_dispose;
	object_class->finalize = dbusmenu_defaults_finalize;
	return;
}

static void
dbusmenu_defaults_init (DbusmenuDefaults *self)
{

	return;
}

static void
dbusmenu_defaults_dispose (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_defaults_parent_class)->dispose (object);
	return;
}

static void
dbusmenu_defaults_finalize (GObject *object)
{

	G_OBJECT_CLASS (dbusmenu_defaults_parent_class)->finalize (object);
	return;
}

static DbusmenuDefaults * default_defaults = NULL;

/**
 * dbusmenu_defaults_ref_default:
 *
 * Get a reference to the default instance.  If it doesn't exist this
 * function will create it.
 *
 * Return value: (transfer full): A reference to the defaults
 */
DbusmenuDefaults *
dbusmenu_defaults_ref_default (void)
{
	if (default_defaults == NULL) {
		default_defaults = DBUSMENU_DEFAULTS(g_object_new(DBUSMENU_TYPE_DEFAULTS, NULL));
	}

	return default_defaults;
}

/**
 * dbusmenu_defaults_default_set:
 * @default: The #DbusmenuDefaults object to add to
 * @type: (allow-none): The #DbusmenuMenuitem type for this default if #NULL will default to #DBUSMENU_CLIENT_TYPE_DEFAULT
 * @property: Property name for the default
 * @prop_type: (allow-none): Type of the property for runtime checking.  To disable checking set to #NULL.
 * @value: (allow-none): Default value for @property.  #NULL if by default it is unset, but you want type checking from @prop_type.
 *
 * Sets up an entry in the defaults database for a given @property
 * and menuitem type @type.  @prop_type and @value can both be #NULL
 * but both of them can not.
 */
void
dbusmenu_defaults_default_set (DbusmenuDefaults * defaults, const gchar * type, const gchar * property, const GVariantType * prop_type, GVariant * value)
{

	return;
}

/**
 * dbusmenu_defaults_default_get:
 * @defaults: The default database to use
 * @type: (allow-none): The #DbusmenuMenuitem type for this default if #NULL will default to #DBUSMENU_CLIENT_TYPE_DEFAULT
 * @property: Property name to lookup
 *
 * Gets an entry in the database for a give @property and @type.
 *
 * Return value: (transfer none): Returns a variant that does not
 * have it's ref count increased.  If you want to keep it, you should
 * do that.
 */
GVariant *
dbusmenu_defaults_default_get (DbusmenuDefaults * defaults, const gchar * type, const gchar * property)
{

	return NULL;
}

/**
 * dbusmenu_defaults_default_get_type:
 * @defaults: The default database to use
 * @type: (allow-none): The #DbusmenuMenuitem type for this default if #NULL will default to #DBUSMENU_CLIENT_TYPE_DEFAULT
 * @property: Property name to lookup
 *
 * Gets the type for an entry in the database for a give @property and @type.
 *
 * Return value: (transfer none): Returns a type for the given
 * @property value.
 */
GVariantType *
dbusmenu_defaults_default_get_type (DbusmenuDefaults * defaults, const gchar * type, const gchar * property)
{

	return NULL;
}

