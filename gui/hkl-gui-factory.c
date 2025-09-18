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
	GListStore *liststore_axes;
	GListStore *liststore_engines;
	GListStore *liststore_pseudo_axes;
	GListStore *liststore_solutions;
};

G_DEFINE_FINAL_TYPE(HklGuiFactory, hkl_gui_factory, G_TYPE_OBJECT);


static void
engine_changed_cb(HklGuiEngine *engine)
{
	fprintf(stdout, "set the engine :)\n");
}

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
update_liststore_solutions(HklGuiFactory *self)
{
	/* liststore_solutions */
	self->liststore_solutions = g_list_store_new(HKL_GUI_TYPE_GEOMETRY);
	const HklGeometryListItem *item;
	if(NULL != self->diffractometer->solutions){
		HKL_GEOMETRY_LIST_FOREACH(item, self->diffractometer->solutions){
			const HklGeometry *geometry = hkl_geometry_list_item_geometry_get(item);
			g_list_store_append (self->liststore_solutions,
					     hkl_gui_geometry_new(geometry));
		}
	}
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
	case PROP_FACTORY:
	{
		HklGuiParameter *g_axis;
		HklGuiEngine *g_engine;
		guint n_engines;

		/* diffractometer */
		HklFactory *new_factory = g_value_get_pointer (value);
		self->diffractometer = create_diffractometer(new_factory);

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

			g_signal_connect(g_engine, "changed", G_CALLBACK(engine_changed_cb), NULL);
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
		update_liststore_solutions(self);

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
hkl_gui_factory_init(HklGuiFactory *factory)
{
	factory->diffractometer = NULL;
}

static void
hkl_gui_factory_class_init (HklGuiFactoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = hkl_gui_factory_dispose;
	object_class->finalize = hkl_gui_factory_finalize;
	object_class->get_property = hkl_gui_factory_get_property;
	object_class->set_property = hkl_gui_factory_set_property;

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

HklGuiFactory *
hkl_gui_factory_new(const HklFactory *factory)
{
	return g_object_new (HKL_GUI_TYPE_FACTORY,
			     "factory", factory,
			     NULL);
}

void hkl_gui_factory_add_engine_frames(HklGuiFactory *self, GtkBox *box)
{
	guint i;
	guint n_items;
	GtkWidget *child;

	g_return_if_fail(HKL_GUI_IS_FACTORY(self));
	g_return_if_fail(GTK_IS_BOX(box));

	/* first remove all the box content */
	for (child = gtk_widget_get_first_child (GTK_WIDGET (box));
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)){
		gtk_box_remove(box, child);
	}

	/* add all frames */
	n_items = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_engines));
	for (i=0;i<n_items; ++i){
		HklGuiEngine *gengine = g_list_model_get_item(G_LIST_MODEL(self->liststore_engines), i);

		gtk_box_append(box, hkl_gui_engine_get_frame(gengine));
	}
}

void
hkl_gui_factory_set_geometry(HklGuiFactory *self,
			     HklGuiGeometry *geometry)
{
	diffractometer_set_geometry(self->diffractometer,
				    hkl_gui_geometry_get_geometry(geometry));
	update_liststore_axes(self);
}


/**************************/
/* Gui ListItem factories */
/**************************/

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

	g_return_if_fail(NULL != self->diffractometer);

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
	return GTK_SELECTION_MODEL(gtk_single_selection_new (G_LIST_MODEL(self->liststore_axes)));
}

GtkSelectionModel *
hkl_gui_factory_get_pseudo_axes_selection_model(const HklGuiFactory *self)
{
	return GTK_SELECTION_MODEL(gtk_single_selection_new (G_LIST_MODEL(self->liststore_pseudo_axes)));
}

/* TODO remove the update */
GtkSelectionModel *
hkl_gui_factory_get_solutions_selection_model(HklGuiFactory *self)
{
	update_liststore_solutions(self);
	return GTK_SELECTION_MODEL(gtk_single_selection_new (G_LIST_MODEL(self->liststore_solutions)));
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

GtkWidget *
hkl_gui_factory_get_column_view_pseudo_axes(void)
{
	GtkWidget *column_view;
	GtkColumnViewColumn *column;

	column_view = gtk_column_view_new(NULL);
	column = gtk_column_view_column_new("name", hkl_gui_parameter_factory_name_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), column);
	column = gtk_column_view_column_new("value", hkl_gui_parameter_factory_value_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), column);

	return column_view;
}

GtkWidget *
hkl_gui_factory_get_column_view_solutions(void)
{
	return gtk_column_view_new(NULL);
}

void
hkl_gui_factory_setup_solutions(HklGuiFactory *self,
				GtkWidget **widget)
{
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
	columns = gtk_column_view_get_columns(GTK_COLUMN_VIEW(*widget));
	n_columns = g_list_model_get_n_items(columns);
	for(idx=n_columns-1; idx>=0; --idx){
		gtk_column_view_remove_column(GTK_COLUMN_VIEW(*widget),
					      g_list_model_get_item(columns, idx));
	}

	/* Add the right columns */
	idx=0;
	darray_foreach(name, *names){
		column = gtk_column_view_column_new(*name, hkl_gui_geometry_axis_value_factory_new(idx));
		gtk_column_view_column_set_header_menu(column, G_MENU_MODEL(menu));
		gtk_column_view_append_column(GTK_COLUMN_VIEW(*widget), column);
		++idx;
	}
}
