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
#include "UI.h"
#include "parser.h"

#define MONITOR_NEW_INDEX	0
#define MONITOR_DEL_INDEX	1
#define MONITOR_COMMAND_PKT	2
#define MONITOR_EVENT_PKT	3
#define MONITOR_ACL_TX_PKT	4
#define MONITOR_ACL_RX_PKT	5
#define MONITOR_SCO_TX_PKT	6
#define MONITOR_SCO_RX_PKT	7

#define HEXDUMP_LENGTH 4096

#define ADDRESS_LEN 21
static char address[ADDRESS_LEN];

struct monitor_new_index {
	uint8_t  type;
	uint8_t  bus;
	bdaddr_t bdaddr;
	char     name[8];
} __attribute__((packed));

#define MONITOR_NEW_INDEX_SIZE 16

#define MONITOR_DEL_INDEX_SIZE 0

#define MAX_INDEX 16

static struct monitor_new_index index_list[MAX_INDEX];

void packet_hexdump(const unsigned char *buf, uint16_t len);

void packet_hexdump(const unsigned char *buf, uint16_t len)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	char buff[68];
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

void packet_hexdump_to_string(const unsigned char *buf, uint16_t len,
								unsigned char *out,char *adr)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	char aux[80];
	uint16_t i,j;
	int has_address = 0;
	
	strcpy(out, "");

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
			sprintf(aux,"%c%s\n", ' ', str);
			strcat(out,aux);
			str[0] = ' ';
			/*take address*/
			if(!has_address){
				for(j = 0; j < 17;j++){
					adr[j] = str[17 - j];
				}
				adr[18]='\0';
				has_address = 1;
				
				printf("HEX Address %s %s\n",str, adr);
			}
			 
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
		sprintf(aux,"%c%s\n", ' ', str);
		strcat(out,aux);
	}
}

int packet_monitor(struct timeval *tv, uint16_t index, uint16_t opcode,
						const void *data, uint16_t size)
{
	const struct monitor_new_index *ni;
	char str[18];
	struct event_t *e;

	e = create_event_object(HEXDUMP_LENGTH);

	strcpy(e->name,"");
	strcpy(e->type_str,"");

	switch (opcode) {
	case MONITOR_NEW_INDEX:
		ni = data;

		if (index < MAX_INDEX)
			memcpy(&index_list[index], ni, MONITOR_NEW_INDEX_SIZE);
		ba2str(&ni->bdaddr, str);
		break;
	case MONITOR_COMMAND_PKT:
		packet_hci_command(e->name, e->type_str,tv, index, data, size);
		break;
	case MONITOR_EVENT_PKT:
		packet_hci_event(e, tv, index, data, size);
		break;
	}

	printf("[hci%d] op 0x%2.2x\n", index, opcode);

	packet_hexdump(data, size);

	if (!e)
		return -ENOMEM;

	/*generate event*/
	e->socket = HCI_CHANNEL_MONITOR;
	memcpy(&e->tv, tv, sizeof(*tv));
	e->adapter_index = index;

	packet_hexdump_to_string(data, size, e->data, address);
	e->type = opcode;
	data_dumped_add_event(e);

	return 0;
}

int packet_control(struct timeval *tv, uint16_t index, uint16_t opcode,
					const void *data, uint16_t size)
{
	struct event_t *e;

	e = create_event_object(HEXDUMP_LENGTH);
	if (!e)
		return -ENOMEM;

	/* Generate event */
	e->socket = HCI_CHANNEL_CONTROL;
	memcpy(&e->tv, tv, sizeof(*tv));
	e->adapter_index = index;

	packet_hexdump_to_string(data, size, e->data,address);
	
	e->type = opcode;

	control_message(opcode, data, size, e);
	
	data_dumped_add_event(e);

	return 0;
}

