/*
 * apollo_net.c - simple APOLLO netserver emulation (netman, ARP, IP echo, ...)
 *
 *  Created on: May 12, 2010
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/AEGIS_Internals_and_Data_Structures_Jan86.pdf
 */

#define VERBOSE 0

// FIXME: these three should not be hardcoded
#define NETSERVER_NODE_ID     0x012340
#define NETSERVER_IP_ADDRESS  {192,168,2,3}    /* 192.168.2.3 */
#define NETSERVER_ROOT_PATH   "apollo"

#include "includes/apollo.h"
#include "machine/3c505.h"

typedef struct {
	UINT8 dest[6];
	UINT8 source[6];
	UINT16 proto;
} ethernet_header;

typedef struct
{
	UINT8 version;
	UINT8 tos;
	UINT16 tot_len;
	UINT16 id;
	UINT16 frag_off;
	UINT8 ttl;
	UINT8 protocol;
	UINT16 check;
	UINT32 saddr;
	UINT32 daddr;
} ip_header;

#define ETHERNET_HEADER_SIZE sizeof(ethernet_header)

/* Ethernet protocol ID's */
#define ETHERNET_PACKET_TYPE_IP 0x0800   /* IP */
#define ETHERNET_PACKET_TYPE_ARP 0x0806  /* Address resolution */
#define ETHERNET_PACKET_TYPE_APOLLO 0x8019

#define ETHERNET_APOLLO_PREFIX 0x08001e
#define ETHERNET_APOLLO_DOMAIN_PREFIX 0x09001e

#define ICMP_ECHOREPLY		0	/* Echo Reply           */
#define ICMP_ECHO		    8	/* Echo Request         */

#define PACKET_HEADER_SIZE 64

// #define ETH_BUFFER_SIZE 2000

// Note: these three pathnames are hardcoded in Apollo DomainOS; don't change them
#define APOLLO_NETBOOT_PATH "/sys/net/netboot"
#define APOLLO_SAU7_PATH "/sau7"
#define APOLLO_SAU8_PATH "/sau8"
#define APOLLO_SAU14_PATH "/sau14"

static emu_file *current_file = NULL;

static UINT16 current_sector = 0;
static UINT16 current_last_sector = 0;
static char current_pathname[256];

static UINT8 current_rx_data_buffer[ETH_BUFFER_SIZE];
static int current_rx_data_length;

static apollo_netserver_transmit tx_callback = NULL;

static UINT32 my_node_id;
static UINT8 my_ip_address[4] = NETSERVER_IP_ADDRESS;  /* 192.168.2.3 */
static UINT8 my_mac_address[6];

static astring root_path;

// boot_$volun (find partner node)
static UINT8 response_0006[] = {
	0x00,0x01,                      // 0x40:
	0x00,0x01,
	0x00,0x07,                      // 0x44: packet type (07 = 06+1)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00
};

static UINT8 list_directory_header[] = {
	0x00,0x01,                      // 0x40:
	0x00,0x01,
	0x00,0x0d,                      // 0x44: packet type (0d = 0c+1)
	0x00,0x00,0x00,0x00,            // 0x46:
	0x00,0x01,0x00,0x01,
	0x73,0x61,0x6d,0x62,0x61,0x2f,0x73,0x61,0x75,0x37, // "samba/sau7"
	0x00,0x00,0x28,0xd8,0x00,0x00,0x00,0x00,
	0x75,0x37,0xd8,0xa0,0x76,0x08,0xb5,0x52,
	0x75,0x37,0xe6,0xc6,0x75,0x37,0xd8,0xac
};

static UINT8 receive_data_header[] = {
    0x00,0x01,                      // 0x40:
    0x00,0x01,                      // 0x42:
    0x00,0x09,                      // 0x44: packet type (09 = 08+1)
    0x00,0x00,0x00,0x00,            // 0x46:
    0x00,0x13,0xfc,0xaf,            // 0x4A: high
    0x00,0x13,0xd8,0x00,            // 0x4E: low
    0x00,0x13,0xd8,0x2a,            // 0x52: start
    0x00,0x13,0xd8,0x00,            // 0x56: current load address
    0x00,0x00                       // 0x5a:
};

