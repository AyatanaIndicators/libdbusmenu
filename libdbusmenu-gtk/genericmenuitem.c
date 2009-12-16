#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "genericmenuitem.h"

/**
	GenericmenuitemPrivate:
	@check_type: What type of check we have, or none at all.
	@state: What the state of our check is.
*/
struct _GenericmenuitemPrivate {
	GenericmenuitemCheckType   check_type;
	GenericmenuitemState       state;
};

/* Private macro */
#define GENERICMENUITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), GENERICMENUITEM_TYPE, GenericmenuitemPrivate))

/* Prototypes */
static void genericmenuitem_class_init (GenericmenuitemClass *klass);
static void genericmenuitem_init       (Genericmenuitem *self);
static void genericmenuitem_dispose    (GObject *object);
static void genericmenuitem_finalize   (GObject *object);
static void draw_indicator (GtkCheckMenuItem *check_menu_item, GdkRectangle *area);
static void set_label (GtkMenuItem * menu_item, const gchar * label);
static const gchar * get_label (GtkMenuItem * menu_item);

/* GObject stuff */
G_DEFINE_TYPE (Genericmenuitem, genericmenuitem, GTK_TYPE_CHECK_MENU_ITEM);

/* Globals */
static void (*parent_draw_indicator) (GtkCheckMenuItem *check_menu_item, GdkRectangle *area) = NULL;

/* Initializing all of the classes.  Most notably we're
   disabling the drawing of the check early. */
static void
genericmenuitem_class_init (GenericmenuitemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GenericmenuitemPrivate));

	object_class->dispose = genericmenuitem_dispose;
	object_class->finalize = genericmenuitem_finalize;

	GtkCheckMenuItemClass * check_class = GTK_CHECK_MENU_ITEM_CLASS (klass);

	parent_draw_indicator = check_class->draw_indicator;
	check_class->draw_indicator = draw_indicator;

	GtkMenuItemClass * menuitem_class = GTK_MENU_ITEM_CLASS (klass);
	menuitem_class->set_label = set_label;
	menuitem_class->get_label = get_label;

	return;
}

/* Sets default values for all the class variables.  Mostly,
   this puts us in a default state. */
static void
genericmenuitem_init (Genericmenuitem *self)
{
	self->priv = GENERICMENUITEM_GET_PRIVATE(self);

	self->priv->check_type = GENERICMENUITEM_CHECK_TYPE_NONE;
	self->priv->state = GENERICMENUITEM_STATE_UNCHECKED;

	return;
}

/* Clean everything up.  Whew, that can be work. */
static void
genericmenuitem_dispose (GObject *object)
{

	G_OBJECT_CLASS (genericmenuitem_parent_class)->dispose (object);
	return;
}

/* Now free memory, we no longer need it. */
static void
genericmenuitem_finalize (GObject *object)
{

	G_OBJECT_CLASS (genericmenuitem_parent_class)->finalize (object);
	return;
}

/* Checks to see if we should be drawing a little box at
   all.  If we should be, let's do that, otherwise we're
   going suppress the box drawing. */
static void
draw_indicator (GtkCheckMenuItem *check_menu_item, GdkRectangle *area)
{
	Genericmenuitem * self = GENERICMENUITEM(check_menu_item);
	if (self->priv->check_type != GENERICMENUITEM_CHECK_TYPE_NONE) {
		parent_draw_indicator(check_menu_item, area);
	}
	return;
}

/* A small helper to look through the widgets in the
   box and find the one that is the label. */
static void
set_label_helper (GtkWidget * widget, gpointer data)
{
	GtkWidget ** labelval = (GtkWidget **)data;
	if (GTK_IS_LABEL(widget)) {
		*labelval = widget;
	}
	return;
}

/* Set the label on the item */
static void
set_label (GtkMenuItem * menu_item, const gchar * label)
{
	GtkWidget * child = gtk_bin_get_child(GTK_BIN(menu_item));

	if (child == NULL) {
		GtkWidget * labelw = gtk_label_new(label);
		gtk_label_set_use_underline(GTK_LABEL(labelw), TRUE);
		gtk_container_add(GTK_CONTAINER(menu_item), labelw);
	} else if (GTK_IS_LABEL(child)) {
		gtk_label_set_label(GTK_LABEL(child), label);
	} else if (GTK_IS_BOX(child)) {
		GtkWidget * labelw = NULL;
		/* Look for the label */
		gtk_container_foreach(GTK_CONTAINER(child), set_label_helper, &labelw);
		
		if (labelw == NULL) {
			/* We don't have a label, so we need to build */
			labelw = gtk_label_new(label);
			gtk_label_set_use_underline(GTK_LABEL(labelw), TRUE);
			gtk_box_pack_end(GTK_BOX(child), labelw, TRUE, TRUE, 0);
		} else {
			/* We can reset the label that we have. */
			gtk_label_set_label(GTK_LABEL(labelw), label);
		}
	} else {
		g_error("Generic item in an indeterminate state.");
		return;
	}

	g_object_notify(G_OBJECT(menu_item), "label");

	return;
}

