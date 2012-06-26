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

#include "draw.h"

#define R_W 375
#define R_H 30
#define SPACE 10
#define FONT_SIZE 9
#define GAP_SIZE 9

#define WINDOW_H 600
#define WINDOW_W 430

#define DAREA_H WINDOW_H
#define DAREA_W WINDOW_W
#define DAREA_HEIGHT_GROW 200

#define PIXMAP_GROW_WIDTH 1
#define PIXMAP_GROW_HEIGHT 70*R_H

/*draw events options*/

#define EVENT_SELECTED ( 1 << 1 )

#define WINDOW_TITLE "Bluez Traffic Visualization"

#define MAX_BUFF 512

static GtkWidget *window;
static GtkWidget *darea;
static GMainLoop *mainloop;

static GtkWidget *loading_dialog;

static GdkPixmap *draw_pixmap = NULL;
static cairo_t *cairo_draw = NULL;

/*Array of events*/
static GArray *events;
static int events_size;

/*Flag for drawing*/
static gboolean need_draw = FALSE;

static int event_selected_seq_number=-1;

static int draw_handler_id;

struct event_t *get_event(int p)
{
	return g_array_index(events, void *, p);
}

void create_draw_pixmap(GtkWidget *widget)
{
	GtkRequisition size;
	gint pixmap_width = 0;
	gint pixmap_height = 0;
	gint new_width;
	gint new_height;

	gtk_widget_size_request(darea, &size);

	if(draw_pixmap)
		gdk_drawable_get_size(draw_pixmap,&pixmap_width,&pixmap_height);

	if(pixmap_width > size.width && pixmap_height > size.height)
		return;

	new_width = size.width + PIXMAP_GROW_WIDTH;
	new_height = size.height + PIXMAP_GROW_HEIGHT;

	if (draw_pixmap)
		g_object_unref(draw_pixmap);

	draw_pixmap = gdk_pixmap_new(widget->window,
				new_width,
				new_height,
				-1);

	gdk_draw_rectangle(draw_pixmap,
				widget->style->white_gc,
				TRUE,
				0, 0,
				new_width,
				new_height);
	if(cairo_draw)
		cairo_destroy(cairo_draw);

	cairo_draw = gdk_cairo_create(draw_pixmap);
}

void draw_event(cairo_t *cr, struct event_t *e, struct point p,int op)
{
	char buff[MAX_BUFF];
	int size;
	time_t t;
	struct tm tm;

	/*Draw retangle */
	cairo_move_to(cr, p.x, p.y);
	p.x += SPACE;
	p.y += SPACE;
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 1);

	cairo_rectangle(cr, p.x,p.y, p.x + R_W, p.y + R_H);
	cairo_stroke_preserve(cr);

	if(op & EVENT_SELECTED){
		cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
	}
	else
		cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);

	p.x += SPACE;
	p.y += SPACE;

	cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL,
						CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, FONT_SIZE);
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);

	/*Print date*/
	t = (e->tv).tv_sec;
	localtime_r(&t, &tm);

	sprintf(buff,"%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
							tm.tm_mday);
	cairo_move_to(cr,p.x, p.y);
	cairo_show_text(cr,buff);
	size=strlen(buff);

	/*Print time*/
	sprintf(buff, "%02d:%02d:%02d.%06lu ", tm.tm_hour, tm.tm_min,
					 tm.tm_sec, (e->tv).tv_usec);
	cairo_move_to(cr, p.x, p.y + 2 * SPACE);
	cairo_show_text(cr, buff);

	/*Print adapter index*/
	cairo_set_source_rgb(cr, 0.3, 0.0, 0.0);
	cairo_set_font_size(cr, FONT_SIZE + 2);

	p.x += size * GAP_SIZE + SPACE;

	cairo_move_to(cr, p.x, p.y);
	if (e->socket == HCI_CHANNEL_MONITOR)
		sprintf(buff, "[ hci%d ]", e->index);
	else
		sprintf(buff, "{ hci%d }", e->index);
	cairo_show_text(cr, buff);
	size=strlen(buff);

	/*Print operation code*/
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_set_font_size(cr, FONT_SIZE + 1);

	sprintf(buff, "OPCODE: 0x%2.2x", e->type);

	p.x += size * GAP_SIZE + SPACE;
	cairo_move_to(cr, p.x, p.y);
	cairo_show_text(cr, buff);

}

/*
 * Draw all events receivede
 */
void draw(int op, int arg1, int arg2)
{
	struct event_t *e;
	int i;

	int selected_old;
	int selected_new;

	int new_height;
	struct point p;
	GtkRequisition size;

	gtk_widget_size_request(darea, &size);

	new_height = events_size * R_H;

	if(new_height > size.height)
		gtk_widget_set_size_request(darea, size.width, new_height);

	create_draw_pixmap(darea);

	p.x = p.y = 0;

	if(op & EVENT_SELECTED){
		selected_old = arg1;
		selected_new = arg2;
	}

	for(i = 0; i < events_size; i++){
		e = get_event(i);
		if(e->seq_number == event_selected_seq_number)
			draw_event(cairo_draw, e, p, EVENT_SELECTED);
		else
			draw_event(cairo_draw, e, p, 0);

		p.y += R_H + SPACE;
	}
}

