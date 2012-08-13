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

#include "packet.h"
#include "data_dumped.h"
#include "util.h"
#include "bt.h"

parser_hci_cmd_create_conn(struct event_t *e, const void *data)
{
	struct bt_hci_cmd_create_conn *p;
	p = (struct bt_hci_cmd_create_conn *) data;

	/* It's a device connection  */
	e->is_device_connection = TRUE;
	/* Address */
	ba2str(&p->bdaddr, e->device_address);
	e->has_device = TRUE;

	/* Atributtes */
	g_hash_table_insert(e->attributes, make_str("Packet type"),
		to_str(p->pkt_type));
	g_hash_table_insert(e->attributes, make_str("pscan_rep_mode"),
		to_str(p->pscan_rep_mode));
	g_hash_table_insert(e->attributes, make_str("pscan_mode"),
		to_str(p->pscan_mode));
	g_hash_table_insert(e->attributes, make_str("Clock offset"),
		to_str(p->clock_offset));
	g_hash_table_insert(e->attributes, make_str("role_switch"),
		to_str(p->role_switch));
}

parser_hci_cmd_accept_conn_request(struct event_t *e, const void *data)
{
	struct bt_hci_cmd_accept_conn_request *p;
	p = (struct bt_hci_cmd_accept_conn_request *) data;

	/* It's a device connection  */
	e->is_device_connection = TRUE;
	/* Address */
	ba2str(&p->bdaddr, e->device_address);
	e->has_device = TRUE;

	/* Atributtes */
	g_hash_table_insert(e->attributes, make_str("Role"),
		to_str(p->role));

}
void parser_command(struct event_t *e, const void *data, uint8_t opcode)
{
	switch(opcode){
		case BT_HCI_CMD_CREATE_CONN:
			parser_hci_cmd_create_conn(e, data);
			break;
		case BT_HCI_CMD_ACCEPT_CONN_REQUEST:
			parser_hci_cmd_accept_conn_request(e, data);
			break;
	}
}

void parser_hci_evt_inquiry_result(struct event_t *e, const void *data)
{
	struct bt_hci_evt_inquiry_result *p;
	p = (struct bt_hci_evt_inquiry_result *) data;

	/* Address */
	ba2str(&p->bdaddr, e->device_address);
	e->has_device = TRUE;

	/* Atributtes */
	g_hash_table_insert(e->attributes, make_str("Number of Response"),
		to_str(p->num_resp));
	g_hash_table_insert(e->attributes, make_str("pscan_rep_mode"),
		to_str(p->pscan_rep_mode));
	g_hash_table_insert(e->attributes, make_str("pscan_period_mode"),
		to_str(p->pscan_period_mode));
	g_hash_table_insert(e->attributes, make_str("pscan_mode"),
		to_str(p->pscan_mode));
	g_hash_table_insert(e->attributes, make_str("Clock offset"),
		to_str(p->clock_offset));

}

void parser_hci_evt_ext_inquiry_result(struct event_t *e, const void *data)
{
	struct bt_hci_evt_ext_inquiry_result *p;
	p = (struct bt_hci_evt_ext_inquiry_result *) data;

	/* Address */
	ba2str(&p->bdaddr, e->device_address);
	e->has_device = TRUE;

	/* Atributtes */
	g_hash_table_insert(e->attributes, make_str("Number of Response"),
		to_str(p->num_resp));
	g_hash_table_insert(e->attributes, make_str("pscan_rep_mode"),
		to_str(p->pscan_rep_mode));
	g_hash_table_insert(e->attributes, make_str("pscan_period_mode"),
		to_str(p->pscan_period_mode));
	g_hash_table_insert(e->attributes, make_str("Clock offset"),
		to_str(p->clock_offset));
	g_hash_table_insert(e->attributes, make_str("rssi"),
		to_str(p->rssi));
}


void parser_event(struct event_t *e, const void *data, uint8_t opcode)
{
	switch(opcode){
		case BT_HCI_EVT_INQUIRY_RESULT:
			parser_hci_evt_inquiry_result(e, data);
			break;
		case BT_HCI_EVT_EXT_INQUIRY_RESULT:
			parser_hci_evt_ext_inquiry_result(e, data);
			break;
	}
}