static const struct {
	uint16_t opcode;
	const char *str;
} opcode2str_table[] = {
	/* OGF 1 - Link Control */
	{ 0x0401, "Inquiry"				},
	{ 0x0402, "Inquiry Cancel"			},
	{ 0x0403, "Periodic Inquiry Mode"		},
	{ 0x0404, "Exit Periodic Inquiry Mode"		},
	{ 0x0405, "Create Connection"			},
	{ 0x0406, "Disconnect"				},
	{ 0x0407, "Add SCO Connection"			},
	{ 0x0408, "Create Connection Cancel"		},
	{ 0x0409, "Accept Connection Request"		},
	{ 0x040a, "Reject Connection Request"		},
	{ 0x040b, "Link Key Request Reply"		},
	{ 0x040c, "Link Key Request Negative Reply"	},
	{ 0x040d, "PIN Code Request Reply"		},
	{ 0x040e, "PIN Code Request Negative Reply"	},
	{ 0x040f, "Change Connection Packet Type"	},
	/* reserved command */
	{ 0x0411, "Authentication Requested"		},
	/* reserved command */
	{ 0x0413, "Set Connection Encryption"		},
	/* reserved command */
	{ 0x0415, "Change Connection Link Key"		},
	/* reserved command */
	{ 0x0417, "Master Link Key"			},
	/* reserved command */
	{ 0x0419, "Remote Name Request"			},
	{ 0x041a, "Remote Name Request Cancel"		},
	{ 0x041b, "Read Remote Supported Features"	},
	{ 0x041c, "Read Remote Extended Features"	},
	{ 0x041d, "Read Remote Version Information"	},
	/* reserved command */
	{ 0x041f, "Read Clock Offset"			},
	{ 0x0420, "Read LMP Handle"			},
	/* reserved commands */
	{ 0x0428, "Setup Synchronous Connection"	},
	{ 0x0429, "Accept Synchronous Connection"	},
	{ 0x042a, "Reject Synchronous Connection"	},
	{ 0x042b, "IO Capability Request Reply"		},
	{ 0x042c, "User Confirmation Request Reply"	},
	{ 0x042d, "User Confirmation Request Neg Reply"	},
	{ 0x042e, "User Passkey Request Reply"		},
	{ 0x042f, "User Passkey Request Negative Reply"	},
	{ 0x0430, "Remote OOB Data Request Reply"	},
	/* reserved commands */
	{ 0x0433, "Remote OOB Data Request Neg Reply"	},
	{ 0x0434, "IO Capability Request Negative Reply"},
	{ 0x0435, "Create Physical Link"		},
	{ 0x0436, "Accept Physical Link"		},
	{ 0x0437, "Disconnect Physical Link"		},
	{ 0x0438, "Create Logical Link"			},
	{ 0x0439, "Accept Logical Link"			},
	{ 0x043a, "Disconnect Logical Link"		},
	{ 0x043b, "Logical Link Cancel"			},
	{ 0x043c, "Flow Specifcation Modify"		},

	/* OGF 2 - Link Policy */
	{ 0x0801, "Holde Mode"				},
	/* reserved command */
	{ 0x0803, "Sniff Mode"				},
	{ 0x0804, "Exit Sniff Mode"			},
	{ 0x0805, "Park State"				},
	{ 0x0806, "Exit Park State"			},
	{ 0x0807, "QoS Setup"				},
	/* reserved command */
	{ 0x0809, "Role Discovery"			},
	/* reserved command */
	{ 0x080b, "Switch Role"				},
	{ 0x080c, "Read Link Policy Settings"		},
	{ 0x080d, "Write Link Policy Settings"		},
	{ 0x080e, "Read Default Link Policy Settings"	},
	{ 0x080f, "Write Default Link Policy Settings"	},
	{ 0x0810, "Flow Specification"			},
	{ 0x0811, "Sniff Subrating"			},

	/* OGF 3 - Host Control */
	{ 0x0c01, "Set Event Mask"			},
	/* reserved command */
	{ 0x0c03, "Reset"				},
	/* reserved command */
	{ 0x0c05, "Set Event Filter"			},
	/* reserved commands */
	{ 0x0c08, "Flush"				},
	{ 0x0c09, "Read PIN Type"			},
	{ 0x0c0a, "Write PIN Type"			},
	{ 0x0c0b, "Create New Unit Key"			},
	/* reserved command */
	{ 0x0c0d, "Read Stored Link Key"		},
	/* reserved commands */
	{ 0x0c11, "Write Stored Link Key"		},
	{ 0x0c12, "Delete Stored Link Key"		},
	{ 0x0c13, "Write Local Name"			},
	{ 0x0c14, "Read Local Name"			},
	{ 0x0c15, "Read Connection Accept Timeout"	},
	{ 0x0c16, "Write Connection Accept Timeout"	},
	{ 0x0c17, "Read Page Timeout"			},
	{ 0x0c18, "Write Page Timeout"			},
	{ 0x0c19, "Read Scan Enable"			},
	{ 0x0c1a, "Write Scan Enable"			},
	{ 0x0c1b, "Read Page Scan Activity"		},
	{ 0x0c1c, "Write Page Scan Activity"		},
	{ 0x0c1d, "Read Inquiry Scan Activity"		},
	{ 0x0c1e, "Write Inquiry Scan Activity"		},
	{ 0x0c1f, "Read Authentication Enable"		},
	{ 0x0c20, "Write Authentication Enable"		},
	{ 0x0c21, "Read Encryption Mode"		},
	{ 0x0c22, "Write Encryption Mode"		},
	{ 0x0c23, "Read Class of Device"		},
	{ 0x0c24, "Write Class of Device"		},
	{ 0x0c25, "Read Voice Setting"			},
	{ 0x0c26, "Write Voice Setting"			},
	{ 0x0c27, "Read Automatic Flush Timeout"	},
	{ 0x0c28, "Write Automatic Flush Timeout"	},
	{ 0x0c29, "Read Num Broadcast Retransmissions"	},
	{ 0x0c2a, "Write Num Broadcast Retransmissions"	},
	{ 0x0c2b, "Read Hold Mode Activity"		},
	{ 0x0c2c, "Write Hold Mode Activity"		},
	{ 0x0c2d, "Read Transmit Power Level"		},
	{ 0x0c2e, "Read Sync Flow Control Enable"	},
	{ 0x0c2f, "Write Sync Flow Control Enable"	},
	/* reserved command */
	{ 0x0c31, "Set Host Controller To Host Flow"	},
	/* reserved command */
	{ 0x0c33, "Host Buffer Size"			},
	/* reserved command */
	{ 0x0c35, "Host Number of Completed Packets"	},
	{ 0x0c36, "Read Link Supervision Timeout"	},
	{ 0x0c37, "Write Link Supervision Timeout"	},
	{ 0x0c38, "Read Number of Supported IAC"	},
	{ 0x0c39, "Read Current IAC LAP"		},
	{ 0x0c3a, "Write Current IAC LAP"		},
	{ 0x0c3b, "Read Page Scan Period Mode"		},
	{ 0x0c3c, "Write Page Scan Period Mode"		},
	{ 0x0c3d, "Read Page Scan Mode"			},
	{ 0x0c3e, "Write Page Scan Mode"		},
	{ 0x0c3f, "Set AFH Host Channel Classification"	},
	/* reserved commands */
	{ 0x0c42, "Read Inquiry Scan Type"		},
	{ 0x0c43, "Write Inquiry Scan Type"		},
	{ 0x0c44, "Read Inquiry Mode"			},
	{ 0x0c45, "Write Inquiry Mode"			},
	{ 0x0c46, "Read Page Scan Type"			},
	{ 0x0c47, "Write Page Scan Type"		},
	{ 0x0c48, "Read AFH Channel Assessment Mode"	},
	{ 0x0c49, "Write AFH Channel Assessment Mode"	},
	/* reserved commands */
	{ 0x0c51, "Read Extended Inquiry Response"	},
	{ 0x0c52, "Write Extended Inquiry Response"	},
	{ 0x0c53, "Refresh Encryption Key"		},
	/* reserved command */
	{ 0x0c55, "Read Simple Pairing Mode"		},
	{ 0x0c56, "Write Simple Pairing Mode"		},
	{ 0x0c57, "Read Local OOB Data"			},
	{ 0x0c58, "Read Inquiry Response TX Power Level"},
	{ 0x0c59, "Write Inquiry Transmit Power Level"	},
	{ 0x0c5a, "Read Default Erroneous Reporting"	},
	{ 0x0c5b, "Write Default Erroneous Reporting"	},
	/* reserved commands */
	{ 0x0c5f, "Enhanced Flush"			},
	/* reserved command */
	{ 0x0c61, "Read Logical Link Accept Timeout"	},
	{ 0x0c62, "Write Logical Link Accept Timeout"	},
	{ 0x0c63, "Set Event Mask Page 2"		},
	{ 0x0c64, "Read Location Data"			},
	{ 0x0c65, "Write Location Data"			},
	{ 0x0c66, "Read Flow Control Mode"		},
	{ 0x0c67, "Write Flow Control Mode"		},
	{ 0x0c68, "Read Enhanced Transmit Power Level"	},
	{ 0x0c69, "Read Best Effort Flush Timeout"	},
	{ 0x0c6a, "Write Best Effort Flush Timeout"	},
	{ 0x0c6b, "Short Range Mode"			},
	{ 0x0c6c, "Read LE Host Supported"		},
	{ 0x0c6d, "Write LE Host Supported"		},

	/* OGF 4 - Information Parameter */
	{ 0x1001, "Read Local Version Information"	},
	{ 0x1002, "Read Local Supported Commands"	},
	{ 0x1003, "Read Local Supported Features"	},
	{ 0x1004, "Read Local Extended Features"	},
	{ 0x1005, "Read Buffer Size"			},
	/* reserved command */
	{ 0x1007, "Read Country Code"			},
	/* reserved command */
	{ 0x1009, "Read BD ADDR"			},
	{ 0x100a, "Read Data Block Size"		},

	/* OGF 5 - Status Parameter */
	{ 0x1401, "Read Failed Contact Counter"		},
	{ 0x1402, "Reset Failed Contact Counter"	},
	{ 0x1403, "Read Link Quality"			},
	/* reserved command */
	{ 0x1405, "Read RSSI"				},
	{ 0x1406, "Read AFH Channel Map"		},
	{ 0x1407, "Read Clock"				},
	{ 0x1408, "Read Encryption Key Size"		},
	{ 0x1409, "Read Local AMP Info"			},
	{ 0x140a, "Read Local AMP ASSOC"		},
	{ 0x140b, "Write Remote AMP ASSOC"		},

	/* OGF 8 - LE Control */
	{ 0x2001, "LE Set Event Mask"			},
	{ 0x2002, "LE Read Buffer Size"			},
	{ 0x2003, "LE Read Local Supported Features"	},
	/* reserved command */
	{ 0x2005, "LE Set Random Address"		},
	{ 0x2006, "LE Set Advertising Parameters"	},
	{ 0x2007, "LE Read Advertising Channel TX Power"},
	{ 0x2008, "LE Set Advertising Data"		},
	{ 0x2009, "LE Set Scan Response Data"		},
	{ 0x200a, "LE Set Advertise Enable"		},
	{ 0x200b, "LE Set Scan Parameters"		},
	{ 0x200c, "LE Set Scan Enable"			},
	{ 0x200d, "LE Create Connection"		},
	{ 0x200e, "LE Create Connection Cancel"		},
	{ 0x200f, "LE Read White List Size"		},
	{ 0x2010, "LE Clear White List"			},
	{ 0x2011, "LE Add Device To White List"		},
	{ 0x2012, "LE Remove Device From White List"	},
	{ 0x2013, "LE Connection Update"		},
	{ 0x2014, "LE Set Host Channel Classification"	},
	{ 0x2015, "LE Read Channel Map"			},
	{ 0x2016, "LE Read Remote Used Features"	},
	{ 0x2017, "LE Encrypt"				},
	{ 0x2018, "LE Rand"				},
	{ 0x2019, "LE Start Encryption"			},
	{ 0x201a, "LE Long Term Key Request Reply"	},
	{ 0x201b, "LE Long Term Key Request Neg Reply"	},
	{ 0x201c, "LE Read Supported States"		},
	{ 0x201d, "LE Receiver Test"			},
	{ 0x201e, "LE Transmitter Test"			},
	{ 0x201f, "LE Test End"				},
	{ }
};

