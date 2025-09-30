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
#if HKL3D
# include "hkl-gui-3d.h"
#endif

/****************/
/* HklGuiWindow */
/****************/

struct _HklGuiWindow {
	GtkApplication parent_instance;

	gint page_sample;

	GListStore *liststore_samples;

	GtkAdjustment *adjustment_wavelength;
	GtkAlertDialog *alert_dialog_solutions;
	GBinding *adjustement_wavelength_binding;

	GtkWidget *column_view_axes;
	GtkWidget *column_view_pseudo_axes;
	GtkWidget *column_view_samples;
	GtkWidget *column_view_solutions;
	GtkWidget *drop_down_samples;
	GtkWidget *flowbox_engines;
	GtkWidget *notebook1;
	GtkWidget *spinbutton_wavelength;
	GtkWidget *window1;

	HklGuiFactory *factory; /* not owned */
};

G_DEFINE_FINAL_TYPE (HklGuiWindow, hkl_gui_window, GTK_TYPE_APPLICATION);

/* static gboolean */
/* finalize_liststore_samples(GtkTreeModel *model, */
/* 			   GtkTreePath *path, */
/* 			   GtkTreeIter *iter, */
/* 			   gpointer data) */
/* { */
/* 	HklSample *sample = NULL; */

/* 	gtk_tree_model_get(model, iter, */
/* 			   SAMPLE_COL_SAMPLE, &sample, */
/* 			   -1); */
/* 	hkl_sample_free(sample); */
/* 	return FALSE; */
/* } */

static void
finalize (GObject* object)
{
	G_OBJECT_CLASS (hkl_gui_window_parent_class)->finalize (object);
}


static void
raise_error(HklGuiWindow *self, GError *error)
{
	g_return_if_fail (error != NULL);

	/* show an error message */
	gtk_alert_dialog_set_message(self->alert_dialog_solutions,
				     error->message);
	gtk_alert_dialog_show(self->alert_dialog_solutions,
			      GTK_WINDOW(self->window1));
}

/* static void */
/* update_reflections (HklGuiWindow *self) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */

/* 	gtk_list_store_clear (priv->liststore_reflections); */

/* 	if(priv->sample){ */
/* 		HklSampleReflection* reflection; */
/* 		guint index = 0; */

/* 		HKL_SAMPLE_REFLECTIONS_FOREACH(reflection, priv->sample){ */
/* 			GtkTreeIter iter = {0}; */
/* 			gdouble h, k, l; */
/* 			gboolean flag; */

/* 			hkl_sample_reflection_hkl_get(reflection, &h, &k, &l); */
/* 			flag = hkl_sample_reflection_flag_get(reflection); */

/* 			gtk_list_store_append (priv->liststore_reflections, &iter); */

/* 			gtk_list_store_set (priv->liststore_reflections, */
/* 					    &iter, */
/* 					    REFLECTION_COL_INDEX, index++, */
/* 					    REFLECTION_COL_H, h, */
/* 					    REFLECTION_COL_K, k, */
/* 					    REFLECTION_COL_L, l, */
/* 					    REFLECTION_COL_FLAG, flag, */
/* 					    REFLECTION_COL_REFLECTION, reflection, */
/* 					    -1); */
/* 		} */
/* 	} */
/* } */

/* static void */
/* update_3d(HklGuiWindow *self) */
/* { */
/* #ifdef HKL3D */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */

/* 	if(priv->frame3d){ */
/* 		hkl_gui_3d_is_colliding(priv->frame3d); */
/* 		hkl_gui_3d_invalidate(priv->frame3d); */
/* 	} */
/* #endif */
/* } */


/* static void */
/* set_up_3D (HklGuiWindow* self) */
/* { */
/* #if HKL3D */

/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */
/* 	char *filename = NULL; */
/* 	const char *name = hkl_factory_name_get(priv->diffractometer->factory); */

/* 	if(!strcmp("K6C", name)) */
/* 		filename = get_model("diffabs.yaml"); */
/* 	else if (!strcmp("K4CV", name)) */
/* 		filename = get_model("cristal4C.yaml"); */

/* 	if(priv->frame3d){ */
/* 		gtk_widget_destroy(GTK_WIDGET(hkl_gui_3d_frame_get(priv->frame3d))); */
/* 		g_object_unref(priv->frame3d); */
/* 		priv->frame3d = NULL; */
/* 	} */

/* 	if (filename){ */
/* 		priv->frame3d = hkl_gui_3d_new(filename, priv->diffractometer->geometry); */

/* 		gtk_box_pack_start (GTK_BOX(priv->vbox7), */
/* 				    GTK_WIDGET(hkl_gui_3d_frame_get(priv->frame3d)), */
/* 				    TRUE, TRUE, (guint) 0); */

/* 		gtk_widget_show_all (GTK_WIDGET(priv->vbox7)); */
/* 	} */
/* #endif */
/* } */

static void
factory_notify_error_cb(HklGuiFactory *factory,
			GParamSpec* pspec,
			gpointer *user_data)
{
	HklGuiWindow *self = HKL_GUI_WINDOW(user_data);

	raise_error(self, hkl_gui_factory_get_error(factory));
}

