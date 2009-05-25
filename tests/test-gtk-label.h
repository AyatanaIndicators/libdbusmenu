
#include <glib.h>

typedef struct _proplayout_t proplayout_t;
struct _proplayout_t {
	guint id;
	gchar ** properties;
	proplayout_t * submenu;
};

gchar * props1[] = {"label", "value1", NULL};
gchar * props2[] = {"label", "value1", NULL};
gchar * props3[] = {"label",
                    "And a property name that is really long should have a value that is really long, because well, that's an important part of the yin and yang of software testing.",
                    NULL};
gchar * props4[] = {"icon-name", "network-status", "label", "Look at network", "right-column", "10:32", NULL};


proplayout_t submenu_4_1[] = {
	{id: 10, properties: props2, submenu: NULL},
	{id: 11, properties: props2, submenu: NULL},
	{id: 12, properties: props2, submenu: NULL},
	{id: 13, properties: props2, submenu: NULL},
	{id: 14, properties: props2, submenu: NULL},
	{id: 15, properties: props2, submenu: NULL},
	{id: 16, properties: props2, submenu: NULL},
	{id: 17, properties: props2, submenu: NULL},
	{id: 18, properties: props2, submenu: NULL},
	{id: 19, properties: props2, submenu: NULL},
	{id: 0, properties: NULL, submenu: NULL}
};

proplayout_t submenu_4_2[] = {
	{id: 20, properties: props2, submenu: NULL},
	{id: 21, properties: props2, submenu: NULL},
	{id: 22, properties: props2, submenu: NULL},
	{id: 23, properties: props2, submenu: NULL},
	{id: 24, properties: props2, submenu: NULL},
	{id: 25, properties: props2, submenu: NULL},
	{id: 26, properties: props2, submenu: NULL},
	{id: 27, properties: props2, submenu: NULL},
	{id: 28, properties: props2, submenu: NULL},
	{id: 29, properties: props2, submenu: NULL},
	{id: 0, properties: NULL, submenu: NULL}
};

proplayout_t submenu_4_3[] = {
	{id: 30, properties: props2, submenu: NULL},
	{id: 31, properties: props2, submenu: NULL},
	{id: 32, properties: props2, submenu: NULL},
	{id: 33, properties: props2, submenu: NULL},
	{id: 34, properties: props2, submenu: NULL},
	{id: 35, properties: props2, submenu: NULL},
	{id: 36, properties: props2, submenu: NULL},
	{id: 37, properties: props2, submenu: NULL},
	{id: 38, properties: props2, submenu: NULL},
	{id: 39, properties: props2, submenu: NULL},
	{id: 0, properties: NULL, submenu: NULL}
};

proplayout_t submenu_4_0[] = {
	{id: 1, properties: props2, submenu: submenu_4_1},
	{id: 2, properties: props2, submenu: submenu_4_2},
	{id: 3, properties: props2, submenu: submenu_4_3},
	{id: 0, properties: NULL, submenu: NULL}
};

proplayout_t layouts[] = {
	{id: 1, properties: props1, submenu: submenu_4_3},
	{id: 10, properties: props2, submenu: submenu_4_2},
	{id: 20, properties: props3, submenu: submenu_4_1},
	{id: 100, properties: props2, submenu: submenu_4_0},
	{id: 0, properties: NULL, submenu: NULL}
};

