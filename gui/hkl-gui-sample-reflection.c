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

/**********/
/* Sample */
/**********/

enum {
	PROP_0,

	PROP_H,
	PROP_K,
	PROP_L,
	PROP_REFLECTION,

	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiSampleReflection HklGuiSampleReflection;

struct _HklGuiSampleReflection {
	GObject parent_instance;

	/* instance members */
	HklSampleReflection *reflection;
	HklGuiGeometry *geometry;
};

G_DEFINE_FINAL_TYPE(HklGuiSampleReflection, hkl_gui_sample_reflection, G_TYPE_OBJECT);

static void
hkl_gui_sample_reflection_set_property (GObject *object,
					guint prop_id,
					const GValue *value,
					GParamSpec *pspec)
{
	HklGuiSampleReflection *self = HKL_GUI_SAMPLE_REFLECTION (object);

	switch (prop_id)
	{
	case PROP_H:
		hkl_gui_sample_reflection_set_h (self, g_value_get_double (value));
		break;
	case PROP_K:
		hkl_gui_sample_reflection_set_k (self, g_value_get_double (value));
		break;
	case PROP_L:
		hkl_gui_sample_reflection_set_l (self, g_value_get_double (value));
		break;
	case PROP_REFLECTION:
		hkl_gui_sample_reflection_set_reflection (self, g_value_get_pointer (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_sample_reflection_get_property (GObject *object,
					guint prop_id,
					GValue *value,
					GParamSpec *pspec)
{
	HklGuiSampleReflection *self = HKL_GUI_SAMPLE_REFLECTION (object);

	switch (prop_id)
	{
	case PROP_H:
		g_value_set_double (value, hkl_gui_sample_reflection_get_h(self));
		break;
	case PROP_K:
		g_value_set_double (value, hkl_gui_sample_reflection_get_k(self));
		break;
	case PROP_L:
		g_value_set_double (value, hkl_gui_sample_reflection_get_l(self));
		break;
	case PROP_REFLECTION:
		g_value_set_pointer (value, hkl_gui_sample_reflection_get_reflection(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_sample_reflection_dispose (GObject *gobject)
{
	/* HklGuiSampleReflection *self = HKL_GUI_SAMPLE_REFLECTION (gobject); */

	/* In dispose(), you are supposed to free all types referenced from this
	 * object which might themselves hold a reference to self. Generally,
	 * the most simple solution is to unref all members on which you own a
	 * reference.
	 */

	/* dispose() might be called multiple times, so we must guard against
	 * calling g_object_unref() on an invalid GObject by setting the member
	 * NULL; g_clear_object() does this for us.
	 */

	/* Always chain up to the parent class; there is no need to check if
	 * the parent class implements the dispose() virtual function: it is
	 * always guaranteed to do so
	 */
	G_OBJECT_CLASS (hkl_gui_sample_reflection_parent_class)->dispose (gobject);
}

static void
hkl_gui_sample_reflection_finalize (GObject *gobject)
{
	//HklGuiSampleReflection *self = HKL_GUI_SAMPLE_REFLECTION(gobject);

	/* Always chain up to the parent class; as with dispose(), finalize()
	 * is guaranteed to exist on the parent's class virtual function table
	 */
	G_OBJECT_CLASS (hkl_gui_sample_reflection_parent_class)->finalize (gobject);
}

static void
hkl_gui_sample_reflection_init(HklGuiSampleReflection *self)
{
	self->reflection = NULL;
	self->geometry = NULL;
}

static void
hkl_gui_sample_reflection_class_init (HklGuiSampleReflectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = hkl_gui_sample_reflection_dispose;
	object_class->finalize = hkl_gui_sample_reflection_finalize;
	object_class->get_property = hkl_gui_sample_reflection_get_property;
	object_class->set_property = hkl_gui_sample_reflection_set_property;

	props[PROP_H] =
		g_param_spec_double ("h",
				     "H",
				     "The h miller coordinate",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);

	props[PROP_K] =
		g_param_spec_double ("k",
				     "K",
				     "The k miller coordinate ",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);

	props[PROP_L] =
		g_param_spec_double ("l",
				     "L",
				     "The l miller coordinate",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);

	props[PROP_REFLECTION] =
		g_param_spec_pointer ("reflection",
				      "Reflection",
				      "A HklSamplereflection",
				      G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}

HklGuiSampleReflection *
hkl_gui_sample_reflection_new(HklSampleReflection *reflection)
{
	return g_object_new (HKL_GUI_TYPE_SAMPLE_REFLECTION,
			     "reflection", reflection,
			     NULL);
}

/* getters */

gdouble
hkl_gui_sample_reflection_get_h(HklGuiSampleReflection* self)
{
	gdouble h, k, l;

	hkl_sample_reflection_hkl_get(self->reflection, &h, &k, &l);

	return h;
}

gdouble
hkl_gui_sample_reflection_get_k(HklGuiSampleReflection* self)
{
	gdouble h, k, l;

	hkl_sample_reflection_hkl_get(self->reflection, &h, &k, &l);

	return k;
}

gdouble
hkl_gui_sample_reflection_get_l(HklGuiSampleReflection* self)
{
	gdouble h, k, l;

	hkl_sample_reflection_hkl_get(self->reflection, &h, &k, &l);

	return l;
}

HklSampleReflection *
hkl_gui_sample_reflection_get_reflection(HklGuiSampleReflection *self)
{
	return self->reflection;
}

/* setters */

void
hkl_gui_sample_reflection_set_h(HklGuiSampleReflection* self, gdouble h_new)
{
	gdouble h, k, l;

	/* TODO error */

	hkl_sample_reflection_hkl_get(self->reflection, &h, &k, &l);

	if (TRUE == hkl_sample_reflection_hkl_set(self->reflection, h_new, k ,l, NULL))
		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_H]);
}

void
hkl_gui_sample_reflection_set_k(HklGuiSampleReflection* self, gdouble k_new)
{
	gdouble h, k, l;

	/* TODO error */

	hkl_sample_reflection_hkl_get(self->reflection, &h, &k, &l);

	if (TRUE == hkl_sample_reflection_hkl_set(self->reflection, h, k_new ,l, NULL))
		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_K]);
}

void
hkl_gui_sample_reflection_set_l(HklGuiSampleReflection* self, gdouble l_new)
{
	gdouble h, k, l;

	/* TODO error */

	hkl_sample_reflection_hkl_get(self->reflection, &h, &k, &l);

	if (TRUE == hkl_sample_reflection_hkl_set(self->reflection, h, k ,l_new, NULL))
		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_L]);
}

void
hkl_gui_sample_reflection_set_reflection(HklGuiSampleReflection *self, HklSampleReflection *reflection)
{
	self->reflection = reflection;
	self->geometry = hkl_gui_geometry_new(hkl_sample_reflection_geometry_get(reflection));

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REFLECTION]);
}


/*********************/
/* List item factory */
/*********************/


static void
bind_spin_button__sample_reflection_geometry_axis_value_cb (GtkListItemFactory *factory,
							    GtkListItem *list_item,
							    gpointer user_data)
{
	GtkWidget *spin_button;
	HklGuiSampleReflection *self;
	gint idx = GPOINTER_TO_INT(user_data);
	gint n_values;
	HklGeometry *geometry;

	spin_button = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_return_if_fail(NULL != self->geometry);

	geometry = hkl_gui_geometry_get_geometry(self->geometry);

	n_values = darray_size(*hkl_geometry_axis_names_get(geometry));
	double values[n_values];

	hkl_geometry_axis_values_get(geometry, values, n_values, HKL_UNIT_USER);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), values[idx]);
}


GtkListItemFactory *
hkl_gui_item_factory_new_spin_button_sample_reflection_geometry_axis(gint idx)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_spin_button_vertical_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_spin_button__sample_reflection_geometry_axis_value_cb), GINT_TO_POINTER(idx));

	return factory;
}
