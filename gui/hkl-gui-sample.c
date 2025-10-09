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

	PROP_NAME,
	PROP_A,
	PROP_B,
	PROP_C,
	PROP_ALPHA,
	PROP_BETA,
	PROP_GAMMA,
	PROP_UX,
	PROP_UY,
	PROP_UZ,
	PROP_REFLECTIONS,

	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiSample HklGuiSample;

struct _HklGuiSample {
	GObject parent_instance;

	/* instance members */
	GError *error;

	GListStore *liststore_reflections;
	HklSample *sample;
};

G_DEFINE_FINAL_TYPE(HklGuiSample, hkl_gui_sample, G_TYPE_OBJECT);


static void
hkl_gui_sample_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	HklGuiSample *self = HKL_GUI_SAMPLE (object);

	switch (prop_id)
	{
	case PROP_NAME:
		hkl_gui_sample_set_name(self, g_value_get_string (value));
		break;
	case PROP_A:
		hkl_gui_sample_set_a(self, g_value_get_double (value));
		break;
	case PROP_B:
		hkl_gui_sample_set_b(self, g_value_get_double (value));
		break;
	case PROP_C:
		hkl_gui_sample_set_c(self, g_value_get_double (value));
		break;
	case PROP_ALPHA:
		hkl_gui_sample_set_alpha(self, g_value_get_double (value));
		break;
	case PROP_BETA:
		hkl_gui_sample_set_beta(self, g_value_get_double (value));
		break;
	case PROP_GAMMA:
		hkl_gui_sample_set_gamma(self, g_value_get_double (value));
		break;
	case PROP_UX:
		hkl_gui_sample_set_ux(self, g_value_get_double (value));
		break;
	case PROP_UY:
		hkl_gui_sample_set_uy(self, g_value_get_double (value));
		break;
	case PROP_UZ:
		hkl_gui_sample_set_uz(self, g_value_get_double (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_sample_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	HklGuiSample *self = HKL_GUI_SAMPLE (object);

	switch (prop_id)
	{
	case PROP_NAME:
		g_value_set_string (value, hkl_gui_sample_get_name (self));;
		break;
	case PROP_A:
		g_value_set_double (value, hkl_gui_sample_get_a (self));;
		break;
	case PROP_B:
		g_value_set_double (value, hkl_gui_sample_get_b (self));;
		break;
	case PROP_C:
		g_value_set_double (value, hkl_gui_sample_get_c (self));;
		break;
	case PROP_ALPHA:
		g_value_set_double (value, hkl_gui_sample_get_alpha (self));;
		break;
	case PROP_BETA:
		g_value_set_double (value, hkl_gui_sample_get_beta (self));;
		break;
	case PROP_GAMMA:
		g_value_set_double (value, hkl_gui_sample_get_gamma (self));;
		break;
	case PROP_UX:
		g_value_set_double (value, hkl_gui_sample_get_ux (self));;
		break;
	case PROP_UY:
		g_value_set_double (value, hkl_gui_sample_get_uy (self));;
		break;
	case PROP_UZ:
		g_value_set_double (value, hkl_gui_sample_get_uz (self));;
		break;
	case PROP_REFLECTIONS:
		g_value_set_object (value, hkl_gui_sample_get_reflections (self));;
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_sample_dispose (GObject *gobject)
{
	/* HklGuiSample *self = HKL_GUI_SAMPLE (gobject); */

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
	G_OBJECT_CLASS (hkl_gui_sample_parent_class)->dispose (gobject);
}

static void
hkl_gui_sample_finalize (GObject *gobject)
{
	HklGuiSample *self = HKL_GUI_SAMPLE(gobject);

	hkl_sample_free(self->sample);

	/* Always chain up to the parent class; as with dispose(), finalize()
	 * is guaranteed to exist on the parent's class virtual function table
	 */
	G_OBJECT_CLASS (hkl_gui_sample_parent_class)->finalize (gobject);
}

static void
hkl_gui_sample_init(HklGuiSample *self)
{
	self->error = NULL;
	self->sample = hkl_sample_new("toto");
	self->liststore_reflections = g_list_store_new(HKL_GUI_TYPE_SAMPLE_REFLECTION);
}

static void
hkl_gui_sample_set_sample(HklGuiSample *self, HklSample *sample)
{
	hkl_sample_free(self->sample);
	self->sample = hkl_sample_new_copy(sample);
}

static void
hkl_gui_sample_class_init (HklGuiSampleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = hkl_gui_sample_dispose;
	object_class->finalize = hkl_gui_sample_finalize;
	object_class->get_property = hkl_gui_sample_get_property;
	object_class->set_property = hkl_gui_sample_set_property;

	props[PROP_NAME] =
		g_param_spec_string ("name",
				     "Name",
				     "the name of the sample",
				     "toto",
				      G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);

	props[PROP_A] =
		g_param_spec_double ("a",
				     "A",
				     "the a sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_B] =
		g_param_spec_double ("b",
				     "B",
				     "the b sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_C] =
		g_param_spec_double ("c",
				     "C",
				     "the c sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_ALPHA] =
		g_param_spec_double ("alpha",
				     "Alpha",
				     "the alpha sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_BETA] =
		g_param_spec_double ("beta",
				     "Beta",
				     "the beta sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_GAMMA] =
		g_param_spec_double ("gamma",
				     "Gamma",
				     "the gamma sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_UX] =
		g_param_spec_double ("ux",
				     "Ux",
				     "the ux sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_UY] =
		g_param_spec_double ("uy",
				     "Uy",
				     "the uy sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_UZ] =
		g_param_spec_double ("uz",
				     "Uz",
				     "the uz sample parameter",
				     -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_REFLECTIONS] =
		g_param_spec_object ("reflections",
				     "Reflections",
				     "the sample reflections model",
				     G_TYPE_LIST_STORE,
				     G_PARAM_STATIC_NAME | G_PARAM_READABLE);

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}

/* constructors */

HklGuiSample *
hkl_gui_sample_new(const char *name)
{
	return g_object_new (HKL_GUI_TYPE_SAMPLE,
			     "name", name,
			     NULL);
}

HklGuiSample *
hkl_gui_sample_new_copy(const HklGuiSample *gsample)
{
	HklGuiSample *self;

	self = hkl_gui_sample_new("toto");
	hkl_gui_sample_set_sample(self, gsample->sample);

	return self;
}


/* getters */

const char *
hkl_gui_sample_get_name(HklGuiSample *self)
{
	return hkl_sample_name_get(self->sample);
}

gdouble
hkl_gui_sample_get_a(HklGuiSample *self)
{
	return hkl_parameter_value_get (hkl_lattice_a_get (hkl_sample_lattice_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_b(HklGuiSample *self)
{
	return hkl_parameter_value_get (hkl_lattice_b_get (hkl_sample_lattice_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_c(HklGuiSample *self)
{
	return hkl_parameter_value_get (hkl_lattice_c_get (hkl_sample_lattice_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_alpha(HklGuiSample *self)
{
	return hkl_parameter_value_get (hkl_lattice_alpha_get (hkl_sample_lattice_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_beta(HklGuiSample *self)
{
	return hkl_parameter_value_get (hkl_lattice_beta_get (hkl_sample_lattice_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_gamma(HklGuiSample *self)
{
	return hkl_parameter_value_get (hkl_lattice_gamma_get (hkl_sample_lattice_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_ux(HklGuiSample *self)
{
	return hkl_parameter_value_get ( (hkl_sample_ux_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_uy(HklGuiSample *self)
{
	return hkl_parameter_value_get ( (hkl_sample_uy_get (self->sample)), HKL_UNIT_USER);
}

gdouble
hkl_gui_sample_get_uz(HklGuiSample *self)
{
	return hkl_parameter_value_get ( (hkl_sample_uz_get (self->sample)), HKL_UNIT_USER);
}

HklSample *
hkl_gui_sample_get_sample(HklGuiSample *self)
{
	return self->sample;
}

GListStore *
hkl_gui_sample_get_reflections(HklGuiSample *self)
{
	return self->liststore_reflections;
}

/* setters */

void
hkl_gui_sample_set_name(HklGuiSample *self, const char *name)
{
	hkl_sample_name_set(self->sample, name);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NAME]);
}

void
hkl_gui_sample_set_a(HklGuiSample *self, gdouble new_a)
{
	double a, b, c, alpha, beta, gamma;
	HklLattice *lattice;

	g_clear_error(&self->error);

	lattice = hkl_lattice_new_copy (hkl_sample_lattice_get (self->sample));

	g_return_if_fail (NULL != lattice);

	hkl_lattice_get(lattice, &a, &b, &c, &alpha, &beta, &gamma, HKL_UNIT_USER);
	if (TRUE == hkl_lattice_set(lattice, new_a, b, c, alpha, beta, gamma, HKL_UNIT_USER, &self->error)){
		hkl_sample_lattice_set(self->sample, lattice);

		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_A]);
	}
	hkl_lattice_free(lattice);
}

void
hkl_gui_sample_set_b(HklGuiSample *self, gdouble new_b)
{
	double a, b, c, alpha, beta, gamma;
	HklLattice *lattice;

	g_clear_error(&self->error);

	lattice = hkl_lattice_new_copy (hkl_sample_lattice_get (self->sample));

	g_return_if_fail (NULL != lattice);

	hkl_lattice_get(lattice, &a, &b, &c, &alpha, &beta, &gamma, HKL_UNIT_USER);
	if (TRUE == hkl_lattice_set(lattice, a, new_b, c, alpha, beta, gamma, HKL_UNIT_USER, &self->error)){
		hkl_sample_lattice_set(self->sample, lattice);

		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_B]);
	}
	hkl_lattice_free(lattice);
}

void
hkl_gui_sample_set_c(HklGuiSample *self, gdouble new_c)
{
	double a, b, c, alpha, beta, gamma;
	HklLattice *lattice;

	g_clear_error(&self->error);

	lattice = hkl_lattice_new_copy (hkl_sample_lattice_get (self->sample));

	g_return_if_fail (NULL != lattice);

	hkl_lattice_get(lattice, &a, &b, &c, &alpha, &beta, &gamma, HKL_UNIT_USER);
	if (TRUE == hkl_lattice_set(lattice, a, b, new_c, alpha, beta, gamma, HKL_UNIT_USER, &self->error)){
		hkl_sample_lattice_set(self->sample, lattice);

		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_C]);
	}
	hkl_lattice_free(lattice);
}

void
hkl_gui_sample_set_alpha(HklGuiSample *self, gdouble new_alpha)
{
	double a, b, c, alpha, beta, gamma;
	HklLattice *lattice;

	g_clear_error(&self->error);

	lattice = hkl_lattice_new_copy (hkl_sample_lattice_get (self->sample));

	g_return_if_fail (NULL != lattice);

	hkl_lattice_get(lattice, &a, &b, &c, &alpha, &beta, &gamma, HKL_UNIT_USER);
	if (TRUE == hkl_lattice_set(lattice, a, b, c, new_alpha, beta, gamma, HKL_UNIT_USER, &self->error)){
		hkl_sample_lattice_set(self->sample, lattice);

		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALPHA]);
	}
	hkl_lattice_free(lattice);
}

void
hkl_gui_sample_set_beta(HklGuiSample *self, gdouble new_beta)
{
	double a, b, c, alpha, beta, gamma;
	HklLattice *lattice;

	g_clear_error(&self->error);

	lattice = hkl_lattice_new_copy (hkl_sample_lattice_get (self->sample));

	g_return_if_fail (NULL != lattice);

	hkl_lattice_get(lattice, &a, &b, &c, &alpha, &beta, &gamma, HKL_UNIT_USER);
	if (TRUE == hkl_lattice_set(lattice, a, b, c, alpha, new_beta, gamma, HKL_UNIT_USER, &self->error)){
		hkl_sample_lattice_set(self->sample, lattice);

		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BETA]);
	}
	hkl_lattice_free(lattice);
}

void
hkl_gui_sample_set_gamma(HklGuiSample *self, gdouble new_gamma)
{
	double a, b, c, alpha, beta, gamma;
	HklLattice *lattice;

	g_clear_error(&self->error);

	lattice = hkl_lattice_new_copy (hkl_sample_lattice_get (self->sample));

	g_return_if_fail (NULL != lattice);

	hkl_lattice_get(lattice, &a, &b, &c, &alpha, &beta, &gamma, HKL_UNIT_USER);
	if (TRUE == hkl_lattice_set(lattice, a, b, c, alpha, beta, new_gamma, HKL_UNIT_USER, &self->error)){
		hkl_sample_lattice_set(self->sample, lattice);

		g_object_notify_by_pspec (G_OBJECT (self), props[PROP_GAMMA]);
	}
	hkl_lattice_free(lattice);
}

void
hkl_gui_sample_set_ux(HklGuiSample *self, gdouble new_value)
{
	HklParameter *parameter;

	g_clear_error(&self->error);

	parameter = hkl_parameter_new_copy (hkl_sample_ux_get (self->sample));

	g_return_if_fail (NULL != parameter);

	if (TRUE == hkl_parameter_value_set(parameter, new_value, HKL_UNIT_USER, &self->error)){
		if (TRUE == hkl_sample_ux_set(self->sample, parameter, &self->error)){
			g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UX]);
		}
	}
	hkl_parameter_free(parameter);
}

void
hkl_gui_sample_set_uy(HklGuiSample *self, gdouble new_value)
{
	HklParameter *parameter;

	g_clear_error(&self->error);

	parameter = hkl_parameter_new_copy (hkl_sample_uy_get (self->sample));

	g_return_if_fail (NULL != parameter);

	if (TRUE == hkl_parameter_value_set(parameter, new_value, HKL_UNIT_USER, &self->error)){
		if (TRUE == hkl_sample_uy_set(self->sample, parameter, &self->error)){
			g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UY]);
		}
	}
	hkl_parameter_free(parameter);
}

void
hkl_gui_sample_set_uz(HklGuiSample *self, gdouble new_value)
{
	HklParameter *parameter;

	g_clear_error(&self->error);

	parameter = hkl_parameter_new_copy (hkl_sample_uz_get (self->sample));

	g_return_if_fail (NULL != parameter);

	if (TRUE == hkl_parameter_value_set(parameter, new_value, HKL_UNIT_USER, &self->error)){
		if (TRUE == hkl_sample_uz_set(self->sample, parameter, &self->error)){
			g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UZ]);
		}
	}
	hkl_parameter_free(parameter);
}


/* methodes */

void hkl_gui_sample_add_reflection(HklGuiSample *self,
				   HklGeometry *geometry, HklDetector *detector,
				   gdouble h, gdouble k, gdouble l)
{
	HklSampleReflection *reflection;

	/* TODO error */

	reflection = hkl_sample_reflection_new(geometry,
					       detector,
					       h,k, l, NULL);

	hkl_sample_add_reflection(self->sample, reflection);

	g_list_store_append(self->liststore_reflections,
			    hkl_gui_sample_reflection_new(reflection));
}


void hkl_gui_sample_del_reflection(HklGuiSample *self, gint idx)
{
	HklGuiSampleReflection *reflection;

	reflection = g_list_model_get_item (G_LIST_MODEL (self->liststore_reflections), idx);

	g_return_if_fail (NULL != reflection);

	hkl_sample_del_reflection(self->sample,
				  hkl_gui_sample_reflection_get_reflection(reflection));

	g_list_store_remove(self->liststore_reflections, idx);
}

/****************/
/* The Gui Part */
/****************/

/* static void */
/* setup_factory_axis_value_cb (GtkListItemFactory *factory, */
/* 			     GtkListItem *list_item, */
/* 			     gpointer user_data) */
/* { */
/* 	GtkWidget *label; */

/* 	label = gtk_label_new (""); */
/* 	gtk_list_item_set_child (list_item, label); */
/* } */

/* static void */
/* bind_factory_axis_value_cb (GtkListItemFactory *factory, */
/* 			    GtkListItem *list_item, */
/* 			    gpointer user_data) */
/* { */
/* 	GtkWidget *label; */
/* 	gint idx = GPOINTER_TO_INT(user_data); */
/* 	HklGuiSample *self; */
/* 	gint n_values; */

/* 	label = gtk_list_item_get_child (list_item); */
/* 	self = gtk_list_item_get_item (list_item); */

/* 	g_return_if_fail(NULL != self->sample); */

/* 	// n_values = darray_size(*hkl_geometry_axis_names_get(self->sample)); */
/* 	double values[n_values]; */

/* 	// hkl_geometry_axis_values_get(self->sample, values, n_values, HKL_UNIT_USER); */

/* 	char *buf = g_strdup_printf ("%0.*f", 6, values[idx]); */
/* 	gtk_label_set_label (GTK_LABEL (label), buf); */
/* 	g_free(buf); */
/* } */

/* GtkListItemFactory * */
/* hkl_gui_sample_axis_value_factory_new(gint idx) */
/* { */
/* 		GtkListItemFactory *factory = gtk_signal_list_item_factory_new (); */
/* 		g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_axis_value_cb), GINT_TO_POINTER(idx)); */
/* 		g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_axis_value_cb), GINT_TO_POINTER(idx)); */

/* 		return factory; */
/* } */
