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

#define TEST_ADDRESS "20:00:00:01:01:21"
#define TEST_ADDRESS2 "20:00:00:34:31:13"
#define TEST_ADDRESS3 "20:00:00:34:43:13"

static GtkWidget *window = NULL;
static GtkWidget *darea = NULL;
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

/* Array of events */
static GArray *events;
static int events_size;

/* Connected devices */
static GHashTable *connected_devices;

/* Session timeline */
struct device_t *session_timeline;

/* Flag for drawing */
static gboolean need_draw = FALSE;

static int event_selected_seq_number=-1;

static int draw_handler_id;

/* Util Functions */
char *make_str(const char c_str[])
{
	char *str;
	int sz = strlen(c_str);
	
	str = (char *) malloc((sz + 1) * sizeof(char));
	
	strcpy(str, c_str);
	
	return str;
}

/* Event section */
void event_attributes_key_destroy(gpointer data)
{
	if(data != NULL)
		free(data);
}

void event_attributes_value_destroy(gpointer data)
{
	if(data != NULL)
		free(data);
}

struct event_t *create_event_object(int data_length)
{
	struct event_t *e;

	e = (struct event_t *) malloc(sizeof(struct event_t));
	
	e->data = (char *) malloc(data_length * sizeof(char)); 
	e->attributes = g_hash_table_new_full(g_str_hash, g_str_equal,
					event_attributes_key_destroy,
					event_attributes_value_destroy);
	
	return e;
}

struct event_t *get_event(int p)
{
	return g_array_index(events, void *, p);
}

/* HASH TABLE FUNCTIONS */
void g_hash_table_key_destroy(gpointer data)
{
	/* do nothing */
}

void g_hash_table_value_destroy(gpointer data)
{
	if(data != NULL)
		free(data);
}

void add_device(struct device_t *device)
{
	g_hash_table_insert(connected_devices, device->address, device);
}

struct device_t *get_device(char *address)
{
	struct device_t *d;
	gboolean has_key;
	d = (struct device_t *) g_hash_table_lookup(connected_devices, address);

	return d;
}

gboolean filter_device(struct device_t *d)
{
	if(d->is_active)
		return TRUE;
	return FALSE;	
}

gboolean filter_event(struct event_t *e)
{
	struct device_t *d;
	d = g_hash_table_lookup(connected_devices, e->device_address);
	if(filter_device(d))
		return TRUE;
	return FALSE;	
}

void clear_pixmap(GtkWidget *widget, int width, int height)
{
		gdk_draw_rectangle(draw_pixmap,
				widget->style->white_gc,
				TRUE,
				0, 0,
				width,
				height);
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

	/* Create a new pixmap with new size */
	if (draw_pixmap)
		g_object_unref(draw_pixmap);

	draw_pixmap = gdk_pixmap_new(widget->window,
				new_width,
				new_height,
				-1);
	clear_pixmap(widget, new_width, new_height);

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

	cairo_set_dash(cr, NULL, 0, 0);

	/* Draw rectangle */
	cairo_set_line_width(cr, 0);

	cairo_rectangle(cr, p.x, p.y, p.x + DAREA_W, p.y + R_H);
	cairo_fill(cr);

	/* Draw underline */
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	cairo_set_line_width(cr, 0.5);
	cairo_move_to(cr, p.x, p.y + R_H + SPACE);
	cairo_line_to(cr, p.x + DAREA_W, p.y + R_H + SPACE);

	cairo_stroke(cr);

	p.x += SPACE;
	p.y += SPACE;

	cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL,
						CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, FONT_SIZE);
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);

	/* Print date */
	t = (e->tv).tv_sec;
	localtime_r(&t, &tm);

	sprintf(buff,"%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
							tm.tm_mday);
	cairo_move_to(cr,p.x, p.y);
	cairo_show_text(cr,buff);
	size=strlen(buff);

	/* Print time */
	sprintf(buff, "%02d:%02d:%02d.%06lu ", tm.tm_hour, tm.tm_min,
					 tm.tm_sec, (e->tv).tv_usec);
	cairo_move_to(cr, p.x, p.y + 2 * SPACE);
	cairo_show_text(cr, buff);

	/* Print adapter index */
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

	/* Print operation code */
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.4);
	cairo_set_font_size(cr, FONT_SIZE + 1);

	if(strcmp(e->type_str, "") == 0)
		sprintf(buff, "OPCODE: 0x%2.2x", e->type);
	else
		sprintf(buff, "%s", e->type_str);

	p.x += size * GAP_SIZE + SPACE;
	cairo_move_to(cr, p.x, p.y);
	cairo_show_text(cr, buff);

	/* Print name */
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_set_font_size(cr, FONT_SIZE + 1);

	sprintf(buff, "%s", e->name);

	p.x = 12 * SPACE;
	p.y += 2 * SPACE;
	cairo_move_to(cr, p.x, p.y);
	cairo_show_text(cr, buff);
}

