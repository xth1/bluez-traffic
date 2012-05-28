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

/* Auxiliar functions */

static void free_data(void *user_data)
{
	struct control_data *data = user_data;

	close(data->fd);

	free(data);
}



/*------------------CallBack-----------------------------*/



static void data_callback(int fd, uint32_t events, void *user_data)
{
	struct control_data *data = user_data;
	unsigned char buf[HCI_MAX_FRAME_SIZE];
	unsigned char control[32];
	struct mgmt_hdr hdr;
	struct msghdr msg;
	struct iovec iov[2];

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(fd);
		return;
	}

	iov[0].iov_base = &hdr; /* Starting address */
	iov[0].iov_len = MGMT_HDR_SIZE; ; /* Size */
	iov[1].iov_base = buf; /* Starting address */
	iov[1].iov_len = sizeof(buf); /* Size */

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);
	
	
	//catch all messages from bluetooth socket
	while (1) {
		struct cmsghdr *cmsg;
		struct timeval *tv = NULL;
		uint16_t opcode, index, pktlen;
		ssize_t len;
		
		/* docs in http://linux.die.net/man/2/recvmsg*/
		len = recvmsg(fd, &msg, MSG_DONTWAIT);
		if (len < 0)
			break;

		if (len < MGMT_HDR_SIZE)
			break;

		
		/*search field with type SCM_TIMESTAMP at msg_control from msg*/
		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_SOCKET)
				continue;

			if (cmsg->cmsg_type == SCM_TIMESTAMP)
				tv = (struct timeval *) CMSG_DATA(cmsg);
		}
		opcode = btohs(hdr.opcode);
		index  = btohs(hdr.index);
		pktlen = btohs(hdr.len);
		switch (data->channel) {
		/*this feature is not supported yet */ 
		/*case HCI_CHANNEL_CONTROL:
			packet_control(tv, index, opcode, buf, pktlen);
			break;
			*/ 
		case HCI_CHANNEL_MONITOR:
			monitor(tv, index, opcode, buf, pktlen);
			break;
		}

	}
}

/*------------------Socket--------------------------------*/

static int open_socket(uint16_t channel)
{
	struct sockaddr_hci addr;
	int fd, opt = 1;

	fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open channel");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = channel;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		if (errno == EINVAL) {
			/* Fallback to hcidump support */
			close(fd);
			return -1;
		}
		perror("Failed to bind channel");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &opt, sizeof(opt)) < 0) {
		perror("Failed to enable timestamps");
		close(fd);
		return -1;
	}

	return fd;
}


static int open_channel(uint16_t channel)
{
	struct control_data *data;

	data = malloc(sizeof(*data));
	if (!data)
		return -1;

	memset(data, 0, sizeof(*data));
	data->channel = channel;

	data->fd = open_socket(channel);
	if (data->fd < 0) {
		free(data);
		return -1;
	}
	
	mainloop_add_monitor(data->fd, EPOLLIN, data_callback, data, free_data);

	return 0;
}

int tracing(void)
{
	if (open_channel(HCI_CHANNEL_MONITOR) < 0){
		return -1;
	}
	open_channel(HCI_CHANNEL_CONTROL);
	mainloop_pre_run();
	return 0;
}
