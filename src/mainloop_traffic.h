#ifndef MAINLOOP_TRAFFIC
#define MAINLOOP_TRAFFIC

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
//mainloop
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include <signal.h>
//bluetooth
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/mgmt.h>


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


//global variables ---------------------------
static int epoll_fd; 
static int epoll_terminate;


static void signal_callback(int signum, void *user_data)
{
	
	printf("Signal CallBack\n"); mainloop_traffic_quit();
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_traffic_quit();
		break;
	}
}
/*-------------------Loop---------------------*/

int mainloop_traffic_init(){
	epoll_terminate=0;
	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
}

void mainloop_traffic_quit(){
	epoll_terminate=1;
}

int mainloop_traffic_remove_fd(int fd)
{
	struct mainloop_data *data;
	int err;

	if (fd < 0 )
		return -EINVAL;

	err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

	
	return err;
}


int mainloop_traffic_add_monitor(int fd, uint32_t events, mainloop_event_func callback,
				void *user_data, mainloop_destroy_func destroy){
					
	
	struct mainloop_data *data;
	struct epoll_event ev;
	int err;

//	if (fd < 0 || fd > MAX_MAINLOOP_ENTRIES - 1 || !callback)
//		return -EINVAL;

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


static struct signal_data *signal_data;

int mainloop_traffic_run(){
	
	/*if (signal_data) {
		
		if (sigprocmask(SIG_BLOCK, &signal_data->mask, NULL) < 0)
			return 1;

		signal_data->fd = signalfd(-1, &signal_data->mask,
						SFD_NONBLOCK | SFD_CLOEXEC);
		if (signal_data->fd < 0)
			return 1;

		if (mainloop_traffic_add_monitor(signal_data->fd, EPOLLIN,
				signal_callback, signal_data, NULL) < 0) {
			close(signal_data->fd);
			return 1;
		}
	}*/
	//while (!epoll_terminate) {
		
		struct epoll_event events[MAX_EPOLL_EVENTS];
		int n, nfds;

		 nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds < 0)
			return 0;
		
		
		for (n = 0; n < nfds; n++) {
			struct mainloop_data *data = events[n].data.ptr;

			data->callback(data->fd, events[n].events,
							data->user_data);
		}
	//}
	return 0;
}

int mainloop_traffic_set_signal(sigset_t *mask, mainloop_signal_func callback,
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

#endif

