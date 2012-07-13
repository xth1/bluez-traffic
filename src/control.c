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
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include <sys/epoll.h>

#include <signal.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/mgmt.h>

#include "packet.h"
#include "control.h"

static guint monitor_watch;
static guint control_watch;


static void mgmt_index_added(uint16_t len, const void *buf,
							char *name_out, char *address_out)
{
	printf("@ Index Added\n");
	packet_hexdump(buf, len);

	sprintf(name_out, "@ Index Added");	
	strcpy(address_out,"");
}

static void mgmt_index_removed(uint16_t len, const void *buf,
							char *name_out, char *address_out)
{
	printf("@ Index Removed\n");

	packet_hexdump(buf, len);
	
	sprintf(name_out, "@ Index Added");	
	strcpy(address_out,"");
}


void control_message(uint16_t opcode, const void *data, uint16_t size,
					char *name_out, char *address_out)
{
	switch (opcode) {
	case MGMT_EV_INDEX_ADDED:
		mgmt_index_added(size, data, name_out, address_out);
		break;
	case MGMT_EV_INDEX_REMOVED:
		mgmt_index_removed(size, data, name_out, address_out);
		break;
/*	case MGMT_EV_CONTROLLER_ERROR:
		mgmt_controller_error(size, data);
		break;
	case MGMT_EV_NEW_SETTINGS:
		mgmt_new_settings(size, data);
		break;
	case MGMT_EV_CLASS_OF_DEV_CHANGED:
		mgmt_class_of_dev_changed(size, data);
		break;
	case MGMT_EV_LOCAL_NAME_CHANGED:
		mgmt_local_name_changed(size, data);
		break;
	case MGMT_EV_NEW_LINK_KEY:
		mgmt_new_link_key(size, data);
		break;
	case MGMT_EV_NEW_LONG_TERM_KEY:
		mgmt_new_long_term_key(size, data);
		break;
	case MGMT_EV_DEVICE_CONNECTED:
		mgmt_device_connected(size, data);
		break;
	case MGMT_EV_DEVICE_DISCONNECTED:
		mgmt_device_disconnected(size, data);
		break;
	case MGMT_EV_CONNECT_FAILED:
		mgmt_connect_failed(size, data);
		break;
	case MGMT_EV_PIN_CODE_REQUEST:
		mgmt_pin_code_request(size, data);
		break;
	case MGMT_EV_USER_CONFIRM_REQUEST:
		mgmt_user_confirm_request(size, data);
		break;
	case MGMT_EV_USER_PASSKEY_REQUEST:
		mgmt_user_passkey_request(size, data);
		break;
	case MGMT_EV_AUTH_FAILED:
		mgmt_auth_failed(size, data);
		break;
	case MGMT_EV_DEVICE_FOUND:
		mgmt_device_found(size, data);
		break;
	case MGMT_EV_DISCOVERING:
		mgmt_discovering(size, data);
		break;
	case MGMT_EV_DEVICE_BLOCKED:
		mgmt_device_blocked(size, data);
		break;
	case MGMT_EV_DEVICE_UNBLOCKED:
		mgmt_device_unblocked(size, data);
		break;
	case MGMT_EV_DEVICE_UNPAIRED:
		mgmt_device_unpaired(size, data);
		break;
	*/
	default:
		printf("* Unknown control (code %d len %d)\n", opcode, size);
		sprintf(name_out,"* Unknown control (code %d len %d)\n", opcode, size);
		packet_hexdump(data, size);
		break;
	}
	
}

static gboolean data_callback(GIOChannel *io, GIOCondition cond,
						gpointer user_data)
{
	unsigned char buf[HCI_MAX_FRAME_SIZE];
	unsigned char control[32];
	struct mgmt_hdr hdr;
	struct msghdr msg;
	struct iovec iov[2];
	struct control_data *data = user_data;
	if (cond & (G_IO_ERR | G_IO_HUP))
		/*run clean up*/
		return FALSE;

	iov[0].iov_base = &hdr;
	iov[0].iov_len = MGMT_HDR_SIZE;
	iov[1].iov_base = buf;
	iov[1].iov_len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);

	while (1) {
		struct cmsghdr *cmsg;
		struct timeval *tv = NULL;
		uint16_t opcode, index, pktlen;
		ssize_t len;

		len = recvmsg(data->fd, &msg, MSG_DONTWAIT);
		if (len < 0)
			break;

		if (len < MGMT_HDR_SIZE)
			break;

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
		case HCI_CHANNEL_CONTROL:
			packet_control(tv, index, opcode, buf, pktlen);
			break;

		case HCI_CHANNEL_MONITOR:
			packet_monitor(tv, index, opcode, buf, pktlen);
			break;

		}
	}
	return TRUE;
}


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

static unsigned int open_channel(uint16_t channel)
{

	struct control_data *data;
	GIOChannel *ch;

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

	ch = g_io_channel_unix_new(data->fd);

	return g_io_add_watch(ch, G_IO_IN | G_IO_HUP | G_IO_ERR,
							data_callback, data);
}

int control_tracing(void)
{
	monitor_watch = open_channel(HCI_CHANNEL_MONITOR);
	if (!monitor_watch)
		return -1;

	control_watch = open_channel(HCI_CHANNEL_CONTROL);
	
	if (!control_watch) {
		g_source_remove(monitor_watch);
		return -1;
	}

	return 0;
}