static UINT8 not_found_data_header[] = {
	0x00,0x01,                      // 0x40:
	0x00,0x01,                      // 0x42: ?
	0x00,0x13,                      // 0x44: packet type (13 = 12+1)
	0x00,0x0e,0x00,0x07,
	0x00,0x12,                      // 0x4a = string length
	0x2f,0x2f,0x73,0x61,0x6d,0x62,0x61,0x2f, 0x73,0x61,0x75,0x37,0x2f,0x78,0x78,0x78, /*//samba/sau7/xxxxx */
	0x78,0x78,

	0x07,0x48,0x00,0x08,0x07,0x6a,0x00,0x08, 0x07,0x6c,0x00,0x04,0x0c,0x94
};

// boot_$get_uids
static UINT8 response_070a[] = {
	0x00,0x01,                               // 0x40:
	0x00,0x01,
	0x00,0x0b,                               // 0x44: packet type (0b = 0a+1)
	0x00,0x00,0x00,0x00,
	// d993195c (2010-04-28 07:27:15)
	0xd9,0x93,0x19,0x5c,0x60,0x04,0x20,0xe8, // FIXME: UID of OS paging file (?)
	// 7742b564 (1996-08-14 19:12:59)
	0x77,0x42,0xb5,0x64,0xf0,0x04,0x20,0xe8, // 7742B564.F00420E8 is UID of //
	0x77,0x42,0xb5,0x64,0xe0,0x04,0x20,0xe8  // 7742B564.E00420E8 is UID of / (resp. //samba)
};

