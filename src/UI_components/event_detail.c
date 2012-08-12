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
 
 #ifndef DIAGRAM_HEADER
#include "../diagram.h"
#endif

#include <sys/time.h>
#include <string.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#ifndef DATA_DUMPED_HEADER
#include "../data_dumped.h"
#endif

#define MAX_BUFF 512
	
static GtkWidget *packet_frame = NULL;
static GtkWidget *ev_type_label = NULL;
static GtkWidget *ev_name_label = NULL;
static GtkWidget *notebook = NULL;



/* Hexdump */
static GtkWidget *hexdump_pane = NULL;
static GtkWidget *hexdump_label = NULL;


void make_hexdump_pane()
{
	hexdump_pane = gtk_frame_new("");
	hexdump_label = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(hexdump_pane),
						hexdump_label);
}

void set_hexdump_label(char *str)
{
	gtk_label_set_text(hexdump_label, str);
}

/* attributes */
static GtkWidget *attributes_pane = NULL;
static GtkWidget *attributes_table = NULL;

void make_attributes_pane()
{
	attributes_pane = gtk_frame_new("");
	
	attributes_table = gtk_table_new(10, 2, TRUE);
	gtk_container_add(GTK_CONTAINER(attributes_pane), attributes_table);
	
}

void set_attributes(struct event_t * e)
{
	char buff[MAX_BUFF], aux[MAX_BUFF];
	gpointer key, value;
	GHashTableIter iter;
	GtkWidget *label;
	
	int size, line;
	
	if(attributes_table != NULL){
		return;
	/*	printf("destroy Table\n");
		gtk_container_remove(GTK_CONTAINER(attributes_pane), attributes_table);
		gtk_widget_destroy(attributes_table);
		* */
	}

	size = g_hash_table_size(e->attributes);
	
	if(size  > 0){
		g_hash_table_iter_init (&iter, e->attributes);

		line = 0;
		while (g_hash_table_iter_next (&iter, &key, &value))
		{
			sprintf(buff,"%s:",key);
		
			gtk_table_attach_defaults(GTK_TABLE(attributes_table),
							gtk_label_new(buff), 0 , 1,
							line, line + 1);
			//gtk_table_attach_defaults(GTK_TABLE(attributes_table),
			//				gtk_label_new(value), line, line + 1,
			//				1, 2);
			line++;
		}
		
	}
	else{
	//	label = gtk_label_new("No attributes.");
	//	gtk_container_add(GTK_CONTAINER(attributes_pane), label);
	}
 
}

void make_notebook()
{

	gtk_label_set_selectable(hexdump_label, TRUE);
	notebook = gtk_notebook_new ();
		
	make_hexdump_pane();
	gtk_notebook_append_page(notebook, hexdump_pane, gtk_label_new("Hexdump"));
	
	make_attributes_pane();
	
	gtk_notebook_append_page(notebook, attributes_pane, 
							gtk_label_new("Attributes"));
}

GtkWidget *event_details_init()
{
	GtkWidget *packet_frame_scroll, *packet_frame_details;
	GtkWidget *hbox;
	GtkWidget *vbox;
	
	/* Init*/
	hbox = gtk_hbox_new(FALSE, 1);
	
	vbox = gtk_vbox_new(FALSE, 1);
	
	packet_frame = gtk_frame_new("Event details");
	packet_frame_details = gtk_frame_new("");

	make_notebook();
	
	/* Event type label */
	ev_type_label = gtk_label_new("");
	gtk_label_set_selectable(ev_type_label, TRUE);
	gtk_widget_modify_font (ev_type_label,
		pango_font_description_from_string ("Arial Bold 12"));
	
	ev_name_label = gtk_label_new("");
	gtk_label_set_selectable(ev_name_label, TRUE);
	
	packet_frame_scroll = gtk_scrolled_window_new(NULL, NULL);
	
	/* Add packet details frame */
	gtk_frame_set_shadow_type(packet_frame_details, GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(packet_frame_scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
	gtk_box_pack_start(GTK_BOX(hbox), ev_type_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), ev_name_label, TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	
	gtk_container_add(GTK_CONTAINER(packet_frame_details), vbox);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(packet_frame_scroll),
								packet_frame_details);
	gtk_container_add(GTK_CONTAINER(packet_frame), packet_frame_scroll);
	
	return packet_frame;	
}
gboolean show_event_details(struct event_diagram *ed,
							int action)
{
	char buff[MAX_BUFF], aux[MAX_BUFF];
	gpointer key, value;
	GHashTableIter iter;

	struct event_t *event = ed->event;

	if(action == EVENT_SELECTED){
		/* Show packet details */

		gtk_label_set_text(ev_type_label, event->type_str);
		gtk_label_set_text(ev_name_label, event->name);
		
		set_attributes(event);
		
		set_hexdump_label(event->data);

		gtk_widget_show(packet_frame);
	}
	else if(action == EVENT_UNSELECTED){
		gtk_widget_hide(packet_frame);
	}
}
