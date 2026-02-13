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
#include "hkl-gui-macros.h"

/***********/
/* Factory */
/***********/

enum {
	PROP_0,
	PROP_GEOMETRY,
	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiGeometry HklGuiGeometry;

struct _HklGuiGeometry {
	GObject parent_instance;

	/* instance members */
	HklGeometry *geometry; /* not owned */
};

G_DEFINE_FINAL_TYPE(HklGuiGeometry, hkl_gui_geometry, G_TYPE_OBJECT);


static void
hkl_gui_geometry_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	HklGuiGeometry *self = HKL_GUI_GEOMETRY (object);


	switch (prop_id)
	{
	case PROP_GEOMETRY:
	{
		self->geometry = g_value_get_pointer (value);

		g_object_notify_by_pspec (object, pspec);
	}
	break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;

	}
}

static void
hkl_gui_geometry_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	HklGuiGeometry *self = HKL_GUI_GEOMETRY (object);

	switch (prop_id)
	{
	case PROP_GEOMETRY:
		g_value_set_pointer (value, self->geometry);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_geometry_dispose (GObject *gobject)
{
	/* HklGuiGeometry *self = HKL_GUI_GEOMETRY (gobject); */

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
	G_OBJECT_CLASS (hkl_gui_geometry_parent_class)->dispose (gobject);
}

static void
hkl_gui_geometry_finalize (GObject *gobject)
{
	/* HklGuiGeometry *self = HKL_GUI_GEOMETRY(gobject); */

	/* Always chain up to the parent class; as with dispose(), finalize()
	 * is guaranteed to exist on the parent's class virtual function table
	 */
	G_OBJECT_CLASS (hkl_gui_geometry_parent_class)->finalize (gobject);
}

static void
hkl_gui_geometry_init(HklGuiGeometry *geometry)
{
	geometry->geometry = NULL;
}

static void
hkl_gui_geometry_class_init (HklGuiGeometryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = hkl_gui_geometry_dispose;
	object_class->finalize = hkl_gui_geometry_finalize;
	object_class->get_property = hkl_gui_geometry_get_property;
	object_class->set_property = hkl_gui_geometry_set_property;

	props[PROP_GEOMETRY] =
		g_param_spec_pointer ("geometry",
				      "Geometry",
				      "the embeded HklGeometry.",
				      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}

HklGuiGeometry *
hkl_gui_geometry_new(const HklGeometry *geometry)
{
	return g_object_new (HKL_GUI_TYPE_GEOMETRY,
			     "geometry", geometry,
			     NULL);
}


gdouble
hkl_gui_geometry_get_axis_value(HklGuiGeometry *self, gint idx)
{
	gint n_values;

	n_values = darray_size(*hkl_geometry_axis_names_get(self->geometry));
	double values[n_values];

	hkl_geometry_axis_values_get(self->geometry, values, n_values, HKL_UNIT_USER);

	return values[idx];
}

HklGeometry *
hkl_gui_geometry_get_geometry(HklGuiGeometry *self)
{
	return self->geometry;
}

/****************/
/* The Gui Part */
/****************/

static void
bind_item_factory_label__geometry_axis_value_cb(GtkListItemFactory *factory,
						GtkListItem *list_item,
						gpointer user_data)
{
	GtkWidget *label;
	gint idx = GPOINTER_TO_INT(user_data);
	HklGuiGeometry *self;

	label = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_return_if_fail(NULL != self->geometry);

	set_label_from_double(label, hkl_gui_geometry_get_axis_value(self, idx));
}

GtkListItemFactory *
hkl_gui_geometry_axis_value_factory_new(gint idx)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), GINT_TO_POINTER(idx));
	g_signal_connect (factory, "bind", G_CALLBACK (bind_item_factory_label__geometry_axis_value_cb), GINT_TO_POINTER(idx));

	return factory;
}
