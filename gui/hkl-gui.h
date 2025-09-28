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

#define HKL_GUI_TYPE_WINDOW (hkl_gui_window_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiWindow, hkl_gui_window, HKL_GUI, WINDOW, GtkApplication)

/* engine */

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

/* factory */

HKLAPI HklGuiFactory* hkl_gui_factory_new(const HklFactory *factory);

HKLAPI struct diffractometer_t* hkl_gui_factory_get_diffractometer(HklGuiFactory *self);

HKLAPI GError* hkl_gui_factory_get_error(HklGuiFactory *self);

HKLAPI HklFactory* hkl_gui_factory_get_factory(HklGuiFactory *self);

HKLAPI gdouble hkl_gui_factory_get_wavelength(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_axes(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_engines(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_pseudo_axes(HklGuiFactory *self);

HKLAPI GListStore* hkl_gui_factory_get_liststore_solutions(HklGuiFactory *self);

HKLAPI GtkListItemFactory* hkl_gui_factory_name_factory_new(void);

HKLAPI void hkl_gui_factory_set_error(HklGuiFactory *self, GError* error);

HKLAPI void hkl_gui_factory_set_geometry(HklGuiFactory *self, HklGuiGeometry *ggeometry);

HKLAPI void hkl_gui_factory_set_sample(HklGuiFactory *self, HklSample *sample);

HKLAPI void hkl_gui_factory_set_wavelength(HklGuiFactory *self, gdouble wavelength);

HKLAPI void hkl_gui_factory_setup_column_view_solutions(HklGuiFactory *self, GtkColumnView *column_view);

/* geometry */

HKLAPI HklGuiGeometry* hkl_gui_geometry_new(const HklGeometry *geometry);

HKLAPI HklGeometry* hkl_gui_geometry_get_geometry(HklGuiGeometry *geometry);

HKLAPI GtkListItemFactory* hkl_gui_geometry_axis_value_factory_new(gint idx);

/*************/
/* parameter */
/*************/

HKLAPI HklGuiParameter* hkl_gui_parameter_new(const HklParameter *parameter);

HKLAPI gdouble hkl_gui_parameter_get_maximum(HklGuiParameter *self);
HKLAPI gdouble hkl_gui_parameter_get_minimum(HklGuiParameter *self);
HKLAPI gdouble hkl_gui_parameter_get_value(HklGuiParameter *self);

HKLAPI void hkl_gui_parameter_set_maximum(HklGuiParameter *self, gdouble value);
HKLAPI void hkl_gui_parameter_set_minimum(HklGuiParameter *self, gdouble value);
HKLAPI void hkl_gui_parameter_set_value(HklGuiParameter *self, gdouble value);

HKLAPI void hkl_gui_parameter_update(HklGuiParameter *self);

/**********/
/* Sample */
/**********/

HKLAPI HklGuiSample* hkl_gui_sample_new(const char *name);

HKLAPI const char *hkl_gui_sample_get_name(HklGuiSample *self);

HKLAPI void hkl_gui_sample_set_name(HklGuiSample *self, const char *name);

/* item factories */

HKLAPI void hkl_gui_setup_item_factory_label_cb (GtkListItemFactory *factory,
						 GtkListItem *list_item);

HKLAPI void hkl_gui_setup_item_factory_spin_button_cb (GtkListItemFactory *factory,
						       GtkListItem *list_item);

void hkl_gui_parameter_bind_factory_label_name_cb (GtkListItemFactory *factory,
						   GtkListItem *list_item);

void hkl_gui_parameter_bind_factory_label_value_cb (GtkListItemFactory *factory,
						    GtkListItem *list_item);

void hkl_gui_parameter_bind_factory_spin_button_value_cb (GtkListItemFactory *factory,
							  GtkListItem *list_item);

HKLAPI GtkListItemFactory *hkl_gui_parameter_factory_name_new(void);
HKLAPI GtkListItemFactory *hkl_gui_parameter_factory_value_new(void);
HKLAPI GtkListItemFactory *hkl_gui_parameter_factory_value_label_new(void);
HKLAPI GtkListItemFactory *hkl_gui_parameter_factory_min_new(void);
HKLAPI GtkListItemFactory *hkl_gui_parameter_factory_max_new(void);
