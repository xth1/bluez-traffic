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
#include "device_diagram.h"
 
#define SPACE 10

static GHashTable *devices_diagram = NULL;

/* HASH TABLE FUNCTIONS */
void devices_diagram_key_destroy(gpointer data)
{
	/* do nothing */
}

void devices_diagram_value_destroy(gpointer data)
{
	if(data != NULL)
		free(data);
}

static gboolean on_device_clicked(CrItem *item, GdkEvent *event, 
			cairo_matrix_t *matrix, CrItem *pick_item, char * user_data)
{
	printf("Device clicked\n");
	
	return TRUE;	
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
	
	struct device_diagram *dd;
	
	/* store device diagram */
	dd = g_new(struct device_diagram, 1);
	
	dd->position = p;
	dd->device = d;
	
	g_hash_table_insert(devices_diagram, d->address, dd);
	
	/* draw device timeline */
	
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
}

GHashTable *make_all_devices_timeline(CrItem *group,GHashTable *devices_hash,
						struct point p,int line_size)
{
	gpointer key, value;
	GHashTableIter iter;
	struct device_t *d;
	/* Create hash table */	
	if(devices_diagram)
		g_hash_table_destroy(devices_diagram);

	devices_diagram = g_hash_table_new_full(g_str_hash, g_str_equal,
					devices_diagram_key_destroy, devices_diagram_value_destroy);
	printf("Device Init\n");
	g_hash_table_iter_init (&iter, devices_hash);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		d = (struct device_t *) value;
		make_device_timeline(group, d, p, line_size);

		p.x += 6 * SPACE;

	}
	
	return devices_diagram;
}
