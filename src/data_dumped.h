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

#define DATA_DUMPED_HEADER 1

#include <gtk/gtk.h>
#include <sys/time.h>
#include <glib.h>

#define ADDRESS_LENGTH 20
#define NAME_LENGTH 32
#define EVENT_NAME_LENGTH 128
#define EVENT_TYPE_LENGTH 32

enum
{
        EVENT_INPUT,
        EVENT_OUTPUT,
        EVENT_CONNECTION
};

typedef void (*data_dumped_update_callback)
	(GArray *events, int size, GHashTable *devices);

struct event_t{
	int socket;
	struct timeval tv;
	int adapter_index;
	int type;
	char *data;
	char type_str[EVENT_TYPE_LENGTH];
	char name[EVENT_NAME_LENGTH];
	int seq_number;

	/* Sequence number of previus event */
	int previus_event;

	GHashTable *attributes;
	/* Device attibutes */
	int has_device;
	char device_address[ADDRESS_LENGTH];
	int comunication_type;
	/* Device connection/disconnection happened in this event */
	gboolean is_device_connection;
	/* Set by filter */
	gboolean is_active;
};

struct device_t{
	char address[ADDRESS_LENGTH];
	char name[NAME_LENGTH];
	int id_initial_event;
	int id_least_event;
	/* Set by filter */
	gboolean is_active;
};

struct data_dumped_t{
	GHashTable *devices;
	GArray *events;
	int events_size;
};
void data_dumped_set_update_callback(data_dumped_update_callback callback);
void data_dumped_init(data_dumped_update_callback callback, int ev_lim);
void data_dumped_add_event(struct event_t *e);
void data_dumped_events_update();
GHashTable *data_dumped_get_connected_devices();
