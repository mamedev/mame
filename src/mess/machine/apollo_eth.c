/*
 * apollo_eth.c - APOLLO ethernet proxy
 *
 *  Created on: Sep 19, 2011
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *  see also:
 *  http://www.linuxjournal.com/article/4659
 *
 *
 *  TODO:
 *  Scrap this unportable stuff, make 3c505 use the core Ethernet layer directly like the 3c503.
 *
 */

#define VERBOSE 0

#include "includes/apollo.h"

#if !defined(APOLLO_FOR_LINUX)

int apollo_eth_transmit(device_t *device, const UINT8 data[], int length)
{
	return 1;
}

int apollo_eth_setfilter(device_t *device, int node_id)
{
	return 0;
}

void apollo_eth_init(device_t *device, apollo_eth_receive rx_data)
{
}

#else  /* defined(APOLLO_FOR_LINUX) */

#include "machine/3c505.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/filter.h>
#include <netinet/in.h>

#define ETH_BUFFER_SIZE 2048

// TODO: should be configurable
#define ETH_INTERFACE_NAME "eth0"
//#define ETH_INTERFACE_NAME "vmnet0"

static const char *eth_if_name;
static int eth_if_index = -1;
static int eth_socket = -1;
static UINT8 eth_hw_address[ETHERNET_ADDR_SIZE];

static apollo_eth_receive rx_callback = NULL;

/***************************************************************************
 log data
 ***************************************************************************/

static void log_data(const device_t *device, const char *type,
		const UINT8 *data, int data_length) {
	if (VERBOSE > 0) {
		int i;
		logerror("%s - %s: %s (length=%2x)", apollo_cpu_context(
				device->machine().firstcpu), device->tag(), type, data_length);
		for (i = 0; i < data_length; i++) {
			logerror(" %02x", data[i]);
		}
		logerror("\n");
	}
}

/***************************************************************************
 apollo_eth_receive_packet
 ***************************************************************************/

static int apollo_eth_receive_packet(device_t *device)
{
	u_char rx_buffer[ETH_BUFFER_SIZE];
	int packet_len;

	do
	{
		packet_len = recv(eth_socket, rx_buffer, sizeof(rx_buffer), MSG_TRUNC);
	} while (packet_len == -1 && errno == EINTR);

	/* Check if an error occurred */
	if (packet_len == -1)
	{
		switch (errno)
		{
		case EAGAIN:
			return 0; /* no packet there */

		case ENETDOWN:
			// The device on which we're capturing went away.
			DLOG1(("apollo_eth_receive_packet: the interface went down (error %d - %s)", errno, strerror(errno)));
			return -1;

		default:
			DLOG(("apollo_eth_receive_packet: recv failed (error %d - %s)", errno, strerror(errno)));
			return -1;
		}
	}
	else if (packet_len > sizeof(rx_buffer))
	{
		DLOG(("apollo_eth_receive_packet: data size (%d) exceeds rx buffer size (%d)!!!", packet_len, sizeof(rx_buffer)));
		return -1;
	}

	log_data(device, "apollo_eth_receive_packet", rx_buffer, packet_len);

	/* Call the user supplied callback function */
	if (rx_callback != NULL)
	{
		// threecom3c505_receive(device, tx_data_buffer, tx_data_length);
		(*rx_callback)(device, rx_buffer, packet_len);
	}

	return 1;
}

/***************************************************************************
 receive_interrupt
 ***************************************************************************/

static TIMER_CALLBACK(receive_interrupt)
{
	device_t *device = (device_t *) ptr;

	/*
     * Save the socket's current mode, and put it in non-blocking mode;
     * we drain it by reading packets until we get an error
     * (which is normally a "nothing more to be read" error).
     */
	int save_mode = fcntl(eth_socket, F_GETFL, 0);
	if (save_mode == -1)
	{
		DLOG(("receive_interrupt: fcntl failed (error %d - %s)", errno, strerror(errno)))
	}
	else if (fcntl(eth_socket, F_SETFL, save_mode | O_NONBLOCK) < 0)
	{
		DLOG(("receive_interrupt: fcntl failed (error %d - %s)", errno, strerror(errno)))
	}
	else
	{
		int i = 0;
		while (apollo_eth_receive_packet(device) > 0 && i++ < 4)
			;

		fcntl(eth_socket, F_SETFL, save_mode);
	}

	machine.scheduler().timer_set(attotime::from_msec(1), FUNC(receive_interrupt), 0, device);
}

/***************************************************************************
 apollo_eth_transmit - transmit an packet to the ethernet
 ***************************************************************************/

int apollo_eth_transmit(device_t *device, const UINT8 data[], int length) {

	if (eth_socket != -1)
	{
		if (memcmp(data, eth_hw_address, ETHERNET_ADDR_SIZE) == 0)
		{
			// destination ethernet address is my own mac address
			// TODO: should send data to "eth0"
		}

		if (send(eth_socket, data, length, 0) == -1) {
			DLOG(("apollo_eth_transmit: send failed with error %d (%s)", errno, strerror(errno)));
		} else {
			log_data(device, "apollo_eth_transmit", data, length);
		}
	}

	return 1;
}

/***************************************************************************
 apollo_eth_setfilter - set the socket filter
 ***************************************************************************/

