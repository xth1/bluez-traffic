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
//mainloop
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <signal.h>
//bluetooth
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


#include "traffic_mainloop.h"
#include "traffic_packet.h"
#include "traffic_control.h"

#include <glib.h>

#define MAINLOOP_INTERVAL 100

static void signal_callback(int signum, void *user_data)
{
	
	mainloop_quit();
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

int main(){
	
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);
	
	mainloop_init();
	
	tracing();
	
	GMainLoop *loop;

    loop = g_main_loop_new ( NULL , FALSE );

    g_timeout_add (MAINLOOP_INTERVAL ,mainloop_run , loop); 
    g_main_loop_run (loop);
    g_main_loop_unref(loop);
	
	
	return 0;
	
}
