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
#include "draw.h"	

static guint monitor_watch;
static guint control_watch;


static void mgmt_index_added(uint16_t len, const void *buf,
							struct event_t *e)
{
	printf("@ Index Added\n");
	packet_hexdump(buf, len);

	sprintf(e->name, "@ Index Added");	
	strcpy(e->device_address,"");
}

static void mgmt_index_removed(uint16_t len, const void *buf,
							struct event_t *e)
{
	printf("@ Index Removed\n");

	packet_hexdump(buf, len);
	
	sprintf(e->name, "@ Index Added");	
	strcpy(e->device_address,"");
}

static void mgmt_controller_error(uint16_t len, const void *buf,	
								struct event_t *e)
{	
	const struct mgmt_ev_controller_error *ev = buf;

	if (len < sizeof(*ev)) {
		printf("* Malformed Controller Error control\n");
		
		sprintf(e->name, "* Malformed Controller Error control");	
		strcpy(e->device_address,"");
		return;
	}

	printf("@ Controller Error: 0x%2.2x\n", ev->error_code);
	
	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name, "@ Controller Error: 0x%2.2x\n", ev->error_code);	
	strcpy(e->device_address,"");
	
}

static const char *settings_str[] = {
	"powered", "connectable", "fast-connectable", "discoverable",
	"pairable", "link-security", "ssp", "br/edr", "hs", "le"
};

static void mgmt_new_settings(uint16_t len, const void *buf, struct event_t *e)
{
	uint32_t settings;
	unsigned int i;
	char buff[512];
	
	if (len < 4) {
		printf("* Malformed New Settings control\n");
		sprintf(e->name, "* Malformed New Settings control\n");	
		strcpy(e->device_address,"");
		return;
	}

	settings = bt_get_le32(buf);

	printf("@ New Settings: 0x%4.4x\n", settings);

	printf("%-12c", ' ');
	for (i = 0; i < NELEM(settings_str); i++) {
		if (settings & (1 << i)){
			printf("%s ", settings_str[i]);
			
			sprintf(buff,"setting %d",i);
			
			g_hash_table_insert(e->attributes, make_str(buff), 
								make_str(settings_str[i]));
			
		}
	}
	
	printf("\n");

	buf += 4;
	len -= 4;

	packet_hexdump(buf, len);
	
	sprintf(e->name, "@ New Settings: 0x%4.4x", settings);	
	strcpy(e->device_address,"");
}

static void mgmt_class_of_dev_changed(uint16_t len, const void *buf,
										struct event_t *e)
{
	const struct mgmt_ev_class_of_dev_changed *ev = buf;

	if (len < sizeof(*ev)) {
		printf("* Malformed Class of Device Changed control\n");
		return;
	}

	printf("@ Class of Device Changed: 0x%2.2x%2.2x%2.2x\n",
						ev->class_of_dev[2],
						ev->class_of_dev[1],
						ev->class_of_dev[0]);

	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ Class of Device Changed: 0x%2.2x%2.2x%2.2x\n",
						ev->class_of_dev[2],
						ev->class_of_dev[1],
						ev->class_of_dev[0]);
	strcpy(e->device_address,"");
}


static void mgmt_new_link_key(uint16_t len, const void *buf,
								struct event_t *e)
{
	const struct mgmt_ev_new_link_key *ev = buf;
	char str[18];

	if (len < sizeof(*ev)) {
		printf("* Malformed New Link Key control\n");
		return;
	}

	ba2str(&ev->key.addr.bdaddr, str);

	printf("@ New Link Key: %s (%d)\n", str, ev->key.addr.type);

	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ New Link Key: %s (%d)\n", str, ev->key.addr.type);
	strcpy(e->device_address, str);
}


static void mgmt_local_name_changed(uint16_t len, const void *buf,
									struct event_t *e)
{
	const struct mgmt_ev_local_name_changed *ev = buf;

	if (len < sizeof(*ev)) {
		printf("* Malformed Local Name Changed control\n");
		return;
	}

	printf("@ Local Name Changed: %s (%s)\n", ev->name, ev->short_name);

	buf += sizeof(*ev);
	len -= sizeof(*ev);
	
	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ Local Name Changed: %s (%s)\n", 
					ev->name, ev->short_name);
	strcpy(e->device_address,"");
}

