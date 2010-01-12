/*
A menuitem subclass that has the ability to do lots of different
things depending on it's settings.

Copyright 2009 Canonical Ltd.

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
static void activate (GtkMenuItem * menu_item);

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
	menuitem_class->activate = activate;

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

/* A quick little function to grab the padding from the
   style.  It should be considered for caching when
   optimizing. */
static gint
get_hpadding (GtkWidget * widget)
{
	gint padding = 0;
	gtk_widget_style_get(widget, "horizontal-padding", &padding, NULL);
	return padding;
}

/* Set the label on the item */
static void
set_label (GtkMenuItem * menu_item, const gchar * label)
{
	GtkWidget * child = gtk_bin_get_child(GTK_BIN(menu_item));
	GtkLabel * labelw = NULL;
	gboolean suppress_update = FALSE;

	/* Try to find if we have a label already */
	if (child != NULL) {
		if (GTK_IS_LABEL(child)) {
			/* We've got a label, let's update it. */
			labelw = GTK_LABEL(child);
		} else if (GTK_IS_BOX(child)) {
			/* Look for the label in the box */
			gtk_container_foreach(GTK_CONTAINER(child), set_label_helper, &labelw);
		} else {
			/* We need to put the child into a new box and
			   make the box the child of the menu item.  Basically
			   we're inserting a box in the middle. */
			GtkWidget * hbox = gtk_hbox_new(FALSE, 0);
			g_object_ref(child);
			gtk_container_remove(GTK_CONTAINER(menu_item), child);
			gtk_box_pack_start(GTK_BOX(hbox), child, FALSE, FALSE, get_hpadding(GTK_WIDGET(menu_item)));
			gtk_container_add(GTK_CONTAINER(menu_item), hbox);
			gtk_widget_show(hbox);
			g_object_unref(child);
			child = hbox;
			/* It's important to notice that labelw is not set
			   by this condition.  There was no label to find. */
		}
	}

	/* No we can see if we need to ethier build a label or just
	   update the one that we already have. */
	if (labelw == NULL) {
		/* Build it */
		labelw = GTK_LABEL(gtk_label_new(label));
		gtk_label_set_use_underline(GTK_LABEL(labelw), TRUE);
		gtk_misc_set_alignment(GTK_MISC(labelw), 0.0, 0.5);
		gtk_widget_show(GTK_WIDGET(labelw));

		/* Check to see if it needs to be in the bin for this
		   menu item or whether it gets packed in a box. */
		if (child == NULL) {
			gtk_container_add(GTK_CONTAINER(menu_item), GTK_WIDGET(labelw));
		} else {
			gtk_box_pack_end(GTK_BOX(child), GTK_WIDGET(labelw), TRUE, TRUE, get_hpadding(GTK_WIDGET(menu_item)));
		}
	} else {
		/* Oh, just an update.  No biggie. */
		if (!g_strcmp0(label, gtk_label_get_label(labelw))) {
			/* The only reason to suppress the update is if we had
			   a label and the value was the same as the one we're
			   getting in. */
			suppress_update = TRUE;
		} else {
			gtk_label_set_label(labelw, label);
		}
	}

	/* If we changed the value, tell folks. */
	if (!suppress_update) {
		g_object_notify(G_OBJECT(menu_item), "label");
	}

	return;
}

/* Get the text of the label for the item */
static const gchar *
get_label (GtkMenuItem * menu_item)
{
	GtkWidget * child = gtk_bin_get_child(GTK_BIN(menu_item));
	GtkLabel * labelw = NULL;

	/* Try to find if we have a label already */
	if (child != NULL) {
		if (GTK_IS_LABEL(child)) {
			/* We've got a label, let's update it. */
			labelw = GTK_LABEL(child);
		} else if (GTK_IS_BOX(child)) {
			/* Look for the label in the box */
			gtk_container_foreach(GTK_CONTAINER(child), set_label_helper, &labelw);
		}
	}

	if (labelw != NULL) {
		return gtk_label_get_label(labelw);
	}

	return NULL;
}

/* Make sure we don't toggle when there is an
   activate like a normal check menu item. */
