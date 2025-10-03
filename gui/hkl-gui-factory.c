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

/***********/
/* Factory */
/***********/

enum {
	PROP_0,

	PROP_ERROR,
	PROP_FACTORY,
	PROP_NAME,
	PROP_SAMPLE,
	PROP_WAVELENGTH,

	NUM_PROPERTIES,
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

typedef struct _HklGuiFactory HklGuiFactory;

struct _HklGuiFactory {
	GObject parent_instance;

	/* instance members */
	GError *error;
	GListStore *liststore_axes;
	GListStore *liststore_engines;
	GListStore *liststore_pseudo_axes;
	GListStore *liststore_solutions;

	HklGuiSample *gsample; /* not owned */
	struct diffractometer_t *diffractometer;
};

G_DEFINE_FINAL_TYPE(HklGuiFactory, hkl_gui_factory, G_TYPE_OBJECT);

static void
update_diffractometer_cb(HklGuiParameter *parameter,
			 GParamSpec* pspec,
			 gpointer *user_data)
{
	HklGuiFactory *self = HKL_GUI_FACTORY(user_data);
	diffractometer_update(self->diffractometer);
}

static void
update_pseudo_axes_cb(HklGuiParameter *parameter,
		      GParamSpec* pspec,
		      gpointer *user_data)
{
	HklGuiParameter *self = HKL_GUI_PARAMETER(user_data);
	hkl_gui_parameter_update(self);
}


static void
update_engines_cb(HklGuiParameter *parameter,
		  GParamSpec* pspec,
		  gpointer *user_data)
{
	HklGuiEngine *self = HKL_GUI_ENGINE(user_data);
	hkl_gui_engine_update(self);
}


static void
update_liststore_axes(HklGuiFactory *self)
{
	guint i;
	guint n_items = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_axes));
	for(i=0; i<n_items; ++i){
		HklGuiParameter *item = HKL_GUI_PARAMETER(g_list_model_get_item(G_LIST_MODEL(self->liststore_axes), i));
		hkl_gui_parameter_update(item);
	}
}

static void
update_liststore_engines(HklGuiFactory *self)
{
	guint i;
	guint n_items = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_engines));
	for(i=0; i<n_items; ++i){
		HklGuiEngine *item = HKL_GUI_ENGINE(g_list_model_get_item(G_LIST_MODEL(self->liststore_engines), i));
		hkl_gui_engine_update(item);
	}
}

static void
update_liststore_pseudo_axes(HklGuiFactory *self)
{
	guint i;
	guint n_items = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_pseudo_axes));
	for(i=0; i<n_items; ++i){
		HklGuiParameter *item = HKL_GUI_PARAMETER(g_list_model_get_item(G_LIST_MODEL(self->liststore_pseudo_axes), i));
		hkl_gui_parameter_update(item);
	}
}

static void
update_liststore_solutions(HklGuiFactory *self)
{
	const HklGeometryListItem *item;

	if(NULL != self->diffractometer->solutions){
		g_list_store_remove_all(self->liststore_solutions);
		HKL_GEOMETRY_LIST_FOREACH(item, self->diffractometer->solutions){
			const HklGeometry *geometry = hkl_geometry_list_item_geometry_get(item);
			g_list_store_append (self->liststore_solutions,
					     hkl_gui_geometry_new(geometry));
		}
	}
}

static void
engine_changed_cb(HklGuiEngine *engine,
		  gpointer user_data)
{
	g_return_if_fail(HKL_GUI_IS_ENGINE(engine));
	g_return_if_fail(HKL_GUI_IS_FACTORY(user_data));

	HklGuiFactory *self = HKL_GUI_FACTORY(user_data);
	HklGeometryList *solutions = hkl_gui_engine_get_solutions(engine);

	if (NULL != solutions){
		diffractometer_set_solutions(self->diffractometer, solutions);
		update_liststore_solutions(self);
	}
	hkl_gui_factory_set_error(self, hkl_gui_engine_get_error(engine));
}

static void
update_sample_cb(HklGuiSample *gsample,
		 GParamSpec* pspec,
		 gpointer *user_data)
{
	HklGuiFactory *self = HKL_GUI_FACTORY (user_data);
	diffractometer_update(self->diffractometer);
	update_liststore_pseudo_axes(self);
	update_liststore_engines(self);
}


