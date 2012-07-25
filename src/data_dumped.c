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

#include "data_dumped.h"

#include "filter.h"

#include <stdlib.h>
#include <time.h>
/* For tests only */
#define TEST_ADDRESS "20:00:00:01:01:21"
#define TEST_ADDRESS2 "20:00:00:34:31:13"
#define TEST_ADDRESS3 "20:00:00:34:43:13"
#define INF 999999999

/* Storage limit of events */
static events_limit = INF;

static int event_count = 0;

/* Array of events */
static GArray *events;
static int events_size;

/* Callback */
data_dumped_update_callback update_callback = NULL;

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
void add_test_fields(struct event_t *e)
{

	/* For test only */
	e->has_device = 1;

	if(rand() % 3)
		strcpy(e->device_address, TEST_ADDRESS);
	else if(rand() % 2)
		strcpy(e->device_address, TEST_ADDRESS2);
	else
		strcpy(e->device_address, TEST_ADDRESS3);

	if(events_size % 2)
		e->direction = EVENT_INPUT;
	else
		e->direction = EVENT_OUTPUT;

	/* Add test connection seq */
	if(rand() %10 == 1){
		e->is_device_connection = TRUE;
	}
	else{
		e->is_device_connection = FALSE;
	}

}

void add_test_devices()
{
	struct device_t *d;

	d = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(d->address, TEST_ADDRESS);
	strcpy(d->name, "Test");
	d->is_active = TRUE;
	d->id_initial_event = events_size;
	d->id_least_event = -1;

	data_dumped_add_device(d);

	d = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(d->address, TEST_ADDRESS2);
	strcpy(d->name, "Test2");
	d->is_active = TRUE;
	d->id_initial_event = events_size;
	d->id_least_event = -1;

	data_dumped_add_device(d);

	d = (struct device_t *) malloc(sizeof(struct device_t));
	strcpy(d->address, TEST_ADDRESS3);
	strcpy(d->name, "Test3");
	d->is_active = TRUE;
	d->id_initial_event = events_size;
	d->id_least_event = -1;

	data_dumped_add_device(d);
}

void data_dumped_set_update_callback(data_dumped_update_callback callback)
{
	update_callback = callback;
}

void data_dumped_update()
{
	struct data_dumped_t *data_in, *data_out;
	/* Filter */
	data_in = g_new(struct data_dumped_t, 1);

	data_in->events = events;
	data_in->events_size = events_size;
	data_in->devices = connected_devices;
	data_out = filter(data_in);

	if(update_callback)
		update_callback(data_out->events, data_out->events_size,
						data_out->devices);
}

void data_dumped_add_device(struct device_t *device)
{
	g_hash_table_insert(connected_devices, device->address, device);

	filter_set_active_device(device, TRUE);
}

GHashTable *data_dumped_get_connected_devices()
{
	return connected_devices;
}

struct device_t *get_device(char *address)
{
	struct device_t *d;
	gboolean has_key;
	d = (struct device_t *) g_hash_table_lookup(connected_devices, address);

	return d;
}

void search_device_in_event(struct event_t *e)
{
	struct device_t *d;

	if(strcmp(e->device_address,""))
		e->has_device = TRUE;
	else
		e->has_device = FALSE;

	if(e->has_device){
		if(has_key(connected_devices, e->device_address) == FALSE){
			d = g_new(struct device_t, 1);
			strcpy(d->address, e->device_address);
			strcpy(d->name, e->device_address);
			data_dumped_add_device(d);
		}
	}

}

struct event_t *create_event_object(int data_length)
{
	struct event_t *e;

	e = (struct event_t *) malloc(sizeof(struct event_t));

	e->data = (char *) malloc(data_length * sizeof(char));
	e->attributes = g_hash_table_new_full(g_str_hash, g_str_equal,
					event_attributes_key_destroy,
					event_attributes_value_destroy);

	strcpy(e->device_address,"");
	e->has_device = FALSE;
	return e;
}

void data_dumped_add_event(struct event_t *e)
{
	e->seq_number = event_count++;

	if( events_size == events_limit){
		g_array_remove_index(events, events_size -1);
		g_array_prepend_val(events, e);
	}
	else{
		g_array_prepend_val(events, e);
		events_size++;
	}

	/*add_test_fields(e);*/
	search_device_in_event(e);

	data_dumped_update();
}

struct event_t *get_event(int p)
{
	return g_array_index(events, void *, p);
}

void data_dumped_init(data_dumped_update_callback callback, int ev_lim)
{
	struct device_t *d;
	events = g_array_new(FALSE, FALSE, sizeof(struct event_t *));
	connected_devices = g_hash_table_new_full(g_str_hash,g_str_equal,
						g_hash_table_key_destroy,
						g_hash_table_value_destroy);

	events_limit = ev_lim;
	update_callback = callback;
	filter_init();

	/* Devices for test */
	/* for tests only */
	srand(time(0));
}