/* select diffractometer */
static void
dropdown1_notify_selected_item_cb(GtkDropDown *dropdown,
				  GParamSpec* pspec,
				  gpointer *user_data)
{
	HklGuiWindow *self = HKL_GUI_WINDOW(user_data);
	HklGuiFactory *factory;

	factory = gtk_drop_down_get_selected_item(dropdown);
	if (NULL != factory) {
		guint i;
		guint n_items;
		GListStore *liststore;
		GtkSingleSelection *single_selection;


		self->factory = factory;

		/* setup spinbutton_wavelength */
		if(self->adjustement_wavelength_binding)
			g_binding_unbind(self->adjustement_wavelength_binding);
		gtk_adjustment_set_value(self->adjustment_wavelength,
					 hkl_gui_factory_get_wavelength(factory));
		gtk_widget_set_sensitive(self->spinbutton_wavelength, TRUE);
		self->adjustement_wavelength_binding = g_object_bind_property(factory, "wavelength",
									      self->adjustment_wavelength, "value",
									      G_BINDING_BIDIRECTIONAL);

		/* sample */
		g_object_bind_property(self->drop_down_samples, "selected-item", factory, "sample", G_BINDING_SYNC_CREATE);

		/* set column view axes model */
		single_selection = GTK_SINGLE_SELECTION(gtk_column_view_get_model(GTK_COLUMN_VIEW(self->column_view_axes)));
		liststore = hkl_gui_factory_get_liststore_axes(factory);
		gtk_single_selection_set_model(single_selection, G_LIST_MODEL(liststore));

		/* set column view pseudo axes model */
		single_selection = GTK_SINGLE_SELECTION(gtk_column_view_get_model(GTK_COLUMN_VIEW(self->column_view_pseudo_axes)));
		liststore = hkl_gui_factory_get_liststore_pseudo_axes(factory);
		gtk_single_selection_set_model(single_selection, G_LIST_MODEL(liststore));

		/* set column view solutions model and columns */
		hkl_gui_factory_setup_column_view_solutions(factory, GTK_COLUMN_VIEW(self->column_view_solutions));

		single_selection = GTK_SINGLE_SELECTION(gtk_column_view_get_model(GTK_COLUMN_VIEW(self->column_view_solutions)));
		liststore = hkl_gui_factory_get_liststore_solutions(factory);
		gtk_single_selection_set_model(single_selection, G_LIST_MODEL(liststore));

		/* add the engines frames to the flowbox engines */
		gtk_flow_box_remove_all(GTK_FLOW_BOX(self->flowbox_engines));
		liststore = hkl_gui_factory_get_liststore_engines(factory);
		n_items = g_list_model_get_n_items(G_LIST_MODEL(liststore));
		for (i=0;i<n_items; ++i){
			HklGuiEngine *gengine = g_list_model_get_item(G_LIST_MODEL(liststore), i);

			gtk_flow_box_append(GTK_FLOW_BOX(self->flowbox_engines),
					    hkl_gui_engine_get_frame(gengine));
		}

		/* set_up_3D(self); */
	}

}

/* select sample */
static void
drop_down_samples_notify_selected_item_cb(GtkDropDown *dropdown,
					  GParamSpec* pspec,
					  gpointer *user_data)
{
	HklGuiWindow *self = HKL_GUI_WINDOW(user_data);
	HklGuiSample *gsample;
	GtkWidget *frame;

	gsample = gtk_drop_down_get_selected_item(dropdown);
	frame = hkl_gui_sample_get_frame(gsample);

	if (self->page_sample == -1){
		self->page_sample = gtk_notebook_append_page (GTK_NOTEBOOK (self->notebook1),
							      frame,
							      NULL);
	}else{
		gtk_notebook_remove_page (GTK_NOTEBOOK (self->notebook1), self->page_sample);
		gtk_notebook_insert_page (GTK_NOTEBOOK (self->notebook1),
					  frame,
					  NULL, self->page_sample);
	}

	gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (self->notebook1),
					 frame,
					 "Crystal Configuration");
}

void
column_view_solutions_activate_cb (GtkColumnView *column_view,
				   guint position,
				   gpointer user_data)
{
	HklGuiWindow* self = HKL_GUI_WINDOW(user_data);
	GtkSingleSelection *selection_model;
	HklGuiGeometry *ggeometry;

	selection_model = GTK_SINGLE_SELECTION(gtk_column_view_get_model(column_view));
	ggeometry = HKL_GUI_GEOMETRY(gtk_single_selection_get_selected_item(selection_model));

	hkl_gui_factory_set_geometry(self->factory, ggeometry);
	/* if (gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_solutions), &iter, path)) { */
	/* 	gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_solutions), &iter, */
	/* 			    SOLUTION_COL_HKL_GEOMETRY_LIST_ITEM, &solution, */
	/* 			    -1); */

	/* 	diffractometer_set_solution(priv->diffractometer, solution); */

	/* 	update_axes (self); */
	/* 	update_pseudo_axes (self); */
	/* 	update_pseudo_axes_frames (self); */
	/* 	update_3d(self); */

	/* 	gtk_tree_path_free (path); */
	/* } */
}

/* /\* reflection h k l *\/ */
/* #define HKL_GUI_WINDOW_CELLRENDERERTEXT_HKL_EDITED_CB(_number, _hkl, _HKL) \ */
/* 	void								\ */
/* 	hkl_gui_window_cellrenderertext ## _number ## _edited_cb(GtkCellRendererText* _sender, const gchar* path, \ */
/* 								 const gchar* new_text, gpointer user_data) \ */
/* 	{								\ */
/* 		HklGuiWindow *self = HKL_GUI_WINDOW(user_data);		\ */
/* 		HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); \ */
/* 									\ */
/* 		g_return_if_fail (self != NULL);			\ */
/* 		g_return_if_fail (path != NULL);			\ */
/* 		g_return_if_fail (new_text != NULL);			\ */
/* 									\ */
/* 		if (priv->sample){					\ */
/* 			gdouble h = 0.0;				\ */
/* 			gdouble k = 0.0;				\ */
/* 			gdouble l = 0.0;				\ */
/* 			HklSampleReflection* reflection = NULL;		\ */
/* 			GtkTreeIter iter = {0};				\ */
/* 			GError *error = NULL;				\ */
/* 									\ */
/* 			gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL(priv->liststore_reflections), \ */
/* 							     &iter, path); \ */
/* 			gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_reflections), \ */
/* 					    &iter,			\ */
/* 					    REFLECTION_COL_REFLECTION, &reflection, \ */
/* 					    -1);			\ */
/* 									\ */
/* 			hkl_sample_reflection_hkl_get (reflection, &h, &k, &l);	\ */
/* 			_hkl = atof(new_text);				\ */
/* 			if(!hkl_sample_reflection_hkl_set (reflection, h, k, l, NULL)) \ */
/* 				raise_error(self, &error);		\ */
/* 			else						\ */
/* 				gtk_list_store_set (priv->liststore_reflections, \ */
/* 						    &iter,		\ */
/* 						    REFLECTION_COL_ ## _HKL, _hkl, \ */
/* 						    -1);		\ */
/* 		}							\ */
/* 	} */

/* HKL_GUI_WINDOW_CELLRENDERERTEXT_HKL_EDITED_CB(7, h, H); */
/* HKL_GUI_WINDOW_CELLRENDERERTEXT_HKL_EDITED_CB(8, k, K); */
/* HKL_GUI_WINDOW_CELLRENDERERTEXT_HKL_EDITED_CB(9, l, L); */


/* /\* reflection flag *\/ */
/* void */
/* hkl_gui_window_cellrenderertoggle1_toggled_cb (GtkCellRendererToggle* renderer, const gchar* path, */
/* 					       gpointer self) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */

/* 	g_return_if_fail (self != NULL); */
/* 	g_return_if_fail (path != NULL); */

