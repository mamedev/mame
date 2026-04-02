// license:BSD-3-Clause
// copyright-holders: Kelvin Sherlock, R. Belmont

#ifndef OSD_MODULES_NETDEV_VMNET_COMMON_H
#define OSD_MODULES_NETDEV_VMNET_COMMON_H

#pragma once

namespace vmnet_common {
	enum {
	eth_dest  = 0,  // destination address
	eth_src   = 6,  // source address
	eth_type  = 12, // packet type
	eth_data  = 14, // packet data
	};

	enum {
	ip_ver_ihl  = 0,
	ip_tos    = 1,
	ip_len    = 2,
	ip_id   = 4,
	ip_frag   = 6,
	ip_ttl    = 8,
	ip_proto    = 9,
	ip_header_cksum = 10,
	ip_src    = 12,
	ip_dest   = 16,
	ip_data   = 20,
	};

	enum {
	udp_source = 0, // source port
	udp_dest = 2, // destination port
	udp_len = 4, // length
	udp_cksum = 6, // checksum
	udp_data = 8, // total length udp header
	};

	enum {
	bootp_op = 0, // operation
	bootp_hw = 1, // hardware type
	bootp_hlen = 2, // hardware len
	bootp_hp = 3, // hops
	bootp_transid = 4, // transaction id
	bootp_secs = 8, // seconds since start
	bootp_flags = 10, // flags
	bootp_ipaddr = 12, // ip address knwon by client
	bootp_ipclient = 16, // client ip from server
	bootp_ipserver = 20, // server ip
	bootp_ipgateway = 24, // gateway ip
	bootp_client_hrd = 28, // client mac address
	bootp_spare = 34,
	bootp_host = 44,
	bootp_fname = 108,
	bootp_data = 236, // total length bootp packet
	};

	enum {
	arp_hw = 14,    // hw type (eth = 0001)
	arp_proto = 16,   // protocol (ip = 0800)
	arp_hwlen = 18,   // hw addr len (eth = 06)
	arp_protolen = 19,  // proto addr len (ip = 04)
	arp_op = 20,    // request = 0001, reply = 0002
	arp_shw = 22,   // sender hw addr
	arp_sp = 28,    // sender proto addr
	arp_thw = 32,   // target hw addr
	arp_tp = 38,    // target protoaddr
	arp_data = 42,  // total length of packet
	};

	enum {
	dhcp_discover = 1,
	dhcp_offer = 2,
	dhcp_request = 3,
	dhcp_decline = 4,
	dhcp_pack = 5,
	dhcp_nack = 6,
	dhcp_release = 7,
	dhcp_inform = 8,
	};

	int is_arp(const uint8_t *packet, uint32_t size);
	int is_broadcast(const uint8_t *packet, uint32_t size);
	int is_unicast(const uint8_t *packet, uint32_t size);
	int is_multicast(const uint8_t *packet, uint32_t size);
	int is_dhcp_out(const uint8_t *packet, uint32_t size);
	int is_dhcp_in(const uint8_t *packet, uint32_t size);
	uint32_t ip_checksum(const uint8_t *packet);
	void recalc_ip_checksum(uint8_t *packet, uint32_t size);
	void recalc_udp_checksum(uint8_t *packet, uint32_t size);
	void fix_incoming_packet(uint8_t *packet, uint32_t size, const char real_mac[6], const char fake_mac[6]);
	void fix_outgoing_packet(uint8_t *packet, uint32_t size, const char real_mac[6], const char fake_mac[6]);
	uint32_t finalize_frame(uint8_t buf[], uint32_t length);
}

#endif