void draw_device_timeline(cairo_t *cr, struct device_t *d, struct point p)
{
	static const double dashed[] = {4.0, 1.0};
	static int len  = sizeof(dashed) / sizeof(dashed[0]);
	int initial_height;
	int least_height;

	/* Print device name */
	cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL,
						CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 11);
	cairo_set_source_rgb(cr, 0.1, 0.5, 0.1);

	cairo_set_dash(cr, dashed, len, 0);
	cairo_move_to(cr, p.x, p.y);
	cairo_show_text(cr, d->name);

	p.y += SPACE;

	/* Set timeline height */
	initial_height = p.y + (events_size - d->id_initial_event) * (R_H + SPACE);

	if(d->id_least_event == -1)
		least_height = p.y;
	else
		least_height = p.y + (events_size - d->id_least_event) * (R_H + SPACE);

	initial_height += 2 * SPACE;

	/* Draw timeline */
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_set_line_width (cr, 0.7);
	cairo_move_to(cr, p.x, initial_height);
	cairo_line_to(cr, p.x, least_height);
	cairo_stroke(cr);
}

void draw_event_device_link(cairo_t *cr, struct point p1, struct point p2)
{
	cairo_set_source_rgb(cr, 0.2, 0.5, 0.2);
	cairo_set_line_width (cr, 0.7);
	cairo_move_to(cr, p1.x, p1.y);
	cairo_line_to(cr, p2.x, p2.y);
	cairo_stroke(cr);
	cairo_arc (cr, p2.x, p2.y, 3.0, 0, 2 * PI);
	cairo_fill (cr);
}

void draw_devices_bar(cairo_t *cr,struct point p1,struct point p2)
{
	cairo_move_to(cr, p1.x, p1.y);
	cairo_set_source_rgb(cr, 0.96, 0.96, 0.96);
	cairo_set_line_width (cr, 0);
	cairo_rectangle(cr, p1.x, p1.y, p2.x, p2.y);
	cairo_fill(cr);
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
	struct point p, p1, p2, sp;

	struct device_t *d;

	GtkRequisition size;

	gpointer key, value;
	GHashTableIter iter;

	/* If necessary, drawing area is resized */
	gtk_widget_size_request(darea, &size);

	new_height = events_size * (R_H + SPACE);

	if(new_height > size.height){
		gtk_widget_set_size_request(darea, size.width, new_height);
		create_draw_pixmap(darea);
	}
	
	clear_pixmap(darea, size.width, size.height);

	/* Draw events */
	p.x = 0;
	p.y = 3 * SPACE;
	if(op & EVENT_SELECTED){
		selected_old = arg1;
		selected_new = arg2;
	}

	for(i = 0; i < events_size; i++){
		e = get_event(i);
		if(filter_event(e) == FALSE)
			continue;
		if(e->seq_number == event_selected_seq_number)
			draw_event(cairo_draw, e, p, EVENT_SELECTED);
		else
			draw_event(cairo_draw, e, p, 0);
		p.y += R_H + SPACE;
	}

	/* Draw timeline bar */
	p1.x = 0;
	p1.y = 0;
	p2.y = 3 * SPACE;
	p2.x = DAREA_W;
	draw_devices_bar(cairo_draw, p1, p2);

	/* Draw session timeline */
	sp.x = R_W + 4 * SPACE;
	sp.y = 2 * SPACE;
	draw_device_timeline(cairo_draw, session_timeline, sp);

	p.y = 2 * SPACE;
	p.x = sp.x + 10 * SPACE;

	/* Draw devices timelines */
	g_hash_table_iter_init (&iter, connected_devices);
	
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		d = (struct device_t *) value;
		if(d->is_active == FALSE)
			continue;
		draw_device_timeline(cairo_draw, d, p);
		d->x_position = p.x;
		p.x += 10 * SPACE;
	}

	p.x = 0;
	p.y = 3* SPACE;

	/* Draw events "links" */
	for(i = 0; i < events_size; i++){
		e = get_event(i);
		if(filter_event(e) && e->has_device){
			d = get_device( e->device_address);

			if(d == NULL)
				continue;
			p1.x = sp.x;
			p1.y = p.y + R_H / 2 + 2 * SPACE;
			p2.x = d->x_position;
			p2.y = p1.y;

			draw_event_device_link(cairo_draw, p1, p2);
		}
		p.y += R_H + SPACE;
	}

}