int find_event_at(struct point p, struct event_t **event_r)
{
	struct event_t *event;
	int id_event;

	id_event = (p.y - SPACE)/(R_H + SPACE);

	if(id_event >= events_size)
		return -1;

	event = get_event(id_event);

	if(event == NULL){
		printf("Error: element with id %d does not exist\n",id_event);
		return -1;
	}

	*event_r = event;

	return id_event;
}

gboolean on_destroy_event(GtkWidget *widget,GdkEventExpose *event,
								gpointer data)
{
	g_main_quit(mainloop);
	gtk_main_quit();

	return FALSE;
}

gboolean on_expose_event(GtkWidget *widget,GdkEventExpose *event,
								gpointer data)
{
	gdk_draw_drawable(widget->window,
				widget->style->fg_gc[gtk_widget_get_state(widget)],
				draw_pixmap,
				event->area.x, event->area.y,
				event->area.x, event->area.y,
				event->area.width, event->area.height);

	if(need_draw){
		draw(0,0,0);
		need_draw = FALSE;
	}

	return TRUE;
}

gboolean on_drawing_clicked(GtkWidget *widget, GdkEventButton *mouse_event,
				gpointer user_data)
{
	struct event_t *event;
	struct point p;
	struct point new_p;
	int id_event;
	p.x = (int)( (double) mouse_event->x);
	p.y = (int)( (double) mouse_event->y);

	id_event = find_event_at(p, &event);

	if(event == NULL || id_event == -1){
		/* Clear selection */
		if(event_selected_seq_number != -1){
			event_selected_seq_number = -1;
			draw(EVENT_SELECTED, -1, -1);
			gtk_widget_queue_draw(darea);
		}
		return TRUE;
	}

	/* redraw event */
	if(event->seq_number != event_selected_seq_number){
		event_selected_seq_number = event->seq_number;

		draw(EVENT_SELECTED,event_selected_seq_number,
				event->seq_number);
		gtk_widget_queue_draw(darea);
	}
	/*
	new_p.x = 0;
	new_p.y = id_event * (R_H + SPACE);

	draw_event(cairo_draw, event, new_p, EVENT_SELECTED);

	gtk_widget_queue_draw_area(darea, new_p.x + SPACE, new_p.y + SPACE, R_W, R_H);
	*/

	return TRUE;
}

gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	create_draw_pixmap(widget);

	return TRUE;
}

void add_event(struct event_t *e)
{
	e->seq_number = events_size;
	g_array_prepend_val(events, e);
	events_size++;

	/*Add expose event handler, if it don't exist*/
	if(draw_handler_id == 0)
		draw_handler_id = g_signal_connect(darea, "expose-event",
					G_CALLBACK(on_expose_event),NULL);
	/*Initial event*/
	if(draw_pixmap == NULL){
		create_draw_pixmap(darea);
		draw(0,0,0);
	}

	need_draw = TRUE;
	create_draw_pixmap(darea);

	/*Launch darea expose event*/
	gtk_widget_queue_draw(darea);
}

int draw_init(int argc,char **argv,GMainLoop *loop)
{
	GtkWidget *sw, *viewport, *vbox, *menubar, *filemenu, *file, *quit;
	GtkRequisition size;

	gtk_init(&argc,&argv);

	/*Set variables */
	mainloop = loop;
	window = gtk_window_new(0);
	darea = gtk_drawing_area_new();
	sw = gtk_scrolled_window_new(NULL, NULL);
	events = g_array_new(FALSE, FALSE, sizeof(struct event_t *));

	draw_handler_id=0;

	/*Set window attributes*/
	 gtk_window_set_title(GTK_WINDOW(window), WINDOW_TITLE);

	/*Add layout manager */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/*Add menubar*/
	menubar = gtk_menu_bar_new();
	filemenu = gtk_menu_new();

	file = gtk_menu_item_new_with_mnemonic("_File");

	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(quit, "activate", G_CALLBACK(on_destroy_event),
							(gpointer) quit);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	/*Add scrollable drawing area*/
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_end(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),
								darea);
	/*Set events handlers */
	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_W, WINDOW_H);

	gtk_widget_add_events (darea,GDK_BUTTON_PRESS_MASK);
	g_signal_connect(darea, "button-press-event",
					G_CALLBACK(on_drawing_clicked), NULL);

	viewport = gtk_widget_get_ancestor(darea, GTK_TYPE_VIEWPORT);
	gtk_widget_size_request(darea, &size);
	gtk_widget_set_size_request(darea, DAREA_W, DAREA_H);
	gtk_widget_set_size_request(sw, DAREA_W, DAREA_H);

	gtk_widget_show_all(window);

	return 0;
}
