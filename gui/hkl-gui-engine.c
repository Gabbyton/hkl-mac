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
 * Copyright (C) 2003-2013, 2022, 2024, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "hkl.h"
#include "hkl-gui.h"

enum {
	PROP_0,

	PROP_ENGINE,

	NUM_PROPERTIES
};

static GParamSpec *props[NUM_PROPERTIES] = { NULL, };

enum {
	CHANGED,

	NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0 };

struct _HklGuiEngine {
	GObject parent_instance;

	HklEngine *engine;
	HklGeometryList *solutions;

	GListStore *liststore_mode_parameters;
	GListStore *liststore_modes;
	GListStore *liststore_pseudo_axes;

	GtkWidget *column_view_parameters;
	GtkWidget *column_view_pseudo_axes;
	GtkWidget *dropdown;
	GtkWidget *frame;
};

G_DEFINE_FINAL_TYPE (HklGuiEngine, hkl_gui_engine, G_TYPE_OBJECT);

static void
set_property (GObject *object, guint prop_id,
	      const GValue *value, GParamSpec *pspec)
{
	HklGuiEngine *self = HKL_GUI_ENGINE (object);

	switch (prop_id) {
	case PROP_ENGINE:
		hkl_gui_engine_set_engine(self, g_value_get_pointer (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
get_property (GObject *object, guint prop_id,
	      GValue *value, GParamSpec *pspec)
{
	HklGuiEngine *self = HKL_GUI_ENGINE (object);

	switch (prop_id)
	{
	case PROP_ENGINE:
		g_value_set_pointer (value, hkl_gui_engine_get_engine (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
finalize (GObject* object)
{
	G_OBJECT_CLASS (hkl_gui_engine_parent_class)->finalize (object);
}


static void
button_apply_clicked_cb (GtkButton* button, HklGuiEngine* self)
{
	HklGeometryList *solutions;

	solutions = hkl_engine_pseudo_axis_values_set(self->engine, NULL, 0, HKL_UNIT_USER, NULL);

	g_return_if_fail(NULL != solutions);

	if (NULL != self->solutions)
		hkl_geometry_list_free(self->solutions);
	self->solutions = solutions;

	g_signal_emit(self, signals[CHANGED], 0);
}


static void
update_liststore_mode_parameters(HklGuiEngine *self)
{
	const char * *name;

	g_list_store_remove_all(self->liststore_mode_parameters);

	darray_foreach(name, *hkl_engine_parameters_names_get(self->engine)){
		const HklParameter *parameter = hkl_engine_parameter_get(self->engine, *name, NULL);
		g_list_store_append(self->liststore_mode_parameters,
				    hkl_gui_parameter_new(parameter));
	}
}

static void
dropdown_notify_selected_item_cb(GtkDropDown *dropdown,
				 GParamSpec* pspec,
				 gpointer *user_data)
{
	HklGuiEngine *self = HKL_GUI_ENGINE(user_data);
	guint mode;

	mode = gtk_drop_down_get_selected(dropdown);

	g_return_if_fail(GTK_INVALID_LIST_POSITION != mode);

	const char *name = gtk_string_list_get_string(GTK_STRING_LIST(self->liststore_modes), mode);
	if(FALSE == hkl_engine_current_mode_set(self->engine, name, NULL)){
		return;
	}else{
		update_liststore_mode_parameters(self);
	}
}

static void
hkl_gui_engine_init (HklGuiEngine *self)
{
	GtkWidget *button_apply;
	GtkWidget *button_init;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkColumnViewColumn *column;

	self->engine = NULL;
	self->solutions = NULL;
	self->liststore_modes = G_LIST_STORE(gtk_string_list_new(NULL));
	self->liststore_pseudo_axes = g_list_store_new(HKL_GUI_TYPE_PARAMETER);
	self->liststore_mode_parameters = g_list_store_new(HKL_GUI_TYPE_PARAMETER);

	self->frame = g_object_ref(gtk_frame_new(""));
	self->dropdown = gtk_drop_down_new_from_strings(NULL);
	self->column_view_parameters = gtk_column_view_new(
		GTK_SELECTION_MODEL(gtk_single_selection_new(G_LIST_MODEL(self->liststore_mode_parameters))));
	self->column_view_pseudo_axes = gtk_column_view_new(
		GTK_SELECTION_MODEL(gtk_single_selection_new(G_LIST_MODEL(self->liststore_pseudo_axes))));

	button_apply = gtk_button_new_with_label("Apply");
	button_init = gtk_button_new_with_label("Initialize");
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* button apply */
	g_signal_connect (button_apply, "clicked",
			  G_CALLBACK (button_apply_clicked_cb), self);

	/* column_view parameters */
	column = gtk_column_view_column_new("name", hkl_gui_parameter_factory_name_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_parameters), column);
	column = gtk_column_view_column_new("value", hkl_gui_parameter_factory_value_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_parameters), column);

	/* column_view pseudo_axes */
	column = gtk_column_view_column_new("name", hkl_gui_parameter_factory_name_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_pseudo_axes), column);
	column = gtk_column_view_column_new("value", hkl_gui_parameter_factory_value_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_pseudo_axes), column);

	/* dropdown */
	gtk_drop_down_set_model(GTK_DROP_DOWN(self->dropdown),
				G_LIST_MODEL(self->liststore_modes));
	g_signal_connect (self->dropdown, "notify::selected-item",
			  G_CALLBACK (dropdown_notify_selected_item_cb), self);

	/* frame */
	gtk_frame_set_child(GTK_FRAME(self->frame), vbox);

	/* hbox */
	gtk_box_append(GTK_BOX(hbox), self->dropdown);
	gtk_box_append(GTK_BOX(hbox), button_apply);
	gtk_box_append(GTK_BOX(hbox), button_init);

	/* vbox */
	gtk_box_append(GTK_BOX(vbox), self->column_view_pseudo_axes);
	gtk_box_append(GTK_BOX(vbox), hbox);
	gtk_box_append(GTK_BOX(vbox), self->column_view_parameters);
}

static void hkl_gui_engine_class_init (HklGuiEngineClass * class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);

	/* virtual methods */
	gobject_class->finalize = finalize;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	/* properties */
	props[PROP_ENGINE] =
		g_param_spec_pointer ("engine",
				      "Engine",
				      "The Hkl Engine used underneath",
				      G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_READWRITE |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class,
					   NUM_PROPERTIES,
					   props);

	/* signals */
	signals[CHANGED] =
		g_signal_new ("changed",
			      HKL_GUI_TYPE_ENGINE,
			      G_SIGNAL_RUN_LAST,
			      0, /* class offset */
			      NULL, /* accumulator */
			      NULL, /* accu_data */
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, /* return_type */
			      0);
}


HklGuiEngine*
hkl_gui_engine_new (HklEngine* engine)
{
	return g_object_new (HKL_GUI_TYPE_ENGINE,
			     "engine", engine,
			     NULL);
}

HklEngine*
hkl_gui_engine_get_engine (HklGuiEngine *self)
{
	return self->engine;
}

GtkWidget *
hkl_gui_engine_get_frame(HklGuiEngine *self)
{
	return self->frame;
}

HklGeometryList *
hkl_gui_engine_get_solutions(HklGuiEngine *self)
{
	return self->solutions;
}

void
hkl_gui_engine_set_engine (HklGuiEngine *self,
			   HklEngine *engine)
{
	const char * *name;

	g_return_if_fail (self != NULL);
	g_return_if_fail (engine != NULL);

	self->engine = engine;

	gtk_frame_set_label(GTK_FRAME(self->frame),
			    hkl_engine_name_get(engine));

	/* modes */
	darray_foreach(name, *hkl_engine_modes_names_get(engine)){
		gtk_string_list_append(GTK_STRING_LIST(self->liststore_modes), *name);
	}
	gtk_drop_down_set_model(GTK_DROP_DOWN(self->dropdown),
				G_LIST_MODEL(self->liststore_modes));

	darray_foreach(name, *hkl_engine_pseudo_axis_names_get(engine)){
		const HklParameter *parameter = hkl_engine_pseudo_axis_get(self->engine, *name, NULL);
		g_list_store_append(self->liststore_pseudo_axes,
				    hkl_gui_parameter_new(parameter));
	}

	update_liststore_mode_parameters(self);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENGINE]);
}

void
hkl_gui_engine_update(HklGuiEngine *self)
{
	guint i;
	guint n_parameters;
	HklGuiParameter *parameter;

	n_parameters = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_pseudo_axes));
	for(i=0; i<n_parameters; ++i){
		parameter = g_list_model_get_item(G_LIST_MODEL(self->liststore_pseudo_axes), i);
		hkl_gui_parameter_update(parameter);
	}

	n_parameters = g_list_model_get_n_items(G_LIST_MODEL(self->liststore_mode_parameters));
	for(i=0; i<n_parameters; ++i){
		parameter = g_list_model_get_item(G_LIST_MODEL(self->liststore_mode_parameters), i);
		hkl_gui_parameter_update(parameter);
	}
}

/* static void */
/* button2_clicked_cb (GtkButton* button, HklGuiEngine* self) */
/* { */
/* 	HklGuiEnginePrivate *priv = hkl_gui_engine_get_instance_private(self); */

/* 	if (HKL_ENGINE_CAPABILITIES_INITIALIZABLE & hkl_engine_capabilities_get(priv->engine)){ */
/* 		if(hkl_engine_initialized_set(priv->engine, TRUE, NULL)){ */
/* 			/\* some init method update the parameters *\/ */
/* 			update_mode_parameters(self); */
/* 		} */
/* 	} */
/* } */
