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

#include "../util.h"
#include "event_diagram.h"
#include "device_diagram.h"

#define LINK_RIGHT_COLOR 0xff9999fe
#define LINK_LEFT_COLOR 0x99ff99dfe

#define CIRCLE_RADIUS 3
#define CIRCLE_COLOR 0x9999ffdfe

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

void make_all_links(CrItem *group,GHashTable *events_diagram, 
					GHashTable *devices_diagram, int event_box_width)
{
	
	gpointer key, value;
	GHashTableIter iter;
	
	struct event_t *e;
	struct device_t *d;
	
	struct event_diagram *ed;
	struct device_diagram *dd;
	
	struct point p_ev, p_dev;
	
	g_hash_table_iter_init (&iter, events_diagram);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		ed = (struct event_diagram *) value;
		
		e = ed->event;
		
		dd = g_hash_table_lookup(devices_diagram, e->device_address);
		
		p_ev = ed->position;
		
		p_ev.x += event_box_width;
		
		p_dev.x = dd->position.x;
		p_dev.y = ed->position.y;
		
		if(e->direction == DIR_RIGHT)
				add_link(group, p_ev, p_dev, e->direction);
		else
				add_link(group, p_dev, p_ev, e->direction);

	}
}
