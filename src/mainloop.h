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

#ifndef MAINLOOP_TRAFFIC
#define MAINLOOP_TRAFFIC
#define MAX_EPOLL_EVENTS 10

typedef void (*mainloop_destroy_func) (void *user_data);

typedef void (*mainloop_event_func) (int fd, uint32_t events, void *user_data);
typedef void (*mainloop_timeout_func) (int id, void *user_data);
typedef void (*mainloop_signal_func) (int signum, void *user_data);


struct mainloop_data {
	int fd;
	uint32_t events;
	mainloop_event_func callback;
	mainloop_destroy_func destroy;
	void *user_data;
};

struct signal_data {
	int fd;
	sigset_t mask;
	mainloop_signal_func callback;
	mainloop_destroy_func destroy;
	void *user_data;
};

int mainloop_init();

void mainloop_quit();

int mainloop_add_monitor(int fd, uint32_t events, mainloop_event_func callback,
				void *user_data, mainloop_destroy_func destroy);
				
int mainloop_set_signal(sigset_t *mask, mainloop_signal_func callback,
				void *user_data, mainloop_destroy_func destroy);
				
int mainloop_pre_run();

int mainloop_remove_fd(int fd);

gboolean  mainloop_run(gpointer data);

#endif

