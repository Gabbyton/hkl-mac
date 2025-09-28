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

#define get_model(filename)						\
	(0 == access(filename, R_OK)) ? filename :			\
	((0 == access("../data/" filename, R_OK)) ? "../data/" filename : \
	 ((0 == access(PKGDATA "/hkl3d/" filename, R_OK)) ? PKGDATA "/hkl3d/" filename : NULL))


static void
hkl_gui_setup_item_factory_label_cb (GtkListItemFactory *factory,
				     GtkListItem        *list_item)
{
	GtkWidget *label;

	label = gtk_label_new ("");
	gtk_list_item_set_child (list_item, label);
}


#endif /* __HKL_GUI_MACROS_H__ */
