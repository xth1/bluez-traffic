#ifndef TRAFFIC_CONTROL_H
#define TRAFFIC_CONTROL_H


struct control_data {
	uint16_t channel;
	int fd;
};

int tracing(void);
#endif
