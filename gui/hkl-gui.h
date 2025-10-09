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
 * Copyright (C) 2003-2019, 2024, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "hkl.h"

#define HKL_GUI_TYPE_ENGINE (hkl_gui_engine_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiEngine, hkl_gui_engine, HKL_GUI, ENGINE, GObject)

#define HKL_GUI_TYPE_FACTORY (hkl_gui_factory_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiFactory, hkl_gui_factory, HKL_GUI, FACTORY, GObject)

#define HKL_GUI_TYPE_GEOMETRY (hkl_gui_geometry_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiGeometry, hkl_gui_geometry, HKL_GUI, GEOMETRY, GObject)

#define HKL_GUI_TYPE_PARAMETER (hkl_gui_parameter_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiParameter, hkl_gui_parameter, HKL_GUI, PARAMETER, GObject)

#define HKL_GUI_TYPE_SAMPLE (hkl_gui_sample_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiSample, hkl_gui_sample, HKL_GUI, SAMPLE, GObject)

#define HKL_GUI_TYPE_SAMPLE_REFLECTION (hkl_gui_sample_reflection_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiSampleReflection, hkl_gui_sample_reflection, HKL_GUI, SAMPLE_REFLECTION, GObject)

#define HKL_GUI_TYPE_WINDOW (hkl_gui_window_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiWindow, hkl_gui_window, HKL_GUI, WINDOW, GtkApplication)

/**********/
/* engine */
/**********/

HKLAPI HklGuiEngine* hkl_gui_engine_new (HklEngine* engine);

HKLAPI HklEngine* hkl_gui_engine_get_engine (HklGuiEngine *gui_engine);

HKLAPI GError* hkl_gui_engine_get_error (HklGuiEngine *gui_engine);

HKLAPI GtkWidget* hkl_gui_engine_get_frame(HklGuiEngine *self);

HKLAPI gboolean hkl_gui_engine_get_initialized (HklGuiEngine *gui_engine);

HKLAPI const char* hkl_gui_engine_get_mode (HklGuiEngine *gui_engine);

HKLAPI HklGeometryList *hkl_gui_engine_get_solutions(HklGuiEngine *self);

HKLAPI void hkl_gui_engine_set_engine (HklGuiEngine *gui_engine,
				       HklEngine *engine);

HKLAPI void hkl_gui_engine_set_initialized (HklGuiEngine *gui_engine,
					    gboolean initialized);

HKLAPI void hkl_gui_engine_set_mode (HklGuiEngine *gui_engine,
				     const char *mode);

HKLAPI void hkl_gui_engine_update (HklGuiEngine* self);

/***********/
/* factory */
/***********/

HKLAPI HklGuiFactory* hkl_gui_factory_new(const HklFactory *factory);

HKLAPI void hkl_gui_factory_add_reflection(HklGuiFactory *self, HklGuiSample *sample);
HKLAPI void hkl_gui_factory_del_reflection(HklGuiFactory *self, HklGuiSample *sample, gint idx);

HKLAPI struct diffractometer_t* hkl_gui_factory_get_diffractometer(HklGuiFactory *self);

HKLAPI GError* hkl_gui_factory_get_error(HklGuiFactory *self);

HKLAPI HklFactory* hkl_gui_factory_get_factory(HklGuiFactory *self);

HKLAPI const char * hkl_gui_factory_get_name(HklGuiFactory *self);

HKLAPI HklGuiSample * hkl_gui_factory_get_sample(HklGuiFactory *self);