static const char *opcode2str(uint16_t opcode)
{
	int i;

	for (i = 0; opcode2str_table[i].str; i++) {
		if (opcode2str_table[i].opcode == opcode)
			return opcode2str_table[i].str;
	}

	return "Unknown";
}

void packet_hci_command(char *name_out,char *type_out,
			struct timeval *tv, uint16_t index,
			const void *data, uint16_t size)
{
	const hci_command_hdr *hdr = data;
	uint16_t opcode = btohs(hdr->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);

	/*btsnoop_write(tv, index, 0x02, data, size);*/

	if (size < HCI_COMMAND_HDR_SIZE){
		printf("* Malformed HCI Command packet\n");
		return;
	}
	sprintf(type_out,"< HCI Command");
	sprintf(name_out,"%s (0x%2.2x) plen %d\n\n",
				opcode2str(opcode), ogf, ocf, hdr->plen);

	data += HCI_COMMAND_HDR_SIZE;
	size -= HCI_COMMAND_HDR_SIZE;
}

static const struct {
	uint8_t event;
	const char *str;
} event2str_table[] = {
	{ 0x01, "Inquiry Complete"			},
	{ 0x02, "Inquiry Result"			},
	{ 0x03, "Connect Complete"			},
	{ 0x04, "Connect Request"			},
	{ 0x05, "Disconn Complete"			},
	{ 0x06, "Auth Complete"				},
	{ 0x07, "Remote Name Req Complete"		},
	{ 0x08, "Encrypt Change"			},
	{ 0x09, "Change Connection Link Key Complete"	},
	{ 0x0a, "Master Link Key Complete"		},
	{ 0x0b, "Read Remote Supported Features"	},
	{ 0x0c, "Read Remote Version Complete"		},
	{ 0x0d, "QoS Setup Complete"			},
	{ 0x0e, "Command Complete"			},
	{ 0x0f, "Command Status"			},
	{ 0x10, "Hardware Error"			},
	{ 0x11, "Flush Occurred"			},
	{ 0x12, "Role Change"				},
	{ 0x13, "Number of Completed Packets"		},
	{ 0x14, "Mode Change"				},
	{ 0x15, "Return Link Keys"			},
	{ 0x16, "PIN Code Request"			},
	{ 0x17, "Link Key Request"			},
	{ 0x18, "Link Key Notification"			},
	{ 0x19, "Loopback Command"			},
	{ 0x1a, "Data Buffer Overflow"			},
	{ 0x1b, "Max Slots Change"			},
	{ 0x1c, "Read Clock Offset Complete"		},
	{ 0x1d, "Connection Packet Type Changed"	},
	{ 0x1e, "QoS Violation"				},
	{ 0x1f, "Page Scan Mode Change"			},
	{ 0x20, "Page Scan Repetition Mode Change"	},
	{ 0x21, "Flow Specification Complete"		},
	{ 0x22, "Inquiry Result with RSSI"		},
	{ 0x23, "Read Remote Extended Features"		},
	/* reserved events */
	{ 0x2c, "Synchronous Connect Complete"		},
	{ 0x2d, "Synchronous Connect Changed"		},
	{ 0x2e, "Sniff Subrate"				},
	{ 0x2f, "Extended Inquiry Result"		},
	{ 0x30, "Encryption Key Refresh Complete"	},
	{ 0x31, "IO Capability Request"			},
	{ 0x32, "IO Capability Response"		},
	{ 0x33, "User Confirmation Request"		},
	{ 0x34, "User Passkey Request"			},
	{ 0x35, "Remote OOB Data Request"		},
	{ 0x36, "Simple Pairing Complete"		},
	/* reserved event */
	{ 0x38, "Link Supervision Timeout Change"	},
	{ 0x39, "Enhanced Flush Complete"		},
	/* reserved event */
	{ 0x3b, "User Passkey Notification"		},
	{ 0x3c, "Keypress Notification"			},
	{ 0x3d, "Remote Host Supported Features"	},
	{ 0x3e, "LE Meta Event"				},
	/* reserved event */
	{ 0x40, "Physical Link Complete"		},
	{ 0x41, "Channel Selected"			},
	{ 0x42, "Disconn Physical Link Complete"	},
	{ 0x43, "Physical Link Loss Early Warning"	},
	{ 0x44, "Physical Link Recovery"		},
	{ 0x45, "Logical Link Complete"			},
	{ 0x46, "Disconn Logical Link Complete"		},
	{ 0x47, "Flow Spec Modify Complete"		},
	{ 0x48, "Number Of Completed Data Blocks"	},
	{ 0x49, "AMP Start Test"			},
	{ 0x4a, "AMP Test End"				},
	{ 0x4b, "AMP Receiver Report"			},
	{ 0x4c, "Short Range Mode Change Complete"	},
	{ 0x4d, "AMP Status Change"			},
	{ 0xfe, "Testing"				},
	{ 0xff, "Vendor"				},
	{ }
};

static const char *event2str(uint8_t event)
{
	int i;

	for (i = 0; event2str_table[i].str; i++) {
		if (event2str_table[i].event == event)
			return event2str_table[i].str;
	}

	return "Unknown";
}


void packet_hci_event(struct event_t *e,
			struct timeval *tv, uint16_t index,
			const void *data, uint16_t size)
{
	e->comunication_type = EVENT_OUTPUT;
	
	const hci_event_hdr *hdr = data;

	if (size < HCI_EVENT_HDR_SIZE) {
		printf("* Malformed HCI Event packet\n");
		return;
	}
	sprintf(e->type_str,"> HCI Event");

	sprintf(e->name,"%s (0x%2.2x) plen %d\n",
				event2str(hdr->evt), hdr->evt, hdr->plen);

	data += HCI_EVENT_HDR_SIZE;
	size -= HCI_EVENT_HDR_SIZE;
	
	parser_event(e, data, hdr->evt);

	packet_hexdump(data, size);
}