/* 	if (priv->sample){ */
/* 		gboolean flag; */
/* 		HklSampleReflection* reflection = NULL; */
/* 		GtkTreeIter iter = {0}; */

/* 		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 						     &iter, path); */
/* 		gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 				    &iter, */
/* 				    REFLECTION_COL_REFLECTION, &reflection, */
/* 				    -1); */

/* 		flag = gtk_cell_renderer_toggle_get_active(renderer); */
/* 		hkl_sample_reflection_flag_set (reflection, flag); */
/* 		gtk_list_store_set (priv->liststore_reflections, */
/* 				    &iter, */
/* 				    REFLECTION_COL_FLAG, flag, */
/* 				    -1); */
/* 	} */
/* } */

/* gboolean */
/* hkl_gui_window_treeview_reflections_key_press_event_cb (GtkWidget* _sender, GdkEvent* event, */
/* 							gpointer self) */
/* { */
/* 	return TRUE; */
/* } */

/* void */
/* hkl_gui_window_toolbutton_add_reflection_clicked_cb(GtkButton* _sender, */
/* 						    gpointer self) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */

/* 	if (priv->diffractometer == NULL){ */
/* 		gtk_statusbar_push (priv->statusbar, 0, */
/* 				    "Please select a diffractometer before adding reflections"); */
/* 		return; */
/* 	} */

/* 	if (priv->sample) { */
/* 		HklSampleReflection *reflection = NULL; */
/* 		GtkTreeIter iter = {0}; */
/* 		gboolean flag; */
/* 		gint n_rows; */
/* 		GError *error = NULL; */

/* 		reflection = hkl_sample_reflection_new(priv->diffractometer->geometry, */
/* 						       priv->diffractometer->detector, */
/* 						       0, 0, 0, &error); */
/* 		if(!reflection) */
/* 			raise_error(self, &error); */
/* 		else{ */
/* 			hkl_sample_add_reflection(priv->sample, reflection); */
/* 			flag = hkl_sample_reflection_flag_get(reflection); */

/* 			n_rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(priv->liststore_reflections), */
/* 								NULL); */
/* 			gtk_list_store_insert_with_values (priv->liststore_reflections, */
/* 							   &iter, -1, */
/* 							   REFLECTION_COL_INDEX, n_rows, */
/* 							   REFLECTION_COL_H, 0., */
/* 							   REFLECTION_COL_K, 0., */
/* 							   REFLECTION_COL_L, 0., */
/* 							   REFLECTION_COL_FLAG, flag, */
/* 							   REFLECTION_COL_REFLECTION, reflection, */
/* 							   -1); */
/* 		} */
/* 	} */
/* } */

/* void */
/* hkl_gui_window_toolbutton_goto_reflection_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */

/* 	g_return_if_fail (self != NULL); */

/* 	if (priv->sample) { */
/* 		GtkTreeSelection* selection = NULL; */
/* 		guint nb_rows = 0U; */

/* 		selection = gtk_tree_view_get_selection (priv->treeview_reflections); */
/* 		nb_rows = gtk_tree_selection_count_selected_rows (selection); */

/* 		if (nb_rows == 1) { */
/* 			HklSampleReflection *reflection; */
/* 			GtkTreeIter iter = {0}; */
/* 			GtkTreeModel* model = NULL; */
/* 			GtkTreePath *treepath; */
/* 			GList* list; */

/* 			model = GTK_TREE_MODEL(priv->liststore_reflections); */

/* 			list = gtk_tree_selection_get_selected_rows (selection, */
/* 								     &model); */

/* 			treepath = g_list_nth_data(list, 0); */

/* 			gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 						 &iter, treepath); */

/* 			gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 					    &iter, */
/* 					    REFLECTION_COL_REFLECTION, &reflection, */
/* 					    -1); */

/* 			hkl_geometry_set (priv->diffractometer->geometry, */
/* 					  hkl_sample_reflection_geometry_get(reflection)); */

/* 			update_source (self); */
/* 			update_axes (self); */
/* 			update_pseudo_axes (self); */
/* 			update_3d(self); */

/* 			g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free); */
/* 		} else */
/* 			if (nb_rows > 1) */
/* 				gtk_statusbar_push (priv->statusbar, 0, */
/* 						    "Please select only one reflection."); */
/* 			else */
/* 				gtk_statusbar_push (priv->statusbar, 0, */
/* 						    "Please select at least one reflection."); */
/* 	} */
/* } */

/* static void */
/* _del_reflection(gpointer data, gpointer user_data) */
/* { */
/* 	HklSampleReflection *reflection; */
/* 	GtkTreeIter iter = {0}; */
/* 	GtkTreePath *treepath = data; */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */

/* 	gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 				 &iter, treepath); */

/* 	gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 			    &iter, */
/* 			    REFLECTION_COL_REFLECTION, &reflection, */
/* 			    -1); */
/* 	hkl_sample_del_reflection(priv->sample, reflection); */
/* } */

/* void */
/* hkl_gui_window_toolbutton_del_reflection_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */

/* 	g_return_if_fail (self != NULL); */

/* 	if (priv->sample) { */
/* 		GtkTreeSelection* selection = NULL; */
/* 		guint nb_rows = 0U; */

/* 		selection = gtk_tree_view_get_selection (priv->treeview_reflections); */
/* 		nb_rows = gtk_tree_selection_count_selected_rows (selection); */
/* 		if (nb_rows > 0) { */
/* 			GtkTreeModel* model = NULL; */
/* 			GList* list; */
/* 			GtkMessageDialog* dialog; */

/* 			model = GTK_TREE_MODEL(priv->liststore_reflections); */
/* 			list = gtk_tree_selection_get_selected_rows (selection, &model); */


/* 			dialog = GTK_MESSAGE_DIALOG( */
/* 				gtk_message_dialog_new (NULL, */
/* 							GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, */
/* 							GTK_MESSAGE_WARNING, */
/* 							GTK_BUTTONS_YES_NO, */
/* 							"Are you sure that you want to delete reflections")); */

/* 			g_signal_connect (dialog, "response", */
/* 					  G_CALLBACK (gtk_window_destroy), */
/* 					  NULL); */

