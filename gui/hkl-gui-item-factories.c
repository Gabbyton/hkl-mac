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
#include <locale.h>

#include <gtk/gtk.h>


/****************/
/* Check Button */
/****************/

void
hkl_gui_setup_item_factory_check_button_cb (GtkListItemFactory *factory,
					    GtkListItem *list_item)
{
	GtkWidget *widget;

	widget = gtk_check_button_new ();
	gtk_list_item_set_child (list_item, widget);
}

/*********/
/* Entry */
/*********/

/* entry */

void
hkl_gui_setup_item_factory_entry_cb (GtkListItemFactory *factory,
				     GtkListItem *list_item)
{
	GtkWidget *entry;

	entry = gtk_entry_new ();
	gtk_list_item_set_child (list_item, entry);
}

void
hkl_gui_bind_item_factory_entry_property_cb (GtkListItemFactory *factory,
					     GtkListItem *list_item,
					     const char *source_property)
{
	GtkWidget *entry;
	GtkEntryBuffer *buffer;
	GObject *self;
	GValue value = G_VALUE_INIT;

	g_return_if_fail (GTK_IS_LIST_ITEM_FACTORY (factory));
	g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
	g_return_if_fail (source_property != NULL);
	g_return_if_fail (g_param_spec_is_valid_name (source_property));

	entry = gtk_list_item_get_child (list_item);

	g_return_if_fail (GTK_IS_ENTRY (entry));

	buffer = gtk_entry_get_buffer (GTK_ENTRY (entry));

	self = gtk_list_item_get_item (list_item);

	g_return_if_fail (G_IS_OBJECT (self));

	g_object_get_property(self, source_property, &value);
	g_object_set_property (G_OBJECT (buffer), "text", &value);

	g_object_bind_property(self, source_property, G_OBJECT (buffer), "text", G_BINDING_BIDIRECTIONAL);
}

GtkListItemFactory *
hkl_gui_item_factory_new_entry_property(char *property)
{
	GtkListItemFactory *item_factory;

	item_factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_entry_cb), NULL);
	g_signal_connect (item_factory, "bind", G_CALLBACK (hkl_gui_bind_item_factory_entry_property_cb), property);

	return item_factory;
}

/* entry numeric */

void
entry_numeric_insert_text (GtkEditable *editable,
			   const char  *new_text,
			   int          new_text_length,
			   int         *position,
			   gpointer     data)
{
	g_signal_stop_emission_by_name (editable, "insert-text");

	struct lconv *lc;
	gboolean sign;
	int dotpos = -1;
	int i;
	int entry_length;
	const char *entry_text;
	int digits = 20;

	entry_text = gtk_editable_get_text (editable);
	entry_length = g_utf8_strlen (entry_text, -1);


	lc = localeconv ();

	for (sign = FALSE, i = 0; i<entry_length; i++)
		if (entry_text[i] == '-' || entry_text[i] == '+')
		{
			sign = TRUE;
			break;
		}

	if (sign && !(*position))
		return;

	for (dotpos = -1, i = 0; i<entry_length; i++)
		if (entry_text[i] == *(lc->decimal_point))
		{
			dotpos = i;
			break;
		}

	if (dotpos > -1 && *position > dotpos &&
	    digits - entry_length
	    + dotpos - new_text_length + 1 < 0)
		return;
	for (i = 0; i < new_text_length; i++)
	{
		if (new_text[i] == '-' || new_text[i] == '+')
		{
			if (sign || (*position) || i)
				return;
			sign = TRUE;
		}
		else if (new_text[i] == *(lc->decimal_point))
		{
			if (!digits || dotpos > -1 ||
			    (new_text_length - 1 - i + entry_length - *position > digits))
				return;
			dotpos = *position + i;
		}
		else if (new_text[i] < 0x30 || new_text[i] > 0x39)
			return;
	}

	g_signal_handlers_block_by_func (editable, entry_numeric_insert_text, data);

	gtk_editable_insert_text (editable,
				  new_text, new_text_length, position);

	g_signal_handlers_unblock_by_func (editable, entry_numeric_insert_text, data);
}


void
hkl_gui_setup_item_factory_entry_numeric_cb (GtkListItemFactory *factory,
					     GtkListItem *list_item)
{
	GtkWidget *entry;

	entry = gtk_entry_new ();
	gtk_entry_set_input_purpose(GTK_ENTRY(entry),
				    GTK_INPUT_PURPOSE_NUMBER);

	g_signal_connect (gtk_editable_get_delegate(GTK_EDITABLE(entry)),
			  "insert-text",
			  G_CALLBACK (entry_numeric_insert_text), entry);

	gtk_list_item_set_child (list_item, entry);
}

