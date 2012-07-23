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
 
#define MAX_NUM_ADAPTERS 64 

#include <glib.h>
#include <string.h>
#include "data_dumped.h"
#include "util.h"

/* Filter parameters */
static GHashTable *devices_filter = NULL;
static int adapters_filter[MAX_NUM_ADAPTERS];

/* Filtered data */
static GHashTable *devices_filtered = NULL;
static GArray *events_filtered = NULL;
static int events_filtered_size = 0;

typedef gboolean (*event_filter_function) (struct event_t *event);

void filter_set_active_device(struct device_t *device, gboolean active)
{

	gpointer pointer;

	if(active){

		pointer = g_hash_table_lookup(devices_filter, device->address);

		if(pointer)
			return;

		g_hash_table_insert(devices_filter, device->address, device);		
	}
	else{
		pointer = g_hash_table_lookup(devices_filter, device->address);
		
		if(pointer)
			g_hash_table_remove(devices_filter, device->address);
	}
}

gboolean filter_is_device_active(struct device_t *device)
{
	return has_key(devices_filter, device->address);
}

void filter_init()
{
	if(devices_filter)
		g_hash_table_destroy(devices_filter);
	devices_filter = g_hash_table_new(g_str_hash, g_str_equal);

	memset(adapters_filter, 0, sizeof(adapters_filter));
}

void filter_devices(GHashTable *devices)
{
	struct device_t *d;

	gpointer key, value;
	GHashTableIter iter;	

	if(devices_filtered)
		g_hash_table_destroy(devices_filtered);
	devices_filtered = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_iter_init (&iter, devices);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		d = (struct device_t *) value;

		if(has_key(devices_filter, d->address)){
			g_hash_table_insert(devices_filtered, d->address, d);
		}
	}
}

gboolean filter_event_by_device(struct event_t *e)
{
	if(e->has_device && !has_key(devices_filter, e->device_address))
		return FALSE;
	return TRUE;
}

#define EVENT_FILTER_FUNCTIONS_SIZE 1
event_filter_function event_filter_functions[] = {
					filter_event_by_device
};

void filter_events(GArray *events, int events_size)
{
	int i, j;

	struct event_t *e;

	gboolean event_accepted;

	if(events_filtered)
		g_array_free(events_filtered, FALSE);
	events_filtered = g_array_new(TRUE, TRUE, sizeof(struct event_t *));

	events_filtered_size = 0;
	for(i = 0; i < events_size; i++){
	
		event_accepted = TRUE;

		e = g_array_index(events, void *, i);

		for(j = 0; j < EVENT_FILTER_FUNCTIONS_SIZE; j++){
			event_accepted &= event_filter_functions[j](e);
		}

		if(event_accepted){
			g_array_append_val(events_filtered, e);
			events_filtered_size++;
			printf("Accept: %d\n", events_filtered_size);
		}
	}
}

struct data_dumped_t *filter(struct data_dumped_t *data_in)
{
	struct data_dumped_t *data_out;

	data_out = g_new(struct data_dumped_t, 1);

	filter_devices(data_in->devices);
	filter_events(data_in->events, data_in->events_size);

	data_out->events = events_filtered;
	data_out->events_size = events_filtered_size;
	data_out->devices = devices_filtered;
}
