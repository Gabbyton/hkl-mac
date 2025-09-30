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
#include "hkl-gui-macros.h"

enum {
	PROP_0,

	PROP_ENGINE,
	PROP_MODE,
	PROP_INITIALIZED,

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

	GError *error;
	HklEngine *engine;
	HklGeometryList *solutions;

	GListStore *liststore_mode_parameters;
	GListStore *liststore_modes;
	GListStore *liststore_pseudo_axes;

	GtkWidget *button_apply;
	GtkWidget *button_init;
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
	case PROP_MODE:
		hkl_gui_engine_set_mode(self, g_value_get_string (value));
		break;
	case PROP_INITIALIZED:
		hkl_gui_engine_set_initialized(self, g_value_get_boolean (value));
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
	case PROP_MODE:
		g_value_set_string (value, hkl_gui_engine_get_mode (self));
		break;
	case PROP_INITIALIZED:
		g_value_set_boolean (value, hkl_gui_engine_get_initialized (self));
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

	g_clear_error(&self->error);

	solutions = hkl_engine_pseudo_axis_values_set(self->engine, NULL, 0, HKL_UNIT_USER, &self->error);

	if (NULL != solutions){
		if (NULL != self->solutions)
			hkl_geometry_list_free(self->solutions);
		self->solutions = solutions;
	}

	g_signal_emit(self, signals[CHANGED], 0);
}


static void
button_init_clicked_cb (GtkButton* button, HklGuiEngine* self)
{
	g_return_if_fail (GTK_IS_BUTTON(button));
	g_return_if_fail (HKL_GUI_IS_ENGINE(self));

	hkl_gui_engine_set_initialized(self, true);

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

	hkl_gui_engine_set_mode(self, name);
}

static void
setup_factory_pseudo_axis_cb (GtkListItemFactory *factory,
			      GtkListItem *list_item,
			      gpointer user_data)
{
	HklGuiEngine *self = HKL_GUI_ENGINE(user_data);

	guint capabilities = hkl_engine_capabilities_get(self->engine);
	if (HKL_ENGINE_CAPABILITIES_WRITABLE & capabilities){
		hkl_gui_setup_item_factory_spin_button_cb(factory, list_item);
	} else {
		hkl_gui_setup_item_factory_label_cb(factory, list_item);
	}
}

static void
bind_factory_pseudo_axis_cb (GtkListItemFactory *factory,
			     GtkListItem *list_item,
			     gpointer user_data)
{
	HklGuiEngine *self = HKL_GUI_ENGINE(user_data);

	guint capabilities = hkl_engine_capabilities_get(self->engine);
	if (HKL_ENGINE_CAPABILITIES_WRITABLE & capabilities){
		hkl_gui_parameter_bind_factory_spin_button_value_cb(factory, list_item);
	} else {
		hkl_gui_bind_item_factory_label_property_cb(factory, list_item, "value");
	}
}

static GtkListItemFactory *
hkl_gui_item_factory_new_engine_pseudo_axis(HklGuiEngine *self)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (setup_factory_pseudo_axis_cb), self);
	g_signal_connect (factory, "bind", G_CALLBACK (bind_factory_pseudo_axis_cb), self);

	return factory;
}


