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

#include "event.h"
#include "util.h"
#include "diagram.h"

#define R_W 375
#define R_H 30
#define SPACE 10
#define FONT_SIZE 9
#define GAP_SIZE 9

#define WINDOW_H 550
#define WINDOW_W 800

#define DAREA_H WINDOW_H
#define DAREA_W WINDOW_W
#define DAREA_HEIGHT_GROW 200

#define PACKET_FRAME_W WINDOW_W
#define PACKET_FRAME_H 200
#define PIXMAP_GROW_WIDTH 1
#define PIXMAP_GROW_HEIGHT 70*R_H


/*draw events options*/

#define EVENT_SELECTED ( 1 << 1 )

#define WINDOW_TITLE "Bluez Traffic Visualization"

#define MAX_BUFF 512

/* others constants */

#define PI (3.14159265358979323846f)

static GtkWidget *window = NULL;
static GtkWidget *diagram = NULL;
static GtkWidget *packet_frame = NULL;
static GtkWidget *packet_detail = NULL;
static GMainLoop *mainloop = NULL;
static GtkWidget *device_filters_dialog = NULL;

static unsigned char hexdump_buff[MAX_BUFF];

static GtkWidget *loading_dialog;

static GdkPixmap *draw_pixmap = NULL;
static cairo_t *cairo_draw = NULL;

/* Devices check-box */

GHashTable *devices_check = NULL;

/* Session timeline */
struct device_t *session_timeline;

/* Flag for drawing */
static int event_selected_seq_number=-1;

static int draw_handler_id;

void destroy_widgets()
{
	gtk_widget_destroy(window);
	gtk_widget_destroy(packet_frame);
	gtk_widget_destroy(packet_detail);
	gtk_widget_destroy(diagram);
	gtk_widget_destroy(draw_pixmap);

	cairo_destroy(cairo_draw);
}

gboolean on_destroy_event(GtkWidget *widget,GdkEventExpose *event,
								gpointer data)
{
	destroy_widgets();
	g_main_quit(mainloop);
	gtk_main_quit();

	return FALSE;
}

int UI_init(int argc,char **argv,GMainLoop *loop)
{
	GtkWidget *sw, *viewport, *vbox, *menubar, *filemenu, *file, *quit;
	GtkWidget *filters, *filters_menu, *device_filter;
	GtkWidget *packet_frame_scroll, *packet_frame_details;
	GtkRequisition size;
	struct device_t *d;

	gtk_init(&argc, &argv);
	
	/* Set variables */
	mainloop = loop;
	window = gtk_window_new(0);

	packet_frame = gtk_frame_new("Packet details");
	packet_frame_details = gtk_frame_new("");
	packet_detail = gtk_label_new("");
	sw = gtk_scrolled_window_new(NULL, NULL);
	packet_frame_scroll = gtk_scrolled_window_new(NULL, NULL);
	
	/* Init diagram */
	diagram = create_diagram(0, 640, 400);	
	
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

	/* Add scrollable diagram */
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), diagram);
						
	/* Set events handlers */
	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_W, WINDOW_H);

	gtk_widget_show_all(window);
	return 0;
}