/* 			/\* switch (gtk_dialog_run (GTK_DIALOG(dialog))) { *\/ */
/* 			/\* case GTK_RESPONSE_YES: *\/ */
/* 			/\* { *\/ */
/* 			/\* 	g_list_foreach(list, _del_reflection, self); *\/ */
/* 			/\* 	update_reflections (self); *\/ */
/* 			/\* 	break; *\/ */
/* 			/\* } *\/ */
/* 			/\* default: *\/ */
/* 			/\* 	break; *\/ */
/* 			/\* } *\/ */
/* 			/\* gtk_widget_destroy (GTK_WIDGET(dialog)); *\/ */
/* 			g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free); */
/* 		} else { */
/* 			gtk_statusbar_push (priv->statusbar, 0, */
/* 					    "Please select at least one reflection."); */
/* 		} */
/* 	} */
/* } */

/* static void */
/* set_up_tree_view_reflections(HklGuiWindow *self) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */
/* 	GtkTreeSelection* selection = NULL; */

/* 	selection = gtk_tree_view_get_selection (priv->treeview_reflections); */
/* 	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE); */
/* } */

/* /\* crystal name *\/ */
/* void */
/* hkl_gui_window_cellrenderertext10_edited_cb(GtkCellRendererText* _sender, const gchar* path, */
/* 					    const gchar* new_text, gpointer user_data) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */

/* 	GtkTreeModel* model = NULL; */
/* 	GtkTreeIter iter = {0}; */
/* 	HklSample* sample = NULL; */

/* 	g_return_if_fail (user_data != NULL); */
/* 	g_return_if_fail (path != NULL); */
/* 	g_return_if_fail (new_text != NULL); */

/* 	model = GTK_TREE_MODEL(priv->liststore_crystals); */

/* 	gtk_tree_model_get_iter_from_string (model, &iter, path); */

/* 	gtk_tree_model_get (model, &iter, */
/* 			    SAMPLE_COL_SAMPLE, &sample, */
/* 			    -1); */

/* 	hkl_sample_name_set (sample, new_text); */

/* 	gtk_list_store_set(priv->liststore_crystals, &iter, */
/* 			   SAMPLE_COL_NAME, new_text, */
/* 			   -1); */
/* } */

/* #define set_reciprocal_lattice(lattice, parameter) do{			\ */
/* 		const HklParameter *p;					\ */
/* 		gdouble value;						\ */
/* 		p = hkl_lattice_## parameter ##_get((lattice));		\ */
/* 		value = hkl_parameter_value_get(p, HKL_UNIT_USER);	\ */
/* 		gtk_spin_button_set_value(priv->spinbutton_## parameter ##_star, value); \ */
/* 	}while(0) */

/* static void */
/* update_reciprocal_lattice (HklGuiWindow* self) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */

/* 	g_return_if_fail (self != NULL); */

/* 	if (priv->sample != NULL) { */
/* 		hkl_lattice_reciprocal (hkl_sample_lattice_get(priv->sample), */
/* 					priv->reciprocal); */

/* 		set_reciprocal_lattice(priv->reciprocal, a); */
/* 		set_reciprocal_lattice(priv->reciprocal, b); */
/* 		set_reciprocal_lattice(priv->reciprocal, c); */
/* 		set_reciprocal_lattice(priv->reciprocal, alpha); */
/* 		set_reciprocal_lattice(priv->reciprocal, beta); */
/* 		set_reciprocal_lattice(priv->reciprocal, gamma); */
/* 	} */
/* } */

/* #define set_UB(i, j) do{						\ */
/* 		gdouble	value = hkl_matrix_get(UB, i - 1, j - 1);	\ */
/* 		const char *format = "<tt> %+.4f </tt>";		\ */
/* 		gchar *markup;						\ */
/* 		markup = g_markup_printf_escaped (format, value);	\ */
/* 		gtk_label_set_markup(GTK_LABEL (priv->label_UB ## i ## j), \ */
/* 				     markup);				\ */
/* 		g_free(markup);						\ */
/* 	}while(0) */

/* static void */
/* update_UB (HklGuiWindow* self) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */

/* 	g_return_if_fail (self != NULL); */

/* 	if (priv->sample != NULL) { */
/* 		const HklMatrix *UB = hkl_sample_UB_get (priv->sample); */
/* 		gchar *text = g_new0 (gchar, G_ASCII_DTOSTR_BUF_SIZE); */

/* 		set_UB(1, 1); */
/* 		set_UB(1, 2); */
/* 		set_UB(1, 3); */
/* 		set_UB(2, 1); */
/* 		set_UB(2, 2); */
/* 		set_UB(2, 3); */
/* 		set_UB(3, 1); */
/* 		set_UB(3, 2); */
/* 		set_UB(3, 3); */

/* 		g_free(text); */
/* 	} */
/* } */

/* void */
/* hkl_gui_window_treeview_crystals_cursor_changed_cb (GtkTreeView* _sender, gpointer user_data) */
/* { */
/* 	GtkTreePath* path = NULL; */
/* 	GtkTreeViewColumn* column = NULL; */
/* 	GtkTreeIter iter = {0}; */

/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	HklSample *sample; */

/* 	g_return_if_fail (user_data != NULL); */

/* 	gtk_tree_view_get_cursor (priv->treeview_crystals, &path, &column); */
/* 	if(path){ */
/* 		if (gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_crystals), */
/* 					     &iter, path) == TRUE){ */
/* 			gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_crystals), */
/* 					    &iter, */
/* 					    SAMPLE_COL_SAMPLE, &sample, */
/* 					    -1); */

/* 			if(sample && sample != priv->sample){ */
/* 				priv->sample = sample; */

/* 				update_reflections(self); */
/* 				update_lattice(self); */
/* 				update_reciprocal_lattice (self); */
/* 				update_ux_uy_uz (self); */
/* 				update_UB (self); */

/* 				if(priv->diffractometer){ */
/* 					diffractometer_set_sample(priv->diffractometer, */
/* 								  priv->sample); */

/* 					update_pseudo_axes (self); */
/* 					update_pseudo_axes_frames (self); */
/* 					update_solutions(self); */
/* 				} */
/* 			} */
/* 		} */
/* 		gtk_tree_path_free (path); */
/* 	} */
/* } */



/* static GtkTreeIter */
/* _add_sample(HklGuiWindow *self, HklSample *sample) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */
/* 	GtkTreeIter iter = {0}; */
/* 	const HklLattice *lattice; */
/* 	gdouble a, b, c, alpha, beta, gamma; */

/* 	g_return_val_if_fail (self != NULL, iter); */

