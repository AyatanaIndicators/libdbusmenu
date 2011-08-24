
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

#include "genericmenuitem-enum-types.h"

#include "genericmenuitem.h"
/**
	genericmenuitem_check_type_get_type:

	Builds a GLib type for the #GenericmenuitemCheckType enumeration.

	Return value: A unique #GType for the #GenericmenuitemCheckType enum.
*/
GType
genericmenuitem_check_type_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ GENERICMENUITEM_CHECK_TYPE_NONE,  "GENERICMENUITEM_CHECK_TYPE_NONE", "none" },
			{ GENERICMENUITEM_CHECK_TYPE_CHECKBOX,  "GENERICMENUITEM_CHECK_TYPE_CHECKBOX", "checkbox" },
			{ GENERICMENUITEM_CHECK_TYPE_RADIO,  "GENERICMENUITEM_CHECK_TYPE_RADIO", "radio" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("GenericmenuitemCheckType"), values);
	}

	return etype;
}

/**
	genericmenuitem_check_type_get_nick:
	@value: The value of GenericmenuitemCheckType to get the nick of

	Looks up in the enum table for the nick of @value.

	Return value: The nick for the given value or #NULL on error
*/
const gchar *
genericmenuitem_check_type_get_nick (GenericmenuitemCheckType value)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(genericmenuitem_check_type_get_type()));
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
	genericmenuitem_check_type_get_value_from_nick:
	@nick: The enum nick to lookup

	Looks up in the enum table for the value of @nick.

	Return value: The value for the given @nick
*/
GenericmenuitemCheckType
genericmenuitem_check_type_get_value_from_nick (const gchar * nick)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(genericmenuitem_check_type_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	GenericmenuitemCheckType ret = 0;
	GEnumValue * val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL) {
		ret = val->value;
	}

	g_type_class_unref(class);
	return ret;
}


/**
	genericmenuitem_state_get_type:

	Builds a GLib type for the #GenericmenuitemState enumeration.

	Return value: A unique #GType for the #GenericmenuitemState enum.
*/
GType
genericmenuitem_state_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ GENERICMENUITEM_STATE_UNCHECKED,  "GENERICMENUITEM_STATE_UNCHECKED", "unchecked" },
			{ GENERICMENUITEM_STATE_CHECKED,  "GENERICMENUITEM_STATE_CHECKED", "checked" },
			{ GENERICMENUITEM_STATE_INDETERMINATE,  "GENERICMENUITEM_STATE_INDETERMINATE", "indeterminate" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("GenericmenuitemState"), values);
	}

	return etype;
}

/**
	genericmenuitem_state_get_nick:
	@value: The value of GenericmenuitemState to get the nick of

	Looks up in the enum table for the nick of @value.

	Return value: The nick for the given value or #NULL on error
*/
const gchar *
genericmenuitem_state_get_nick (GenericmenuitemState value)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(genericmenuitem_state_get_type()));
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
	genericmenuitem_state_get_value_from_nick:
	@nick: The enum nick to lookup

	Looks up in the enum table for the value of @nick.

	Return value: The value for the given @nick
*/
GenericmenuitemState
genericmenuitem_state_get_value_from_nick (const gchar * nick)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(genericmenuitem_state_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	GenericmenuitemState ret = 0;
	GEnumValue * val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL) {
		ret = val->value;
	}

	g_type_class_unref(class);
	return ret;
}


/**
	genericmenuitem_disposition_get_type:

	Builds a GLib type for the #GenericmenuitemDisposition enumeration.

	Return value: A unique #GType for the #GenericmenuitemDisposition enum.
*/
GType
genericmenuitem_disposition_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ GENERICMENUITEM_DISPOSITION_NORMAL,  "GENERICMENUITEM_DISPOSITION_NORMAL", "normal" },
			{ GENERICMENUITEM_DISPOSITION_INFORMATIONAL,  "GENERICMENUITEM_DISPOSITION_INFORMATIONAL", "informational" },
			{ GENERICMENUITEM_DISPOSITION_WARNING,  "GENERICMENUITEM_DISPOSITION_WARNING", "warning" },
			{ GENERICMENUITEM_DISPOSITION_ALERT,  "GENERICMENUITEM_DISPOSITION_ALERT", "alert" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("GenericmenuitemDisposition"), values);
	}

	return etype;
}

/**
	genericmenuitem_disposition_get_nick:
	@value: The value of GenericmenuitemDisposition to get the nick of

	Looks up in the enum table for the nick of @value.

	Return value: The nick for the given value or #NULL on error
*/
const gchar *
genericmenuitem_disposition_get_nick (GenericmenuitemDisposition value)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(genericmenuitem_disposition_get_type()));
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
	genericmenuitem_disposition_get_value_from_nick:
	@nick: The enum nick to lookup

	Looks up in the enum table for the value of @nick.

	Return value: The value for the given @nick
*/
GenericmenuitemDisposition
genericmenuitem_disposition_get_value_from_nick (const gchar * nick)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(genericmenuitem_disposition_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	GenericmenuitemDisposition ret = 0;
	GEnumValue * val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL) {
		ret = val->value;
	}

	g_type_class_unref(class);
	return ret;
}



/* Generated data ends here */

