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

/* Attributes */
static GtkWidget *attributes_pane = NULL;
static GtkWidget *attributes_view = NULL;

void make_attributes_model (struct event_t *e)
{
	GtkTreeStore *treestore;
	GtkTreeIter toplevel;

	char buff[MAX_BUFF], aux[MAX_BUFF];
	gpointer key, value;
	GHashTableIter iter;
	GtkWidget *label;

	int size;

	treestore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

	size = g_hash_table_size(e->attributes);

	if(size > 0){
		g_hash_table_iter_init (&iter, e->attributes);

		while (g_hash_table_iter_next (&iter, &key, &value))
		{
		  gtk_tree_store_append(treestore, &toplevel, NULL);
		  gtk_tree_store_set(treestore, &toplevel,
					0, key,
					1, value,
					-1);
		}
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(attributes_view),
							GTK_TREE_MODEL(treestore));
}

void make_attributes_view (void)
{
	GtkTreeViewColumn   *col;
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model;

	attributes_view = gtk_tree_view_new();

	/* --- Attribute Column  --- */

	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(col, "Attribute");

	gtk_tree_view_append_column(GTK_TREE_VIEW(attributes_view), col);

	renderer = gtk_cell_renderer_text_new();

	g_object_set(renderer,
               "weight", PANGO_WEIGHT_BOLD,
               "weight-set", TRUE,
               NULL);
	gtk_tree_view_column_pack_start(col, renderer, TRUE);

	gtk_tree_view_column_add_attribute(col, renderer, "text", 0);

	/* --- Value Column --- */

	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(col, "Value");

	gtk_tree_view_append_column(GTK_TREE_VIEW(attributes_view), col);

	renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_column_pack_start(col, renderer, TRUE);

	gtk_tree_view_column_add_attribute(col, renderer, "text", 1);

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(attributes_view)),
                              GTK_SELECTION_NONE);
}

void make_attributes_pane()
{

	attributes_pane = gtk_frame_new("");

	make_attributes_view();

	gtk_container_add(GTK_CONTAINER(attributes_pane), attributes_view);

}

/* Notebook */

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
	GtkWidget *info_pane;
	GtkRequisition req;
	
	/* Init*/
	hbox = gtk_hbox_new(FALSE, 1);

	vbox = gtk_vbox_new(FALSE, 1);

	packet_frame = gtk_frame_new("Event details");
	packet_frame_details = gtk_frame_new("");
	info_pane = gtk_frame_new("");
	
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
	
	gtk_container_add(GTK_CONTAINER(info_pane), hbox);
	req.width = 400; req.height = 200;
	gtk_widget_size_request(info_pane, &req);
	
	gtk_container_add(GTK_CONTAINER(packet_frame_details), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), info_pane, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

	

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(packet_frame_scroll),
								packet_frame_details);
	gtk_container_add(GTK_CONTAINER(packet_frame), packet_frame_scroll);

	return packet_frame;
}

gboolean show_event_details(struct event_diagram *ed, int action)
{
	char buff[MAX_BUFF], aux[MAX_BUFF];
	gpointer key, value;
	GHashTableIter iter;

	struct event_t *event = ed->event;

	if(action == EVENT_SELECTED){
		/* Show packet details */

		gtk_label_set_text(ev_type_label, event->type_str);
		gtk_label_set_text(ev_name_label, event->name);

		set_hexdump_label(event->data);
		make_attributes_model(event);

	}
	else if(action == EVENT_UNSELECTED){
		gtk_widget_hide(packet_frame);
	}
	return TRUE;
}
