/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "io_channel.h"

typedef struct  {
	int fd;
	uint32_t events;
	io_event_func callback;
	io_destroy_func destroy;
	GMainLoop *loop;
	GSource *source;
	void *user_data;
} io_data;

gboolean io_channel_callback(gpointer user_data);	
			
static GHashTable* io_watching;
static GMainContext * io_context;

gboolean io_channel_callback(gpointer user_data)
{
	
	printf("callback\n");

	io_data *data=(io_data*)user_data;

	data->callback(data->fd,data->events,data->user_data);

	return TRUE;
}
/*
int io_watch_channel(GMainLoop *loop,io_data *data)
{
	
}
*/
GMainLoop *io_watch_all_channels()
{

	io_data *data;
	GHashTableIter iter;
	gpointer key, value;
	GMainLoop *loop;
	
	

	g_hash_table_iter_init (&iter, io_watching);
	
	io_context=g_main_context_new();
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		data=(io_data *)value;
		g_source_attach(data->source,io_context);
	}
	
	loop=g_main_loop_new(io_context,FALSE);
	
	printf("FIM\n");
	//g_io_channel_shutdown(channel,TRUE,NULL);

	return loop;
	
}

int io_init(void)
{
	io_watching = g_hash_table_new(g_int_hash, g_int_equal);
	return 0;
}

int io_add_channel(int fd,uint32_t events,io_event_func callback,
		void *user_data)
{
	
	int *key=malloc(sizeof(int));
	io_data *data=user_data;
	GIOChannel *channel;
	
	*key=fd;
	
	channel=g_io_channel_unix_new(data->fd);
	
	data=malloc(sizeof(io_data));
	data->fd=fd;
	data->callback=callback;
	data->user_data=user_data;
	/*data->loop=loop;*/
	data->events=events;
	data->source=g_io_create_watch(channel,events);
	
	if(data->events==0)
			data->events=G_IO_IN | G_IO_HUP | G_IO_ERR;
	
	g_source_set_callback(data->source,io_channel_callback,data,NULL);
	
	g_hash_table_insert(io_watching,key,data);
	
	return 0;
}
