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

enum {
	PROP_0,
	PROP_PARAMETER,
	PROP_MINIMUM,
	PROP_MAXIMUM,
	PROP_VALUE,
	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiParameter HklGuiParameter;

struct _HklGuiParameter {
	GObject parent_instance;

	/* instance members */
	HklParameter *parameter;
};

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
	case PROP_MINIMUM:
	{
		gdouble min;
		gdouble max;

		gdouble new_min = g_value_get_double (value);
		hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
		if (TRUE == hkl_parameter_min_max_set(self->parameter, new_min, max, HKL_UNIT_USER, NULL)){
			g_object_notify_by_pspec (object, pspec);
		}
	}
	break;
	case PROP_MAXIMUM:
	{
		gdouble min;
		gdouble max;

		gdouble new_max = g_value_get_double (value);
		hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
		if (TRUE == hkl_parameter_min_max_set(self->parameter, min, new_max, HKL_UNIT_USER, NULL)){
			g_object_notify_by_pspec (object, pspec);
		}
	}
	break;
	case PROP_VALUE:
	{
		gdouble new_value = g_value_get_double (value);
		if (TRUE == hkl_parameter_value_set(self->parameter, new_value, HKL_UNIT_USER, NULL)){
			g_object_notify_by_pspec (object, pspec);
		}
	}
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
	case PROP_MINIMUM:
	{
		gdouble min;
		gdouble max;
		hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
		g_value_set_double (value, min);
	}
	break;
	case PROP_MAXIMUM:
	{
		gdouble min;
		gdouble max;
		hkl_parameter_min_max_get(self->parameter, &min, &max, HKL_UNIT_USER);
		g_value_set_double (value, max);
	}
	break;
	case PROP_VALUE:
		g_value_set_double (value,
				    hkl_parameter_value_get(self->parameter, HKL_UNIT_USER));
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


G_DEFINE_FINAL_TYPE(HklGuiParameter, hkl_gui_parameter, G_TYPE_OBJECT);

HklGuiParameter *hkl_gui_parameter_new(const HklParameter *parameter)
{
	return g_object_new (HKL_GUI_TYPE_PARAMETER,
			     "parameter", parameter,
			     NULL);
}

static void
setup_factory_axis_label_cb (GtkListItemFactory *factory,
			     GtkListItem        *list_item)
{
	GtkWidget *label;

	label = gtk_label_new ("");
	gtk_list_item_set_child (list_item, label);
}

static void
bind_factory_axis_name_cb (GtkListItemFactory *factory,
			   GtkListItem        *list_item)
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
	g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_axis_label_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_axis_name_cb), NULL);

	return factory;
}

static void
setup_factory_axis_spin_button_cb (GtkListItemFactory *factory,
				   GtkListItem        *list_item)
{
	GtkWidget *widget;

	widget = gtk_spin_button_new_with_range (-360, 360, 0.0001);
	gtk_list_item_set_child (list_item, widget);
}

static void
bind_factory_axis_value_cb (GtkListItemFactory *factory,
			    GtkListItem        *list_item)
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
	g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_axis_spin_button_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_axis_value_cb), NULL);

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
	g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_axis_spin_button_cb), NULL);
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
	g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_axis_spin_button_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_axis_max_cb), NULL);

	return factory;
}
