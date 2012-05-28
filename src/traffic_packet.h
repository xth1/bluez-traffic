

#ifndef TRAFFIC_PACKET_H 
#define TRAFFIC_PACKET_H

#define PACKET_FILTER_SHOW_INDEX	(1 << 0)
#define PACKET_FILTER_SHOW_DATE		(1 << 1)
#define PACKET_FILTER_SHOW_TIME		(1 << 2)
#define PACKET_FILTER_SHOW_ACL_DATA	(1 << 3)
#define PACKET_FILTER_SHOW_SCO_DATA	(1 << 4)


void monitor(struct timeval *tv, uint16_t index, uint16_t opcode,
					const void *data, uint16_t size);

#endif
