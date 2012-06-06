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
	GIOChannel *channel;
	void *user_data;
} io_data;

/*FUNCTIONS HEADERS */
gboolean channel_callback(GIOChannel *source,GIOCondition condition,
		gpointer data);
void io_destroy_channel(GIOChannel *chanel);

void g_hash_table_key_destroy(gpointer data);
void g_hash_table_value_destroy(gpointer data);

/*STATIC VARIABLES */
static GHashTable* io_watching;

/*HASH TABLE FUNCTIONS */
void g_hash_table_key_destroy(gpointer data){
	free(data);
}

void g_hash_table_value_destroy(gpointer data){
	io_data *r_data=(io_data *)data;

	io_destroy_channel(r_data->channel);

	free(r_data->channel);
	free(r_data->user_data);
	free(r_data);
}


void io_destroy_channel(GIOChannel *channel)
{
	g_io_channel_shutdown(channel,TRUE,NULL);
}

int io_init(void)
{
	io_watching = g_hash_table_new_full(g_int_hash, g_int_equal,
	g_hash_table_key_destroy,g_hash_table_value_destroy);

	return 0;
}

int io_quit()
{
	io_data *data;
	GHashTableIter iter;
	gpointer key, value;

	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		data=(io_data *)value;
		io_destroy_channel(data->channel);

	}

	printf("quitting...\n");

	return 0;
}


gboolean channel_callback(GIOChannel *source,GIOCondition events,
		gpointer user_data)
{
	io_data *data;
	char *buff;

	data=(io_data*)user_data;

	if(events & (G_IO_ERR | G_IO_HUP)){
		buff=events&G_IO_ERR?"G_IO_ERR":"G_IO_HUP";
		printf("An error happened: IOChanel, %s\n",buff);

		return FALSE;
	}

	data->callback(data->fd,events,data->user_data);

	return TRUE;
}

void io_watch_all_channels()
{
	io_data *data;
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, io_watching);

	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		data=(io_data *)value;
		g_io_add_watch(data->channel,data->events,channel_callback,
		data);
	}

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
	data->events=events;
	data->channel=channel;

	data->events|=G_IO_IN | G_IO_HUP | G_IO_ERR;

	g_hash_table_insert(io_watching,key,data);

	return 0;
}


int io_remove_channel(int fd)
{
	gboolean response;
	response=g_hash_table_remove(io_watching,&fd);

	if(response==FALSE){
		printf("An error happened: removing a key %d from hash table\n",
		fd);
		return 1;
	}
	return 0;
}

