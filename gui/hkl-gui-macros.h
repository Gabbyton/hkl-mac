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
 * Copyright (C) 2003-2019, 2025 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */

#ifndef __HKL_GUI_MACROS_H__
#define __HKL_GUI_MACROS_H__

#include <gtk/gtk.h>

#include "hkl.h"

#define add_column(column_view, name, item_factory, ...) do {	\
		GtkListItemFactory *factory;				\
		GtkColumnViewColumn *column;				\
									\
		factory = hkl_gui_item_factory_new_ ## item_factory (__VA_ARGS__); \
		column = gtk_column_view_column_new((name), factory);	\
		gtk_column_view_append_column ( GTK_COLUMN_VIEW (column_view), column); \
	} while (0)


#define get_model(filename)						\
	(0 == access(filename, R_OK)) ? filename :			\
	((0 == access("../data/" filename, R_OK)) ? "../data/" filename : \
	 ((0 == access(PKGDATA "/hkl3d/" filename, R_OK)) ? PKGDATA "/hkl3d/" filename : NULL))

#define set_label_from_double(label, value) do {			\
		char *buf = g_strdup_printf ("%0.*f", 6, value);	\
		gtk_label_set_label (GTK_LABEL (label), buf);		\
		g_free(buf);						\
	} while (0)

#endif /* __HKL_GUI_MACROS_H__ */
