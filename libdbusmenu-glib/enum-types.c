
/* Generated data (by glib-mkenums) */

/*
Enums from the dbusmenu headers

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

#include "enum-types.h"

#include "types.h"
/**
	dbusmenu_text_direction_get_type:

	Builds a GLib type for the #DbusmenuTextDirection enumeration.

	Return value: A unique #GType for the #DbusmenuTextDirection enum.
*/
GType
dbusmenu_text_direction_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ DBUSMENU_TEXT_DIRECTION_NONE,  "DBUSMENU_TEXT_DIRECTION_NONE", "none" },
			{ DBUSMENU_TEXT_DIRECTION_LTR,  "DBUSMENU_TEXT_DIRECTION_LTR", "ltr" },
			{ DBUSMENU_TEXT_DIRECTION_RTL,  "DBUSMENU_TEXT_DIRECTION_RTL", "rtl" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("DbusmenuTextDirection"), values);
	}

	return etype;
}

/**
	dbusmenu_text_direction_get_nick:
	@value: The value of DbusmenuTextDirection to get the nick of

	Looks up in the enum table for the nick of @value.

	Return value: The nick for the given value or #NULL on error
*/
const gchar *
dbusmenu_text_direction_get_nick (DbusmenuTextDirection value)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(dbusmenu_text_direction_get_type()));
	g_return_val_if_fail(class != NULL, NULL);

	const gchar * ret = NULL;
	GEnumValue * val = g_enum_get_value(class, value);
	if (val != NULL) {
		ret = val->value_nick;
	}

	g_type_class_unref(class);
	return ret;
}

/**
	dbusmenu_text_direction_get_value_from_nick:
	@nick: The enum nick to lookup

	Looks up in the enum table for the value of @nick.

	Return value: The value for the given @nick
*/
DbusmenuTextDirection
dbusmenu_text_direction_get_value_from_nick (const gchar * nick)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(dbusmenu_text_direction_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	DbusmenuTextDirection ret = 0;
	GEnumValue * val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL) {
		ret = val->value;
	}

	g_type_class_unref(class);
	return ret;
}


/**
	dbusmenu_status_get_type:

	Builds a GLib type for the #DbusmenuStatus enumeration.

	Return value: A unique #GType for the #DbusmenuStatus enum.
*/
GType
dbusmenu_status_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ DBUSMENU_STATUS_NORMAL,  "DBUSMENU_STATUS_NORMAL", "normal" },
			{ DBUSMENU_STATUS_NOTICE,  "DBUSMENU_STATUS_NOTICE", "notice" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("DbusmenuStatus"), values);
	}

	return etype;
}

/**
	dbusmenu_status_get_nick:
	@value: The value of DbusmenuStatus to get the nick of

	Looks up in the enum table for the nick of @value.

	Return value: The nick for the given value or #NULL on error
*/
const gchar *
dbusmenu_status_get_nick (DbusmenuStatus value)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(dbusmenu_status_get_type()));
	g_return_val_if_fail(class != NULL, NULL);

	const gchar * ret = NULL;
	GEnumValue * val = g_enum_get_value(class, value);
	if (val != NULL) {
		ret = val->value_nick;
	}

	g_type_class_unref(class);
	return ret;
}

/**
	dbusmenu_status_get_value_from_nick:
	@nick: The enum nick to lookup

	Looks up in the enum table for the value of @nick.

	Return value: The value for the given @nick
*/
DbusmenuStatus
dbusmenu_status_get_value_from_nick (const gchar * nick)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(dbusmenu_status_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	DbusmenuStatus ret = 0;
	GEnumValue * val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL) {
		ret = val->value;
	}

	g_type_class_unref(class);
	return ret;
}



/* Generated data ends here */

