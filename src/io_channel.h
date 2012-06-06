#ifndef IO_CHANNEL_H
#define IO_CHANNEL_H
#include <stdint.h>
typedef void (*io_destroy_func) (void *user_data);

typedef void (*io_event_func) (int fd, uint32_t events, void *user_data);
typedef void (*io_timeout_func) (int id, void *user_data);
typedef void (*io_signal_func) (int signum, void *user_data);

int io_init(void);

int io_quit(GMainLoop * loop);

int io_add_channel(int fd,uint32_t events,io_event_func callback,
		void *user_data);

int io_remove_channel(int fd);

GMainLoop *io_watch_all_channels();
#endif