static void
activate (GtkMenuItem * menu_item)
{
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

	GtkCheckMenuItem * check = GTK_CHECK_MENU_ITEM(item);

	gboolean old_active = check->active;
	gboolean old_inconsist = check->inconsistent;

	switch (item->priv->state) {
	case GENERICMENUITEM_STATE_UNCHECKED:
		check->active = FALSE;
		check->inconsistent = FALSE;
		break;
	case GENERICMENUITEM_STATE_CHECKED:
		check->active = TRUE;
		check->inconsistent = FALSE;
		break;
	case GENERICMENUITEM_STATE_INDETERMINATE:
		check->active = TRUE;
		check->inconsistent = TRUE;
		break;
	default:
		g_warning("Generic Menuitem invalid check state: %d", state);
		return;
	}

	if (old_active != check->active) {
		g_object_notify(G_OBJECT(item), "active");
	}

	if (old_inconsist != check->inconsistent) {
		g_object_notify(G_OBJECT(item), "inconsistent");
	}

	gtk_widget_queue_draw(GTK_WIDGET(item));

	return;
}

/* A small helper to look through the widgets in the
   box and find the one that is the image. */
static void
set_image_helper (GtkWidget * widget, gpointer data)
{
	GtkWidget ** labelval = (GtkWidget **)data;
	if (GTK_IS_IMAGE(widget)) {
		*labelval = widget;
	}
	return;
}

/**
	genericmenuitem_set_image:
	@item: A #Genericmenuitem
	@image: The image to set as the image of @item

	Sets the image of the menu item.
*/
void
genericmenuitem_set_image (Genericmenuitem * menu_item, GtkWidget * image)
{
	GtkWidget * child = gtk_bin_get_child(GTK_BIN(menu_item));
	GtkImage * imagew = NULL;

	/* Try to find if we have a label already */
	if (child != NULL) {
		if (GTK_IS_IMAGE(child)) {
			/* We've got a label, let's update it. */
			imagew = GTK_IMAGE(child);
		} else if (GTK_IS_BOX(child)) {
			/* Look for the label in the box */
			gtk_container_foreach(GTK_CONTAINER(child), set_image_helper, &imagew);
		} else if (image != NULL) {
			/* We need to put the child into a new box and
			   make the box the child of the menu item.  Basically
			   we're inserting a box in the middle. */
			GtkWidget * hbox = gtk_hbox_new(FALSE, 0);
			g_object_ref(child);
			gtk_container_remove(GTK_CONTAINER(menu_item), child);
			gtk_box_pack_end(GTK_BOX(hbox), child, TRUE, TRUE, get_hpadding(GTK_WIDGET(menu_item)));
			gtk_container_add(GTK_CONTAINER(menu_item), hbox);
			gtk_widget_show(hbox);
			g_object_unref(child);
			child = hbox;
			/* It's important to notice that imagew is not set
			   by this condition.  There was no label to find. */
		}
	}

	/* No we can see if we need to ethier replace and image or
	   just put ourselves into the structures */
	if (imagew != NULL) {
		gtk_widget_destroy(GTK_WIDGET(imagew));
	}

	/* Check to see if it needs to be in the bin for this
	   menu item or whether it gets packed in a box. */
	if (image != NULL) {
		if (child == NULL) {
			gtk_container_add(GTK_CONTAINER(menu_item), GTK_WIDGET(image));
		} else {
			gtk_box_pack_start(GTK_BOX(child), GTK_WIDGET(image), FALSE, FALSE, get_hpadding(GTK_WIDGET(menu_item)));
		}

		gtk_widget_show(image);
	}

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
genericmenuitem_get_image (Genericmenuitem * menu_item)
{
	GtkWidget * child = gtk_bin_get_child(GTK_BIN(menu_item));
	GtkWidget * imagew = NULL;

	/* Try to find if we have a label already */
	if (child != NULL) {
		if (GTK_IS_IMAGE(child)) {
			/* We've got a label, let's update it. */
			imagew = child;
		} else if (GTK_IS_BOX(child)) {
			/* Look for the label in the box */
			gtk_container_foreach(GTK_CONTAINER(child), set_image_helper, &imagew);
		}
	}

	return imagew;
}
