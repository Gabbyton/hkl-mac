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

#include <gtk/gtk.h>

void
hkl_gui_setup_item_factory_entry_cb (GtkListItemFactory *factory,
				     GtkListItem *list_item)
{
	GtkWidget *entry;

	entry = gtk_entry_new ();
	gtk_list_item_set_child (list_item, entry);
}


void
hkl_gui_setup_item_factory_label_cb (GtkListItemFactory *factory,
				     GtkListItem        *list_item)
{
	GtkWidget *label;

	label = gtk_label_new ("");
	gtk_list_item_set_child (list_item, label);
}


void
hkl_gui_setup_item_factory_spin_button_cb (GtkListItemFactory *factory,
					   GtkListItem *list_item)
{
	GtkWidget *spin_button;

	spin_button = gtk_spin_button_new_with_range (-G_MAXDOUBLE, G_MAXDOUBLE, 0.0001);
	gtk_list_item_set_child (list_item, spin_button);
}

void
hkl_gui_bind_item_factory_entry_property_cb (GtkListItemFactory *factory,
					     GtkListItem *list_item,
					     const char *property)
{
	GtkWidget *entry;
	GtkEntryBuffer *buffer;
	GObject *self;
	GValue value = G_VALUE_INIT;

	entry = gtk_list_item_get_child (list_item);
	buffer = gtk_entry_get_buffer (GTK_ENTRY (entry));
	self = gtk_list_item_get_item (list_item);

	g_return_if_fail(NULL != self);

	g_object_get_property(self, property, &value);
	g_object_set_property (G_OBJECT (buffer), "text", &value);

	g_object_bind_property(self, property, G_OBJECT (buffer), "text", G_BINDING_BIDIRECTIONAL);
}

void
hkl_gui_bind_item_factory_label_property_cb (GtkListItemFactory *factory,
					     GtkListItem *list_item,
					     const char *property)
{
	GtkWidget *label;
	GObject *self;


	label = gtk_list_item_get_child (list_item);
	self = gtk_list_item_get_item (list_item);

	g_return_if_fail(NULL != self);

	g_object_bind_property(self, property, label, "label", G_BINDING_SYNC_CREATE);
}
