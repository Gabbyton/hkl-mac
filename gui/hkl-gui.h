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

/* factory */

#define HKL_GUI_TYPE_FACTORY (hkl_gui_factory_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiFactory, hkl_gui_factory, HKL_GUI, FACTORY, GObject)

HKLAPI HklGuiFactory* hkl_gui_factory_new(const HklFactory *factory);

HKLAPI struct diffractometer_t* hkl_gui_factory_get_diffractometer(HklGuiFactory *self);

HKLAPI GtkSelectionModel* hkl_gui_factory_get_axes_selection_model(const HklGuiFactory *self);

HKLAPI GtkSelectionModel* hkl_gui_factory_get_pseudo_axes_selection_model(const HklGuiFactory *self);

HKLAPI GtkListItemFactory* hkl_gui_factory_name_factory_new(void);

HKLAPI GListStore* hkl_gui_factory_has_liststore(void);

HKLAPI GtkWidget* hkl_gui_factory_get_column_view_axes(void);

HKLAPI GtkWidget* hkl_gui_factory_get_column_view_pseudo_axes(void);

/* parameter */

#define HKL_GUI_TYPE_PARAMETER (hkl_gui_parameter_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiParameter, hkl_gui_parameter, HKL_GUI, PARAMETER, GObject)

HklGuiParameter* hkl_gui_parameter_new(const HklParameter *parameter);

gdouble hkl_gui_parameter_get_maximum(HklGuiParameter *self);
gdouble hkl_gui_parameter_get_minimum(HklGuiParameter *self);
gdouble hkl_gui_parameter_get_value(HklGuiParameter *self);

void hkl_gui_parameter_set_maximum(HklGuiParameter *self, gdouble value);
void hkl_gui_parameter_set_minimum(HklGuiParameter *self, gdouble value);
void hkl_gui_parameter_set_value(HklGuiParameter *self, gdouble value);

void hkl_gui_parameter_update(HklGuiParameter *self);

GtkListItemFactory *hkl_gui_parameter_factory_name_new(void);
GtkListItemFactory *hkl_gui_parameter_factory_value_new(void);
GtkListItemFactory *hkl_gui_parameter_factory_min_new(void);
GtkListItemFactory *hkl_gui_parameter_factory_max_new(void);

/* window */

#define HKL_GUI_TYPE_WINDOW (hkl_gui_window_get_type ())
G_DECLARE_FINAL_TYPE (HklGuiWindow, hkl_gui_window, HKL_GUI, WINDOW, GtkApplication)