static void
hkl_gui_engine_init (HklGuiEngine *self)
{
	GtkWidget *hbox;
	GtkWidget *label_mode;
	GtkWidget *vbox;

	self->engine = NULL;
	self->error = NULL;
	self->solutions = NULL;
	self->button_apply = gtk_button_new_with_label("Apply");
	self->button_init = gtk_button_new_with_label("Initialize");
	self->liststore_modes = G_LIST_STORE(gtk_string_list_new(NULL));
	self->liststore_pseudo_axes = g_list_store_new(HKL_GUI_TYPE_PARAMETER);
	self->liststore_mode_parameters = g_list_store_new(HKL_GUI_TYPE_PARAMETER);

	self->frame = g_object_ref(gtk_frame_new(""));
	self->dropdown = gtk_drop_down_new_from_strings(NULL);
	self->column_view_parameters = gtk_column_view_new(
		GTK_SELECTION_MODEL(gtk_no_selection_new(G_LIST_MODEL(self->liststore_mode_parameters))));
	self->column_view_pseudo_axes = gtk_column_view_new(
		GTK_SELECTION_MODEL(gtk_no_selection_new(G_LIST_MODEL(self->liststore_pseudo_axes))));

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	label_mode = gtk_label_new("Mode: ");
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* button apply */
	g_signal_connect (self->button_apply, "clicked",
			  G_CALLBACK (button_apply_clicked_cb), self);

	/* button init */
	g_signal_connect (self->button_init, "clicked",
			  G_CALLBACK (button_init_clicked_cb), self);

	/* column_view parameters */
	add_column(self->column_view_parameters, "name", label_property, "name");
	add_column(self->column_view_parameters, "value", spin_button_parameter_value);

	/* column_view pseudo_axes */
	add_column(self->column_view_pseudo_axes, "name", label_property, "name");
	add_column(self->column_view_pseudo_axes, "value", engine_pseudo_axis, self);

	/* dropdown */
	gtk_drop_down_set_model(GTK_DROP_DOWN(self->dropdown),
				G_LIST_MODEL(self->liststore_modes));
	g_signal_connect (self->dropdown, "notify::selected-item",
			  G_CALLBACK (dropdown_notify_selected_item_cb), self);

	/* frame */
	gtk_frame_set_child(GTK_FRAME(self->frame), vbox);

	/* hbox */
	gtk_box_append(GTK_BOX(hbox), label_mode);
	gtk_box_append(GTK_BOX(hbox), self->dropdown);
	gtk_box_append(GTK_BOX(hbox), self->button_init);

	/* vbox */
	gtk_box_append(GTK_BOX(vbox), self->column_view_pseudo_axes);
	gtk_box_append(GTK_BOX(vbox), self->button_apply);
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

	props[PROP_MODE] =
		g_param_spec_string ("mode",
				     "Mode",
				     "The curren tmode name of the engine",
				     "invalid-mode",
				     G_PARAM_READWRITE |
				     G_PARAM_STATIC_STRINGS);

	props[PROP_INITIALIZED] =
		g_param_spec_boolean ("initialized",
				      "Initialized",
				      "The current mode was initialized or not.",
				      FALSE,
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

GError*
hkl_gui_engine_get_error (HklGuiEngine *self)
{
	return self->error;
}

GtkWidget *
hkl_gui_engine_get_frame(HklGuiEngine *self)
{
	return self->frame;
}

gboolean
hkl_gui_engine_get_initialized (HklGuiEngine *self)
{
	return hkl_engine_initialized_get(self->engine);
}

const char *
hkl_gui_engine_get_mode (HklGuiEngine *self)
{
	return hkl_engine_current_mode_get(self->engine);
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
	const char *current_mode;
	const char **mode;
	const darray_string *modes;
	const char **name;

	g_return_if_fail (self != NULL);
	g_return_if_fail (engine != NULL);

	self->engine = engine;

	/* engine name */
	const char *engine_name = hkl_engine_name_get(engine);
	char *buf;
	guint capabilities = hkl_engine_capabilities_get(engine);
	if (HKL_ENGINE_CAPABILITIES_WRITABLE & capabilities){
		buf = g_strdup_printf ("%s", engine_name);
	}else{
		buf = g_strdup_printf ("%s (Read Only)", engine_name);
	}

	gtk_frame_set_label (GTK_FRAME (self->frame), buf);
	g_free(buf);

	/* pseudo axes only once */
	darray_foreach(name, *hkl_engine_pseudo_axis_names_get(engine)){
		const HklParameter *parameter = hkl_engine_pseudo_axis_get(self->engine, *name, NULL);
		g_list_store_append(self->liststore_pseudo_axes,
				    hkl_gui_parameter_new(parameter));
	}

	/* modes names only once */
	modes = hkl_engine_modes_names_get(engine);
	darray_foreach(mode, *modes){
		gtk_string_list_append(GTK_STRING_LIST(self->liststore_modes), *mode);
	}
	gtk_drop_down_set_model(GTK_DROP_DOWN(self->dropdown),
				G_LIST_MODEL(self->liststore_modes));
	if(darray_size(*modes) == 1)
		gtk_widget_set_sensitive(self->dropdown, false);

	current_mode = hkl_engine_current_mode_get(self->engine);

	hkl_gui_engine_set_mode(self, current_mode);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENGINE]);
}

void
hkl_gui_engine_set_mode (HklGuiEngine *self, const char *mode)
{
	g_return_if_fail (self != NULL);
	g_return_if_fail (mode != NULL);

	g_return_if_fail(TRUE == hkl_engine_current_mode_set(self->engine, mode, NULL));

	update_liststore_mode_parameters(self);

	guint capabilities = hkl_engine_capabilities_get(self->engine);

	gtk_widget_set_sensitive(self->button_init, HKL_ENGINE_CAPABILITIES_INITIALIZABLE & capabilities);
	gtk_widget_set_sensitive(self->button_apply, HKL_ENGINE_CAPABILITIES_WRITABLE & capabilities);

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODE]);
}

void
hkl_gui_engine_set_initialized (HklGuiEngine *self, gboolean initialized)
{
	g_return_if_fail (self != NULL);

	g_return_if_fail (TRUE == hkl_engine_initialized_set(self->engine, initialized, NULL));

	g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODE]);
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
