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

#ifndef TRAFFIC_PACKET_H 
#define TRAFFIC_PACKET_H

#define PACKET_FILTER_SHOW_INDEX	(1 << 0)
#define PACKET_FILTER_SHOW_DATE		(1 << 1)
#define PACKET_FILTER_SHOW_TIME		(1 << 2)
#define PACKET_FILTER_SHOW_ACL_DATA	(1 << 3)
#define PACKET_FILTER_SHOW_SCO_DATA	(1 << 4)


void packet_monitor(struct timeval *tv, uint16_t index, uint16_t opcode,
					const void *data, uint16_t size);
#endif