/* 	lattice = hkl_sample_lattice_get(sample); */
/* 	a = hkl_parameter_value_get(hkl_lattice_a_get(lattice), HKL_UNIT_USER); */
/* 	b = hkl_parameter_value_get(hkl_lattice_b_get(lattice), HKL_UNIT_USER); */
/* 	c = hkl_parameter_value_get(hkl_lattice_c_get(lattice), HKL_UNIT_USER); */
/* 	alpha = hkl_parameter_value_get(hkl_lattice_alpha_get(lattice), */
/* 					HKL_UNIT_USER); */
/* 	beta = hkl_parameter_value_get(hkl_lattice_beta_get(lattice), */
/* 				       HKL_UNIT_USER); */
/* 	gamma = hkl_parameter_value_get(hkl_lattice_gamma_get(lattice), */
/* 					HKL_UNIT_USER); */

/* 	gtk_list_store_insert_with_values(priv->liststore_crystals, */
/* 					  &iter, -1, */
/* 					  SAMPLE_COL_SAMPLE, sample, */
/* 					  SAMPLE_COL_NAME, hkl_sample_name_get(sample), */
/* 					  SAMPLE_COL_A, a, */
/* 					  SAMPLE_COL_B, b, */
/* 					  SAMPLE_COL_C, c, */
/* 					  SAMPLE_COL_ALPHA, alpha, */
/* 					  SAMPLE_COL_BETA, beta, */
/* 					  SAMPLE_COL_GAMMA, gamma, */
/* 					  -1); */
/* 	return iter; */
/* } */

/* static void */
/* _add_sample_and_edit_name(HklGuiWindow *self, HklSample *sample) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(self); */
/* 	GtkTreeIter iter = {0}; */
/* 	GtkTreePath* path = NULL; */
/* 	GtkTreeViewColumn* column = NULL; */

/* 	iter = _add_sample(self, sample); */

/* 	path = gtk_tree_model_get_path(GTK_TREE_MODEL(priv->liststore_crystals), */
/* 				       &iter); */
/* 	column = gtk_tree_view_get_column (priv->treeview_crystals, 0); */
/* 	gtk_tree_view_set_cursor (priv->treeview_crystals, path, column, TRUE); */

/* 	gtk_tree_path_free(path); */
/* } */

/* void */
/* hkl_gui_window_toolbutton_add_crystal_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklSample *sample; */

/* 	g_return_if_fail (user_data != NULL); */

/* 	sample = hkl_sample_new ("new"); */
/* 	if(sample) */
/* 		_add_sample_and_edit_name(self, sample); */
/* } */

/* void */
/* hkl_gui_window_toolbutton_copy_crystal_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	HklSample *copy = NULL; */

/* 	g_return_if_fail (self != NULL); */

/* 	if(priv->sample) { */
/* 		copy = hkl_sample_new_copy(priv->sample); */
/* 		if (copy) */
/* 			_add_sample_and_edit_name(self, copy); */
/* 	}else */
/* 		gtk_statusbar_push (priv->statusbar, (guint) 0, "Please select a crystal to copy."); */
/* } */

/* void */
/* hkl_gui_window_toolbutton_del_crystal_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */

/* 	g_return_if_fail (user_data != NULL); */

/* 	if (priv->sample != NULL) { */
/* 		guint n_rows; */

/* 		n_rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(priv->liststore_crystals), */
/* 							NULL ); */
/* 		if (n_rows == 1) */
/* 			return; */
/* 		else { */
/* 			GtkTreeIter iter = {0}; */
/* 			GtkTreePath *path = NULL; */
/* 			GtkTreeViewColumn *column = NULL; */

/* 			gtk_tree_view_get_cursor(priv->treeview_crystals, */
/* 						 &path, &column); */
/* 			if (path){ */
/* 				if (gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_crystals), */
/* 							     &iter, path) == TRUE) { */
/* 					gtk_tree_path_free(path); */

/* 					hkl_sample_free(priv->sample); */
/* 					if (gtk_list_store_remove(priv->liststore_crystals, */
/* 								  &iter) == TRUE){ */
/* 						path = gtk_tree_model_get_path(GTK_TREE_MODEL(priv->liststore_crystals), */
/* 									       &iter); */
/* 						gtk_tree_view_set_cursor(priv->treeview_crystals, */
/* 									 path, NULL, FALSE); */
/* 					} */
/* 				} */
/* 			} */
/* 		} */
/* 	} */
/* } */


/* void */
/* hkl_gui_window_spinbutton_ux_value_changed_cb (GtkSpinButton *_senser, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	GError *error = NULL; */

/* 	get_ux_uy_uz(priv->sample, ux, &error); */

/* 	if(priv->diffractometer) */
/* 		diffractometer_set_sample(priv->diffractometer, */
/* 					  priv->sample); */

/* 	update_UB (self); */
/* 	update_pseudo_axes (self); */
/* 	update_pseudo_axes_frames (self); */
/* } */

/* void */
/* hkl_gui_window_spinbutton_uy_value_changed_cb (GtkSpinButton *_senser, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	GError *error = NULL; */

/* 	get_ux_uy_uz(priv->sample, uy, &error); */

/* 	if(priv->diffractometer) */
/* 		diffractometer_set_sample(priv->diffractometer, */
/* 					  priv->sample); */

/* 	update_UB (self); */
/* 	update_pseudo_axes (self); */
/* 	update_pseudo_axes_frames (self); */
/* } */

/* void */
/* hkl_gui_window_spinbutton_uz_value_changed_cb (GtkSpinButton *_senser, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	GError *error = NULL; */

/* 	get_ux_uy_uz(priv->sample, uz, &error); */

/* 	if(priv->diffractometer) */
/* 		diffractometer_set_sample(priv->diffractometer, */
/* 					  priv->sample); */

/* 	update_UB (self); */
/* 	update_pseudo_axes (self); */
/* 	update_pseudo_axes_frames (self); */
/* } */

/* void */
/* hkl_gui_window_toolbutton_setUB_clicked_cb(GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */

/* 	HklMatrix *UB; */
/* 	GError *error = NULL; */

/* 	UB = hkl_matrix_new_full(gtk_spin_button_get_value(priv->spinbutton_U11), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U12), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U13), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U21), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U22), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U23), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U31), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U32), */
/* 				 gtk_spin_button_get_value(priv->spinbutton_U33)); */

/* 	if(!hkl_sample_UB_set (priv->sample, UB, &error)) */
/* 		raise_error(self, &error); */
/* 	else{ */
/* 		if(priv->diffractometer){ */
/* 			diffractometer_set_sample(priv->diffractometer, */
/* 						  priv->sample); */

