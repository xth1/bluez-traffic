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

#define EVENT_DIAGRAM_HEADER 1
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

#include "../data_dumped.h"
#include "../util.h"
#include "event_diagram.h"

#define TEXT_LEFT_MARGIN 5
#define TEXT_TOP_MARGIN 5

#define EVENT_BG_COLOR 0xffffffff
#define EVENT_SELECTED_BG_COLOR 0xefefefff
#define GAP_SIZE 8
#define SPACE 10

#define MONITOR_NEW_INDEX	0
#define MONITOR_DEL_INDEX	1
#define MONITOR_COMMAND_PKT	2
#define MONITOR_EVENT_PKT	3
#define MONITOR_ACL_TX_PKT	4
#define MONITOR_ACL_RX_PKT	5
#define MONITOR_SCO_TX_PKT	6
#define MONITOR_SCO_RX_PKT	7

#define CONTROL_BG_COLOR 0xfffdb1ff
#define CONTROL_SELECTED_BG_COLOR 0xfaf686ff
static guint event_bg_color[] = {
								 0xeeffeeff, /* MONITOR_NEW_INDEX*/
								 0xdededeff, /* MONITOR_DEL_INDEX*/
								 0xffeeeeff, /* MONITOR_COMMAND_PKT */
								 0xeeeeffff, /* MONITOR_EVENT_PKT */
								 0xeeffffff, /* MONITOR_ACL_TX_PKT */
								 0xffffeeff, /* MONITOR_ACL_RX_PKT */
								 0xeeeeeeff, /* MONITOR_SCO_TX_PKT */
								 0xefefefff /* MONITOR_SCO_RX_PKT */
								};
static guint event_selected_bg_color[] = {
								 0xddffddff, /* MONITOR_NEW_INDEX*/
								 0xcececeff, /* MONITOR_DEL_INDEX*/
								 0xffddddff, /* MONITOR_COMMAND_PKT */
								 0xddddffff, /* MONITOR_EVENT_PKT */
								 0xddffffff, /* MONITOR_ACL_TX_PKT */
								 0xffffddff, /* MONITOR_ACL_RX_PKT */
								 0xddddddff, /* MONITOR_SCO_TX_PKT */
								 0xdfdfdfff /* MONITOR_SCO_RX_PKT */
								};

static CrItem *selected_event = NULL;

static GHashTable *events_diagram = NULL;

static event_diagram_callback event_callback;

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

guint get_event_bg_color(struct event_t *e)
{
	if(e->socket == HCI_CHANNEL_MONITOR)
		return event_bg_color[e->type];
	return CONTROL_BG_COLOR;
}

guint get_event_selected_bg_color(struct event_t *e)
{
	if(e->socket == HCI_CHANNEL_MONITOR)
		return event_selected_bg_color[e->type];
	return CONTROL_SELECTED_BG_COLOR;
}

static gboolean
on_event_box_clicked(CrItem *item, GdkEvent *event, cairo_matrix_t *matrix,
                CrItem *pick_item, gpointer *user_data)
{
	static CrItem *last_item = NULL;
	static struct event_t *last_event = NULL;
	static double init_x, init_y;
	static int last_msec = 0;
	guint color;

	struct event_diagram *ed = (struct event_diagram *)user_data;
	struct event_t *e = ed->event;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {

			if(item != last_item){
				if(selected_event != NULL){
					g_object_set(selected_event, "fill_color_rgba",
								get_event_bg_color(last_event),
								"line_width", 0.5,
								"outline_color_rgba", get_event_bg_color(last_event), NULL);

				}
				g_object_set(item, "fill_color_rgba",
							get_event_selected_bg_color(e),
							"line_width", 2.5,
							"outline_color_rgba", 0xcdcdcdff,
							NULL);
				selected_event = item;

				/* Call event callback function */
				event_callback(ed, EVENT_SELECTED);

			}

			last_item = item;
			last_event = e;
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

guint select_event_bg_color(int opcode)
{
	guint color;
	switch (opcode) {

		default:
			color = 0xffffffff;
	}

	printf("Opcode %u color %x\n",opcode,color);
	return color;
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
	char buff[512];

	struct point pp;
	struct point ppn;
	struct point dim;

	struct event_diagram *ed;

	int *key;

	time_t t;
	struct tm tm;
	int size;


	/* store event diagram */
	ed = g_new(struct event_diagram, 1);

	ed->position = p;
	ed->event = e;

	key = g_new(int, 1);
	*key = e->seq_number;
	g_hash_table_insert(events_diagram, key, ed);

	/* Set dimension */
	dim.x = w;
	dim.y = h;

	/* Draw rectangle */
	color = get_event_bg_color(e);
	rectangle = cr_rectangle_new(group, p.x, p.y, w, h,
                        "line_scaleable", FALSE,
                        "line_width", 0.5,
                        "outline_color_rgba", color,
                        "fill_color_rgba", color,
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

	operation_code_text = cr_text_new(rectangle, ppn.x, ppn.y, buff,
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
					(GCallback) on_event_box_clicked, ed);
}

GHashTable *make_all_events(CrItem *group, GArray *events,
						event_diagram_callback callback, int events_size,
						struct point p, int w, int h)
{
	struct event_t *e;
	int i;

	event_callback = callback;

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

	printf("End make_all_events\n");
	return events_diagram;
}