static void
hkl_gui_factory_set_factory(HklGuiFactory *self,
			    HklFactory *factory)
{
	HklGuiParameter *g_axis;
	HklGuiEngine *g_engine;
	guint n_engines;

	/* diffractometer */
	self->diffractometer = create_diffractometer(factory);

	/* liststore_axes */
	self->liststore_axes = g_list_store_new(HKL_GUI_TYPE_PARAMETER);
	const darray_string *names;
	const char **name;

	names = hkl_geometry_axis_names_get(self->diffractometer->geometry);
	darray_foreach(name, *names){
		const HklParameter *parameter = hkl_geometry_axis_get(self->diffractometer->geometry, *name, NULL);
		g_list_store_append (self->liststore_axes,
				     hkl_gui_parameter_new(parameter));
	}

	/* liststore_engines and pseudo_axes */
	self->liststore_engines = g_list_store_new(HKL_GUI_TYPE_ENGINE);
	self->liststore_pseudo_axes = g_list_store_new(HKL_GUI_TYPE_PARAMETER);
	const darray_engine *engines;
	HklEngine **engine;

	engines = hkl_engine_list_engines_get(self->diffractometer->engines);
	n_engines = darray_size(*engines);
	darray_foreach(engine, *engines){
		g_engine = hkl_gui_engine_new(*engine);
		g_list_store_append (self->liststore_engines, g_engine);

		const darray_string *pseudo_axes = hkl_engine_pseudo_axis_names_get(*engine);

		darray_foreach(name, *pseudo_axes){
			const HklParameter *parameter = hkl_engine_pseudo_axis_get(*engine,
										   *name, NULL);
			g_list_store_append (self->liststore_pseudo_axes,
					     hkl_gui_parameter_new(parameter));
		}

		g_signal_connect(g_engine, "changed", G_CALLBACK(engine_changed_cb), self);
	}

	/* connect pseudo axes and engines parameters to axes */
	guint n_axes = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_axes));
	guint n_pseudo_axes = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_pseudo_axes));
	guint i;
	guint j;

	for(i=0; i<n_axes; ++i){
		g_axis = g_list_model_get_item(G_LIST_MODEL(self->liststore_axes), i);

		/* update the diffractometer */
		g_signal_connect(g_axis, "notify::value", G_CALLBACK(update_diffractometer_cb), self);

		/* update the pseudo axes */
		for(j=0; j<n_pseudo_axes; ++j){
			HklGuiParameter *g_pseudo_axis =  g_list_model_get_item(G_LIST_MODEL(self->liststore_pseudo_axes), j);
			g_signal_connect(g_axis, "notify::value", G_CALLBACK(update_pseudo_axes_cb), g_pseudo_axis);
		}

		/* update the gui engines */
		for(j=0; j<n_engines; ++j){
			g_engine = g_list_model_get_item(G_LIST_MODEL(self->liststore_engines), j);
			g_signal_connect(g_axis, "notify::value", G_CALLBACK(update_engines_cb), g_engine);
		}
	}

	/* liststore_solutions */
	self->liststore_solutions = g_list_store_new(HKL_GUI_TYPE_GEOMETRY);
	update_liststore_solutions(self);


	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FACTORY]);
}