static gint string_to_double(GBinding *binding,
			     const GValue * value_a,
			     GValue *value_b,
			     gpointer user_data)
{
	g_assert (G_VALUE_HOLDS_STRING (value_a));
	g_assert (G_VALUE_HOLDS_DOUBLE (value_b));

	g_value_set_double(value_b, atof(g_value_get_string(value_a)));

	return TRUE;
}

void
hkl_gui_bind_item_factory_entry_numeric_property_cb (GtkListItemFactory *factory,
						     GtkListItem *list_item,
						     const char *source_property)
{
	GtkWidget *entry;
	GtkEntryBuffer *buffer;
	GObject *self;
	GValue value = G_VALUE_INIT;

	g_return_if_fail (GTK_IS_LIST_ITEM_FACTORY (factory));
	g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
	g_return_if_fail (source_property != NULL);
	g_return_if_fail (g_param_spec_is_valid_name (source_property));

	entry = gtk_list_item_get_child (list_item);

	g_return_if_fail (GTK_IS_ENTRY (entry));

	buffer = gtk_entry_get_buffer (GTK_ENTRY (entry));

	self = gtk_list_item_get_item (list_item);

	g_return_if_fail (G_IS_OBJECT (self));

	/* synchrotronise the object at first */
	g_object_get_property(self, source_property, &value);
	g_object_set_property (G_OBJECT (buffer), "text", &value);

	g_object_bind_property_full(self, source_property,
				    G_OBJECT (buffer), "text",
				    G_BINDING_BIDIRECTIONAL,
				    NULL,
				    string_to_double,
				    NULL,
				    NULL);
}

GtkListItemFactory *
hkl_gui_item_factory_new_entry_numeric_property(char *property)
{
	GtkListItemFactory *item_factory;

	item_factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_entry_numeric_cb), NULL);
	g_signal_connect (item_factory, "bind", G_CALLBACK (hkl_gui_bind_item_factory_entry_numeric_property_cb), property);

	return item_factory;
}

/*********/
/* Label */
/*********/

void
hkl_gui_setup_item_factory_label_cb (GtkListItemFactory *factory,
				     GtkListItem        *list_item)
{
	GtkWidget *label;

	label = gtk_label_new ("");
	gtk_list_item_set_child (list_item, label);
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

GtkListItemFactory *
hkl_gui_item_factory_new_label_property(char *property)
{
	GtkListItemFactory *item_factory;

	item_factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_label_cb), NULL);
	g_signal_connect (item_factory, "bind", G_CALLBACK (hkl_gui_bind_item_factory_label_property_cb), property);

	return item_factory;
}

/***************/
/* Spin Button */
/***************/

void
hkl_gui_setup_item_factory_spin_button_cb (GtkListItemFactory *factory,
					   GtkListItem *list_item)
{
	GtkWidget *spin_button;

	spin_button = gtk_spin_button_new_with_range (-G_MAXDOUBLE, G_MAXDOUBLE, 0.0001);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON (spin_button), 1, 10);
	gtk_list_item_set_child (list_item, spin_button);
}

void
hkl_gui_setup_item_factory_spin_button_vertical_cb (GtkListItemFactory *factory,
						    GtkListItem *list_item)
{
	GtkWidget *spin_button;

	spin_button = gtk_spin_button_new_with_range (-G_MAXDOUBLE, G_MAXDOUBLE, 0.0001);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON (spin_button), 1, 10);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (spin_button), GTK_ORIENTATION_VERTICAL);
	gtk_list_item_set_child (list_item, spin_button);
}


void
hkl_gui_bind_item_factory_spin_button_property_cb (GtkListItemFactory *factory,
						   GtkListItem *list_item,
						   const char *source_property)
{
	GtkWidget *spin_button;
	GObject *self;
	GValue value = G_VALUE_INIT;

	g_return_if_fail (GTK_IS_LIST_ITEM_FACTORY (factory));
	g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
	g_return_if_fail (source_property != NULL);
	g_return_if_fail (g_param_spec_is_valid_name (source_property));

	spin_button = gtk_list_item_get_child (list_item);

	g_return_if_fail (GTK_IS_SPIN_BUTTON (spin_button));

	self = gtk_list_item_get_item (list_item);

	g_return_if_fail (G_IS_OBJECT (self));

	g_object_get_property(self, source_property, &value);
	g_object_set_property (G_OBJECT (spin_button), "value", &value);

	g_object_bind_property(self, source_property, G_OBJECT (spin_button), "value", G_BINDING_BIDIRECTIONAL);
}


GtkListItemFactory *
hkl_gui_item_factory_new_spin_button_vertical_property(char *property)
{
	GtkListItemFactory *item_factory;

	item_factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (item_factory, "setup", G_CALLBACK (hkl_gui_setup_item_factory_spin_button_vertical_cb), NULL);
	g_signal_connect (item_factory, "bind", G_CALLBACK (hkl_gui_bind_item_factory_spin_button_property_cb), property);

	return item_factory;
}
