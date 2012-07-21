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
 */
 
#include "event.h"

#include <stdlib.h>
#include <time.h>
/* For tests only */
#define TEST_ADDRESS "20:00:00:01:01:21"
#define TEST_ADDRESS2 "20:00:00:34:31:13"
#define TEST_ADDRESS3 "20:00:00:34:43:13"
#define INF 999999999

/* Storage limit of events */
static events_limit = INF;
 
/* Array of events */
static GArray *events;
static int events_size;
/* Callback */

events_update_callback update_callback = NULL;

/* Connected devices */
static GHashTable *connected_devices;

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

void events_init(events_update_callback callback, int ev_lim)
{
		srand(time(0));
	struct device_t *d;
	events = g_array_new(FALSE, FALSE, sizeof(struct event_t *));
	connected_devices = g_hash_table_new_full(g_str_hash,g_str_equal,
						g_hash_table_key_destroy,
						g_hash_table_value_destroy);
	
	events_limit = ev_lim;
	
	update_callback = callback;

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

}

void set_events_update_callback(events_update_callback callback)
{
	update_callback = callback;
}

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

void add_event(struct event_t *e)
{
	if( events_size == events_limit){
		g_array_remove_index(events, events_size -1);
		g_array_prepend_val(events, e);
	}
	else{
		g_array_prepend_val(events, e);
		events_size++;
	}
	
	/* For test only */
	e->has_device = 1;

	if(rand() % 3)
		strcpy(e->device_address, TEST_ADDRESS);
	else if(rand() % 2)
		strcpy(e->device_address, TEST_ADDRESS2);
	else
		strcpy(e->device_address, TEST_ADDRESS3);
	
	if(events_size % 2)
		e->direction = -1;
	else
		e->direction = 1;
	
	if(update_callback)
		update_callback(events, events_size, connected_devices);
}

struct event_t *get_event(int p)
{
	return g_array_index(events, void *, p);
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
