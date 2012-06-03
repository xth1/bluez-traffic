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
	void *user_data;
} io_data;

gboolean channel_callback(GIOChannel *source,GIOCondition condition,
		gpointer data);

gboolean channel_callback(GIOChannel *source,GIOCondition events,
		gpointer user_data)
{

	io_data *data=(io_data*)user_data;

	if(events & G_IO_IN)
		printf("IN\n");
	if(events & G_IO_HUP)
		printf("HUP\n");
	if(events &  G_IO_ERR)
		printf("ERR\n");

	data->callback(data->fd,events,data->user_data);

	return TRUE;
}

int create_io_channel(int fd,uint32_t events,io_event_func callback,
		void *user_data)
{
	GMainLoop *loop;
	GIOChannel *channel;
	io_data *data;

	channel = g_io_channel_unix_new(fd);
	loop = g_main_loop_new(NULL,FALSE);

	if(events==0)
		events=G_IO_IN | G_IO_HUP | G_IO_ERR;

	data=malloc(sizeof(io_data));
	data->fd=fd;
	data->callback=callback;
	data->user_data=user_data;
	data->loop=loop;
	data->events=events;

	g_io_add_watch(channel,events,(GIOFunc)channel_callback,data);
	g_main_loop_run(loop);
	g_io_channel_shutdown(channel,TRUE,NULL);

	return 0;
}
