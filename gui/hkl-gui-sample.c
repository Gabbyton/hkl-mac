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

	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiSample HklGuiSample;

struct _HklGuiSample {
	GObject parent_instance;

	/* instance members */
	GtkWidget *frame;

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
	{
		hkl_gui_sample_set_name(self, g_value_get_string (value));
	}
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
	self->sample = hkl_sample_new("toto");

	self->frame = g_object_ref(gtk_frame_new("Sample configuration"));
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

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}

HklGuiSample *
hkl_gui_sample_new(const char *name)
{
	return g_object_new (HKL_GUI_TYPE_SAMPLE,
			     "name", name,
			     NULL);
}


/* getters */

GtkWidget* hkl_gui_sample_get_frame(HklGuiSample *self)
{
	return self->frame;
}

const char *
hkl_gui_sample_get_name(HklGuiSample *self)
{
	return hkl_sample_name_get(self->sample);
}

HklSample *
hkl_gui_sample_get_sample(HklGuiSample *self)
{
	return self->sample;
}

/* setters */

void
hkl_gui_sample_set_name(HklGuiSample *self, const char *name)
{
	hkl_sample_name_set(self->sample, name);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NAME]);
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
