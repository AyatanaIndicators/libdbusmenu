
typedef struct _layout_t layout_t;
struct _layout_t {
	guint id;
	layout_t * submenu;
};

layout_t submenu_2[] = {
	{id: 2, submenu: NULL},
	{id: 3, submenu: NULL},
	{id: 0, submenu: NULL}
};
layout_t submenu_3_1[] = {
	{id: 3, submenu: NULL},
	{id: 4, submenu: NULL},
	{id: 5, submenu: NULL},
	{id: 0, submenu: NULL}
};
layout_t submenu_3_2[] = {
	{id: 7, submenu: NULL},
	{id: 8, submenu: NULL},
	{id: 9, submenu: NULL},
	{id: 0, submenu: NULL}
};
layout_t submenu_3[] = {
	{id: 2, submenu: submenu_3_1},
	{id: 6, submenu: submenu_3_2},
	{id: 0, submenu: NULL}
};
layout_t submenu_4_1[] = {
	{id: 6, submenu: NULL},
	{id: 0, submenu: NULL}
};
layout_t submenu_4_2[] = {
	{id: 5, submenu: submenu_4_1},
	{id: 0, submenu: NULL}
};
layout_t submenu_4_3[] = {
	{id: 4, submenu: submenu_4_2},
	{id: 0, submenu: NULL}
};
layout_t submenu_4_4[] = {
	{id: 3, submenu: submenu_4_3},
	{id: 0, submenu: NULL}
};
layout_t submenu_4_5[] = {
	{id: 2, submenu: submenu_4_4},
	{id: 0, submenu: NULL}
};

layout_t layouts[] = {
	{id: 5, submenu: NULL},
	{id: 1, submenu: submenu_2},
	{id: 1, submenu: submenu_3},
	{id: 1, submenu: submenu_4_5},
	{id: 0, submenu: NULL}
};

