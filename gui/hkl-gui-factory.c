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

/***********/
/* Factory */
/***********/

enum {
	PROP_0,
	PROP_FACTORY,
	PROP_WAVELENGTH,
	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiFactory HklGuiFactory;

struct _HklGuiFactory {
	GObject parent_instance;

	/* instance members */
	struct diffractometer_t *diffractometer;
};

static void
hkl_gui_factory_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	HklGuiFactory *self = HKL_GUI_FACTORY (object);


	switch (prop_id)
	{
	case PROP_FACTORY:
	{
		HklFactory *new_factory = g_value_get_pointer (value);
		self->diffractometer = create_diffractometer(new_factory);
		g_object_notify_by_pspec (object, pspec);
	}
	break;
	case PROP_WAVELENGTH:
	{
		gdouble wavelength = g_value_get_double (value);
		diffractometer_set_wavelength(self->diffractometer, wavelength);

		g_object_notify_by_pspec (object, pspec);
	}
	break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;

	}
}

static void
hkl_gui_factory_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	HklGuiFactory *self = HKL_GUI_FACTORY (object);

	switch (prop_id)
	{
	case PROP_FACTORY:
		g_value_set_pointer (value, self->diffractometer->factory);
		break;
	case PROP_WAVELENGTH:
		g_value_set_double (value,
				    diffractometer_get_wavelength(self->diffractometer));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_factory_init(HklGuiFactory *factory)
{
	factory->diffractometer = NULL;
}

static void
hkl_gui_factory_class_init (HklGuiFactoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = hkl_gui_factory_set_property;
	object_class->get_property = hkl_gui_factory_get_property;

	props[PROP_FACTORY] =
		g_param_spec_pointer ("factory",
				      "Factory",
				      "the embeded HklFactory.",
				      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	props[PROP_WAVELENGTH] =
		g_param_spec_double ("wavelength",
				     "Wavelength",
				     "the diffractometer wavelength",
				     0.0, 100.0, 1.54,
				     G_PARAM_READWRITE);

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}


G_DEFINE_FINAL_TYPE(HklGuiFactory, hkl_gui_factory, G_TYPE_OBJECT);

HklGuiFactory *
hkl_gui_factory_new(const HklFactory *factory)
{
	return g_object_new (HKL_GUI_TYPE_FACTORY,
			     "factory", factory,
			     NULL);
}

static void
setup_factory_name_cb (GtkListItemFactory *factory,
		       GtkListItem        *list_item)
{
	GtkWidget *label;

	label = gtk_label_new ("");
	gtk_list_item_set_child (list_item, label);
}

static void
bind_factory_name_cb (GtkListItemFactory *factory,
		      GtkListItem        *list_item)
{
	GtkWidget *label;
	HklGuiFactory *self;


	label = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_assert(NULL != self->diffractometer);

	gtk_label_set_label (GTK_LABEL (label), hkl_factory_name_get(self->diffractometer->factory));
}

GtkListItemFactory *
hkl_gui_factory_name_factory_new(void)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_name_cb), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_name_cb), NULL);

	return factory;
}

struct diffractometer_t *
hkl_gui_factory_get_diffractometer(HklGuiFactory *self)
{
	return self->diffractometer;
}


GtkSelectionModel *
hkl_gui_factory_get_axes_selection_model(const HklGuiFactory *self)
{
	GListStore *liststore = g_list_store_new(HKL_GUI_TYPE_PARAMETER);
	GtkSelectionModel *selection_model;
	const darray_string *names;
	const char **name;

	names = hkl_geometry_axis_names_get(self->diffractometer->geometry);
	darray_foreach(name, *names){
		const HklParameter *parameter = hkl_geometry_axis_get(self->diffractometer->geometry, *name, NULL);
		g_list_store_append (liststore,
				    hkl_gui_parameter_new(parameter));
	}

	selection_model = GTK_SELECTION_MODEL(gtk_single_selection_new (G_LIST_MODEL(liststore)));

	return selection_model;

}

/* Sort of class method */

GListStore *
hkl_gui_factory_has_liststore(void)
{
	HklFactory **factories;
	size_t i, n;

	GListStore *liststore = g_list_store_new (HKL_GUI_TYPE_FACTORY);

	factories = hkl_factory_get_all(&n);
	for(i=0; i<n; ++i){
		g_list_store_append (liststore, hkl_gui_factory_new(factories[i]));
	}

	return liststore;
}


GtkWidget *
hkl_gui_factory_get_column_view_axes(void)
{
	GtkWidget *column_view;
	GtkColumnViewColumn *column;

	/* columnview1 */
	column_view = gtk_column_view_new(NULL);
	column = gtk_column_view_column_new("name", hkl_gui_parameter_factory_name_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), column);
	column = gtk_column_view_column_new("value", hkl_gui_parameter_factory_value_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), column);
	column = gtk_column_view_column_new("min", hkl_gui_parameter_factory_min_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), column);
	column = gtk_column_view_column_new("max", hkl_gui_parameter_factory_max_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), column);

	return column_view;
}