static void
hkl_gui_factory_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	HklGuiFactory *self = HKL_GUI_FACTORY (object);


	switch (prop_id)
	{
	case PROP_ERROR:
		hkl_gui_factory_set_error(self, g_value_get_pointer(value));
		break;
	case PROP_FACTORY:
		hkl_gui_factory_set_factory(self, g_value_get_pointer(value));
		break;
	case PROP_SAMPLE:
		hkl_gui_factory_set_sample(self, g_value_get_object (value));
		break;
	case PROP_WAVELENGTH:
		hkl_gui_factory_set_wavelength(self, g_value_get_double (value));
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
	case PROP_ERROR:
		g_value_set_pointer (value, hkl_gui_factory_get_error(self));
		break;
	case PROP_FACTORY:
		g_value_set_pointer (value, hkl_gui_factory_get_factory(self));
		break;
	case PROP_NAME:
		g_value_set_string (value, hkl_gui_factory_get_name(self));
		break;
	case PROP_SAMPLE:
		g_value_set_object (value, hkl_gui_factory_get_sample(self));
		break;
	case PROP_WAVELENGTH:
		g_value_set_double (value, hkl_gui_factory_get_wavelength(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
hkl_gui_factory_dispose (GObject *gobject)
{
	HklGuiFactory *self = HKL_GUI_FACTORY (gobject);

	/* In dispose(), you are supposed to free all types referenced from this
	 * object which might themselves hold a reference to self. Generally,
	 * the most simple solution is to unref all members on which you own a
	 * reference.
	 */

	/* dispose() might be called multiple times, so we must guard against
	 * calling g_object_unref() on an invalid GObject by setting the member
	 * NULL; g_clear_object() does this for us.
	 */
	g_clear_object (&self->liststore_pseudo_axes);
	g_clear_object (&self->liststore_axes);

	/* Always chain up to the parent class; there is no need to check if
	 * the parent class implements the dispose() virtual function: it is
	 * always guaranteed to do so
	 */
	G_OBJECT_CLASS (hkl_gui_factory_parent_class)->dispose (gobject);
}

static void
hkl_gui_factory_finalize (GObject *gobject)
{
	HklGuiFactory *self = HKL_GUI_FACTORY(gobject);

	if (NULL != self->diffractometer)
		diffractometer_free(self->diffractometer);

	/* Always chain up to the parent class; as with dispose(), finalize()
	 * is guaranteed to exist on the parent's class virtual function table
	 */
	G_OBJECT_CLASS (hkl_gui_factory_parent_class)->finalize (gobject);
}

static void
hkl_gui_factory_init(HklGuiFactory *self)
{
	self->diffractometer = NULL;
	self->error = NULL;
}

static void
hkl_gui_factory_class_init (HklGuiFactoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = hkl_gui_factory_dispose;
	object_class->finalize = hkl_gui_factory_finalize;
	object_class->get_property = hkl_gui_factory_get_property;
	object_class->set_property = hkl_gui_factory_set_property;

	props[PROP_ERROR] =
		g_param_spec_pointer ("error",
				      "Error",
				      "the last GError",
				      G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_FACTORY] =
		g_param_spec_pointer ("factory",
				      "Factory",
				      "the embeded HklFactory.",
				      G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_NAME] =
		g_param_spec_string ("name",
				     "Name",
				     "the HklFactory name.",
				     "",
				     G_PARAM_STATIC_NAME | G_PARAM_READABLE);

	props[PROP_SAMPLE] =
		g_param_spec_object ("sample",
				     "Sample",
				     "the embeded HklGuiSample.",
				     HKL_GUI_TYPE_SAMPLE,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	props[PROP_WAVELENGTH] =
		g_param_spec_double ("wavelength",
				     "Wavelength",
				     "the diffractometer wavelength",
				     0.0, 100.0, 1.54,
				     G_PARAM_STATIC_NAME | G_PARAM_READWRITE);

	g_object_class_install_properties (object_class,
					   NUM_PROPERTIES,
					   props);
}

/* constructors */

HklGuiFactory *
hkl_gui_factory_new(const HklFactory *factory)
{
	return g_object_new (HKL_GUI_TYPE_FACTORY,
			     "factory", factory,
			     NULL);
}

/* getters */

GError *
hkl_gui_factory_get_error(HklGuiFactory *self)
{
	return self->error;
}

HklFactory *
hkl_gui_factory_get_factory(HklGuiFactory *self)
{
	return self->diffractometer->factory;
}

const char *
hkl_gui_factory_get_name(HklGuiFactory *self)
{
	return hkl_factory_name_get(self->diffractometer->factory);
}

HklGuiSample *
hkl_gui_factory_get_sample(HklGuiFactory *self)
{
	return self->gsample;
}

gdouble
hkl_gui_factory_get_wavelength(HklGuiFactory *self)
{
	return diffractometer_get_wavelength(self->diffractometer);
}

GListStore *
hkl_gui_factory_get_liststore_axes(HklGuiFactory *self)
{
	return self->liststore_axes;
}

GListStore *
hkl_gui_factory_get_liststore_engines(HklGuiFactory *self)
{
	return self->liststore_engines;
}

GListStore *
hkl_gui_factory_get_liststore_pseudo_axes(HklGuiFactory *self)
{
	return self->liststore_pseudo_axes;
}

GListStore *
hkl_gui_factory_get_liststore_solutions(HklGuiFactory *self)
{
	return self->liststore_solutions;
}

/* setters */

void
hkl_gui_factory_set_error(HklGuiFactory *self,
			  GError *error)
{
	g_clear_error(&self->error);
	self->error = g_error_copy(error);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ERROR]);
}

void
hkl_gui_factory_set_geometry(HklGuiFactory *self,
                             HklGuiGeometry *geometry)
 {
        diffractometer_set_geometry(self->diffractometer,
                                    hkl_gui_geometry_get_geometry(geometry));
        update_liststore_axes(self);
}


void
hkl_gui_factory_set_sample(HklGuiFactory *self,
			   HklGuiSample *gsample)
{
	g_return_if_fail (HKL_GUI_IS_FACTORY (self));
	g_return_if_fail (HKL_GUI_IS_SAMPLE (gsample));


	self->gsample = gsample;

	diffractometer_set_sample(self->diffractometer,
				  hkl_gui_sample_get_sample (gsample));

	g_signal_connect(gsample, "notify::a", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::b", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::c", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::alpha", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::beta", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::gamma", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::ux", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::ux", G_CALLBACK(update_sample_cb), self);
	g_signal_connect(gsample, "notify::uz", G_CALLBACK(update_sample_cb), self);

	update_liststore_pseudo_axes(self);
	update_liststore_engines(self);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SAMPLE]);
}

void
hkl_gui_factory_set_wavelength(HklGuiFactory *self,
			       gdouble wavelength)
{
	diffractometer_set_wavelength(self->diffractometer, wavelength);

	update_liststore_pseudo_axes(self);
	update_liststore_engines(self);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WAVELENGTH]);
}

