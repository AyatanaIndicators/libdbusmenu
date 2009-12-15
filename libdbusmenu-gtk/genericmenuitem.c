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

	switch (item->priv->check_type) {
	case GENERICMENUITEM_CHECK_TYPE_NONE:
		break;
	case GENERICMENUITEM_CHECK_TYPE_CHECKBOX:
		break;
	case GENERICMENUITEM_CHECK_TYPE_RADIO:
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

	switch (item->priv->state) {
	case GENERICMENUITEM_STATE_UNCHECKED:
		break;
	case GENERICMENUITEM_STATE_CHECKED:
		break;
	case GENERICMENUITEM_STATE_INDETERMINATE:
		break;
	default:
		g_warning("Generic Menuitem invalid check state: %d", state);
		return;
	}

	gtk_widget_queue_draw(GTK_WIDGET(item));

	return;
}
