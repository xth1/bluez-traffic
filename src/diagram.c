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
#include <gdk/gdkkeysyms.h>
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

#ifndef DATA_DUMPED_HEADER
#include "data_dumped.h"
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

static CrItem *root;

/* Callback function for event */
static event_diagram_callback event_callback;

static void do_remove(CrItem *child, CrItem *parent)
{
        cr_item_remove(parent, child);
}

static void on_clear(CrItem *group)
{
        g_list_foreach(group->items, (GFunc) do_remove, group);
}

gboolean on_key_press (GtkWidget *widget, GdkEventKey *event,
				gpointer user_data)
{
	switch(event->keyval)
	{
		case GDK_Up:
			printf("UP\n");
			break;
		case GDK_Down:
			printf("Down\n");
			break;
	}
	printf("key press: %d\n",event->keyval);
	return TRUE;
}

gboolean diagram_update(GArray *events, int size, GHashTable *devices)
{
	struct event_t *e;
	struct device_t *d;

	struct event_diagram *ed;
	struct device_diagram *dd;
	struct point p;

	int line_size;
	int i;

	gpointer key, value;
	GHashTableIter iter;

	/* Set global variables */
	events_list = events;
	events_size = size;
	devices_hash = devices;


	/* Clear diagram */
	on_clear(root);

	/* Make all events */
	p.x = EVENT_BOX_LEFT_MARGIN;
	p.y = EVENT_BOX_TOP_MARGIN;
	events_diagram = make_all_events(root, events, event_callback, size,
							p, EVENT_BOX_W, EVENT_BOX_H);

	/* Make all devices timeline */
	line_size = events_size * EVENT_BOX_H + EVENT_BOX_TOP_MARGIN;

	p.x = EVENT_BOX_W / 2 + 6 * SPACE;
	p.y = 0;

	devices_diagram = make_all_devices_timeline(root, devices_hash, p,
						line_size);

	/* Make all links */
	/* Half of EVENT_BOX_W to use  CrCanvas positioning system */
	make_all_links(root, events_diagram, devices_diagram, EVENT_BOX_W / 2);

	return TRUE;
}

GtkWidget *create_diagram(int param, int width, int height,
							event_diagram_callback ev_callback)
{
	CrZoomer *zoomer;
	CrPanner *panner;
	CrRotator *rotator;

	GdkColor white = {0, 0xffff, 0xffff, 0xffff};

	event_callback = ev_callback;

	diagram = cr_canvas_new("maintain_aspect", TRUE,
				"auto_scale", FALSE,
				"maintain_center", TRUE,
				"repaint_mode", TRUE,
				NULL);

	cr_canvas_set_scroll_factor(CR_CANVAS(diagram), 3, 3);

	gtk_widget_set_size_request(diagram, width, height);
	panner = cr_panner_new(CR_CANVAS(diagram), "button", 1, NULL);
	cr_canvas_center_on (diagram, 0, height / 2);

	data_dumped_set_update_callback(diagram_update);

	gtk_widget_modify_bg(diagram, GTK_STATE_NORMAL, &white);

	/* Get root item */
	g_object_get(diagram, "root", &root);

	g_signal_connect (G_OBJECT (diagram), "key_press_event",
					G_CALLBACK (on_key_press), NULL);

	return diagram;
}
