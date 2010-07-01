/*
A loader to turn JSON into dbusmenu menuitems

Copyright 2010 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "json-loader.h"
#include <dbus/dbus-gtype-specialized.h>

static GValue *
node2value (JsonNode * node)
{
	if (node == NULL) {
		return NULL;
	}

	GValue * value = g_new0(GValue, 1);

	if (JSON_NODE_TYPE(node) == JSON_NODE_VALUE) {
		json_node_get_value(node, value);
		return value;
	}

	if (JSON_NODE_TYPE(node) == JSON_NODE_ARRAY) {
		JsonArray * array = json_node_get_array(node);
		JsonNode * first = json_array_get_element(array, 0);

		if (JSON_NODE_TYPE(first) == JSON_NODE_VALUE) {
			GValue subvalue = {0};
			json_node_get_value(first, &subvalue);

			if (G_VALUE_TYPE(&subvalue) == G_TYPE_STRING) {
				GArray * garray = g_array_sized_new(TRUE, TRUE, sizeof(gchar *), json_array_get_length(array));
				g_value_init(value, G_TYPE_STRV);
				g_value_take_boxed(value, garray->data);

				int i;
				for (i = 0; i < json_array_get_length(array); i++) {
					const gchar * str = json_node_get_string(json_array_get_element(array, i));
					gchar * dupstr = g_strdup(str);
					g_array_append_val(garray, dupstr);
				}

				g_array_free(garray, FALSE);
			} else {
				GValueArray * varray = g_value_array_new(json_array_get_length(array));
				g_value_init(value, G_TYPE_VALUE_ARRAY);
				g_value_take_boxed(value, varray);

				g_value_array_append(varray, &subvalue);
				g_value_unset(&subvalue);

				int i;
				for (i = 1; i < json_array_get_length(array); i++) {
					json_node_get_value(first, &subvalue);
					g_value_array_append(varray, &subvalue);
					g_value_unset(&subvalue);
				}
			}

		} else {
			GValue * subvalue = node2value(first);
			GType type = dbus_g_type_get_collection("GPtrArray", G_VALUE_TYPE(subvalue));
			gpointer * wrapper = dbus_g_type_specialized_construct(type);

			g_value_init(value, type);
			g_value_take_boxed(value, wrapper);

			DBusGTypeSpecializedAppendContext ctx;
			dbus_g_type_specialized_init_append(value, &ctx);

			dbus_g_type_specialized_collection_append(&ctx, subvalue);
			int i;
			for (i = 1; i < json_array_get_length(array); i++) {
				GValue * subvalue = node2value(node);
				dbus_g_type_specialized_collection_append(&ctx, subvalue);
			}

			dbus_g_type_specialized_collection_end_append(&ctx);
		}
	}

	if (JSON_NODE_TYPE(node) == JSON_NODE_OBJECT) {
		JsonObject * obj = json_node_get_object(node);

		GType type = dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE);
		GHashTable * hash = (GHashTable *)dbus_g_type_specialized_construct(type);

		g_value_init(value, type);
		g_value_take_boxed(value, hash);

		DBusGTypeSpecializedAppendContext ctx;
		dbus_g_type_specialized_init_append(value, &ctx);
		
		GList * members = NULL;
		for (members = json_object_get_members(obj); members != NULL; members = g_list_next(members)) {
			const gchar * member = members->data;

			JsonNode * lnode = json_object_get_member(obj, member);
			GValue * value = node2value(lnode);

			if (value != NULL) {
				GValue name = {0};
				g_value_init(&name, G_TYPE_STRING);
				g_value_set_static_string(&name, member);

				dbus_g_type_specialized_map_append(&ctx, &name, value);
			
				g_value_unset(&name);
				g_value_unset(value);
				g_free(value);
			}
		}
	}

	return value;
}

static void
set_props (DbusmenuMenuitem * mi, JsonObject * node)
{
	if (node == NULL) return;

	GList * members = NULL;
	for (members = json_object_get_members(node); members != NULL; members = g_list_next(members)) {
		const gchar * member = members->data;

		if (!g_strcmp0(member, "id")) { continue; }
		if (!g_strcmp0(member, "submenu")) { continue; }

		JsonNode * lnode = json_object_get_member(node, member);
		GValue * value = node2value(lnode);

		if (value != NULL) {
			dbusmenu_menuitem_property_set_value(mi, member, value);
			g_value_unset(value);
			g_free(value);
		}
	}

	return;
}

DbusmenuMenuitem *
dbusmenu_json_build_from_node (const JsonNode * cnode)
{
	JsonNode * node = (JsonNode *)cnode; /* To match the jsonglib API :( */

	if (node == NULL) return NULL;
	if (JSON_NODE_TYPE(node) != JSON_NODE_OBJECT) return NULL;

	JsonObject * layout = json_node_get_object(node);

	DbusmenuMenuitem * local = NULL;
	if (json_object_has_member(layout, "id")) {
		JsonNode * node = json_object_get_member(layout, "id");
		g_return_val_if_fail(JSON_NODE_TYPE(node) == JSON_NODE_VALUE, NULL);
		local = dbusmenu_menuitem_new_with_id(json_node_get_int(node));
	} else {
		local = dbusmenu_menuitem_new();
	}

	set_props(local, layout);
	
	if (json_object_has_member(layout, "submenu")) {
		JsonNode * node = json_object_get_member(layout, "submenu");
		g_return_val_if_fail(JSON_NODE_TYPE(node) == JSON_NODE_ARRAY, local);
		JsonArray * array = json_node_get_array(node);
		guint count;
		for (count = 0; count < json_array_get_length(array); count++) {
			DbusmenuMenuitem * child = dbusmenu_json_build_from_node(json_array_get_element(array, count));
			if (child != NULL) {
				dbusmenu_menuitem_child_append(local, child);
			}
		}
	}

	/* g_debug("Layout to menu return: 0x%X", (unsigned int)local); */
	return local;
}

DbusmenuMenuitem *
dbusmenu_json_build_from_file (const gchar * filename)
{
	JsonParser * parser = json_parser_new();

	GError * error = NULL;
	if (!json_parser_load_from_file(parser, filename, &error)) {
		g_warning("Failed parsing file %s because: %s", filename, error->message);
		g_error_free(error);
		return NULL;
	}

	JsonNode * root_node = json_parser_get_root(parser);
	if (JSON_NODE_TYPE(root_node) != JSON_NODE_OBJECT) {
		g_warning("Root node is not an object, fail.  It's an: %s", json_node_type_name(root_node));
		return NULL;
	}

	DbusmenuMenuitem * mi = dbusmenu_json_build_from_node(root_node);

	g_object_unref(parser);

	return mi;
}
