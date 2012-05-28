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

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/mgmt.h>

#include "traffic_packet.h"

static void print_channel_header(struct timeval *tv, uint16_t index,
							uint16_t channel)
{
//	if (filter_mask & PACKET_FILTER_SHOW_INDEX) {
		switch (channel) {
		case HCI_CHANNEL_CONTROL:
			printf("{hci%d} ", index);
			break;
		case HCI_CHANNEL_MONITOR:
			printf("[hci%d] ", index);
			break;
		}
//	}

	if (tv) {
		time_t t = tv->tv_sec;
		struct tm tm;

		localtime_r(&t, &tm);

	//	if (filter_mask & PACKET_FILTER_SHOW_DATE)
			printf("%04d-%02d-%02d ", tm.tm_year + 1900,
						tm.tm_mon + 1, tm.tm_mday);

	//	if (filter_mask & PACKET_FILTER_SHOW_TIME)
			printf("%02d:%02d:%02d.%06lu ", tm.tm_hour,
					tm.tm_min, tm.tm_sec, tv->tv_usec);
	}
}

static void print_header(struct timeval *tv, uint16_t index)
{
	print_channel_header(tv, index, HCI_CHANNEL_MONITOR);
}


void packet_hexdump(const unsigned char *buf, uint16_t len)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	uint16_t i;

	if (!len)
		return;

	for (i = 0; i < len; i++) {
		str[((i % 16) * 3) + 0] = hexdigits[buf[i] >> 4];
		str[((i % 16) * 3) + 1] = hexdigits[buf[i] & 0xf];
		str[((i % 16) * 3) + 2] = ' ';
		str[(i % 16) + 49] = isprint(buf[i]) ? buf[i] : '.';

		if ((i + 1) % 16 == 0) {
			str[47] = ' ';
			str[48] = ' ';
			str[65] = '\0';
			printf("%-12c%s\n", ' ', str);
			str[0] = ' ';
		}
	}

	if (i % 16 > 0) {
		uint16_t j;
		for (j = (i % 16); j < 16; j++) {
			str[(j * 3) + 0] = ' ';
			str[(j * 3) + 1] = ' ';
			str[(j * 3) + 2] = ' ';
			str[j + 49] = ' ';
		}
		str[47] = ' ';
		str[48] = ' ';
		str[65] = '\0';
		printf("%-12c%s\n", ' ', str);
	}
}

/*
 * Fields 
 *  @tv: time value
 *  @index: device (adapter) index
 *  @opcode: code used to classify packets at HCI_CHANNEL_MONITOR
 *  
 * 
 */

void monitor(struct timeval *tv, uint16_t index, uint16_t opcode,
					const void *data, uint16_t size){					
printf("OPCODE %d\n",opcode);
print_header(tv,index);
printf("\n");
packet_hexdump(data,size);

}
