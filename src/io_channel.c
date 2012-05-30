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
#include "io_channel.h"

gboolean callback(GIOChannel *source,GIOCondition condition,gpointer data);
void print_bytes(gchar *buf,int sz);

void print_bytes(gchar *buf,int sz)
{
		int i;
		for(i=0;i<sz;i++){
			printf("%x ",buf[i]);
		}
		printf("\n");
}

gboolean callback(GIOChannel *source,GIOCondition condition,gpointer data)
{
	GIOChannel *channel;
	gchar buf[100];
	gsize count=100;
	gsize bytes_read;
	GError *error=NULL;

	channel = source;
	g_io_channel_read_chars(channel,buf,count,&bytes_read,&error);
	if(error){
		printf("A error happened\n");
		return FALSE;
	}
	else{
		printf("bytes read: %d\n", bytes_read);
		print_bytes(buf,bytes_read);
	}

	return TRUE;
}

void create_io_channel(int fd)
{
	GMainLoop *loop;
	GIOChannel *channel;

	channel = g_io_channel_unix_new(fd);
	loop = g_main_loop_new(NULL,FALSE);

	g_io_add_watch(channel,G_IO_IN | G_IO_HUP | G_IO_ERR,(GIOFunc)callback,loop);
	g_main_loop_run(loop);
	g_io_channel_shutdown(channel,TRUE,NULL);
}
