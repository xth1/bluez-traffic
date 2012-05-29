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
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>
#include <glib.h>
/*mainloop*/
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include <signal.h>
/*bluetooth*/
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/mgmt.h>

#include "mainloop.h"

static int epoll_fd; 
static int epoll_terminate;
static gpointer loop_pointer;

static struct signal_data *signal_data;

int mainloop_init(){
	epoll_terminate=0;
	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	return 0;
}

void mainloop_quit(){
	epoll_terminate=1;
}

static void traffic_signal_callback(int fd, uint32_t events, void *user_data)
{
	struct signal_data *data = user_data;
	struct signalfd_siginfo si;
	ssize_t result;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_quit();
		return;
	}

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return;

	if (data->callback)
		data->callback(si.ssi_signo, data->user_data);
}

int mainloop_remove_fd(int fd)
{
	int err;

	if (fd < 0 )
		return -EINVAL;

	err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

	
	return err;
}

int mainloop_add_monitor(int fd, uint32_t events, mainloop_event_func callback,
				void *user_data, mainloop_destroy_func destroy){
					
	
	struct mainloop_data *data;
	struct epoll_event ev;
	int err;

	data = malloc(sizeof(*data));
	if (!data)
		return -ENOMEM;
		
	memset(data, 0, sizeof(*data));
	data->fd = fd;
	data->events = events;
	data->callback = callback;
	data->destroy = destroy;
	data->user_data = user_data;

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = data;

	err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->fd, &ev);
	if (err < 0) {
		printf("\t Error\n");
		free(data);
		return err;
	}

	return 0;
}

int mainloop_pre_run(){
	if (signal_data) {
		
		if (sigprocmask(SIG_BLOCK, &signal_data->mask, NULL) < 0)
			return 1;

		signal_data->fd = signalfd(-1, &signal_data->mask,
						SFD_NONBLOCK | SFD_CLOEXEC);
		if (signal_data->fd < 0)
			return 1;

		if (mainloop_add_monitor(signal_data->fd, EPOLLIN,
				traffic_signal_callback, signal_data, NULL) < 0) {
			close(signal_data->fd);
			return 1;
		}
	}
	return 0;
	
}

gboolean  mainloop_run(gpointer data){
	
		struct epoll_event events[MAX_EPOLL_EVENTS];
		int n, nfds;
		if(epoll_terminate){
			g_main_loop_quit( (GMainLoop*)data );
			return FALSE;
		}
		loop_pointer=data;
		
		

		 nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds < 0)
			return 0;
		
		
		for (n = 0; n < nfds; n++) {
			struct mainloop_data *data = events[n].data.ptr;

			data->callback(data->fd, events[n].events,
							data->user_data);
		}
		 
	return TRUE;
}

int mainloop_set_signal(sigset_t *mask, mainloop_signal_func callback,
				void *user_data, mainloop_destroy_func destroy)
{
	struct signal_data *data;

	if (!mask || !callback)
		return -EINVAL;

	data = malloc(sizeof(*data));
	if (!data)
		return -ENOMEM;

	memset(data, 0, sizeof(*data));
	data->callback = callback;
	data->destroy = destroy;
	data->user_data = user_data;

	data->fd = -1;
	memcpy(&data->mask, mask, sizeof(sigset_t));

	free(signal_data);
	signal_data = data;

	return 0;
}