static void mgmt_new_long_term_key(uint16_t len, const void *buf,
									struct event_t *e)
{
	const struct mgmt_ev_new_long_term_key *ev = buf;
	char str[18];

	if (len < sizeof(*ev)) {
		printf("* Malformed New Long Term Key control\n");
		return;
	}

	ba2str(&ev->key.addr.bdaddr, str);

	printf("@ New Long Term Key: %s (%d)\n", str, ev->key.addr.type);
	
	
	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ New Long Term Key: %s (%d)\n", 
				str, ev->key.addr.type);
	strcpy(e->device_address, str);
}

static void mgmt_device_connected(uint16_t len, const void *buf,
									struct event_t *e)
{
	const struct mgmt_ev_device_connected *ev = buf;
	uint32_t flags;
	char str[18];

	if (len < sizeof(*ev)) {
		printf("* Malformed Device Connected control\n");
		return;
	}

	flags = btohs(ev->flags);
	ba2str(&ev->addr.bdaddr, str);

	printf("@ Device Connected: %s (%d) flags 0x%4.4x\n",
						str, ev->addr.type, flags);

	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ Device Connected: %s (%d) flags 0x%4.4x\n",
						str, ev->addr.type, flags);
	strcpy(e->device_address, str);
}

static void mgmt_device_disconnected(uint16_t len, const void *buf,
									struct event_t *e)
{
	const struct mgmt_ev_device_disconnected *ev = buf;
	char str[18];

	if (len < sizeof(*ev)) {
		printf("* Malformed Device Disconnected control\n");
		return;
	}

	ba2str(&ev->addr.bdaddr, str);

	printf("@ Device Disconnected: %s (%d)\n", str, ev->addr.type);

	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ Device Disconnected: %s (%d)\n", str, ev->addr.type);
	strcpy(e->device_address, str);
}

static void mgmt_connect_failed(uint16_t len, const void *buf,
								struct event_t *e)
{
	const struct mgmt_ev_connect_failed *ev = buf;
	char str[18];

	if (len < sizeof(*ev)) {
		printf("* Malformed Connect Failed control\n");
		return;
	}

	ba2str(&ev->addr.bdaddr, str);

	printf("@ Connect Failed: %s (%d) status 0x%2.2x\n",
					str, ev->addr.type, ev->status);

	buf += sizeof(*ev);
	len -= sizeof(*ev);

	packet_hexdump(buf, len);
	
	sprintf(e->name,"@ Connect Failed: %s (%d) status 0x%2.2x\n",
					str, ev->addr.type, ev->status);
	strcpy(e->device_address, str);
}

void control_message(uint16_t opcode, const void *data, uint16_t size,
					struct event_t *e)
{
	switch (opcode) {
	case MGMT_EV_INDEX_ADDED:
		mgmt_index_added(size, data, e);
		break;
	case MGMT_EV_INDEX_REMOVED:
		mgmt_index_removed(size, data, e);
		break;
	case MGMT_EV_CONTROLLER_ERROR:
		mgmt_controller_error(size, data, e);
		break;
	case MGMT_EV_NEW_SETTINGS:
		mgmt_new_settings(size, data, e);
		break;
	case MGMT_EV_CLASS_OF_DEV_CHANGED:
		mgmt_class_of_dev_changed(size,data, e);
		break;
	case MGMT_EV_LOCAL_NAME_CHANGED:
		mgmt_local_name_changed(size, data, e);
		break;
	case MGMT_EV_NEW_LINK_KEY:
		mgmt_new_link_key(size, data, e);
		break;
	case MGMT_EV_NEW_LONG_TERM_KEY:
		mgmt_new_long_term_key(size, data, e);
		break;
	case MGMT_EV_DEVICE_CONNECTED:
		mgmt_device_connected(size, data, e);
		break;
	case MGMT_EV_DEVICE_DISCONNECTED:
		mgmt_device_disconnected(size, data, e);
		break;
/*	case MGMT_EV_CONNECT_FAILED:
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
		sprintf(e->name,"* Unknown control (code %d len %d)\n", opcode, size);
		strcpy(e->device_address,"");
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