int find_event_at(struct point p, struct event_t **event_r)
{
	struct event_t *event;
	int id_event;

	id_event = (p.y - 4 * SPACE) / (R_H + SPACE);

	if( (p.y - 4 * SPACE) < 0 || id_event >= events_size)
		return -1;

	event = get_event(id_event);

	if(event == NULL){
		printf("Error: element with id %d does not exist\n",id_event);
		return -1;
	}

	*event_r = event;

	return id_event;
}

void destroy_widgets()
{

	gtk_widget_destroy(window);
	gtk_widget_destroy(packet_frame);
	gtk_widget_destroy(packet_detail);
	gtk_widget_destroy(darea);
	gtk_widget_destroy(draw_pixmap);

	cairo_destroy(cairo_draw);

	g_hash_table_destroy(connected_devices);

	g_array_free(events, TRUE);
}

gboolean on_destroy_event(GtkWidget *widget,GdkEventExpose *event,
								gpointer data)
{
	destroy_widgets();
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
		draw(0, 0, 0);
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
	
	GHashTableIter iter;
	char buff[MAX_BUFF];
	char aux[MAX_BUFF];
	
	gpointer key, value;
	p.x = (int)( (double) mouse_event->x);
	p.y = (int)( (double) mouse_event->y);

	id_event = find_event_at(p, &event);

	if(event == NULL || id_event == -1){
		/* Clear selection */
		if(event_selected_seq_number != -1){
			event_selected_seq_number = -1;
			draw(EVENT_SELECTED, -1, -1);
			gtk_widget_queue_draw(darea);

			gtk_widget_hide(packet_frame);
		}

		return TRUE;
	}

	/* Redraw event */
	if(event->seq_number != event_selected_seq_number){
		event_selected_seq_number = event->seq_number;

		draw(EVENT_SELECTED,event_selected_seq_number,
				event->seq_number);
		gtk_widget_queue_draw(darea);
		
		/* Show packet details */
		
		strcpy(buff, "");
		if(g_hash_table_size(event->attributes) > 0){
			g_hash_table_iter_init (&iter, event->attributes);
			strcpy(buff, "Attributes\n\n");
			while (g_hash_table_iter_next (&iter, &key, &value))
			{
				sprintf(aux,"%s : %s\n",(char *) key,(char *) value);
				strcat(buff, aux);
				
			}
			gtk_label_set_text(packet_detail, buff);
		}
		else{
			gtk_label_set_text(packet_detail, event->data);
		}
		
		gtk_widget_show(packet_frame);
	}

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

	/* For test only */
	e->has_device = 1;

	if(events_size % 3)
		strcpy(e->device_address, TEST_ADDRESS);
	else if(events_size % 2)
		strcpy(e->device_address, TEST_ADDRESS2);
	else
		strcpy(e->device_address, TEST_ADDRESS3);

	/* Add expose event handler, if it don't exist */
	if(draw_handler_id == 0)
		draw_handler_id = g_signal_connect(darea, "expose-event",
					G_CALLBACK(on_expose_event),NULL);
	/* Initial event */
	if(draw_pixmap == NULL){
		create_draw_pixmap(darea);
		draw(0, 0, 0);
	}

	need_draw = TRUE;
	create_draw_pixmap(darea);

	/* Launch darea expose event */
	gtk_widget_queue_draw(darea);
}

