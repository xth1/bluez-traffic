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

#ifndef EVENT_HEADER
#include "event.h"
#endif

#include "util.h"
#include "diagram_elements/event_diagram.h"
#include "diagram_elements/device_diagram.h"
#include "diagram_elements/link_diagram.h"

#define EVENT_BOX_W 375
#define EVENT_BOX_H 30
#define SPACE 10

#define EVENT_BOX_LEFT_MARGIN 0
#define EVENT_BOX_TOP_MARGIN 60

static GtkWidget *diagram = NULL;

static GArray *events_list = NULL;
static int events_size = -1;
static GHashTable *devices_hash = NULL;


static GHashTable *devices_diagram = NULL;
static GHashTable *events_diagram = NULL;


//void make_all_links(CrItem *group,

static void do_remove(CrItem *child, CrItem *parent)
{
        cr_item_remove(parent, child);
}

static void on_clear(CrItem *group)
{
        g_list_foreach(group->items, (GFunc) do_remove, group);
}

gboolean diagram_update(GArray *events, int size, GHashTable *devices)
{	
	struct event_t *e;
	struct device_t *d;
	
	struct event_diagram *ed;
	struct device_diagram *dd;
	struct point p;

	int line_size;
	CrItem *root;
	int i;
	
	gpointer key, value;
	GHashTableIter iter;
	
	/*Buggy: without following line this program crash (!?)*/
	printf("Update diagram\n");
	
	/* Set global vaiables */
	events_list = events;
	events_size = size;
	devices_hash = devices;

	/* Get root item */
	g_object_get(diagram, "root", &root);

	/* Clear */
	on_clear(root);
	
	/* Draw all events */
	p.x = EVENT_BOX_LEFT_MARGIN;
	p.y = EVENT_BOX_TOP_MARGIN;
	events_diagram = make_all_events(root, events, size, p, 
									EVENT_BOX_W, EVENT_BOX_H);
	
	/* Draw all devices timeline */
	line_size = events_size * EVENT_BOX_H + EVENT_BOX_TOP_MARGIN;
	
	p.x = EVENT_BOX_W / 2 + 6 * SPACE;
	p.y = 0;
	
	devices_diagram = make_all_devices_timeline(root, devices_hash, p, line_size);
	
	/* Draw all links */
	printf("draw links");
	/* Half of EVENT_BOX_W because CrCanvas positioning system (better it)*/
	make_all_links(root, events_diagram, devices_diagram, EVENT_BOX_W / 2);
	
	/* Test iteration */
	g_hash_table_iter_init (&iter, devices_diagram);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		dd = (struct event_diagram *) value;
		printf("%s : %d %d\n", (dd->device)->address,(dd->position).x,(dd->position).y);		
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
	
	set_events_update_callback(diagram_update);

	return diagram;
}
