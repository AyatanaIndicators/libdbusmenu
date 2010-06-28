
#ifndef __DBUSMENU_JSON_LOADER_H__
#define __DBUSMENU_JSON_LOADER_H__

#include <libdbusmenu-glib/menuitem.h>
#include <json-glib/json-glib.h>

DbusmenuMenuitem *  dbusmenu_json_build_from_node (const JsonNode * node);
DbusmenuMenuitem *  dbusmenu_json_build_from_file (const gchar * filename);

#endif /* __DBUSMENU_JSON_LOADER_H__ */