void on_device_dialog_response(GtkWidget *widget, GdkEventButton *mouse_event,
				gpointer user_data)
{
	GHashTableIter iter;
	GtkCheckButton *chk;
	gpointer key, value;
	struct device_t *d;
	
	/* */
	g_hash_table_iter_init (&iter, devices_check);
	
	while (g_hash_table_iter_next (&iter, &key, &value)){
		chk = (GtkCheckButton *) value;
		d = (struct device_t *) g_hash_table_lookup(connected_devices, (char *) key);
		d->is_active = gtk_toggle_button_get_active(chk);
	}
	
	/* Free resourses */
	gtk_widget_destroy(device_filters_dialog);
	device_filters_dialog = NULL;
	
	g_hash_table_destroy(devices_check);
	devices_check = NULL;
	/* Redraw */
	
	draw(0, 0, 0);
	gtk_widget_queue_draw(darea);
}

void create_device_filters_dialog()
{
	GtkWidget *dialog, *label, *content_area;
	GtkWidget *check;
	char buff[256];
	gpointer key, value;
	GHashTableIter iter;
	struct device_t *d;
   
	if(devices_check != NULL || device_filters_dialog != NULL)
		return;
	devices_check = g_hash_table_new (g_str_hash, g_str_equal);

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
		gtk_toggle_button_set_active(check, d->is_active);
		g_hash_table_insert(devices_check, d->address, check);
		gtk_container_add (GTK_CONTAINER (content_area), check);
	}

   g_signal_connect_swapped (device_filters_dialog,
                             "response",
                             G_CALLBACK (on_device_dialog_response),
                             devices_check);
   gtk_widget_show_all (device_filters_dialog);
}

void on_device_filters_click(GtkWidget *widget, GdkEventButton *mouse_event,
				gpointer user_data)
{
	create_device_filters_dialog();
}

int draw_init(int argc,char **argv,GMainLoop *loop)
{
	GtkWidget *sw, *viewport, *vbox, *menubar, *filemenu, *file, *quit;
	GtkWidget *filters, *filters_menu, *device_filter;
	GtkWidget *packet_frame_scroll, *packet_frame_details;
	GtkRequisition size;
	struct device_t *d;

	gtk_init(&argc, &argv);

	/* Init data structures */
	events = g_array_new(FALSE, FALSE, sizeof(struct event_t *));
	connected_devices = g_hash_table_new_full(g_str_hash,g_str_equal,
						g_hash_table_key_destroy,
						g_hash_table_value_destroy);
	draw_handler_id = 0;

	/* Set variables */
	mainloop = loop;
	window = gtk_window_new(0);
	darea = gtk_drawing_area_new();
	packet_frame = gtk_frame_new("Packet details");
	packet_frame_details = gtk_frame_new("");
	packet_detail = gtk_label_new("");
	sw = gtk_scrolled_window_new(NULL, NULL);
	packet_frame_scroll = gtk_scrolled_window_new(NULL, NULL);

	/* Session timeline */
	session_timeline = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(session_timeline->address, "");
	strcpy(session_timeline->name, "Session");

	session_timeline->id_initial_event = 0;
	session_timeline->id_least_event = -1;

	/* Devices for test */
	d = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(d->address, TEST_ADDRESS);
	strcpy(d->name, "Test");
	d->is_active = TRUE;
	d->id_initial_event = events_size;
	d->id_least_event = -1;

	add_device(d);

	d = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(d->address, TEST_ADDRESS2);
	strcpy(d->name, "Test2");
	d->is_active = TRUE;
	d->id_initial_event = events_size;
	d->id_least_event = -1;

	add_device(d);

	d = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(d->address, TEST_ADDRESS3);
	strcpy(d->name, "Test3");
	d->is_active = TRUE;
	d->id_initial_event = events_size;
	d->id_least_event = -1;

	add_device(d);

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

	/* Add scrollable drawing area */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox),
	 sw, TRUE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),
								darea);
	/* Add packet details frame */
	gtk_frame_set_shadow_type(packet_frame_details, GTK_SHADOW_NONE);   
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(packet_frame_scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_label_set_selectable(packet_detail, TRUE);
	gtk_container_add(GTK_CONTAINER(packet_frame_details), packet_detail);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(packet_frame_scroll),
								packet_frame_details);
	gtk_container_add(GTK_CONTAINER(packet_frame), packet_frame_scroll);
	gtk_box_pack_end(GTK_BOX(vbox), packet_frame, TRUE, TRUE, 0);
	
	/* Set events handlers */
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
	gtk_widget_hide(packet_frame);

	return 0;
}
