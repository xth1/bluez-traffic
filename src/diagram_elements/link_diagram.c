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

#define LINK_CONNECTION_COLOR 0x9999ffdcc
#define LINK_CONNECTION_UPPER_COLOR 0x9999ffdfe

#define CIRCLE_RADIUS 5
#define CIRCLE_COLOR 0x9999ffdfe

void add_circle(CrItem *group, struct point p)
{
	CrItem *circle;
	cr_ellipse_new(group, p.x , p.y,CIRCLE_RADIUS,CIRCLE_RADIUS, 0,
					"outline_color_rgba", CIRCLE_COLOR,
					"line_width", 1.5,
					NULL);
}

void add_comunication_link(CrItem *group, struct point p1, struct point p2, int dir)
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
					"fill_color_rgba", color,
					"end_scaleable", FALSE,
					"line_scaleable", FALSE,
					"line_width", 1.5,
					NULL);
		
	/*add_circle(link, p1);*/
	cr_arrow_new(link, 0, NULL);	
}

void make_all_comunication_links(CrItem *group,GHashTable *events_diagram, 
					GHashTable *devices_diagram, int event_box_width)
{
	gpointer key, value;
	GHashTableIter iter;
	
	struct event_t *e;
	struct device_t *d;
	
	struct event_diagram *ed;
	struct device_diagram *dd;
	
	struct point p_ev, p_dev;
	
	struct point p1, p2, p3, p4;
	
	struct point *pp;
	
	gpointer pointer;
	
	char *address_key;
	
	g_hash_table_iter_init (&iter, events_diagram);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		
		ed = (struct event_diagram *) value;
		
		e = ed->event;
		
		if(e->has_device == FALSE)
			continue;
		
		dd = g_hash_table_lookup(devices_diagram, e->device_address);
		if(e->is_device_connection == FALSE){ 
			/* draw comunicacion link */
			p_ev = ed->position;
			
			p_ev.x += event_box_width;
			
			p_dev.x = dd->position.x;
			p_dev.y = ed->position.y;
			
			if(e->direction == DIR_RIGHT)
					add_comunication_link(group, p_ev, p_dev, e->direction);
			else
					add_comunication_link(group, p_dev, p_ev, e->direction);
		}	
	}
}

void add_connection_link(CrItem *group, struct point p1, struct point p2, 
						struct point p3, struct point p4, gboolean has_end_point)
{
	CrItem *link1, *link2;
	CrItem *upper_line;
	guint color;
	
/*	printf("Add connection link \n");
	
	printf("[%d %d]\n",p1.x, p1.y);	
	printf("[%d %d]\n",p2.x, p2.y);
	printf("[%d %d]\n",p3.x, p3.y);
	printf("[%d %d]\n",p4.x, p4.y);
*/	
	/* Draw connection link 1 */
	if(has_end_point){
		link1 = cr_vector_new(group, p1.x , p1.y, 
					p3.x - p1.x, p3.y - p1.y,
					"outline_color_rgba", LINK_CONNECTION_COLOR,
					"end_scaleable", FALSE,
					"line_scaleable", FALSE,
					"line_width", 1.5,
					NULL);
	}
	
	/* Draw connection link 2 */
	link2 = cr_vector_new(group, p2.x , p2.y, 
				p4.x - p2.x, p4.y - p4.y,
				"outline_color_rgba", LINK_CONNECTION_COLOR,
				"end_scaleable", FALSE,
				"line_scaleable", FALSE,
				"line_width", 1.5,
				NULL);
	
	/* Draw line up timeline */
	upper_line = cr_vector_new(group, p3.x , p3.y, 
					p4.x - p3.x, p4.y - p3.y,
					"outline_color_rgba", LINK_CONNECTION_UPPER_COLOR,
					"end_scaleable", FALSE,
					"line_scaleable", FALSE,
					"line_width", 1.5,
					NULL);
	
	/* Draw circles */
	add_circle(group, p3);
	add_circle(group, p4);	
	
}

void make_all_connection_links(CrItem *group,GHashTable *events_diagram, 
					GHashTable *devices_diagram, int event_box_width)
{
	gpointer key, value;
	GHashTableIter iter;
	
	GHashTable *device_connection;
	
	struct event_t *e;
	struct device_t *d;
	
	struct event_diagram *ed;
	struct device_diagram *dd;
	
	struct point p_ev, p_dev;
	
	struct point p1, p2, p3, p4;
	
	struct point *pp;
	
	gpointer pointer;
	
	char *address_key;
	
	device_connection = g_hash_table_new(g_str_hash, g_str_equal); 
		
	g_hash_table_iter_init (&iter, events_diagram);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		ed = (struct event_diagram *) value;
		
		e = ed->event;
		
		if(e->has_device == FALSE)
			continue;
		
		dd = g_hash_table_lookup(devices_diagram, e->device_address);
		
		if(e->is_device_connection){ 
			/* draw connection link */

			pointer = g_hash_table_lookup(device_connection, 
										e->device_address);	
			if(pointer != NULL){

				pp = ( (struct point *)pointer);
				
				p_ev = *pp;
				
				p1 = ed->position;
				p1.x += event_box_width;
				
				p2 = p_ev;
				p2.x += event_box_width;
				
				p3.x = (dd->position).x;
				p3.y = p1.y;
				
				p4.x = (dd->position).x;
				p4.y = p2.y;

				add_connection_link(group, p1, p2, p3, p4, TRUE);
				
				g_free(pp);
				
				g_hash_table_remove(device_connection, e->device_address);
			}
			else{
				pp = g_new(struct point, 1);
				*pp = ed->position;
				g_hash_table_insert(device_connection, e->device_address, pp);
			}
		}
	}
	
	/* complete draw connection link */
	g_hash_table_iter_init (&iter, device_connection);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		if( value == NULL  || key == NULL)
			continue;
		
		address_key = (char *) key;
		pp = (struct point *) value;
		
		p_ev = *pp;
		
		p1.x = 0;
		p1.y = 0;
		
		p2 = p_ev;
		p2.x += event_box_width;
		
		p3.x = (dd->position).x;
		p3.y = (dd->position).y;
		
		p4.x = (dd->position).x;
		p4.y = p2.y;
		
		add_connection_link(group, p1, p2, p3, p4, FALSE);
		
		g_free(pp);
		
		g_hash_table_remove(device_connection, e->device_address);
	}
	
	g_hash_table_key_destroy(device_connection);
}

void make_all_links(CrItem *group,GHashTable *events_diagram, 
					GHashTable *devices_diagram, int event_box_width)
{
	make_all_connection_links(group, events_diagram, devices_diagram, 
								event_box_width);

	make_all_comunication_links(group, events_diagram, devices_diagram, 
								event_box_width);
}
