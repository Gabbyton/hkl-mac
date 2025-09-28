/* This file is part of the hkl library.
 *
 * The hkl library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The hkl library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the hkl library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2003-2019, 2022, 2024, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "hkl.h"
#include "hkl-gui.h"
#include "hkl-gui-diffractometer-private.h"
#include "hkl-gui-macros.h"

enum {
	PROP_0,

	PROP_PARAMETER,
	PROP_VALUE,
	PROP_MINIMUM,
	PROP_MAXIMUM,

	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiParameter HklGuiParameter;

struct _HklGuiParameter {
	GObject parent_instance;

	/* instance members */
	HklParameter *parameter;
};

G_DEFINE_FINAL_TYPE(HklGuiParameter, hkl_gui_parameter, G_TYPE_OBJECT);

static void
hkl_gui_parameter_set_property (GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	HklGuiParameter *self = HKL_GUI_PARAMETER (object);


	switch (prop_id)
	{
	case PROP_PARAMETER:
	{
		HklParameter *new_parameter = g_value_get_pointer (value);
		self->parameter = new_parameter;
		g_object_notify_by_pspec (object, pspec);
	}
	break;
	case PROP_VALUE:
		hkl_gui_parameter_set_value(self, g_value_get_double (value));
		break;
	case PROP_MINIMUM:
		hkl_gui_parameter_set_minimum(self, g_value_get_double (value));
		break;
	case PROP_MAXIMUM:
		hkl_gui_parameter_set_maximum(self, g_value_get_double (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;

	}
}

static void
hkl_gui_parameter_get_property (GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	HklGuiParameter *self = HKL_GUI_PARAMETER (object);

	switch (prop_id)
	{
	case PROP_PARAMETER:
		g_value_set_pointer (value, self->parameter);
		break;
	case PROP_VALUE:
		g_value_set_double (value, hkl_gui_parameter_get_value(self));
		break;
	case PROP_MINIMUM:
		g_value_set_double (value, hkl_gui_parameter_get_minimum(self));
		break;
	case PROP_MAXIMUM:
		g_value_set_double (value, hkl_gui_parameter_get_maximum(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_parameter_init(HklGuiParameter *parameter)
{
	parameter->parameter = NULL;
}

static void
hkl_gui_parameter_class_init (HklGuiParameterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = hkl_gui_parameter_set_property;
	object_class->get_property = hkl_gui_parameter_get_property;

	props[PROP_PARAMETER] =
		g_param_spec_pointer ("parameter",
				      "Parameter",
				      "the embeded HklParameter.",
				      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	props[PROP_MINIMUM] =
		g_param_spec_double ("minimum",
				     "Minimum",
				     "the parameter minimum value",
				     -361, 361, 0,
				     G_PARAM_READWRITE);

	props[PROP_MAXIMUM] =
		g_param_spec_double ("maximum",
				     "Maximum",
				     "the parameter maximum value",
				     -361, 361, 0,
				     G_PARAM_READWRITE);

	props[PROP_VALUE] =
		g_param_spec_double ("value",
				     "Value",
				     "the parameter value",
				     -361, 361, 0,
				     G_PARAM_READWRITE);

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}

HklGuiParameter *hkl_gui_parameter_new(const HklParameter *parameter)
{
	return g_object_new (HKL_GUI_TYPE_PARAMETER,
			     "parameter", parameter,
			     NULL);
}

/* getter */

gdouble hkl_gui_parameter_get_maximum(HklGuiParameter *self)
{
	gdouble min;
	gdouble max;
	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
	return max;
}

gdouble hkl_gui_parameter_get_minimum(HklGuiParameter *self)
{
	gdouble min;
	gdouble max;
	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
	return min;
}

gdouble hkl_gui_parameter_get_value(HklGuiParameter *self)
{
	return hkl_parameter_value_get(self->parameter, HKL_UNIT_USER);
}

/* setters */

void hkl_gui_parameter_set_value(HklGuiParameter *self, gdouble value)
{
	if (TRUE == hkl_parameter_value_set(self->parameter, value, HKL_UNIT_USER, NULL)){
		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
	}
}

void hkl_gui_parameter_set_minimum(HklGuiParameter *self, gdouble value)
{
	gdouble min;
	gdouble max;

	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
	if (TRUE == hkl_parameter_min_max_set(self->parameter, value, max, HKL_UNIT_USER, NULL)){
		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MINIMUM]);
	}
}

void hkl_gui_parameter_set_maximum(HklGuiParameter *self, gdouble value)
{
	gdouble min;
	gdouble max;

	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
	if (TRUE == hkl_parameter_min_max_set(self->parameter, min, value, HKL_UNIT_USER, NULL)){
		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM]);
	}
}

void hkl_gui_parameter_update(HklGuiParameter *self)
{
	g_return_if_fail (HKL_GUI_IS_PARAMETER (self));

	gdouble value = hkl_gui_parameter_get_value(self);
	gdouble minimum = hkl_gui_parameter_get_minimum(self);
	gdouble maximum = hkl_gui_parameter_get_maximum(self);

	hkl_gui_parameter_set_minimum(self, minimum);
	hkl_gui_parameter_set_maximum(self, maximum);
	hkl_gui_parameter_set_value(self, value);
}


/**********************/
/* GtkListItemFactory */
/**********************/

void
hkl_gui_parameter_bind_factory_label_name_cb (GtkListItemFactory *factory,
					      GtkListItem *list_item)
{
	GtkWidget *label;
	HklGuiParameter *self;


	label = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_assert(NULL != self->parameter);

	gtk_label_set_label (GTK_LABEL (label), hkl_parameter_name_get(self->parameter));
}

GtkListItemFactory *
hkl_gui_parameter_factory_name_new(void)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (hkl_gui_parameter_bind_factory_label_name_cb), NULL);

	return factory;
}

void
hkl_gui_parameter_bind_factory_spin_button_value_cb (GtkListItemFactory *factory,
						     GtkListItem *list_item)
{
	gdouble min;
	gdouble max;
	gdouble value;
	GtkWidget *widget;
	GtkAdjustment *adjustment;
	HklGuiParameter *self;

	widget = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_assert(NULL != self->parameter);

	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
	value = hkl_parameter_value_get(self->parameter, HKL_UNIT_USER);

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), min, max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), value);
	adjustment = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(widget));

	g_object_bind_property(self, "value", widget, "value", G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "minimum", adjustment, "lower", G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "maximum", adjustment, "upper", G_BINDING_BIDIRECTIONAL);
}

