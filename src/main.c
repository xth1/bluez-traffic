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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/epoll.h>

#include <glib.h>

#include "control.h"
#include "diagram.h"

#ifndef EVENT_HEADER
#include "event.h"
#endif

#define MAX_EVENTS_STORAGE 200

int main(int argc, char **argv)
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);
	if(!loop) {
		fprintf(stderr, "Failed to create mainloop\n");
		return -1;
	}

	if (control_tracing() < 0)
		return -1;

	events_init(NULL, MAX_EVENTS_STORAGE);

	UI_init(argc, argv, loop);

	printf("bluez-traffic %s\n", VERSION);

	g_main_loop_run(loop);
	
	return 0;
}
