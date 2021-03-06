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

#include <sys/time.h>
#include <string.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

//#include "data_dumped.h"
#include "util.h"
#include "UI_components/device_filter_dialog.h"
#include "UI_components/event_detail.h"
#ifndef DIAGRAM_HEADER
#include "diagram.h"
#endif

#define WINDOW_H 550
#define WINDOW_W 800

#define PACKET_FRAME_W WINDOW_W
#define PACKET_FRAME_H 200

#define WINDOW_TITLE "Bluez Traffic Visualization"

#define MAX_BUFF 512

static GtkWidget *window = NULL;
static GtkWidget *diagram = NULL;

static GMainLoop *mainloop = NULL;

void destroy_widgets()
{
	gtk_widget_destroy(window);
	gtk_widget_destroy(diagram);
}

gboolean on_destroy_event(GtkWidget *widget,GdkEventExpose *event,
								gpointer data)
{
	destroy_widgets();
	g_main_quit(mainloop);
	gtk_main_quit();

	return FALSE;
}

void on_device_filters_click(GtkWidget *widget, GdkEventButton *mouse_event,
				gpointer user_data)
{
	create_device_filters_dialog(window);
}

int UI_init(int argc,char **argv,GMainLoop *loop)
{
	GtkWidget *sw, *viewport, *vbox, *menubar, *filemenu, *file, *quit;
	GtkWidget *filters, *filters_menu, *device_filter;
	GtkWidget *v_paned;
	GtkWidget *packet_frame;
	GtkRequisition size;
	struct device_t *d;

	gtk_init(&argc, &argv);

	/* Set variables */
	mainloop = loop;
	window = gtk_window_new(0);


	sw = gtk_scrolled_window_new(NULL, NULL);
	

	v_paned = gtk_vpaned_new();

	/* Init diagram */
	diagram = create_diagram(0, 640, 400,
					(event_diagram_callback) show_event_details);

	/* Set window attributes */
	gtk_window_set_title(GTK_WINDOW(window), WINDOW_TITLE);

	/* Add layout manager */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Add menubar */
	menubar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	/* File menu */
	filemenu = gtk_menu_new();
	file = gtk_menu_item_new_with_mnemonic("_File");

	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(quit, "activate", G_CALLBACK(on_destroy_event),
							(gpointer) quit);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	
	/* Filters menu */
	filters_menu = gtk_menu_new();
	filters = gtk_menu_item_new_with_mnemonic("_Filters");
	device_filter = gtk_menu_item_new_with_mnemonic("_Devices");
	g_signal_connect(device_filter, "activate", G_CALLBACK(on_device_filters_click),
							(gpointer) device_filter);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(filters), filters_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), device_filter);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), filters);

	/* Add scrollable diagram */
	gtk_paned_pack1(GTK_PANED(v_paned), sw, FALSE, FALSE);
	gtk_container_add(GTK_CONTAINER(sw), diagram);

	/* Add packet details frame */
	packet_frame = event_details_init();
	gtk_paned_pack2(GTK_PANED(v_paned), packet_frame, TRUE, FALSE);

	/* Add box */
	gtk_box_pack_start(GTK_BOX(vbox), v_paned, TRUE, TRUE, 0);

	/* Set events handlers */
	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_W, WINDOW_H);

	gtk_widget_show_all(window);
	return 0;
}
