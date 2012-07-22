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
#define EVENT_HEADER 1

#include <gtk/gtk.h>
#include <sys/time.h>
#include <glib.h>

#define ADDRESS_LENGTH 20
#define NAME_LENGTH 32
#define EVENT_NAME_LENGTH 128
#define EVENT_TYPE_LENGTH 32

#define DIR_RIGHT 1
#define DIR_LEFT -1

typedef void (*events_update_callback) 
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
	
	GHashTable *attributes;
	
	/* Device attibutes */
	int has_device;
	char device_address[ADDRESS_LENGTH];
	int direction;
	/*Device connection/disconnection happened in this event */
	gboolean device_connection;
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

void events_init(events_update_callback callback, int ev_lim);
void add_event(struct event_t *e);