/* 			update_lattice (self); */
/* 			update_crystal_model (self); */
/* 			update_reciprocal_lattice (self); */
/* 			update_UB (self); */
/* 			update_ux_uy_uz (self); */
/* 			update_pseudo_axes (self); */
/* 			update_pseudo_axes_frames (self); */
/* 		} */
/* 	} */

/* 	hkl_matrix_free(UB); */
/* } */

/* void */
/* hkl_gui_window_toolbutton_computeUB_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	GtkTreeSelection* selection = NULL; */
/* 	guint nb_rows = 0U; */

/* 	selection = gtk_tree_view_get_selection (priv->treeview_reflections); */
/* 	nb_rows = gtk_tree_selection_count_selected_rows (selection); */
/* 	if (nb_rows > 1) { */
/* 		GtkTreeModel* model = NULL; */
/* 		GList* list; */
/* 		GtkTreeIter iter = {0}; */
/* 		GtkTreePath *path; */
/* 		HklSampleReflection *ref1, *ref2; */
/* 		GError *error = NULL; */

/* 		model = GTK_TREE_MODEL(priv->liststore_reflections); */
/* 		list = gtk_tree_selection_get_selected_rows (selection, &model); */

/* 		/\* get the first reflection *\/ */
/* 		path = g_list_nth_data(list, 0); */
/* 		gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 					 &iter, */
/* 					 path); */
/* 		gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_reflections), &iter, */
/* 				    REFLECTION_COL_REFLECTION, &ref1, */
/* 				    -1); */

/* 		/\* get the second one *\/ */
/* 		path = g_list_nth_data(list, 1); */
/* 		gtk_tree_model_get_iter (GTK_TREE_MODEL(priv->liststore_reflections), */
/* 					 &iter, */
/* 					 path); */
/* 		gtk_tree_model_get (GTK_TREE_MODEL(priv->liststore_reflections), &iter, */
/* 				    REFLECTION_COL_REFLECTION, &ref2, */
/* 				    -1); */

/* 		if(!hkl_sample_compute_UB_busing_levy(priv->sample, */
/* 						      ref1, ref2, &error)){ */
/* 			raise_error(self, &error); */
/* 		}else{ */
/* 			if(priv->diffractometer) */
/* 				diffractometer_set_sample(priv->diffractometer, */
/* 							  priv->sample); */

/* 			update_UB (self); */
/* 			update_ux_uy_uz (self); */
/* 			update_pseudo_axes (self); */
/* 			update_pseudo_axes_frames (self); */
/* 		} */
/* 		g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free); */
/* 	} else { */
/* 		gtk_statusbar_push (priv->statusbar, 0, */
/* 				    "Please select at least two reflection."); */
/* 	} */
/* } */

/* void */
/* hkl_gui_window_toolbutton_affiner_clicked_cb (GtkButton* _sender, gpointer user_data) */
/* { */
/* 	HklGuiWindow *self = HKL_GUI_WINDOW(user_data); */
/* 	HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); */
/* 	GError *error = NULL; */

/* 	if(!hkl_sample_affine (priv->sample, &error)){ */
/* 		raise_error(self, &error); */
/* 	}else{ */
/* 		if(priv->diffractometer) */
/* 			diffractometer_set_sample(priv->diffractometer, */
/* 						  priv->sample); */

/* 		update_lattice (self); */
/* 		update_crystal_model (self); */
/* 		update_reciprocal_lattice (self); */
/* 		update_UB (self); */
/* 		update_ux_uy_uz (self); */
/* 		update_pseudo_axes (self); */
/* 		update_pseudo_axes_frames (self); */
/* 	} */
/* } */

/* #define TOGGLE_LATTICE_CB(_parameter)					\ */
/* 	void hkl_gui_window_checkbutton_ ## _parameter ## _toggled_cb(GtkCheckButton *checkbutton, \ */
/* 								      gpointer user_data) \ */
/* 	{								\ */
/* 		HklGuiWindow *self = HKL_GUI_WINDOW(user_data);		\ */
/* 		HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); \ */
/* 		HklLattice *lattice;					\ */
/* 		HklParameter *p;					\ */
/* 		GError *error = NULL;					\ */
/* 		lattice = hkl_lattice_new_copy(hkl_sample_lattice_get(priv->sample)); \ */
/* 		p = hkl_parameter_new_copy(hkl_lattice_## _parameter ##_get(lattice)); \ */
/* 		hkl_parameter_fit_set(p,				\ */
/* 				      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton))); \ */
/* 		if(!hkl_lattice_## _parameter ##_set(lattice, p, &error)){ \ */
/* 			raise_error(self, &error);			\ */
/* 		}else{							\ */
/* 			hkl_sample_lattice_set(priv->sample, lattice);	\ */
/* 		}							\ */
/* 		hkl_parameter_free(p);					\ */
/* 		hkl_lattice_free(lattice);				\ */
/* 	} */

/* TOGGLE_LATTICE_CB(a); */
/* TOGGLE_LATTICE_CB(b); */
/* TOGGLE_LATTICE_CB(c); */
/* TOGGLE_LATTICE_CB(alpha); */
/* TOGGLE_LATTICE_CB(beta); */
/* TOGGLE_LATTICE_CB(gamma); */

/* #define TOGGLE_UX_UY_UZ(_parameter)					\ */
/* 	void hkl_gui_window_checkbutton_ ## _parameter ## _toggled_cb(GtkCheckButton *checkbutton, \ */
/* 								      gpointer user_data) \ */
/* 	{								\ */
/* 		HklGuiWindow *self = HKL_GUI_WINDOW(user_data);		\ */
/* 		HklGuiWindowPrivate *priv = hkl_gui_window_get_instance_private(user_data); \ */
/* 		HklParameter *p;					\ */
/* 		GError *error = NULL;					\ */
/* 		p = hkl_parameter_new_copy(hkl_sample_ ## _parameter ## _get(priv->sample)); \ */
/* 		hkl_parameter_fit_set(p,				\ */
/* 				      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton))); \ */
/* 		if(!hkl_sample_ ## _parameter ## _set(priv->sample, p, &error)){ \ */
/* 			raise_error(self, &error);			\ */
/* 		}							\ */
/* 		hkl_parameter_free(p);					\ */
/* 	} */

/* TOGGLE_UX_UY_UZ(ux); */
/* TOGGLE_UX_UY_UZ(uy); */
/* TOGGLE_UX_UY_UZ(uz); */

/* /\* */