// get (dex) file parameters
static UINT8 response_0718[] = {
	0x00,0x01,                      // 0x40:
	0x00,0x01,                      // 0x42:
	0x00,0x19,                      // 0x44: packet type (19 = 18+1)
	0x00,0x00,0x00,0x00,            // 0x46:
	0x01,
	0x01,                           // 0x4b: systyp (1=dir, 0=file) i.e. is directory
	0x90,0x00,
	0x77,0x43,0x02,0x58,0xe0,0x04,0x20,0xe8, // 0x4e: source UID
	0x00,0x00,                      // 0x56
	0x00,0x00,                      // 0x58: file type (0x0321 = UASC file)
	0x00,0x00,0x00,0x00,            // 0x5A:
	0x77,0x43,0x02,0x58,0xe8,0x04,0x20,0xe8, // 0x5E: source UID
	0x00,0x00,0x10,0x00,            // 0x66: file size
	0x00,0x00,0x00,0x04,            // 0x6A: n sectors (?)
	0xd9,0x8f,0xb5,0xef,            // 0x6E:
	0x80,0x27,0x94,0x1e,            // 0x72:
	0x77,0x42,0xb5,0x64,0xe0,0x04,0x20,0xe8, // source UID
	0x8d,0x15,0x00,0x02,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

static UINT8 get_byte(const UINT8 *data) {
	return data[0];
}

static void set_byte(UINT8 *data, const UINT8 value) {
	data[0] = value;
}

static UINT16 get_word(const UINT8 *data) {
	return (data[0] << 8) | data[1];
}

static void set_word(UINT8 *data, const UINT16 value) {
	data[0] = value >> 8;
	data[1] = value & 0xff;
}

static UINT32 get_3byte(const UINT8 *data) {
	return (((data[0] << 8) | data[1]) << 8) | data[2];
}

static void set_3byte(UINT8 *data, const UINT32 value) {
	data[0] = (value >> 16) & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[2] = value & 0xff;
}

static UINT32 get_long(const UINT8 *data) {
	return (((((data[0] << 8) | data[1]) << 8) | data[2]) << 8) | data[3];
}

static void set_long(UINT8 *data, const UINT32 value) {
	data[0] = (value >> 24) & 0xff;
	data[1] = (value >> 16) & 0xff;
	data[2] = (value >> 8) & 0xff;
	data[3] = value & 0xff;
}

static char *get_string(const UINT8 *data) {
	static char sb[256];
	int i;
	int stringLength = get_word(data);
	for (i = 0; i < stringLength && i < sizeof(sb) - 1; i++) {
		sb[i] = data[i + 2];
	}
	sb[stringLength] = 0;
	return sb;
}

static char *get_file_name(const UINT8 *data) {
	char * s = get_string(data);
	int i = strlen(s);
	while (--i >= 0) {
		s[i] = tolower(s[i]);
	}
	return s;
}

static int get_file_size(const char *path) {
	int file_size = 0;
	emu_file file(OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD);
	file_error filerr = file.open(path);
	if (filerr == FILERR_NONE)
	{
		file_size = file.size();
		file.close();
	}
	LOG1(("##### %s - file_size=%d",path, file_size));
	return file_size;
}

static int is_directory(const char *path) {
	int is_directory = 0;
	osd_directory *curdir;
	if ((curdir = osd_opendir(path)) != NULL)
	{
		osd_closedir(curdir);
		is_directory = 1;
	}
	LOG1(("##### %s - is_directory=%d",path, is_directory));
	return is_directory;
}

static int get_file_type(const char *path) {
	if (!is_directory(path)) {
		const char *suffix = path + strlen(path);
		while (suffix != path && *--suffix != '.') {
		}

		if (strcmp(suffix, ".cmd") == 0 || strcmp(suffix, ".hlp") == 0) {
			return 0x0321; // UASC file
		}
	}
	return 0;
}

static int set_ethernet_header(UINT8 *data, UINT8 client_ethernet_address[6]) {
	memcpy(data + 0, client_ethernet_address, 6);
	set_3byte(data + 6, ETHERNET_APOLLO_PREFIX);
	set_3byte(data + 9, my_node_id);
	set_word(data + 12, ETHERNET_PACKET_TYPE_APOLLO);
	return ETHERNET_HEADER_SIZE;
}

static int set_packet_header(UINT8 *packet_buffer, UINT32 dest_node_id,
		UINT32 source_node_id) {
	// --------- apollo header --------------
	set_word(packet_buffer, 0xffff);
	//  set_word(packet_buffer + 2, data_size + PACKET_HEADER_SIZE);
	set_word(packet_buffer + 4, 0x00be);
	set_word(packet_buffer + 6, 0x0000);
	set_word(packet_buffer + 8, 0x0000); // Apollo network number
	set_3byte(packet_buffer + 10, ETHERNET_APOLLO_PREFIX);
	set_3byte(packet_buffer + 13, dest_node_id);
	set_word(packet_buffer + 16, 0x0000);
	set_word(packet_buffer + 18, 0x0000);
	set_word(packet_buffer + 20, 0x0000);
	set_3byte(packet_buffer + 22, ETHERNET_APOLLO_PREFIX);
	set_3byte(packet_buffer + 25, source_node_id);
	set_word(packet_buffer + 28, 0x0003);
	// --------- data header --------------
	set_long(packet_buffer + 30, dest_node_id);
	set_long(packet_buffer + 34, 0x24000000L);
	set_long(packet_buffer + 38, source_node_id);
	set_long(packet_buffer + 42, 0x01002400L);
	//  set_word(packet_buffer + 46, data_size + 0x22);
	//  set_word(packet_buffer + 48, data_size);
	set_word(packet_buffer + 50, 0x0000);
	set_word(packet_buffer + 52, 0x0000);
	// --------- data --------------
	set_word(packet_buffer + 54, 0x0401);
	set_word(packet_buffer + 56, 0x0003);
	set_word(packet_buffer + 58, 0x20e8);
	set_word(packet_buffer + 60, 0x616d);
	set_word(packet_buffer + 62, 0x0000);
	return PACKET_HEADER_SIZE;
}

static int set_packet(UINT8 *packet_buffer, UINT8 *packet_data,
		int packet_data_size) {
	set_word(packet_buffer + 2, packet_data_size + PACKET_HEADER_SIZE);
	set_word(packet_buffer + 46, packet_data_size + 0x22);
	set_word(packet_buffer + 48, packet_data_size);

	memcpy(packet_buffer + PACKET_HEADER_SIZE, packet_data, packet_data_size);

	return packet_data_size;
}

static int get_packet_size(UINT8 *packet_buffer) {
	return get_word(packet_buffer + 2);
}

static int get_dir_data(const char *pathname, UINT8 *packet_buffer) {
	int nBytes, lineLength;
	char *data_buffer;
	osd_directory *curdir;
	const osd_directory_entry *dir;

	char dir_path[1024];
	strcpy(dir_path, root_path);
	strcat (dir_path, pathname);

	set_packet(packet_buffer, list_directory_header,
			sizeof(list_directory_header));
	data_buffer = (char *) (packet_buffer + get_packet_size(packet_buffer));

	strcpy(data_buffer, "\n\r");
	strcat(data_buffer, pathname);
	strcat(data_buffer, "\n\r");

	if ((curdir = osd_opendir(dir_path)) == NULL)
	{
		strcat(data_buffer, "Error: failed to open directory ");
		strcat(data_buffer, dir_path);
		strcat(data_buffer, "\r\n");
	}
	else
	{
		osd_closedir(curdir);

		// open a path to the netserver directory
		file_enumerator path(dir_path);

		lineLength = 99;
		while ((dir = path.next()) != NULL)
		{
			LOG1(("##### %d %s", (int)strlen(dir->name), dir->name));

			if (strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0)
			{
				lineLength += 3 + strlen(dir->name);
				if (lineLength > 72) {
					strcat(data_buffer, "\n\r");
					lineLength = 3 + strlen(dir->name);
				}
				strcat(data_buffer, "   ");
				strcat(data_buffer, dir->name);
			}
		}
		strcat(data_buffer, "\n\r");
	}

	nBytes = strlen(data_buffer);
	nBytes = (nBytes + 1) & 0xfffffffe;

	set_word(packet_buffer + 2, get_word(packet_buffer + 2) + nBytes);
	set_word(packet_buffer + 50, nBytes);

	return nBytes + sizeof(list_directory_header);
}

static int get_file_data(const char *pathname, UINT8 *packet_buffer,
		UINT16 first_sector, UINT16 last_sector) {
	UINT8 *packet_data = packet_buffer + PACKET_HEADER_SIZE;
	UINT8 *data_buffer;
	int nBytes;
	UINT32 fileSize = 0;
	file_error filerr;

	if(current_file == NULL)
	{
		current_file = global_alloc(emu_file(root_path, OPEN_FLAG_READ));
		// skip leading '/'
		filerr = current_file->open( pathname+1);

		if (filerr != FILERR_NONE)
		{
			global_free(current_file);
			current_file = NULL;
		}

		fileSize = current_file == NULL ? 0 : current_file->size();
		current_sector = 0;

		LOG1(("##### %s - %s filerr=%d fileSize=%d", root_path.cstr(), pathname, filerr, fileSize));
	} else {
		current_sector++;
	}
	current_last_sector = last_sector;

	if (current_file == NULL)
	{
		set_packet(packet_buffer, not_found_data_header,
				sizeof(not_found_data_header));

		set_word(packet_data + 0x0A, strlen(pathname));
		strncpy((char *) packet_data + 0x0C, pathname, 0x24);
		return sizeof(not_found_data_header);
	} else {
		set_packet(packet_buffer, receive_data_header,
				sizeof(receive_data_header));

		data_buffer = (packet_buffer + get_packet_size(packet_buffer));

		nBytes = current_file->read(data_buffer, 1024);

		if (nBytes < 0) {
			nBytes = 0;
		}

		// help in dex will fail, if size is odd (5.5.2010 - ost)
		nBytes = (nBytes + 1) & 0xfffffffe;

		if (nBytes < 1024)
		{
			current_file->close();
			global_free(current_file);
			current_file = NULL;
		}
		else if (current_sector == 0) {
			// set header addresses from first sector read
			UINT32 low = get_long(data_buffer + 0);
			UINT32 start = get_long(data_buffer + 4);
			UINT32 high = low + fileSize - 1;

			set_long(receive_data_header + 0x0A, high);
			set_long(receive_data_header + 0x0E, low);
			set_long(receive_data_header + 0x12, start);
			set_long(receive_data_header + 0x16, low);

			set_packet(packet_buffer, receive_data_header,
					sizeof(receive_data_header));
		}

		if (VERBOSE > 1) {
			LOG1(("##### %s: 3C505 current address = %0x first/last/current sector = %0x / %0x / %0x",
							pathname, get_long(receive_data_header + 0x16), first_sector, last_sector, current_sector));
		}

		set_long(receive_data_header + 0x16, get_long(receive_data_header + 0x16) + nBytes);
		set_word(packet_data + 0x1A, nBytes < 1024 ? 0xffff : current_sector == last_sector ? 0x00ff : 0);

		set_word(packet_buffer + 2, get_word(packet_buffer + 2) + nBytes);
		set_word(packet_buffer + 50, nBytes);

		return nBytes + sizeof(receive_data_header);
	}
}

static void log_data(const device_t *device, const char *type,
		const UINT8 *data, int data_length) {
	if (VERBOSE > 1) {
		int i;
		logerror("%s - %s: Net Server %s Data (length=%2x)", apollo_cpu_context(
				device->machine().firstcpu), device->tag(), type, data_length);
		for (i = 0; i < data_length; i++) {
			logerror(" %02x", data[i]);
		}
		logerror("\n");
	}
}

static UINT16 get_socket(const UINT8 rx_data_buffer[], int rx_data_length) {

	const UINT8 * packet_buffer = rx_data_buffer + ETHERNET_HEADER_SIZE;
	int packet_length = rx_data_length - ETHERNET_HEADER_SIZE;

	UINT16 socket = packet_length < 0x11 ? 0xffff : get_word(packet_buffer + 0x10); // 0x10: dest socket

	return socket;
}

/***************************************************************************
 netman server
 ***************************************************************************/

static int netman_server(const device_t *device,
		const UINT8 rx_data_buffer[], int rx_data_length, UINT8 tx_data_buffer[]) {
	UINT16 service;
	UINT16 node_type; // SAU7, SAU8 or SAU14
	char pathname[256];

	int tx_data_length;

	UINT8 *packet_buffer = tx_data_buffer + ETHERNET_HEADER_SIZE;

	UINT32 client_node_id;
	UINT8 client_ethernet_address[6];

	UINT16 last_sector;
	UINT16 first_sector;

	client_node_id = get_3byte(rx_data_buffer + 9);
	memcpy(client_ethernet_address, rx_data_buffer + 6, 6);

	log_data(device, "Rx", rx_data_buffer, rx_data_length);

	// strip off the ethernet header;
	rx_data_buffer += ETHERNET_HEADER_SIZE;
	rx_data_length -= ETHERNET_HEADER_SIZE;

	service = (rx_data_length > 0x45) ? get_word(rx_data_buffer + 0x44) : 0xffff;
	first_sector = (rx_data_length > 0x6b) ? get_word(rx_data_buffer + 0x6a) : 0;
	// FIXME: hack
//  last_sector = (rx_data_length > 0x6d) ? get_word(rx_data_buffer + 0x6c) : 0;
	last_sector = (rx_data_length > 0x6d) ? get_word(rx_data_buffer + 0x6c) : 0x0f;

	set_ethernet_header(tx_data_buffer, client_ethernet_address);
	set_packet_header(packet_buffer, client_node_id, my_node_id);

	node_type = service >> 8;

	switch (service & 0xff) {
	case 6: // boot_$volun (find partner node)
		DLOG1(("Net Server service=%04x (boot_$volun)", service));
		set_packet(packet_buffer, response_0006, sizeof(response_0006));
		break;

	case 8: // boot_$sysboot  (get netboot)
		get_file_data(APOLLO_NETBOOT_PATH, packet_buffer, first_sector, last_sector);
		DLOG1(("Net Server service=%04x (boot_$sysboot - sector %x of %x-%x)", service, current_sector, first_sector, last_sector));
		break;
	case 0x0a: // boot_$get_uids    (get UIDs for OS paging file, network root and the node entry directory)
		set_packet(packet_buffer, response_070a, sizeof(response_070a));
		DLOG1(("Net Server service=%04x (boot_$get_uids)", service));
		break;
	case 0x0c: // boot_$mult_ld (list directory for sau7, SAU8, SAU14)
		get_dir_data( node_type == 7 ? APOLLO_SAU7_PATH : node_type == 8 ? APOLLO_SAU8_PATH : APOLLO_SAU14_PATH, packet_buffer);
		DLOG1(("Net Server service=%04x (boot_$mult_ld)", service));
		break;
	case 0x12: // boot_$load    (ex /sau7/self_test, domain_os, ...)
		strcpy(pathname, node_type == 7 ? APOLLO_SAU7_PATH : node_type == 8 ? APOLLO_SAU8_PATH : APOLLO_SAU14_PATH);
		strcat(pathname, "/");
		strcat(pathname, get_file_name(rx_data_buffer + 0x48));
		get_file_data(pathname, packet_buffer, first_sector, last_sector);
		DLOG1(("Net Server service=%04x (boot_$load %s - sector %x of %x-%x)", service, pathname, current_sector, first_sector, last_sector));
		break;
	case 0x18: // boot_$set_path (set current path and get attributes of current path)
		set_packet(packet_buffer, response_0718, sizeof(response_0718));

		if (get_long(rx_data_buffer + 0x6e) == 0L && get_long(rx_data_buffer + 0x72) == 0L) {
			// reset path name to root dir
			strcpy(current_pathname, root_path);
		}

		// FIXME: why is this necessary???
		if (is_directory(current_pathname)) {
			strcat(current_pathname, "/");
			strcat(current_pathname, get_file_name(rx_data_buffer + 0x48));
		}

		// 0x4b: systyp (1=dir, 0=file)
		set_byte(packet_buffer + 0x4b, is_directory(current_pathname));
		set_word(packet_buffer + 0x58, get_file_type(current_pathname));
		set_long(packet_buffer + 0x66, get_file_size(current_pathname));

		DLOG1(("Net Server service=%04x (boot_$set_path %s)", service, current_pathname + strlen(root_path)));
		break;
	case 0x1a: // boot_$load_file (?): load /sau7/cmd/startup.cmd, /sau7/cpu.dex, ...
		get_file_data(current_pathname + strlen(root_path), packet_buffer, first_sector, last_sector);
		DLOG1(("Net Server service=%04x (boot_$load_file %s - sector %x of %x-%x)", service, current_pathname + strlen(root_path), current_sector, first_sector, last_sector));
		break;
	default:
		DLOG(("Net Server - unexpected service %04x", service&0xff));
		return 0;
	}

	set_word(packet_buffer + 0x44, (service & 0xff) + 1); // 0x44: response packet type

	tx_data_length = ETHERNET_HEADER_SIZE + get_packet_size(packet_buffer);

	log_data(device, "Tx", tx_data_buffer, tx_data_length);
	return tx_data_length;
}

static int is_ethernet_broadcast_address(const UINT8 ethernet_header[]) {
	return get_3byte(ethernet_header + 0) == 0xffffff && //
			get_3byte(ethernet_header + 3) == 0xffffff;
}

static int is_apollo_multicast_address(const UINT8 ethernet_header[]) {
	return get_3byte(ethernet_header + 0) == ETHERNET_APOLLO_DOMAIN_PREFIX && //
			(get_3byte(ethernet_header + 3) == 0L || get_3byte(ethernet_header + 3) == 1L);
}

static int is_my_ethernet_address(const UINT8 mac_address[]) {
	return memcmp(mac_address, my_mac_address, 6) == 0;
}

static int is_my_ip_address(const UINT8 ip_address[]) {
	return memcmp (ip_address, my_ip_address, 4) == 0;
}

static int is_my_node_id(const UINT8 node_id[]) {
	return get_long(node_id) == my_node_id;
}

static int ethernet_packet_is_for_me(const UINT8 ethernet_packet_header[],
		const int data_length) {
	return data_length >= ETHERNET_HEADER_SIZE && (
			is_ethernet_broadcast_address(ethernet_packet_header) ||
			is_apollo_multicast_address(ethernet_packet_header) ||
			is_my_ethernet_address(ethernet_packet_header));
}

static int is_apollo_request(const UINT8 ethernet_header[],
		const int data_length) {

	return data_length >= 0x30 && //
		get_word(ethernet_header + 0x0c) == ETHERNET_PACKET_TYPE_APOLLO;
}

static int is_apollo_arp_request(const UINT8 ethernet_header[], const int data_length) {
	return data_length >= 0x30 && //
			get_word(ethernet_header + 0x0c) == ETHERNET_PACKET_TYPE_ARP && //
			get_word(ethernet_header + 0x10) == ETHERNET_PACKET_TYPE_APOLLO && //
			is_apollo_multicast_address(ethernet_header) && //
			is_my_node_id(ethernet_header + 0x26);
}

static int is_ip_arp_request(const UINT8 ethernet_header[], const int data_length) {
	return data_length >= 0x30 && //
			get_word(ethernet_header + 0x0c) == ETHERNET_PACKET_TYPE_ARP && //
			get_word(ethernet_header + 0x10) == ETHERNET_PACKET_TYPE_IP && //
			is_ethernet_broadcast_address(ethernet_header) && //
			is_my_ip_address (ethernet_header + 0x26);
}

static int is_ip_echo_request(const UINT8 ethernet_header[], const int data_length) {
	return data_length >= 0x30 && //
			get_word(ethernet_header + 0x0c) == ETHERNET_PACKET_TYPE_IP && //
        	get_byte(ethernet_header + 0x22) == ICMP_ECHO && //
        	is_my_ethernet_address(ethernet_header);
}

static UINT16 in_checksum(UINT8 *ptr, UINT32 nbytes)
{
	UINT32 sum = 0;
	while (nbytes > 1)
	{
		// MSB first
		sum += (*ptr++) << 8;
		sum += *ptr++;
		nbytes -=2;
	}
	if (nbytes == 1)
	{
		sum += (*ptr++) << 8;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return (~sum) & 0xffff;
}

/***************************************************************************
 simple ARP Server emulation
 ***************************************************************************/

static int arp_server(const device_t *device,
		const UINT8 rx_data[], int rx_data_length,
		UINT8 tx_data[]) {

//  static UINT8 arp_response[] = {
//      0x08,0x00,0x1e,0x02,0x61,0x6d, // 0x00: Ethernet address of destination
//      0x08,0x00,0x1e,0x04,0x20,0xe8, // 0x06: Ethernet address of sender
//      0x08,0x06,                     // 0x0c: Protocol type
//      0x00,0x01,                     // 0x0e: Hardware address space
//      0x80,0x19,                     // 0x10: Protocol address space
//      0x06,                          // 0x12: byte length of each hardware address
//      0x04,                          // 0x13: byte length of each protocol address
//      0x00,0x02,                     // 0x14: opcode (ares_op$REQUEST | ares_op$REPLY)
//      0x08,0x00,0x1e,0x04,0x20,0xe8, // 0x16: Hardware address of sender of this packet
//      0x00,0x04,0x20,0xe8,           // 0x1c: Protocol address of sender of this packet
//      0x08,0x00,0x1e,0x02,0x61,0x6d, // 0x20: Hardware address of target of this packet (if known)
//      0x00,0x02,0x61,0x6d,           // 0x26: Protocol address of target
//      ...
//  };

	UINT16 tx_data_length = rx_data_length;
	memcpy(tx_data, rx_data, tx_data_length);

	// 0x00: Ethernet address of destination
	memcpy(tx_data + 0x00, rx_data + 0x06, 6);
	// 0x06: Ethernet address of sender
	memcpy(tx_data + 0x06, my_mac_address, 6);
	// 0x14: opcode (ares_op$REQUEST | ares_op$REPLY)
	set_word(tx_data + 0x14, 2);
	// 0x16: Hardware address of sender
	memcpy(tx_data + 0x16, my_mac_address, 6);
	// 0x1c: Protocol address of sender
	memcpy(tx_data + 0x1c, rx_data + 0x26, 4);
	// 0x20: Hardware address of target (if known)
	memcpy(tx_data + 0x20, rx_data + 0x16, 6);
	// 0x26: Protocol address of target
	memcpy(tx_data + 0x26, rx_data + 0x1c, 4);

	log_data(device, "ARP Rx", rx_data, rx_data_length);
	log_data(device, "ARP Tx", tx_data, tx_data_length);
	return tx_data_length;
}

/***************************************************************************
 simple Echo (ping) Server emulation
 ***************************************************************************/

static int echo_server(const device_t *device,
		const UINT8 rx_data[], int rx_data_length,
		UINT8 tx_data[]) {

    static const UINT32 ip_offset = sizeof(ethernet_header);
    static const UINT32 icmp_offset = ip_offset + sizeof(ip_header);

	UINT16 tx_data_length = rx_data_length;
	memcpy(tx_data, rx_data, tx_data_length);

	// 0x00: Ethernet address of destination
	memcpy(tx_data + 0x00, rx_data + 0x06, 6);
	// 0x06: Ethernet address of sender
	memcpy(tx_data + 0x06, rx_data + 0x00, 6);
	// 0x12: ip.id
	set_word(tx_data + 0x12, 0x0573);
	// 0x18: ip.checksum
	set_word(tx_data + 0x18, 0);
	set_word(tx_data + 0x18, in_checksum(tx_data + ip_offset, sizeof(ip_header)));

	// 0x1a: Protocol address of sender
	memcpy(tx_data + 0x1a, rx_data + 0x1e, 4);
	// 0x1e: Protocol address of target
	memcpy(tx_data + 0x1e, rx_data + 0x1a, 4);
	// 0x22: icmp.type
	set_byte(tx_data + 0x22, ICMP_ECHOREPLY);
	// 0x24: icmp.cksump
	set_word(tx_data + 0x24, 0);
	set_word(tx_data + 0x24, in_checksum(tx_data + icmp_offset, tx_data_length-icmp_offset));

	log_data(device, "Echo Rx", rx_data, rx_data_length);
	log_data(device, "Echo Tx", tx_data, tx_data_length);
	return tx_data_length;
}

/***************************************************************************
 netserver
 ***************************************************************************/

static int netserver(const device_t *device,
		const UINT8 rx_data[], int rx_data_length,
		UINT8 tx_data[]) {

	if (!ethernet_packet_is_for_me(rx_data, rx_data_length))
		return 0; // skip
	else if (is_apollo_request(rx_data, rx_data_length))
	{
		UINT16 socket = get_socket(rx_data, rx_data_length);

		switch (socket) {
		case 1: // Paging socket
			break;
		case 2: // File socket
//          return file_server(device, rx_data, rx_data_length, tx_data);
			break;
		case 3: // NETMAN socket
			return netman_server(device, rx_data, rx_data_length, tx_data);
		case 4: // Information socket
			break;
		case 5: // Receives internet asknode "who" replies
			break;
		case 6: // File server overflow socket
			break;
		case 7: // Software diagnostic socket
			break;
		case 8: // routing information protocol (RIP) socket
			break;
		case 9: // Mailbox socket
			break;
		case 10: // NS Helper socket
			break;
		case 11: // TCP/IP
			break;
		case 12: // ?
			break;
		default:
			break;
		}

		DLOG1(("Net Server - socket %d not yet supported", socket));
		return 0;
	} else if (is_apollo_arp_request(rx_data, rx_data_length)) {
		return arp_server(device, rx_data, rx_data_length, tx_data);
	} else if (is_ip_arp_request(rx_data, rx_data_length)) {
		return arp_server(device, rx_data, rx_data_length, tx_data);
	} else if (is_ip_echo_request(rx_data, rx_data_length)) {
		return echo_server(device, rx_data, rx_data_length, tx_data);
	} else {
		log_data(device, "unexpected Rx", rx_data, rx_data_length);
		return 0;
	}
}

/***************************************************************************
 receive_interrupt
 ***************************************************************************/

static TIMER_CALLBACK(receive_interrupt) {
	device_t *device = (device_t *)ptr;

	UINT8 tx_data_buffer[ETH_BUFFER_SIZE];

	int tx_data_length = netserver(device, current_rx_data_buffer, current_rx_data_length, tx_data_buffer);

	DLOG2(("Net Server rx/tx_data_length = %x/%x current_sector=%x current_last_sector=%x",
			current_rx_data_length, tx_data_length, current_sector, current_last_sector ));

	if (tx_data_length > 0 && tx_callback != NULL) {
		// threecom3c505_receive(device, tx_data_buffer, tx_data_length);
		(*tx_callback)(device, tx_data_buffer, tx_data_length);
	}

	if (current_file != NULL && current_sector < current_last_sector)
	{
		// more packets pending
		machine.scheduler().timer_set(attotime::from_msec(1), FUNC(receive_interrupt), 0, device);
	} else {
		current_rx_data_length = 0;
	}
}

/***************************************************************************
 apollo_netserver_receive - receive and process an ethernet packet
 ***************************************************************************/

int apollo_netserver_receive(device_t *device, const UINT8 rx_data_buffer[],
		int rx_data_length)
{
	if (!ethernet_packet_is_for_me(rx_data_buffer, rx_data_length))
	{
		return 1; // skip packet
	}
	else if (current_rx_data_length > 0)
	{
		LOG(("!!!! apollo_netserver_receive: busy - skipped data with length %02x",rx_data_length));
		return 0;
	}
	else
	{
		memcpy(current_rx_data_buffer, rx_data_buffer, rx_data_length);
		current_rx_data_length = rx_data_length;

		// delay response to multicast requests
		int ms = is_apollo_multicast_address(rx_data_buffer) ? 100 : 1;
		device->machine().scheduler().timer_set(attotime::from_msec(ms), FUNC(receive_interrupt), 0, device);
		return 1;
	}
}

/***************************************************************************
 apollo_netserver_init - specify the transmitter call back function
 ***************************************************************************/

void apollo_netserver_init(const char *root_path, apollo_netserver_transmit tx_data)
{
	::root_path.cat(root_path);
	::root_path.cat(PATH_SEPARATOR);
	::root_path.cat(NETSERVER_ROOT_PATH);

	::tx_callback = tx_data;

	my_node_id = NETSERVER_NODE_ID;
	set_3byte(my_mac_address, ETHERNET_APOLLO_PREFIX);
	set_3byte(my_mac_address + 3, my_node_id);

	current_rx_data_length = 0;
}