/* methods */

void
hkl_gui_factory_add_reflection(HklGuiFactory *self)
{
	hkl_gui_sample_add_reflection(self->gsample,
				      self->diffractometer->geometry,
				      self->diffractometer->detector,
				      0, 0, 0);
}

void
hkl_gui_factory_setup_column_view_solutions(HklGuiFactory *self,
					    GtkColumnView *column_view)
{
	g_return_if_fail(HKL_GUI_IS_FACTORY(self));
	g_return_if_fail(GTK_IS_COLUMN_VIEW(column_view));

	gint idx, n_columns;
	GListModel *columns;
	GMenu *menu;
	GMenuItem *item;
	GtkColumnViewColumn *column;
	const darray_string *names = hkl_geometry_axis_names_get(self->diffractometer->geometry);
	const char **name;

	/* menu */
	menu = g_menu_new ();
	item = g_menu_item_new ("GoTo position", "app.go-to-position");
	g_menu_append_item (menu, item);

	/* remove all columns */
	columns = gtk_column_view_get_columns(column_view);
	n_columns = g_list_model_get_n_items(columns);
	for(idx=n_columns-1; idx>=0; --idx){
		gtk_column_view_remove_column(column_view,
					      g_list_model_get_item(columns, idx));
	}

	/* Add the right columns */
	idx=0;
	darray_foreach(name, *names){
		column = gtk_column_view_column_new(*name, hkl_gui_geometry_axis_value_factory_new(idx));
		gtk_column_view_column_set_header_menu(column, G_MENU_MODEL(menu));
		gtk_column_view_append_column(column_view, column);
		++idx;
	}
}

void
hkl_gui_factory_setup_column_view_sample_reflections(HklGuiFactory *self,
						     GtkColumnView *column_view)
{
	g_return_if_fail(HKL_GUI_IS_FACTORY(self));
	g_return_if_fail(GTK_IS_COLUMN_VIEW(column_view));

	gint idx, n_columns;
	GListModel *columns;
	const darray_string *names = hkl_geometry_axis_names_get(self->diffractometer->geometry);
	const char **name;

	/* remove all columns */
	columns = gtk_column_view_get_columns(column_view);
	n_columns = g_list_model_get_n_items(columns);
	for(idx=n_columns-1; idx>=0; --idx){
		gtk_column_view_remove_column(column_view,
					      g_list_model_get_item(columns, idx));
	}

	/* Add the right columns */
	add_column(column_view, "h", entry_numeric_property, "h");
	add_column(column_view, "k", entry_numeric_property, "k");
	add_column(column_view, "l", entry_numeric_property, "l");
	idx=0;
	darray_foreach(name, *names){
		add_column(column_view, *name, spin_button_sample_reflection_geometry_axis, idx);
		++idx;
	}
}

/**************************/
/* Gui ListItem factories */
/**************************/

struct diffractometer_t *
hkl_gui_factory_get_diffractometer(HklGuiFactory *self)
{
	return self->diffractometer;
}

/* Sort of class method */