/*   static gboolean _hkl_gui_window_on_tree_view_crystals_key_press_event_gtk_widget_key_press_event (GtkWidget* _sender, GdkEventKey* event, gpointer self) { */
/*   gboolean result; */
/*   result = hkl_gui_window_on_tree_view_crystals_key_press_event (event, self); */

/*   return result; */

/*   } */

/*   static gboolean hkl_gui_window_on_tree_view_crystals_key_press_event (GdkEventKey* event, HklGuiWindow* self) { */
/*   gboolean result = FALSE; */

/*   g_return_val_if_fail (self != NULL, FALSE); */

/*   g_return_val_if_fail (event != NULL, FALSE); */

/*   result = TRUE; */

/*   return result; */

/*   } */

/* *\/ */


static GtkListItemFactory *
hkl_gui_item_factory_new_label_property(char *property)
{
	GtkListItemFactory *item_factory;

	item_factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), NULL);
	g_signal_connect (item_factory, "bind", G_CALLBACK (hkl_gui_bind_item_factory_label_property_cb), property);

	return item_factory;
}

static void
hkl_gui_column_view_add_column_property(GtkColumnView *column_view, char *property)
{
	GtkListItemFactory *item_factory = hkl_gui_item_factory_new_label_property(property);
	GtkColumnViewColumn *column = gtk_column_view_column_new(property, item_factory);

	gtk_column_view_append_column(column_view, column);
}

static void
new_window (GApplication *app,
            GFile        *file)
{
	HklFactory **factories;
	size_t i, n;

	GListStore *liststore1;

	GtkColumnViewColumn *column;
	GtkListItemFactory *item_factory_drop_down_factories;
	GtkListItemFactory *item_factory_drop_down_samples;

	GtkWidget *dropdown1;
	GtkWidget *frame_diffractometer;
	GtkWidget *frame_wavelength;
	GtkWidget *frame_axes;
	GtkWidget *frame_pseudo_axes;
	GtkWidget *frame_samples;
	GtkWidget *frame_solutions;
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *vbox1;
	GtkWidget *vbox2;

	HklGuiWindow *self = HKL_GUI_WINDOW(app);

        /**********/
	/* Models */
	/**********/

	/* liststore1 factories */
	liststore1 = g_list_store_new (HKL_GUI_TYPE_FACTORY);
	factories = hkl_factory_get_all(&n);
	for(i=0; i<n; ++i){
		HklGuiFactory *factory = hkl_gui_factory_new(factories[i]);
		g_signal_connect (factory, "notify::error",
				  G_CALLBACK (factory_notify_error_cb), self);
		g_list_store_append (liststore1, factory);
	}

	/* liststore samples */
	self->liststore_samples = g_list_store_new (HKL_GUI_TYPE_SAMPLE);
	g_list_store_append(self->liststore_samples, hkl_gui_sample_new("default"));
	g_list_store_append(self->liststore_samples, hkl_gui_sample_new("tutu"));

	/*********************/
	/* ListItemFactories */
	/*********************/

	/* drop down factories */

	item_factory_drop_down_factories = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory_drop_down_factories, "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), NULL);
	g_signal_connect (item_factory_drop_down_factories, "bind", G_CALLBACK (hkl_gui_bind_item_factory_label_property_cb), "name");

	/* drop down samples */

	item_factory_drop_down_samples = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory_drop_down_samples,
			  "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), NULL);
	g_signal_connect (item_factory_drop_down_samples,
			  "bind", G_CALLBACK (hkl_gui_bind_item_factory_label_property_cb), "name");

	/***********/
	/* widgets */
	/***********/

	self->alert_dialog_solutions = gtk_alert_dialog_new("Solutions");
	self->column_view_axes = gtk_column_view_new(GTK_SELECTION_MODEL(gtk_single_selection_new(NULL)));
	self->column_view_pseudo_axes = gtk_column_view_new(GTK_SELECTION_MODEL(gtk_single_selection_new(NULL)));
	self->column_view_samples = gtk_column_view_new (GTK_SELECTION_MODEL (gtk_single_selection_new( G_LIST_MODEL (self->liststore_samples))));
	self->column_view_solutions = gtk_column_view_new(GTK_SELECTION_MODEL(gtk_single_selection_new(NULL)));
	self->drop_down_samples = gtk_drop_down_new(G_LIST_MODEL(self->liststore_samples), NULL);
	self->flowbox_engines = gtk_flow_box_new();
	self->notebook1 = gtk_notebook_new();
	self->spinbutton_wavelength = gtk_spin_button_new(self->adjustment_wavelength, 0.0001, 4);
	self->window1 = gtk_application_window_new (GTK_APPLICATION (app));

	dropdown1 = gtk_drop_down_new(G_LIST_MODEL(liststore1), NULL);
	frame_diffractometer = gtk_frame_new("Diffractometer");
	frame_wavelength = gtk_frame_new("Wavelength");
	frame_axes = gtk_frame_new("Axes");
	frame_pseudo_axes = gtk_frame_new("Pseudo Axes");
	frame_samples = gtk_frame_new("Samples");
	frame_solutions = gtk_frame_new("Solutions");
	hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	scrolledwindow1 = gtk_scrolled_window_new();
	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* column view axes */
	column = gtk_column_view_column_new("name", hkl_gui_parameter_factory_name_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_axes), column);
	column = gtk_column_view_column_new("value", hkl_gui_parameter_factory_value_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_axes), column);
	column = gtk_column_view_column_new("min", hkl_gui_parameter_factory_min_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_axes), column);
	column = gtk_column_view_column_new("max", hkl_gui_parameter_factory_max_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_axes), column);

	/* column view pseudo axes */
	column = gtk_column_view_column_new("name", hkl_gui_parameter_factory_name_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_pseudo_axes), column);
	column = gtk_column_view_column_new("value", hkl_gui_parameter_factory_value_label_new());
	gtk_column_view_append_column(GTK_COLUMN_VIEW(self->column_view_pseudo_axes), column);

	/* column view samples */
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "name");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "a");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "b");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "c");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "alpha");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "beta");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "gamma");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "ux");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "uy");
	hkl_gui_column_view_add_column_property (GTK_COLUMN_VIEW (self->column_view_samples), "uz");

	/* column view solutions */
	g_signal_connect (self->column_view_solutions, "activate", G_CALLBACK (column_view_solutions_activate_cb), self);

	/* dropdown1 */
	gtk_drop_down_set_factory(GTK_DROP_DOWN(dropdown1), item_factory_drop_down_factories);
	g_signal_connect (dropdown1, "notify::selected-item", G_CALLBACK (dropdown1_notify_selected_item_cb), self);

	/* drop_down_samples */
	gtk_drop_down_set_factory(GTK_DROP_DOWN(self->drop_down_samples), item_factory_drop_down_samples);
	g_signal_connect (self->drop_down_samples, "notify::selected-item",
			  G_CALLBACK (drop_down_samples_notify_selected_item_cb), self);

	/* flowbox engines */
	gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(self->flowbox_engines), FALSE);
	gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(self->flowbox_engines),
					GTK_SELECTION_NONE);

	/* frame diffractometer */
	gtk_frame_set_child(GTK_FRAME(frame_diffractometer), dropdown1);

	/* frame wavelength */
	gtk_frame_set_child(GTK_FRAME(frame_wavelength), self->spinbutton_wavelength);

	/* frame axes */
	gtk_frame_set_child(GTK_FRAME(frame_axes), self->column_view_axes);

        /* frame pseudo axes */
	// gtk_frame_set_child(GTK_FRAME(frame_pseudo_axes), scrolledwindow1);
	gtk_frame_set_child(GTK_FRAME(frame_pseudo_axes), self->column_view_pseudo_axes);

	/* frame samples */
	gtk_frame_set_child(GTK_FRAME(frame_samples), self->drop_down_samples);

	/* frame solutions*/
	gtk_frame_set_child(GTK_FRAME(frame_solutions), self->column_view_solutions);

	/* notebook1 */
	gtk_notebook_append_page (GTK_NOTEBOOK (self->notebook1),
				  scrolledwindow1,
				  NULL);
	gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (self->notebook1),
					 scrolledwindow1,
					 "Pseudo Axes");
	gtk_notebook_append_page (GTK_NOTEBOOK (self->notebook1),
				  self->column_view_samples,
				  NULL);
	gtk_widget_set_hexpand(self->notebook1, true);

	/* scrolledwindow1 */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow1),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolledwindow1), self->flowbox_engines);

	/* spinbutton_wavelength */
	gtk_widget_set_sensitive(self->spinbutton_wavelength, FALSE);

	/* vbox1 */
	gtk_box_append(GTK_BOX(vbox1), hbox1);
	gtk_box_set_homogeneous(GTK_BOX (vbox1), true);

	/* vbox2 */
	gtk_box_append(GTK_BOX(vbox2), frame_diffractometer);
	gtk_box_append(GTK_BOX(vbox2), frame_wavelength);
	gtk_box_append(GTK_BOX(vbox2), frame_samples);
	gtk_box_append(GTK_BOX(vbox2), frame_axes);
	gtk_box_append(GTK_BOX(vbox2), frame_solutions);

	/* hbox1 */
	gtk_box_append(GTK_BOX(hbox1), vbox2);
	gtk_box_append(GTK_BOX(hbox1), self->notebook1);


	/* gtk_paned_set_start_child (GTK_PANED (hpaned1), vbox2); */
	/* gtk_paned_set_shrink_start_child (GTK_PANED (hpaned1), false); */

	/* gtk_paned_set_end_child (GTK_PANED (hpaned1), notebook1); */
	/* gtk_paned_set_shrink_end_child (GTK_PANED (hpaned1), false); */

	/*  window1 */
	gtk_window_set_default_size (GTK_WINDOW(self->window1), 1024, 768);
	/* g_action_map_add_action_entries (G_ACTION_MAP (window), win_entries, G_N_ELEMENTS (win_entries), window); */
	gtk_window_set_title (GTK_WINDOW (self->window1), "hkl library GUI");
	gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self->window1), TRUE);
	gtk_window_set_child (GTK_WINDOW (self->window1), vbox1);

	gtk_window_present (GTK_WINDOW (self->window1));
}

