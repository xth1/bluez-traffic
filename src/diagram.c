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
 
#include <math.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <cr-canvas.h>
#include <cr-pixbuf.h>
#include <cr-text.h>
#include <cr-line.h>
#include <cr-rectangle.h>
#include <cr-ellipse.h>
#include <cr-vector.h>
#include <cr-zoomer.h>
#include <cr-panner.h>
#include <cr-rotator.h>
#include <cr-inverse.h>
#include <cr-arrow.h>

#include <glib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "event.h"
#include "util.h"


#define EVENT_BOX_W 375
#define EVENT_BOX_H 30

#define EVENT_BOX_LEFT_MARGIN 0
#define EVENT_BOX_TOP_MARGIN 60
#define EVENT_BG_COLOR 0xffffffff
#define EVENT_SELECTED_BG_COLOR 0xefefefff
#define GAP_SIZE 8
#define SPACE 10

#define TEXT_LEFT_MARGIN 5
#define TEXT_TOP_MARGIN 5

#define LINK_RIGHT_COLOR 0xff9999fe
#define LINK_LEFT_COLOR 0x99ff99dfe

#define CIRCLE_RADIUS 3
#define CIRCLE_COLOR 0x9999ffdfe

static *selected_item = NULL;
static GtkWidget *diagram = NULL;

static GArray *events_list = NULL;
static int events_size = -1;
static GHashTable *devices_hash = NULL;
/*static *canvas = NULL;*/