int apollo_eth_setfilter(device_t *device, int node_id)
{
	/*
    tcpdump -i eth0 -dd ether dst 08:00:1e:01:ae:a5 or ether dst 09:00:1e:00:00:00 or ether dst 09:00:1e:00:00:01 or ether broadcast

    (000) ld       [2]
    (001) jeq      #0x1e01aea5      jt 2    jf 4
    (002) ldh      [0]
    (003) jeq      #0x800           jt 11   jf 12
    (004) jeq      #0x1e000000      jt 6    jf 5
    (005) jeq      #0x1e000001      jt 6    jf 8
    (006) ldh      [0]
    (007) jeq      #0x900           jt 11   jf 12
    (008) jeq      #0xffffffff      jt 9    jf 12
    (009) ldh      [0]
    (010) jeq      #0xffff          jt 11   jf 12
    (011) ret      #65535
    (012) ret      #0
    */
	static struct sock_filter bpf_code[]= {
		{ 0x20, 0, 0, 0x00000002 },
		{ 0x15, 0, 2, 0x1e01aea5 },
		{ 0x28, 0, 0, 0x00000000 },
		{ 0x15, 7, 8, 0x00000800 },
		{ 0x15, 1, 0, 0x1e000000 },
		{ 0x15, 0, 2, 0x1e000001 },
		{ 0x28, 0, 0, 0x00000000 },
		{ 0x15, 3, 4, 0x00000900 },
		{ 0x15, 0, 3, 0xffffffff },
		{ 0x28, 0, 0, 0x00000000 },
		{ 0x15, 0, 1, 0x0000ffff },
		{ 0x6, 0, 0, 0x0000ffff },
		{ 0x6, 0, 0, 0x00000000 }
	};

	if (eth_socket != -1)
	{
		struct sock_fprog filter;
		filter.len = sizeof(bpf_code) / sizeof(sock_filter);
		filter.filter = bpf_code;

		// set node id in filter
		bpf_code[1].k = (bpf_code[1].k & 0xff000000) | (node_id & 0x00ffffff);

		/* Attach the filter to the socket */
		if (setsockopt(eth_socket, SOL_SOCKET, SO_ATTACH_FILTER, &filter,
				sizeof(filter)) < 0)
		{
			DLOG(("apollo_eth_setfilter: failed with error %d (%s)", errno, strerror(errno)));
			return 0;
		}
		else
		{
			DLOG1(("apollo_eth_setfilter: set filter for node id %x", node_id & 0x00ffffff));
		}
	}
	return 1;
}

/***************************************************************************
 apollo_eth_init - specify node id and transmitter call back function
 ***************************************************************************/

void apollo_eth_init(device_t *device, apollo_eth_receive rx_data)
{
	rx_callback = rx_data;

	eth_if_name = ETH_INTERFACE_NAME;
	eth_if_index = -1;
	memset(eth_hw_address, 0 , sizeof(eth_hw_address));

	// Note: Only  processes  with effective UID 0 or the CAP_NET_RAW capability may open packet sockets.
	// see also: man 7 packet

	eth_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (eth_socket == -1)
	{
		if (errno == EPERM)
		{
			DLOG(("apollo_eth_init: %s - Ethernet access disabled", strerror(errno)));
		}
		else
		{
			DLOG1(("apollo_eth_init: socket failed (error %d - %s)", errno, strerror(errno)));
		}
	}
	else
	{
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, eth_if_name, sizeof(ifr.ifr_name));

		// Get interface index
		if (ioctl(eth_socket, SIOCGIFINDEX, &ifr) == -1)
		{
			DLOG(("apollo_eth_init: interface %s not found (error %d - %s)", eth_if_name, errno, strerror(errno)));
			close(eth_socket);
			eth_socket = -1;
		}
		else
		{
			eth_if_index = ifr.ifr_ifindex;

			// Get hardware address
			memset(&ifr, 0, sizeof(ifr));
			(void) strncpy(ifr.ifr_name, eth_if_name, sizeof(ifr.ifr_name));
			if (ioctl(eth_socket, SIOCGIFHWADDR, (char *) &ifr) < 0)
			{
				DLOG(("apollo_eth_init: SIOCGIFHWADDR failed for %s (error %d - %s)", eth_if_name, errno, strerror(errno)));
			}
			else
			{
				memcpy(eth_hw_address, ifr.ifr_hwaddr.sa_data, sizeof(eth_hw_address));
			}

			struct sockaddr_ll sll;
			memset(&sll, 0, sizeof(sll));
			sll.sll_family = AF_PACKET;
			sll.sll_ifindex = eth_if_index;
			sll.sll_protocol = htons(ETH_P_ALL);

			if (bind(eth_socket, (struct sockaddr *) &sll, sizeof(sll)) == -1)
			{
				DLOG(("apollo_eth_init: bind to interface %s failed (error %d - %s)", eth_if_name, errno, strerror(errno)));
				close(eth_socket);
				eth_socket = -1;
			}
			else
			{
				struct packet_mreq mr;
				memset(&mr, 0, sizeof(mr));
				mr.mr_ifindex = eth_if_index;
				mr.mr_type = PACKET_MR_PROMISC;
				if (setsockopt(eth_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
				{
					DLOG(("apollo_eth_init: failed to set promiscuous mode (error %d - %s)", errno, strerror(errno)));
					close(eth_socket);
					eth_socket = -1;
				}
				else
				{
					// Set the socket buffer size to the specified value.
					// by default request 2M for the ring buffer
					int rx_bufsize = 2*1024*1024;
					if (setsockopt(eth_socket, SOL_SOCKET, SO_RCVBUF, &rx_bufsize, sizeof(rx_bufsize)) == -1)
					{
						DLOG(("apollo_eth_init: failed to set buffer size (error %d - %s)", errno, strerror(errno)));
						close(eth_socket);
						eth_socket = -1;
					}
					else
					{
						device->machine().scheduler().timer_set(attotime::from_msec(1), FUNC(receive_interrupt), 0, device);
					}
				}
			}
		}
	}
}
#endif /* defined(APOLLO_FOR_LINUX) */