static void hkl_gui_window_startup (GApplication *application)
{
	G_APPLICATION_CLASS (hkl_gui_window_parent_class)->startup (application);

  /* priv->diffractometer = NULL; */
  /* priv->sample = hkl_sample_new("default"); */

  /* darray_init(priv->pseudo_frames); */

  /* priv->reciprocal = hkl_lattice_new_default (); */

  /* hkl_gui_window_get_widgets_and_objects_from_ui (ghkl); */

  /* set_up_diffractometer_model (ghkl); */

  /* set_up_tree_view_crystals (ghkl); */

  /* set_up_tree_view_reflections(ghkl); */
}

static void
hkl_gui_window_shutdown (GApplication *application)
{
	G_APPLICATION_CLASS (hkl_gui_window_parent_class)->shutdown (application);
}


static void hkl_gui_window_activate (GApplication *application)
{
	G_APPLICATION_CLASS (hkl_gui_window_parent_class)->activate (application);
	new_window (application, NULL);
}

static void hkl_gui_window_open (GApplication  *application,
				 GFile        **files,
				 int            n_files,
				 const char    *hint)
{
}

static void hkl_gui_window_init (HklGuiWindow * self)
{
	self->adjustment_wavelength = gtk_adjustment_new (0.0, 0.0, G_MAXDOUBLE,
							  0.0001, 0.01, 0.0);
	self->adjustement_wavelength_binding = NULL;

	self->page_sample = -1;
}

static void hkl_gui_window_class_init (HklGuiWindowClass *class)
{
	GApplicationClass *application_class = G_APPLICATION_CLASS (class);
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	application_class->startup = hkl_gui_window_startup;
	application_class->shutdown = hkl_gui_window_shutdown;
	application_class->activate = hkl_gui_window_activate;
	application_class->open = hkl_gui_window_open;

	/* virtual method */
	object_class->finalize = finalize;
}

static HklGuiWindow* hkl_gui_window_new (void)
{
	HklGuiWindow *ghkl;

	g_set_application_name ("Ghkl");
	ghkl = g_object_new (hkl_gui_window_get_type (),
			     "application-id", "fr.synchrotron-soleil.ghkl",
			     "flags", G_APPLICATION_HANDLES_OPEN,
			     "inactivity-timeout", 30000,
			     "register-session", TRUE,
			     NULL);
	return ghkl;
}

int main (int argc, char ** argv)
{
	HklGuiWindow *ghkl;
	int status;
	const char *accels[] = { "F11", NULL };

	ghkl = hkl_gui_window_new ();
	gtk_application_set_accels_for_action (GTK_APPLICATION (ghkl),
					       "win.fullscreen", accels);
	status = g_application_run (G_APPLICATION (ghkl), argc, argv);
	g_object_unref (ghkl);

	return status;
}
