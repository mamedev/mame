// license:BSD-3-Clause
// copyright-holders: Kelvin Sherlock, R. Belmont

#include "emu.h"
#include "vmnet_common.h"

static constexpr int ETHERNET_MIN_FRAME = 64;
static uint8_t ff[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

int is_arp(const uint8_t *packet, u32 size) {
  return size >= arp_data
	&& packet[12] == 0x08 && packet[13] == 0x06 /* ARP */
	&& packet[14] == 0x00 && packet[15] == 0x01 /* ethernet */
	&& packet[16] == 0x08 && packet[17] == 0x00 /* ipv4 */
	&& packet[18] == 0x06 /* hardware size */
	&& packet[19] == 0x04 /* protocol size */
  ;
}

int is_broadcast(const uint8_t *packet, u32 size) {
  return !memcmp(packet + 0, ff, 6);
}

int is_unicast(const uint8_t *packet, u32 size) {
  return (*packet & 0x01) == 0;
}

int is_multicast(const uint8_t *packet, u32 size) {
  return (*packet & 0x01) == 0x01 && !is_broadcast(packet, size);
}

int is_dhcp_out(const uint8_t *packet, u32 size) {
  static uint8_t cookie[] = { 0x63, 0x82, 0x53, 0x63 };
  return size >= 282
	&& packet[12] == 0x08 && packet[13] == 0x00
	&& packet[14] == 0x45 /* version 4 */
	&& packet[23] == 0x11 /* UDP */
	&& packet[34] == 0x00 && packet[35] == 0x44 /* source port */
	&& packet[36] == 0x00 && packet[37] == 0x43 /* dest port */
	&& packet[43] == 0x01 /* ethernet */
	&& packet[44] == 0x06 /* 6 byte mac */
	&& !memcmp(&packet[278], cookie, 4)
  ;
}


int is_dhcp_in(const uint8_t *packet, u32 size) {
  static uint8_t cookie[] = { 0x63, 0x82, 0x53, 0x63 };
  return size >= 282
	&& packet[12] == 0x08 && packet[13] == 0x00
	&& packet[14] == 0x45 /* version 4 */
	&& packet[23] == 0x11 /* UDP */
	&& packet[34] == 0x00 && packet[35] == 0x43 /* source port */
	&& packet[36] == 0x00 && packet[37] == 0x44 /* dest port */
	&& packet[43] == 0x01 /* ethernet */
	&& packet[44] == 0x06 /* 6 byte mac */
	&& !memcmp(&packet[278], cookie, 4)
  ;
}

u32 ip_checksum(const uint8_t *packet) {
  u32 x = 0;
  u32 i;
  for (i = 0; i < ip_data; i += 2) {
	if (i == ip_header_cksum) continue;
	x += packet[eth_data + i + 0 ] << 8;
	x += packet[eth_data + i + 1];
  }

  /* add the carry */
  x += (x >> 16);
  x &= 0xffff;
  return ~x & 0xffff;
}

void recalc_ip_checksum(uint8_t *packet, u32 size)
{
  u32 x = 0;
  u32 i;

  if (size < eth_data + ip_data) return;

  packet[eth_data+ip_header_cksum+0] = 0;
  packet[eth_data+ip_header_cksum+1] = 0;

  for (i = 0; i < ip_data; i += 2) {
	x += packet[eth_data + i + 0 ] << 8;
	x += packet[eth_data + i + 1];
  }

  /* add the carry */
  x += (x >> 16);
  x = ~x & 0xffff;

  packet[eth_data+ip_header_cksum+0] = x >> 8;
  packet[eth_data+ip_header_cksum+1] = x;

}



void recalc_udp_checksum(uint8_t *packet, u32 size) {
  if (size < eth_data + ip_data + udp_data) return;

  uint8_t *udp_ptr = packet + eth_data + ip_data;

  // checksum optional for UDP.
  if (udp_ptr[udp_cksum+0] == 0 && udp_ptr[udp_cksum+1] == 0)
	return;

  udp_ptr[udp_cksum+0] = 0;
  udp_ptr[udp_cksum+1] = 0;

  u32 packet_len = (packet[eth_data + ip_len + 0] << 8) | (packet[eth_data + ip_len + 1]);

  if (packet_len + eth_data < size)
	return;

  packet_len -= ip_data;


  u32 sum = 0;
  u32 i;

  // pseudo header = src address, dest address, protocol (17), udp + data length
  sum = 17 + packet_len;
  for (i = 0; i < 4; i += 2) {
	sum += (uint32_t)packet[eth_data+ip_src + i + 0] << 8;
	sum += (uint32_t)packet[eth_data+ip_src + i + 1];
	sum += (uint32_t)packet[eth_data+ip_dest + i + 0] << 8;
	sum += (uint32_t)packet[eth_data+ip_dest + i + 1];
  }

  if (packet_len & 0x01) {
	sum += (uint32_t)udp_ptr[packet_len - 1] << 8;
	--packet_len;
  }

  for(i = 0; i < packet_len; i += 2) {
	sum += (uint32_t)udp_ptr[i + 0] << 8;
	sum += (uint32_t)udp_ptr[i + 1];
  }

  sum += sum >> 16;
  sum = ~sum & 0xffff;
  if (sum == 0) sum = 0xffff;

  udp_ptr[udp_cksum+0] = (sum >> 8) & 0xff;
  udp_ptr[udp_cksum+1] = (sum >> 0) & 0xff;

}


void fix_incoming_packet(uint8_t *packet, u32 size, const char real_mac[6], const char fake_mac[6]) {

  if (memcmp(packet + 0, real_mac, 6) == 0)
	memcpy(packet + 0, fake_mac, 6);

  if (is_arp(packet, size)) {
	/* receiver mac address */
	if (!memcmp(packet + 32, real_mac, 6))
	  memcpy(packet + 32, fake_mac, 6);
	return;
  }

  /* dhcp request - fix the hardware address */
  if (is_dhcp_in(packet, size) && !memcmp(packet + 70, real_mac, 6)) {
	memcpy(packet + 70, fake_mac, 6);
	recalc_udp_checksum(packet, size);
	return;
  }

}

void fix_outgoing_packet(uint8_t *packet, u32 size, const char real_mac[6], const char fake_mac[6]) {

  if (memcmp(packet + 6, fake_mac, 6) == 0)
	memcpy(packet + 6, real_mac, 6);

  if (is_arp(packet, size)) {
	/* sender mac address */
	if (!memcmp(packet + 22, fake_mac, 6))
	  memcpy(packet + 22, real_mac, 6);
	return;
  }

  /* dhcp request - fix the hardware address */
  if (is_dhcp_out(packet, size) && !memcmp(packet + 70, fake_mac, 6)) {

	memcpy(packet + 70, real_mac, 6);

	/* work-around for old IP65 bug where ip address used before it should be */
	if (size > 284 && packet[282] == 0x35 && packet[283] == 1 && packet[284] <= dhcp_request)
	{
	  memset(packet + 14 + ip_src, 0, 4);
	  recalc_ip_checksum(packet, size);
	}

	recalc_udp_checksum(packet, size);
	return;
  }

}

u32 finalize_frame(u8 buf[], u32 length)
{
  /*
   * The taptun driver receives frames which are shorter than the Ethernet
   * minimum. Partly this is because it can't see the frame check sequence
   * bytes, but mainly it's because the OS expects the lower level device
   * to add the required padding.
   *
   * We do the equivalent padding here (i.e. pad with zeroes to the
   * minimum Ethernet length minus FCS), so that devices which check
   * for this will not reject these packets.
   */
  if (length < ETHERNET_MIN_FRAME - 4)
  {
	std::fill_n(&buf[length], ETHERNET_MIN_FRAME - length - 4, 0);

	length = ETHERNET_MIN_FRAME - 4;
  }

  // compute and append the frame check sequence
  const u32 fcs = util::crc32_creator::simple(buf, length);

  buf[length++] = (fcs >> 0) & 0xff;
  buf[length++] = (fcs >> 8) & 0xff;
  buf[length++] = (fcs >> 16) & 0xff;
  buf[length++] = (fcs >> 24) & 0xff;

  return length;
}