GtkListItemFactory *
hkl_gui_parameter_factory_value_new(void)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_spin_button_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (hkl_gui_parameter_bind_factory_spin_button_value_cb), NULL);

	return factory;
}

void
hkl_gui_parameter_bind_factory_label_value_cb (GtkListItemFactory *factory,
					       GtkListItem *list_item)
{
	gdouble value;
	GtkWidget *label;
	HklGuiParameter *self;

	label = gtk_list_item_get_child (list_item);

	g_return_if_fail(GTK_IS_LABEL(label));

	self = gtk_list_item_get_item (list_item);

	g_return_if_fail(HKL_GUI_IS_PARAMETER(self));

	value = hkl_parameter_value_get(self->parameter, HKL_UNIT_USER);

	char *buf = g_strdup_printf ("%0.*f", 6, value);
	gtk_label_set_label (GTK_LABEL (label), buf);
	g_free(buf);

	g_object_bind_property(self, "value", label, "label", G_BINDING_BIDIRECTIONAL);
}

GtkListItemFactory *
hkl_gui_parameter_factory_value_label_new(void)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (hkl_gui_parameter_bind_factory_label_value_cb), NULL);

	return factory;
}

static void
bind_factory_axis_min_cb (GtkListItemFactory *factory,
			  GtkListItem        *list_item)
{
	gdouble min;
	gdouble max;
	GtkWidget *widget;
	HklGuiParameter *self;

	widget = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_assert(NULL != self->parameter);

	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), min, max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), min);
	g_object_bind_property(self, "minimum", widget, "value", G_BINDING_BIDIRECTIONAL);
}

GtkListItemFactory *
hkl_gui_parameter_factory_min_new(void)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_spin_button_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_axis_min_cb), NULL);

	return factory;
}

static void
bind_factory_axis_max_cb (GtkListItemFactory *factory,
			  GtkListItem        *list_item)
{
	gdouble min;
	gdouble max;
	GtkWidget *widget;
	HklGuiParameter *self;

	widget = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_assert(NULL != self->parameter);

	hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), min, max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), max);
	g_object_bind_property(self, "maximum", widget, "value", G_BINDING_BIDIRECTIONAL);
}

GtkListItemFactory *
hkl_gui_parameter_factory_max_new(void)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_spin_button_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_axis_max_cb), NULL);

	return factory;
}
