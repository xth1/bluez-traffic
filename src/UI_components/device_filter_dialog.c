/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Thiago da Silva Arruda <thiago.xth1@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <gtk/gtk.h>
#include <glib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#ifndef EVENT_HEADER
#include "../event.h"
#endif

#ifndef UTIL_HEADER
#include "../util.h"
#endif

#ifndef DIAGRAM_HEADER
#include "../diagram.h"
#endif

/* Devices check-box */
static GHashTable *devices_check = NULL;
static GtkWidget *device_filters_dialog = NULL;
void on_device_dialog_response(GtkWidget *widget, GdkEventButton *mouse_event,
				gpointer user_data)
{
	
	GHashTable *connected_devices;
	GHashTableIter iter;
	GtkCheckButton *chk;
	gpointer key, value;
	struct device_t *d;
	
	if(devices_check == NULL)
		return;
	
	connected_devices = ev_get_connected_devices();
	
	g_hash_table_iter_init (&iter, devices_check);
	
	while (g_hash_table_iter_next (&iter, &key, &value)){
		chk = (GtkCheckButton *) value;
		d = (struct device_t *) g_hash_table_lookup(connected_devices, (char *) key);		
		filter_set_active_device(d, gtk_toggle_button_get_active(chk));
	}
	
	events_update();

	/* Free resourses */
	gtk_widget_destroy(device_filters_dialog);
	device_filters_dialog = NULL;
	
	g_hash_table_destroy(devices_check);
	devices_check = NULL;
}

void create_device_filters_dialog(GtkWidget *window)
{
	GtkWidget *dialog, *label, *content_area;
	GtkWidget *check;
	
	GHashTable *connected_devices;
	
	char buff[256];
	gpointer key, value;
	GHashTableIter iter;
	struct device_t *d;
	
	gboolean is_active;
   
	if(devices_check != NULL || device_filters_dialog != NULL)
		return;
	
	devices_check = g_hash_table_new (g_str_hash, g_str_equal);
	
	connected_devices = ev_get_connected_devices();

	device_filters_dialog = gtk_dialog_new_with_buttons ("Devices fiters",
                                         window,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (device_filters_dialog));
	label = gtk_label_new ("Devices");
	gtk_container_add (GTK_CONTAINER (content_area), label);

   /* Add devices check box */
	g_hash_table_iter_init (&iter, connected_devices);
	
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		d = (struct device_t *) value;
		sprintf(buff,"%s: %s", d->address, d->name);
		check =  gtk_check_button_new_with_label(buff);
		
		is_active = filter_is_device_active(d);
		
		gtk_toggle_button_set_active(check, is_active);
		g_hash_table_insert(devices_check, d->address, check);
		gtk_container_add (GTK_CONTAINER (content_area), check);
	}

   g_signal_connect_swapped (device_filters_dialog,
                             "response",
                             G_CALLBACK(on_device_dialog_response),
                             devices_check);
   gtk_widget_show_all (device_filters_dialog);
}
