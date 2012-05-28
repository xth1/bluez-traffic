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

gboolean  mainloop_run(gpointer data);

#endif