/* Get the text of the label for the item */
static const gchar *
get_label (GtkMenuItem * menu_item)
{


	return NULL;
}

/**
	genericmenuitem_set_check_type:
	@item: #Genericmenuitem to set the type on
	@check_type: Which type of check should be displayed

	This function changes the type of the checkmark that
	appears in the left hand gutter for the menuitem.
*/
void
genericmenuitem_set_check_type (Genericmenuitem * item, GenericmenuitemCheckType check_type)
{
	if (item->priv->check_type == check_type) {
		return;
	}

	item->priv->check_type = check_type;
	GValue value = {0};

	switch (item->priv->check_type) {
	case GENERICMENUITEM_CHECK_TYPE_NONE:
		/* We don't need to do anything here as we're queuing the
		   draw and then when it draws it'll avoid drawing the
		   check on the item. */
		break;
	case GENERICMENUITEM_CHECK_TYPE_CHECKBOX:
		g_value_init(&value, G_TYPE_BOOLEAN);
		g_value_set_boolean(&value, FALSE);
		g_object_set_property(G_OBJECT(item), "draw-as-radio", &value);
		break;
	case GENERICMENUITEM_CHECK_TYPE_RADIO:
		g_value_init(&value, G_TYPE_BOOLEAN);
		g_value_set_boolean(&value, TRUE);
		g_object_set_property(G_OBJECT(item), "draw-as-radio", &value);
		break;
	default:
		g_warning("Generic Menuitem invalid check type: %d", check_type);
		return;
	}

	gtk_widget_queue_draw(GTK_WIDGET(item));

	return;
}

/**
	genericmenuitem_set_state:
	@item: #Genericmenuitem to set the type on
	@check_type: What is the state of the check 

	Sets the state of the check in the menu item.  It does
	not require, but isn't really useful if the type of
	check that the menuitem is set to #GENERICMENUITEM_CHECK_TYPE_NONE.
*/
void
genericmenuitem_set_state (Genericmenuitem * item, GenericmenuitemState state)
{
	if (item->priv->state == state) {
		return;
	}

	item->priv->state = state;
	GValue value = {0};
	g_value_init(&value, G_TYPE_BOOLEAN);

	switch (item->priv->state) {
	case GENERICMENUITEM_STATE_UNCHECKED:
		g_value_set_boolean(&value, FALSE);
		g_object_set_property(G_OBJECT(item), "active", &value);
		g_value_set_boolean(&value, FALSE);
		g_object_set_property(G_OBJECT(item), "inconsistent", &value);
		break;
	case GENERICMENUITEM_STATE_CHECKED:
		g_value_set_boolean(&value, TRUE);
		g_object_set_property(G_OBJECT(item), "active", &value);
		g_value_set_boolean(&value, FALSE);
		g_object_set_property(G_OBJECT(item), "inconsistent", &value);
		break;
	case GENERICMENUITEM_STATE_INDETERMINATE:
		g_value_set_boolean(&value, TRUE);
		g_object_set_property(G_OBJECT(item), "active", &value);
		g_value_set_boolean(&value, TRUE);
		g_object_set_property(G_OBJECT(item), "inconsistent", &value);
		break;
	default:
		g_warning("Generic Menuitem invalid check state: %d", state);
		return;
	}

	gtk_widget_queue_draw(GTK_WIDGET(item));

	return;
}

/**
	genericmenuitem_set_image:
	@item: A #Genericmenuitem
	@image: The image to set as the image of @item

	Sets the image of the menu item.
*/
void
genericmenuitem_set_image (Genericmenuitem * item, GtkWidget * image)
{


	return;
}

/**
	genericmenuitem_get_image:
	@item: A #Genericmenuitem

	Returns the image if there is one.

	Return value: A pointer to the image of the item or #NULL
		if there isn't one.
*/
GtkWidget *
genericmenuitem_get_image (Genericmenuitem * item)
{


	return NULL;
}