HKLAPI gdouble hkl_gui_factory_get_wavelength(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_axes(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_engines(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_pseudo_axes(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_solutions(HklGuiFactory *self);

HKLAPI void hkl_gui_factory_set_error(HklGuiFactory *self, GError* error);

HKLAPI void hkl_gui_factory_set_geometry(HklGuiFactory *self, HklGuiGeometry *ggeometry);

HKLAPI void hkl_gui_factory_set_sample(HklGuiFactory *self, HklGuiSample *sample);

HKLAPI void hkl_gui_factory_set_wavelength(HklGuiFactory *self, gdouble wavelength);

HKLAPI void hkl_gui_factory_setup_column_view_sample_reflections(HklGuiFactory *self, GtkColumnView *column_view);

HKLAPI void hkl_gui_factory_setup_column_view_solutions(HklGuiFactory *self, GtkColumnView *column_view);

/* geometry */

HKLAPI HklGuiGeometry* hkl_gui_geometry_new(const HklGeometry *geometry);

HKLAPI HklGeometry* hkl_gui_geometry_get_geometry(HklGuiGeometry *geometry);

HKLAPI gdouble hkl_gui_geometry_get_axis_value(HklGuiGeometry *geometry, gint idx);

/*************/
/* parameter */
/*************/

HKLAPI HklGuiParameter* hkl_gui_parameter_new(const HklParameter *parameter);

HKLAPI gdouble hkl_gui_parameter_get_maximum(HklGuiParameter *self);
HKLAPI gdouble hkl_gui_parameter_get_minimum(HklGuiParameter *self);
HKLAPI const char * hkl_gui_parameter_get_name(HklGuiParameter *self);
HKLAPI gdouble hkl_gui_parameter_get_value(HklGuiParameter *self);

HKLAPI void hkl_gui_parameter_set_maximum(HklGuiParameter *self, gdouble value);
HKLAPI void hkl_gui_parameter_set_minimum(HklGuiParameter *self, gdouble value);
HKLAPI void hkl_gui_parameter_set_value(HklGuiParameter *self, gdouble value);

HKLAPI void hkl_gui_parameter_update(HklGuiParameter *self);

/**********/
/* Sample */
/**********/

HKLAPI HklGuiSample* hkl_gui_sample_new(const char *name);
HKLAPI HklGuiSample* hkl_gui_sample_new_copy(const HklGuiSample *gsample);

/* get */
HKLAPI const char *hkl_gui_sample_get_name(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_a(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_b(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_c(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_alpha(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_beta(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_gamma(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_ux(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_uy(HklGuiSample *self);
HKLAPI gdouble hkl_gui_sample_get_uz(HklGuiSample *self);
HKLAPI GListStore *hkl_gui_sample_get_reflections(HklGuiSample *self);
HKLAPI HklSample *hkl_gui_sample_get_sample(HklGuiSample *self);

/* set */
HKLAPI void hkl_gui_sample_set_name(HklGuiSample *self, const char *name);
HKLAPI void hkl_gui_sample_set_a(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_b(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_c(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_alpha(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_beta(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_gamma(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_ux(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_uy(HklGuiSample *self, gdouble value);
HKLAPI void hkl_gui_sample_set_uz(HklGuiSample *self, gdouble value);

/* methods */
HKLAPI void hkl_gui_sample_add_reflection(HklGuiSample *self,
					  HklGeometry *goemetry, HklDetector *detector,
					  gdouble h, gdouble k, gdouble l);
HKLAPI void hkl_gui_sample_del_reflection(HklGuiSample *self, gint idx);

/*********************/
/* Sample Reflection */
/*********************/

HKLAPI HklGuiSampleReflection* hkl_gui_sample_reflection_new(HklSampleReflection *reflection);

HklSampleReflection *hkl_gui_sample_reflection_get_reflection(HklGuiSampleReflection* self);
HKLAPI gdouble hkl_gui_sample_reflection_get_h(HklGuiSampleReflection* self);
HKLAPI gdouble hkl_gui_sample_reflection_get_k(HklGuiSampleReflection* self);
HKLAPI gdouble hkl_gui_sample_reflection_get_l(HklGuiSampleReflection* self);

void hkl_gui_sample_reflection_set_reflection(HklGuiSampleReflection* self, HklSampleReflection *reflection);
HKLAPI void hkl_gui_sample_reflection_set_h(HklGuiSampleReflection* self, gdouble h);
HKLAPI void hkl_gui_sample_reflection_set_k(HklGuiSampleReflection* self, gdouble k);
HKLAPI void hkl_gui_sample_reflection_set_l(HklGuiSampleReflection* self, gdouble l);

/******************/
/* item factories */
/******************/

/* gui geometry */

HKLAPI GtkListItemFactory* hkl_gui_geometry_axis_value_factory_new(gint idx);

HKLAPI void hkl_gui_bind_item_factory_label__geometry_axis_value_cb(GtkListItemFactory *factory,
								    GtkListItem *list_item,
								    gpointer user_data);

/* gobject */
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_entry_property(char *property);
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_entry_numeric_property(char *property);
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_label_property(char *property);
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_spin_button_vertical_property(char *property);

/* parameter */
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_spin_button_parameter_value(void);
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_spin_button_parameter_min(void);
HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_spin_button_parameter_max(void);

/* sample reflection */

HKLAPI GtkListItemFactory *hkl_gui_item_factory_new_label__sample_reflection_geometry_axis(int idx);

HKLAPI void hkl_gui_setup_item_factory_entry_cb (GtkListItemFactory *factory,
						 GtkListItem *list_item);

HKLAPI void hkl_gui_setup_item_factory_label_cb (GtkListItemFactory *factory,
						 GtkListItem *list_item);

HKLAPI void hkl_gui_setup_item_factory_spin_button_cb (GtkListItemFactory *factory,
						       GtkListItem *list_item);

HKLAPI void hkl_gui_setup_item_factory_spin_button_vertical_cb (GtkListItemFactory *factory,
								GtkListItem *list_item);

HKLAPI void hkl_gui_bind_item_factory_entry_property_cb (GtkListItemFactory *factory,
							 GtkListItem *list_item,
							 const char *property);

HKLAPI void hkl_gui_bind_item_factory_label_property_cb (GtkListItemFactory *factory,
							 GtkListItem *list_item,
							 const char *property);

HKLAPI void hkl_gui_bind_item_factory_spin_button_property_cb (GtkListItemFactory *factory,
							       GtkListItem *list_item,
							       const char *property);

/* GuiParameter */

void hkl_gui_parameter_bind_factory_label_name_cb (GtkListItemFactory *factory,
						   GtkListItem *list_item);

void hkl_gui_parameter_bind_factory_label_value_cb (GtkListItemFactory *factory,
						    GtkListItem *list_item);

void hkl_gui_parameter_bind_factory_spin_button_value_cb (GtkListItemFactory *factory,
							  GtkListItem *list_item);