static gboolean
on_event_box_clicked(CrItem *item, GdkEvent *event, cairo_matrix_t *matrix, 
                CrItem *pick_item, char * user_data)
{
	static CrItem *last_item = NULL;
	static double init_x, init_y;
	static int last_msec = 0;
	guint color;
	char *data;
	fflush(stdin);
	if ( user_data != NULL)
		printf("User data %s \n", user_data);
	else
		printf("User data is null\n");
      
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		color = g_random_int_range(0,255) << 24 |
		g_random_int_range(0,255) << 16 |
		g_random_int_range(0,255) << 8 |
		0xff;					
		
		if (event->button.button == 1) {
			
			if(item != last_item){
				if(selected_item != NULL)
					g_object_set(selected_item, "fill_color_rgba", 
								EVENT_BG_COLOR, NULL);
				g_object_set(item, "fill_color_rgba", 
							EVENT_SELECTED_BG_COLOR, NULL);
				selected_item = item;
			}
			last_item = item;
			init_x = event->button.x;
			init_y = event->button.y;
			
			
			return TRUE;
		}

		break;
	/*case GDK_MOTION_NOTIFY:
		if (last_item && event->motion.time - last_msec >= 10) 
		{
			cairo_matrix_translate(cr_item_get_matrix(item),
							event->motion.x - init_x,
							event->motion.y - init_y);
			cr_item_request_update(item);
			last_msec = event->motion.time;
			return TRUE;
		}
		else if( last_item == NULL){
			color = g_random_int_range(0,255) << 24 |
			g_random_int_range(0,255) << 16 |
			g_random_int_range(0,255) << 8 |
			0xff;

			g_object_set(item, "fill_color_rgba", color, NULL);
			return TRUE;
		}
		break;
		*/
	case GDK_BUTTON_RELEASE:
		if (last_item) {
			last_item = NULL;
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static gboolean
on_device_clicked(CrItem *item, GdkEvent *event, cairo_matrix_t *matrix, 
                CrItem *pick_item, char * user_data)
{
	printf("Device clicked\n");
	
	return TRUE;	
}
                
                
struct point get_pos(struct point p, struct point ref)
{
	struct point r;
	
	r.x = -1 * ref.x / 2 + p.x;
	r.y = -1 * ref.y / 2 + p.y;
	
	return r; 
}

static void make_event(CrItem *group,struct event_t *e, 
						struct point p,double w, double h)
{
	CrItem *rectangle;
	CrItem *date_text;
	CrItem *time_text;
	CrItem *adapter_index_text;
	CrItem *operation_code_text;
	CrItem *name_text;
	guint color;
	
	struct point pp;
	struct point ppn;
	struct point dim;
	
	time_t t;
	struct tm tm;
	
	int size;
	
	char buff[512];	
	char *text = "retangle";
	
	dim.x = w;
	dim.y = h;
	/* Draw rectangle */
	rectangle = cr_rectangle_new(group, p.x, p.y, w, h,
                        "line_scaleable", FALSE,
                        "line_width", 0.0,
                        "fill_color_rgba", EVENT_BG_COLOR,
                        "data","data here",
                        NULL);
        
	/* Print date */
	t = (e->tv).tv_sec;
	localtime_r(&t, &tm);
	
	sprintf(buff,"%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
							tm.tm_mday);
	size = strlen(buff);
	
	pp.x = TEXT_LEFT_MARGIN; 
	pp.y = TEXT_TOP_MARGIN;
	ppn = get_pos(pp, dim);
	date_text = cr_text_new(rectangle, ppn.x, ppn.y, buff,

                        "font", "Courier 6",
                        "anchor", GTK_ANCHOR_NW,
                        "use-markup", TRUE,
                        "fill_color_rgba", 0x000000ffL, NULL);
                        
   /* Print time */
    sprintf(buff, "%02d:%02d:%02d.%06lu ", tm.tm_hour, tm.tm_min,
					 tm.tm_sec, (e->tv).tv_usec);
	
	pp.x = TEXT_LEFT_MARGIN; 
	pp.y = 3 * TEXT_TOP_MARGIN;
	ppn = get_pos(pp, dim);
	time_text = cr_text_new(rectangle, ppn.x, ppn.y, buff,
                        "font", "Courier 6",
                        "anchor", GTK_ANCHOR_NW,
                        "use-markup", TRUE,
                        "fill_color_rgba", 0x000000ffL, NULL);

	/* Print adapter index */

	if (e->socket == HCI_CHANNEL_MONITOR)
		sprintf(buff, "[ hci%d ]", e->adapter_index);
	else
		sprintf(buff, "{ hci%d }", e->adapter_index);

	pp.x = size * GAP_SIZE + SPACE;
	pp.y = TEXT_TOP_MARGIN;
	ppn = get_pos(pp, dim);
	
	adapter_index_text =cr_text_new(rectangle, ppn.x, ppn.y, buff,
                        "font", "Courier Bold 6",
                        "anchor", GTK_ANCHOR_NW,
                        "use-markup", TRUE,
                        "fill_color_rgba", 0xdd0000ffL, NULL);

	size = strlen(buff);
	
	/* Print operation code */
	
	if(strcmp(e->type_str, "") == 0)
		sprintf(buff, "OPCODE: 0x%2.2x", e->type);
	else
		sprintf(buff, "%s", e->type_str);

	pp.x += size * GAP_SIZE + SPACE;
	pp.y = TEXT_TOP_MARGIN;
	ppn = get_pos(pp, dim);
	
	operation_code_text =cr_text_new(rectangle, ppn.x, ppn.y, buff,
                        "font", "Courier Medium 8",
                        "anchor", GTK_ANCHOR_NW,
                        "use-markup", TRUE,
                        "fill_color_rgba", 0x3333eeffL, NULL);
                        
    /* Print name */

	sprintf(buff, "%s", e->name);

	pp.x = 10 * SPACE;
	pp.y = 3 * TEXT_TOP_MARGIN;
	ppn = get_pos(pp, dim);
	name_text = cr_text_new(rectangle, ppn.x, ppn.y, buff,
                        "font", "Courier Medium 6",
                        "anchor", GTK_ANCHOR_NW,
                        "use-markup", TRUE,
                        "fill_color_rgba", 0x333333ffL, NULL);

	/* Signal event */
	g_signal_connect(rectangle, "event", 
					(GCallback) on_event_box_clicked, e);
}

void add_circle(CrItem *group, struct point p)
{
	CrItem *circle;
	cr_ellipse_new(group, p.x , p.y,CIRCLE_RADIUS,CIRCLE_RADIUS, 0,
					"outline_color_rgba", CIRCLE_COLOR,
					"line_width", 1.5,
					NULL);
}

void add_link(CrItem *group, struct point p1, struct point p2, int dir)
{
	CrItem *link;
	
	unsigned long long int color;
	
	if(dir == DIR_LEFT)
		color = LINK_RIGHT_COLOR;
	else
		color = LINK_LEFT_COLOR;
	
	link = cr_vector_new(group, p1.x , p2.y, 
					p2.x - p1.x, p2.y - p1.y,
					"outline_color_rgba", color,
					"end_scaleable", FALSE,
					"line_scaleable", FALSE,
					"line_width", 1.5,
					NULL);
		
	/*add_circle(link, p1);*/
	cr_arrow_new(link, 0, NULL);
	
}

void make_device_timeline(CrItem *group, struct device_t *d, 
							struct point p, int line_size)	
{
	CrItem *device_name_text;
	CrItem *timeline;
	int i;
	
	struct point p2;
	struct point p_ev, p_dev;
	
	struct event_t *e;
	
	p2 = p;
	
	p2.y += line_size;
	
	printf("make timeline\n");
	
	/* Print device name */
	
	device_name_text = cr_text_new(group, p.x, p.y, d->name,
                        "font", "Courier Medium 8",
                        "anchor", GTK_ANCHOR_NW,
                        "use-markup", TRUE,
                        "fill_color_rgba", 0x000000ffL, NULL);

	/* Draw timeline */
	timeline = cr_vector_new(group, p.x , p.y + SPACE, 0, 
						p2.y + 3 * SPACE,
                        "outline_color_rgba", 0xccccccff,
                        "end_scaleable", FALSE,
                        "line_scaleable", FALSE,
                        "line_width", 1.5,
                        NULL);
	
	g_signal_connect(device_name_text, "event", 
					(GCallback) on_device_clicked, d);
	
	/* Add links */
	p_ev.x = EVENT_BOX_W / 2;
	p_ev.y = EVENT_BOX_TOP_MARGIN;
	
	p_dev.x = p.x;
	
	for(i = 0; i < events_size; i++){
		
		e = g_array_index(events_list, void *, i);
		
		if(strcmp(e->device_address,d->address) == 0){
			p_dev.y = p_ev.y;
			
			if(e->direction == 1)
				add_link(group, p_ev, p_dev, e->direction);
			else
				add_link(group, p_dev, p_ev, e->direction);
			
		}
		p_ev.y += EVENT_BOX_H;
	}	
}

static void do_remove(CrItem *child, CrItem *parent)
{
        cr_item_remove(parent, child);
}

static void on_clear(CrItem *group)
{
        g_list_foreach(group->items, (GFunc) do_remove, group);
}

gboolean update_events(GArray *events, int size, GHashTable *devices)
{
	int i;
	int line_size;	
	struct event_t *e;
	struct device_t *d;
	struct point p;
	CrItem *root;
	
	gpointer key, value;
	GHashTableIter iter;
	
	/*Buggy*/
	printf("Update diagram\n");
	
	/* Set global vaiables */
	events_list = events;
	events_size = size;
	
	devices_hash = devices;
	
	p.x = EVENT_BOX_LEFT_MARGIN;
	p.y = EVENT_BOX_TOP_MARGIN;
	
	/* Clear */
	g_object_get(diagram, "root", &root);
	on_clear(root);
	
	/* Draw all events */
	p.x = EVENT_BOX_LEFT_MARGIN;
	p.y = EVENT_BOX_TOP_MARGIN;
	for(i = 0; i < size; i++){
		e = g_array_index(events, void *, i);
		make_event(root, e, p, EVENT_BOX_W, EVENT_BOX_H);
		
		p.y += EVENT_BOX_H;
	}
	
	line_size = events_size * EVENT_BOX_H + EVENT_BOX_TOP_MARGIN;
	p.x = EVENT_BOX_W / 2 + 6 * SPACE;
	p.y = 0;

	g_hash_table_iter_init (&iter, devices_hash);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		d = (struct device_t *) value;
		make_device_timeline(root, d, p, line_size);

		p.x += 6 * SPACE;		
	}

	return TRUE;
}

GtkWidget *create_diagram(int param,int width,int height)
{
	CrZoomer *zoomer;
	CrPanner *panner;
	CrRotator *rotator;

	diagram = cr_canvas_new("maintain_aspect", TRUE,
						"auto_scale", FALSE,
						"maintain_center", TRUE, NULL);
	cr_canvas_set_scroll_factor(CR_CANVAS(diagram), 3, 3);
	
	gtk_widget_set_size_request(diagram, width, height);
	
	panner = cr_panner_new(CR_CANVAS(diagram), "button", 1, NULL);
	
	cr_canvas_center_on (diagram, 0, height/2);
	
	set_events_update_callback(update_events);

	return diagram;
}
