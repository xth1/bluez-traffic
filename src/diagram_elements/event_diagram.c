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

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "../event.h"
#include "../util.h"
#include "event_diagram.h"

#define TEXT_LEFT_MARGIN 5
#define TEXT_TOP_MARGIN 5

#define EVENT_BG_COLOR 0xffffffff
#define EVENT_SELECTED_BG_COLOR 0xefefefff
#define GAP_SIZE 8
#define SPACE 10

static CrItem *selected_event = NULL;

static GHashTable *events_diagram = NULL;

void events_diagram_key_destroy(gpointer data)
{
	if(data != NULL)
		free(data);
}

void events_diagram_value_destroy(gpointer data)
{
	if(data != NULL)
		free(data);
}

static gboolean
on_event_box_clicked(CrItem *item, GdkEvent *event, cairo_matrix_t *matrix, 
                CrItem *pick_item, char * user_data)
{
	static CrItem *last_item = NULL;
	static double init_x, init_y;
	static int last_msec = 0;
	guint color;
	char *data;
     
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			
			if(item != last_item){
				if(selected_event != NULL)
					g_object_set(selected_event, "fill_color_rgba", 
								EVENT_BG_COLOR, NULL);
				g_object_set(item, "fill_color_rgba", 
							EVENT_SELECTED_BG_COLOR, NULL);
				selected_event = item;
			}
			last_item = item;
			init_x = event->button.x;
			init_y = event->button.y;
			
			return TRUE;
		}

		break;
	case GDK_BUTTON_RELEASE:
		if (last_item) {
			last_item = NULL;
			return TRUE;
		}
		break;
	}

	return FALSE;
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
	
	struct event_diagram *ed;
	
	int *key;
	
	time_t t;
	struct tm tm;
	
	int size;
	
	char buff[512];	
	char *text = "retangle";
	
	/* store event diagram */
	ed = g_new(struct event_diagram, 1);
	
	ed->position = p;
	ed->event = e;
	
	key = g_new(int,1);
	*key = e->seq_number;
	g_hash_table_insert(events_diagram, key, ed);
	
	/* Set dimension */
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
	
	adapter_index_text = cr_text_new(rectangle, ppn.x, ppn.y, buff,
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

GHashTable *make_all_events(CrItem *group,GArray *events, int events_size,
						struct point p, int w, int h)
{
	struct event_t *e;
	int i;

	/* Create hash table */	
	if(events_diagram)
		g_hash_table_destroy(events_diagram);

	events_diagram = g_hash_table_new_full(g_int_hash, g_int_equal,
					events_diagram_key_destroy, events_diagram_value_destroy);
	
	for(i = 0; i < events_size; i++){
		e = g_array_index(events, void *, i);
		make_event(group, e, p, w, h);
		
		p.y += h;
	}
	
	return events_diagram;
}
