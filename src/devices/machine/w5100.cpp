// license:BSD-3-Clause
// copyright-holders: Kelvin Sherlock


/*
	WIZnet W5100

	Used in: Uthernet II (Apple II), Spectranet (ZX Spectrum)

	Based on:
	W5100, W5100S, W5200, W5500 datasheets
	WIZnet ioLibrary Driver (https://github.com/Wiznet/ioLibrary_Driver)
	https://docs.wiznet.io/Product/iEthernet/W5100/
	Uthernet II User's and Programmer's Manual
	Testing with Uthernet II and W5100S-EVB-Pico

	0x0000-0x00ff - common registers
	0x0100-0x03ff - reserved
	0x0400-0x07ff - socket registers
	0x0800-0x3fff - reserved
	0x4000-0x5fff - tx memory
	0x6000-0x7fff - rx memory

	"reserved" memory mirrors the common/socket registers, i.e.:
	x000-x3ff, x800-xbff are common registers,
    x400-x7ff, xc00-xfff are socket registers.
    on the w5100, all undocumented socket registers mirror socket register 3.


 */

/*
	Not supported:
	- anything PPPoE related
*/

#include "emu.h"
#include "machine/w5100.h"
#include "machine/w5100_socket.h"
#include "util/internet_checksum.h"

// #define LOG_GENERAL (1U << 0)
#define LOG_PACKETS (1U << 1)
#define LOG_ARP     (1U << 2)
#define LOG_ICMP    (1U << 3)
#define LOG_IGMP    (1U << 4)

#define VERBOSE (LOG_GENERAL|LOG_PACKETS|LOG_ARP|LOG_ICMP|LOG_IGMP)
#include "logmacro.h"


/* indirect mode addresses */
enum : uint8_t {
	IDM_OR = 0x00,
	IDM_AR0 = 0x01,
	IDM_AR1 = 0x02,
	IDM_DR = 0x03
};

/* Common registers */
enum : uint8_t {
	MR = 0x00,
	GAR0,
	GAR1,
	GAR2,
	GAR3,
	SUBR0,
	SUBR1,
	SUBR2,
	SUBR3,
	SHAR0,
	SHAR1,
	SHAR2,
	SHAR3,
	SHAR4,
	SHAR5,
	SIPR0,
	SIPR1,
	SIPR2,
	SIPR3,
	/* 0x13-0x14 reserved */
	IR = 0x15,
	IMR,
	RTR0,
	RTR1,
	RCR,
	RMSR,
	TMSR,
	PATR0,
	PATR1,
	/* 0x1c-0x1f reserved */
	/* 0x22-0x27 reserved */
	PTIMER = 0x28,
	PMAGIC,
	UIPR0,
	UIPR1,
	UIPR2,
	UIPR3,
	UPORT0,
	UPORT1,

};


/* Mode Register bits */
enum : uint8_t {
	MR_RST = 0x80,
	MR_PB = 0x10,
	MT_PPPoE = 0x08,
	MR_AI = 0x02,
	MR_IND = 0x01
};


/* Interrupt Register bits */
enum : uint8_t {
	IR_CONFLICT = 0x80,
	IR_UNREACH = 0x40,
	IR_PPPTERM = 0x20,
	IR_S3_INT = 0x08,
	IR_S2_INT = 0x04,
	IR_S1_INT = 0x02,
	IR_S0_INT = 0x01
};


/* IP, ARP, etc offsets and constants */

enum {
	o_ETHERNET_DEST = 0,
	o_ETHERNET_SRC = 6,
	o_ETHERNET_TYPE = 12,

	ETHERNET_TYPE_IP = 0x0800,
	ETHERNET_TYPE_ARP = 0x0806,
	ETHERNET_TYPE_IPV6 = 0x86dd,
};

enum {
	o_IP_IHL = 0, // version + header length
	o_IP_TOS = 1,
	o_IP_LENGTH = 2,
	o_IP_IDENTIFICATION = 4,
	o_IP_FLAGS = 6, // flags + fragment
	o_IP_TTL = 8,
	o_IP_PROTOCOL = 9,
	o_IP_CHECKSUM = 10,
	o_IP_SRC_ADDRESS = 12,
	o_IP_DEST_ADDRESS = 16,

	IP_PROTOCOL_ICMP = 1,
	IP_PROTOCOL_IGMP = 2,
	IP_PROTOCOL_TCP = 6,
	IP_PROTOCOL_UDP = 17,
};

enum {
	o_UDP_SRC_PORT = 0,
	o_UDP_DEST_PORT = 2,
	o_UDP_LENGTH = 4,
	o_UDP_CHECKSUM = 6,
};

enum {
	o_TCP_SRC_PORT = 0,
	o_TCP_DEST_PORT = 2,
	o_TCP_SEQ_NUMBER = 4,
	o_TCP_ACK_NUMBER = 8,
	o_TCP_DATA_OFFSET = 12,
	o_TCP_FLAGS = 13,
	o_TCP_WINDOW_SIZE = 14,
	o_TCP_CHECKSUM = 16,
	o_TCP_URGENT = 18,
};



enum {
	o_ICMP_TYPE = 0,
	o_ICMP_CODE = 1,
	o_ICMP_CHECKSUM = 2,

	ICMP_ECHO_REPLY = 0x00, // rfc 792
	ICMP_DESTINATION_UNREACHABLE = 0x03,
	ICMP_ECHO_REQUEST = 0x8,
};

enum {
	o_IGMP_TYPE = 0,
	o_IGMP_MAX_RESP_TIME = 1,
	o_IGMP_CHECKSUM = 2,
	o_IGMP_GROUP_ADDRESS = 4,

	IGMP_TYPE_MEMBERSHIP_QUERY = 0x11,
	IGMP_TYPE_MEMBERSHIP_REPORT_V1 = 0x12,
	IGMP_TYPE_MEMBERSHIP_REPORT_V2 = 0x16,
	IGMP_TYPE_LEAVE_GROUP = 0x17
};


enum {
	// offsets.  assumes 4-byte PLEN, 6-byte HLEN
	o_ARP_HTYPE = 0,
	o_ARP_PTYPE = 2,
	o_ARP_HLEN = 4,
	o_ARP_PLEN = 5,
	o_ARP_OPCODE = 6,
	o_ARP_SHA = 8,
	o_ARP_SPA = 14,
	o_ARP_THA = 18,
	o_ARP_TPA = 24,

	ARP_OPCODE_REQUEST = 0x01, // rfc 826
	ARP_OPCODE_REPLY = 0x02,
	ARP_HTYPE_ETHERNET = 1,
};


static const int MAX_FRAME_SIZE = 1514;
static const uint8_t ETHERNET_BROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static const uint8_t ETHERNET_MULTICAST[6] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x02 };


[[maybe_unused]] static std::string ip_to_string(uint32_t ip)
{
	char buffer[sizeof("255.255.255.255")];
	snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
		(ip >> 24) & 0xff,
		(ip >> 16) & 0xff,
		(ip >> 8) & 0xff,
		(ip >> 0) & 0xff
	);
	return buffer;
}

[[maybe_unused]] static std::string ip_to_string(uint32_t ip, uint16_t port)
{
	char buffer[sizeof("255.255.255.255:65535")];
	snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d:%d",
		(ip >> 24) & 0xff,
		(ip >> 16) & 0xff,
		(ip >> 8) & 0xff,
		(ip >> 0) & 0xff,
		port
	);
	return buffer;
}


static inline uint32_t be_read(const uint8_t *base, int numbytes)
{
	uint32_t result = 0;
	while (numbytes--)
		result = (result << 8) | *base++;
	return result;
}

static inline void be_write(uint8_t *base, uint32_t value, int numbytes)
{
	base += numbytes;
	while (numbytes--)
	{
		*--base = value;
		value >>= 8;
	}
}

static wiznet_eth_info parse_eth(const uint8_t *buffer, int length)
{
	wiznet_eth_info info{};

	if (length >= 14)
	{
		memcpy(info.dest_mac, buffer + o_ETHERNET_DEST, 6);
		memcpy(info.src_mac, buffer + o_ETHERNET_SRC, 6);
		info.type = be_read(buffer + o_ETHERNET_TYPE, 2);
	}
	return info;
}

static wiznet_udp_info parse_udp(const uint8_t *buffer, int length)
{
	wiznet_udp_info info{};

	if (length >= 8)
	{
		info.udp_length = be_read(buffer + o_UDP_LENGTH, 2);
		info.src_port = be_read(buffer + o_UDP_SRC_PORT, 2);
		info.dest_port = be_read(buffer + o_UDP_DEST_PORT, 2);
	}
	return info;
}

static wiznet_ip_info parse_ip(const uint8_t *buffer, int length)
{
	wiznet_ip_info info{};

	if (length >= 20)
	{
		info.header_length = (buffer[o_IP_IHL] & 0x0f) << 2;
		info.tos = buffer[o_IP_TOS];
		info.total_length = be_read(buffer + o_IP_LENGTH, 2);
		info.fragment = be_read(buffer + o_IP_FLAGS, 2);
		info.ttl = buffer[o_IP_TTL];
		info.proto = buffer[o_IP_PROTOCOL];
		info.src_ip = be_read(buffer + o_IP_SRC_ADDRESS, 4);
		info.dest_ip = be_read(buffer + o_IP_DEST_ADDRESS, 4);
	}
	return info;
}

// w5100 only looks at mss.
static void parse_tcp_options(wiznet_tcp_info &info, const uint8_t *buffer, int length)
{
	while (length > 0)
	{
		int ol = 0;
		switch(buffer[0])
		{
			case 0:
				/* end of option list */
				return;
			case 1:
				/* no operation */
				ol = 1;
				break;
			case 2:
				/* max segment size */
				ol = buffer[1];
				if (ol == 4 && length >= 4)
				{
					info.option_mss = be_read(buffer + 2, 2);
				}
				break;
			default:
				/* everything else.  buffer[1] is the length */
				ol = buffer[1];
				break;
		}
		length -= ol;
		buffer += ol;
	}
}
static wiznet_tcp_info parse_tcp(const uint8_t *buffer, int length)
{
	wiznet_tcp_info info{};

	if (length >= 20)
	{
		info.header_length = (buffer[o_TCP_DATA_OFFSET] >> 4) << 2;
		info.flags = buffer[o_TCP_FLAGS];
		info.sequence_number = be_read(buffer + o_TCP_SEQ_NUMBER, 4);
		info.ack_number = be_read(buffer + o_TCP_ACK_NUMBER, 4);
		info.src_port = be_read(buffer + o_TCP_SRC_PORT, 2);
		info.dest_port = be_read(buffer + o_TCP_DEST_PORT, 2);
		info.window_size = be_read(buffer + o_TCP_WINDOW_SIZE, 2);

		// check for mss option
		if (info.header_length > 20 && length >= info.header_length)
			parse_tcp_options(info, buffer + 20, info.header_length - 20);
	}
	return info;
}


static bool verify_arp(const uint8_t *arp, int length)
{
	if (length < 28) return false;
	if (be_read(arp + o_ARP_HTYPE, 2) != ARP_HTYPE_ETHERNET) return false;
	if (arp[o_ARP_HLEN] != 6 || arp[o_ARP_PLEN] != 4) return false;
	return true;
}

static bool verify_ip(const uint8_t *ip, int length)
{
	if (length < 20) return false;

	int ihl = (ip[o_IP_IHL] & 0x0f) << 2;
	if (ihl < 20) return false;

	if (util::internet_checksum_creator::simple(ip, ihl)) return false;

	int proto = ip[o_IP_PROTOCOL];

	const uint8_t *data = ip + ihl;
	int ip_length = be_read(ip + o_IP_LENGTH, 2);

	if (length < ip_length) return false;

	length = ip_length - ihl;

	if (proto == IP_PROTOCOL_ICMP || proto == IP_PROTOCOL_IGMP)
	{
		if (util::internet_checksum_creator::simple(data, length)) return false;
	}
	else if (proto == IP_PROTOCOL_UDP)
	{
		if (length < be_read(data + o_UDP_LENGTH, 2)) return false;

		uint16_t crc = be_read(data + o_UDP_CHECKSUM, 2);
		if (crc)
		{
			uint8_t pseudo_header[12];
			memcpy(pseudo_header + 0, ip + o_IP_SRC_ADDRESS, 4);
			memcpy(pseudo_header + 4, ip + o_IP_DEST_ADDRESS, 4);
			be_write(pseudo_header + 8, IP_PROTOCOL_UDP, 2);
			be_write(pseudo_header + 10, length, 2);

			util::internet_checksum_creator cr;
			cr.append(pseudo_header, sizeof(pseudo_header));
			cr.append(data, length);
			if (cr.finish() != 0) return false;
		}
	}
	else if (proto == IP_PROTOCOL_TCP)
	{
		if (length < 20) return false;

		int offset = (data[o_TCP_DATA_OFFSET] >> 4) << 2;
		if (offset < 20) return false;
		if (length < offset) return false;

		uint8_t pseudo_header[12];
		memcpy(pseudo_header + 0, ip + o_IP_SRC_ADDRESS, 4);
		memcpy(pseudo_header + 4, ip + o_IP_DEST_ADDRESS, 4);
		be_write(pseudo_header + 8, IP_PROTOCOL_TCP, 2);
		be_write(pseudo_header + 10, length, 2);

		util::internet_checksum_creator cr;
		cr.append(pseudo_header, sizeof(pseudo_header));
		cr.append(data, length);
		if (cr.finish() != 0) return false;
	}

	return true;
}

static uint16_t udp_tcp_checksum(unsigned proto, const uint8_t *ip_ptr, const uint8_t *data, unsigned length)
{
	uint8_t pseudo_header[12];

	util::internet_checksum_creator cc;

	memcpy(pseudo_header + 0, ip_ptr + o_IP_SRC_ADDRESS, 4);
	memcpy(pseudo_header + 4, ip_ptr + o_IP_DEST_ADDRESS, 4);
	be_write(pseudo_header + 8, proto, 2);
	be_write(pseudo_header + 10, length, 2);

	cc.append(pseudo_header, sizeof(pseudo_header));
	cc.append(data, length);

	return cc.finish();
}

w5100_device::w5100_device(machine_config const& mconfig, char const *tag, device_t *owner, u32 clock)
	: w5100_device(mconfig, W5100, tag, owner, clock)
{}

w5100_device::w5100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 100)
	, m_sockets(*this, "%u", 0)
	, m_irq_handler(*this)
{
}

void w5100_device::device_add_mconfig(machine_config &config)
{
	for (int sn = 0; sn < m_sockets.size(); ++sn)
		W5100_SOCKET(config, m_sockets[sn], clock(), sn);
}


void w5100_device::device_start()
{
	m_rx_buffer = std::make_unique<uint8_t[]>(0x2000);
	m_tx_buffer = std::make_unique<uint8_t[]>(0x2000);


	save_item(NAME(m_idm));
	save_item(NAME(m_identification));
	save_item(NAME(m_irq_state));

	save_item(NAME(m_mr));
	save_item(NAME(m_gateway));
	save_item(NAME(m_subnet));
	save_item(NAME(m_ip));
	save_item(NAME(m_shar));
	save_item(NAME(m_ir));
	save_item(NAME(m_imr));
	save_item(NAME(m_rtr));
	save_item(NAME(m_rcr));
	save_item(NAME(m_rmsr));
	save_item(NAME(m_tmsr));
	save_item(NAME(m_ptimer));
	save_item(NAME(m_pmagic));

	save_item(NAME(m_uipr));
	save_item(NAME(m_uport));

	save_pointer(NAME(m_rx_buffer), 0x2000);
	save_pointer(NAME(m_tx_buffer), 0x2000);


}

void w5100_device::device_reset()
{
	m_idm = 0;
	m_identification = 0;

	if (m_irq_state)
		m_irq_handler(CLEAR_LINE);
	m_irq_state = 0;

	// tx/rx buffers not cleared on reset.

	m_frame_queue.clear();

	m_mr = 0;
	m_gateway = 0;
	m_subnet = 0;
	memset(m_shar, 0, 6);
	m_ip = 0;
	m_ir = 0;
	m_imr = 0;
	m_rtr = 0x07d0;
	m_rcr = 0x08;
	m_rmsr = 0x55;
	m_tmsr = 0x55;
	m_ptimer = 0x28;
	m_pmagic = 0;
	m_uipr = 0;
	m_uport = 0;

	for (int i = 0; i < 4; ++i)
	{
		m_rx_buffer_size[i] = 0x0800;
		m_tx_buffer_size[i] = 0x0800;
		m_rx_buffer_offset[i] = i * 0x0800;
		m_tx_buffer_offset[i] = i * 0x0800;
	}


	set_mac(reinterpret_cast<char *>(m_shar));
	set_promisc(false);
}


void w5100_device::update_ethernet_irq()
{

	m_ir &= 0b11100000;

	unsigned bit = 0x01;
	for (auto socket : m_sockets)
	{
		if (socket->ir()) m_ir |= bit;
		bit <<= 1;
	}
	int ir = m_ir & m_imr;


	if ((bool)ir ^ (bool)m_irq_state)
	{
		m_irq_state = ir;
		m_irq_handler(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
	}
}

/*
 * Direct bus interface: 15-bit address, 8-bit data bus
 * Indirect bus interface: 2-bit address, 8-bit data bus
 * W5100s is always indirect w/ auto-increment
 */

uint8_t w5100_device::read(uint16_t offset)
{
	if (m_mr & MR_IND)
	{
		offset &= 0x03;
		switch(offset)
		{
			case IDM_OR:
				return m_mr;
			case IDM_AR0:
			case IDM_AR1:
				return util::big_endian_cast<uint8_t>(&m_idm)[offset - IDM_AR0];
			case IDM_DR:
				offset = m_idm;

				if ((m_mr & MR_AI) && !machine().side_effects_disabled())
				{
					m_idm++;
					if ((m_idm & 0x1fff) == 0x0000)
						m_idm -= 0x2000;
				}
				break;
		}
	}

	offset &= 0x7fff;

	if (offset >= 0x6000) return m_rx_buffer[offset - 0x6000];
	if (offset >= 0x4000) return m_tx_buffer[offset - 0x4000];

	// officially, 0x00-0xff are common registers and 0x400-0x7ff are socket registers.
	// in reality, these are mirrored to all other reserved memory, but on the
	// w5100, socket 3 is mirrored.
	if (!(offset & 0x0400)) return read_register(offset & 0xff);

	int sn = (offset >> 8) & 0x03;
	if (offset > 0x7ff) sn = 3;
	return m_sockets[sn]->read_register(offset & 0xff);
}


void w5100_device::write(uint16_t offset, uint8_t data)
{
	if (m_mr & MR_IND)
	{
		offset &= 0x03;
		switch(offset)
		{
			case IDM_OR:
				write_register(0, data);
				return;
			case IDM_AR0:
			case IDM_AR1:
				util::big_endian_cast<uint8_t>(&m_idm)[offset - IDM_AR0] = data;
				return;
			case IDM_DR:
				offset = m_idm;

				if ((m_mr & MR_AI))
				{
					m_idm++;
					if ((m_idm & 0x1fff) == 0x0000)
						m_idm -= 0x2000;
				}
				break;
		}
	}

	offset &= 0x7fff;

	if (offset >= 0x6000)
	{
		m_rx_buffer[offset - 0x6000] = data;
		return;
	}
	if (offset >= 0x4000)
	{
		m_tx_buffer[offset - 0x4000] = data;
		return;
	}
	if (!(offset & 0x0400))
	{
		write_register(offset & 0xff, data);
		return;
	}

	int sn = (offset >> 8) & 0x03;
	if (offset > 0x7ff) sn = 3;
	m_sockets[sn]->write_register(offset & 0xff, data);
}


uint8_t w5100_device::read_register(offs_t offset)
{

	switch(offset)
	{
		case MR:
			return m_mr;
		case GAR0:
		case GAR1:
		case GAR2:
		case GAR3:
			return util::big_endian_cast<uint8_t>(&m_gateway)[offset - GAR0];
		case SUBR0:
		case SUBR1:
		case SUBR2:
		case SUBR3:
			return util::big_endian_cast<uint8_t>(&m_subnet)[offset - SUBR0];
		case SHAR0:
		case SHAR1:
		case SHAR2:
		case SHAR3:
		case SHAR4:
		case SHAR5:
			return m_shar[offset - SHAR0];
		case SIPR0:
		case SIPR1:
		case SIPR2:
		case SIPR3:
			return util::big_endian_cast<uint8_t>(&m_ip)[offset - SIPR0];
		case IR:
			return m_ir;
		case IMR:
			return m_imr;
		case RTR0:
		case RTR1:
			return util::big_endian_cast<uint8_t>(&m_rtr)[offset - RTR0];
		case RCR:
			return m_rcr;
		case RMSR:
			return m_rmsr;
		case TMSR:
			return m_tmsr;
		case PATR0:
		case PATR1:
			return 0;
		case PTIMER:
			return m_ptimer;
		case PMAGIC:
			return m_pmagic;
		case UIPR0:
		case UIPR1:
		case UIPR2:
		case UIPR3:
			return util::big_endian_cast<uint8_t>(&m_uipr)[offset - UIPR0];
		case UPORT0:
		case UPORT1:
			return util::big_endian_cast<uint8_t>(&m_uport)[offset - UPORT0];
		default:
			return 0;
	}
}

void w5100_device::write_register(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case MR:
			m_mr = data;
			if (m_mr & MR_RST)
				device_reset();
			break;
		case GAR0:
		case GAR1:
		case GAR2:
		case GAR3:
			util::big_endian_cast<uint8_t>(&m_gateway)[offset - GAR0] = data;
			break;
		case SUBR0:
		case SUBR1:
		case SUBR2:
		case SUBR3:
			util::big_endian_cast<uint8_t>(&m_subnet)[offset - SUBR0] = data;
			break;
		case SHAR0:
		case SHAR1:
		case SHAR2:
		case SHAR3:
		case SHAR4:
		case SHAR5:
			m_shar[offset - SHAR0] = data;
			set_mac(reinterpret_cast<char *>(m_shar));
			break;
		case SIPR0:
		case SIPR1:
		case SIPR2:
		case SIPR3:
			util::big_endian_cast<uint8_t>(&m_ip)[offset - SIPR0] = data;
			break;
		case IR:
			data &= 0b11100000;
			m_ir &= ~data;
			update_ethernet_irq();
			break;
		case IMR:
			m_imr = data;
			update_ethernet_irq();
			break;
		case RTR0:
		case RTR1:
			util::big_endian_cast<uint8_t>(&m_rtr)[offset - RTR0] = data;
			break;
		case RCR:
			m_rcr = data;
			break;
		case RMSR:
			m_rmsr = data;
			update_rmsr();
			break;
		case TMSR:
			m_tmsr = data;
			update_tmsr();
			break;
		case PTIMER:
			m_ptimer = data;
			break;
		case PMAGIC:
			m_pmagic = data;
			break;

		// read-only
		case PATR0:
		case PATR1:
		case UIPR0:
		case UIPR1:
		case UIPR2:
		case UIPR3:
		case UPORT0:
		case UPORT1:
			break;
	}
}


void w5100_device::update_tmsr()
{
	unsigned offset = 0;
	unsigned value = m_tmsr;

	for (int sn = 0; sn < 4; ++sn, value >>= 2)
	{
		const int size = 1024 << (value & 0x03);

		m_sockets[sn]->set_tx_buffer_size(size);
		m_tx_buffer_size[sn] = size;
		m_tx_buffer_offset[sn] = offset & 0x1fff;
		offset += size;
	}
}

void w5100_device::update_rmsr()
{
	unsigned offset = 0;
	unsigned value = m_rmsr;

	for (int sn = 0; sn < 4; ++sn, value >>= 2)
	{
		const int size = 1024 << (value & 0x03);

		m_sockets[sn]->set_rx_buffer_size(size);
		m_rx_buffer_size[sn] = size;
		m_rx_buffer_offset[sn] = offset & 0x1fff;
		offset += size;
	}
}

// called by a socket after TXBUF_SIZE set.
void w5100_device::update_socket_tx_bufsize()
{
	unsigned offset = 0;
	for (int sn = 0; sn < 4; ++sn)
	{
		const int size = m_sockets[sn]->get_tx_buffer_size();
		m_tx_buffer_size[sn] = size;
		m_tx_buffer_offset[sn] = offset & 0x1fff;
		offset += size;
	}
}

void w5100_device::update_socket_rx_bufsize()
{
	unsigned offset = 0;
	for (int sn = 0; sn < 4; ++sn)
	{
		const int size = m_sockets[sn]->get_rx_buffer_size();
		m_rx_buffer_size[sn] = size;
		m_rx_buffer_offset[sn] = offset & 0x1fff;
		offset += size;
	}
}

// copy data to rx buffer, accounting for wrapping within the buffer window
void w5100_device::copy_to_rx_buffer(int sn, unsigned offset, const uint8_t *data, int length)
{
	const int buffer_offset = m_rx_buffer_offset[sn];
	const int buffer_size = m_rx_buffer_size[sn];
	const int mask = buffer_size - 1;

	const unsigned limit = 0x2000 - buffer_offset;
	uint8_t *buffer_base = m_rx_buffer.get() + buffer_offset;

	if (limit <= 0x2000)
	{
		for (int i = 0; i < length; ++i, ++offset)
		{
			buffer_base[offset & mask] = data[i];
		}

	}
	else
	{
		// wraps around to the start of the buffer.
		for (int i = 0; i < length; ++i, ++offset)
		{
			offset &= mask;
			if (offset < limit) buffer_base[offset] = data[i];
			else m_rx_buffer[offset] = data[i];

		}
	}
}

void w5100_device::copy_from_tx_buffer(int sn, unsigned offset, uint8_t *data, int length) const
{
	const int buffer_offset = m_tx_buffer_offset[sn];
	const int buffer_size = m_tx_buffer_size[sn];
	const int mask = buffer_size - 1;

	const unsigned limit = 0x2000 - buffer_offset;
	const uint8_t *buffer_base = m_tx_buffer.get() + buffer_offset;

	if (limit <= 0x2000)
	{
		for (int i = 0; i < length; ++i, ++offset)
		{
			data[i] = buffer_base[offset & mask];
		}

	}
	else
	{
		// wraps around to the start of the buffer.
		for (int i = 0; i < length; ++i, ++offset)
		{
			offset &= mask;
			if (offset < limit) data[i] = buffer_base[offset];
			else data[i] = m_tx_buffer[offset];
		}
	}
}

void w5100_device::send_complete_cb(int result)
{

	if (!m_frame_queue.empty())
	{
		send(m_frame_queue.data(), m_frame_queue.size());
		m_frame_queue.clear();
		return;
	}

	for (auto socket : m_sockets)
	{
		socket->process_pending();
		if (busy())
			break;
	}
}

void w5100_device::send_or_queue(uint8_t *buffer, int length)
{
	if (busy())
	{
		if (!m_frame_queue.empty())
		{
			LOG("queue full, dropping packet\n");
			return;
		}
		m_frame_queue.assign(buffer, buffer + length);
	}
	else
	{
		send(buffer, length);
	}
}



void w5100_device::recv_cb(uint8_t *buffer, int length)
{
	// bool is_multicast = false;
	// bool is_broadcast = false;

	LOGMASKED(LOG_PACKETS, "Received %d bytes\n", length);

	length -= 4; // strip the FCS
	if (length < 14) return;

	auto eth = parse_eth(buffer, length);


	uint8_t *data = buffer + eth.header_length;
	int data_length = length - eth.header_length;

	if (eth.type == ETHERNET_TYPE_IP)
	{
		if (!verify_ip(data, data_length)) return;

		auto ip = parse_ip(data, data_length);
		data += ip.header_length;
		data_length = ip.total_length - ip.header_length;

		bool is_unicast = ip.dest_ip == m_ip;

		if (ip.proto == IP_PROTOCOL_UDP)
		{
			auto udp = parse_udp(data, data_length);
			data += udp.header_length;
			data_length = udp.udp_length - udp.header_length;
			for (auto socket : m_sockets)
			{
				if (socket->process_udp(eth.src_mac, ip, udp, data, data_length))
					return;
			}
			// macraw?
			if (m_sockets.front()->process_macraw(buffer, length))
				return;

			if (is_unicast)
				send_icmp_unreachable(eth.src_mac, ip, buffer + eth.header_length);
			return;
		}

		if (ip.proto == IP_PROTOCOL_TCP && is_unicast)
		{
			auto tcp = parse_tcp(data, data_length);
			data += tcp.header_length;
			data_length -= tcp.header_length;

			for (auto socket : m_sockets)
			{
				if (socket->process_tcp(eth.src_mac, ip, tcp, data, data_length))
					return;
			}
			// macraw?
			if (m_sockets.front()->process_macraw(buffer, length))
				return;

			m_sockets.front()->tcp_send_reset(eth.src_mac, ip, tcp);
			return;
		}

		// ipraw takes precedence over icmp and igmp
		for (auto socket : m_sockets)
		{
			if (socket->process_ipraw(eth.src_mac, ip, data, data_length))
				return;
		}

		if (ip.proto == IP_PROTOCOL_ICMP && is_unicast)
		{
			unsigned type = data[o_ICMP_TYPE];
			unsigned code = data[o_ICMP_CODE];

			if (type == ICMP_ECHO_REQUEST && (m_mr & MR_PB) == 0)
			{
				send_icmp_reply(eth.src_mac, ip, data, data_length);
			}
			if (type == ICMP_DESTINATION_UNREACHABLE && code == 3)
			{
				process_icmp_unreachable(data + 8, data_length - 8);
			}
		}

		if (ip.proto == IP_PROTOCOL_IGMP && data_length >= 8)
		{
			unsigned type = data[o_IGMP_TYPE];
			uint32_t ip = be_read(data + o_IGMP_GROUP_ADDRESS, 4);

			if (type == IGMP_TYPE_MEMBERSHIP_QUERY)
			{
				for (auto socket : m_sockets)
				{
					socket->process_igmp_query(ip);
				}
			}
		}
	}

	if (eth.type == ETHERNET_TYPE_ARP)
	{
		if (verify_arp(data, data_length))
		{
			switch(be_read(data + o_ARP_OPCODE, 2))
			{
				case ARP_OPCODE_REPLY:
					process_arp_reply(data, data_length);
					break;
				case ARP_OPCODE_REQUEST:
					process_arp_request(data, data_length);
					break;
			}
		}
	}


	m_sockets.front()->process_macraw(buffer, length);
}


/* arp-related */
bool w5100_device::is_broadcast_ip(uint32_t ip, uint8_t *mac) const
{
	if (ip == 0xffffffff || ip == (m_ip | ~m_subnet))
	{
		std::memcpy(mac, ETHERNET_BROADCAST, 6);
		return true;
	}
	else
		return false;
}

/* if ip is outside the subnet, return arp is for the gateway */
uint32_t w5100_device::arp_ip(uint32_t ip) const
{
	if ((ip & m_subnet) != (m_ip & m_subnet) && (ip & ~m_subnet) != 0)
		return m_gateway;
	return ip;
}


void w5100_device::send_arp_request(uint32_t ip)
{
	static const int FRAME_SIZE = 60;
	uint8_t frame[FRAME_SIZE];
	memset(frame, 0, sizeof(frame));

	memcpy(frame + o_ETHERNET_DEST, ETHERNET_BROADCAST, 6);
	memcpy(frame + o_ETHERNET_SRC, m_shar, 6);
	be_write(frame + o_ETHERNET_TYPE, ETHERNET_TYPE_ARP, 2);

	uint8_t *ptr = frame + 14;

	be_write(ptr + o_ARP_HTYPE, ARP_HTYPE_ETHERNET, 2);
	be_write(ptr + o_ARP_PTYPE, ETHERNET_TYPE_IP, 2);
	ptr[o_ARP_HLEN] = 6; // hardware size
	ptr[o_ARP_PLEN] = 4; // protocol size;
	be_write(ptr + o_ARP_OPCODE, ARP_OPCODE_REQUEST, 2);

	std::memcpy(ptr + o_ARP_SHA, m_shar, 6); //sender mac
	be_write(ptr + o_ARP_SPA, m_ip, 4); // sender ip

	std::memset(ptr + o_ARP_THA, 0, 6); // target mac
	be_write(ptr + o_ARP_TPA, ip, 4); // sender ip

	LOGMASKED(LOG_ARP, "Sending ARP request for %s\n", ip_to_string(ip));
	send_or_queue(frame, FRAME_SIZE);
}

void w5100_device::process_arp_request(const uint8_t *arp, int length)
{
	static const int FRAME_SIZE = 60;
	uint8_t frame[FRAME_SIZE];
	memset(frame, 0, sizeof(frame));

	if (m_ip == be_read(arp + o_ARP_SPA, 4))
	{
		// "there is ARP request with same IP address as Source IP address."
		m_ir |= IR_CONFLICT;
		update_ethernet_irq();
		return;
	}

	if (m_ip != be_read(arp + o_ARP_TPA, 4))
		return;

	std::memcpy(frame + o_ETHERNET_DEST, arp + o_ARP_SHA, 6);
	std::memcpy(frame + o_ETHERNET_SRC, m_shar, 6);
	be_write(frame + o_ETHERNET_TYPE, ETHERNET_TYPE_ARP, 2);

	uint8_t *ptr = frame + 14;

	be_write(ptr + o_ARP_HTYPE, ARP_HTYPE_ETHERNET, 2);
	be_write(ptr + o_ARP_PTYPE, ETHERNET_TYPE_IP, 2);
	ptr[o_ARP_HLEN] = 6; // hardware size
	ptr[o_ARP_PLEN] = 4; // protocol size;
	be_write(ptr + o_ARP_OPCODE, ARP_OPCODE_REPLY, 2);
	std::memcpy(ptr + o_ARP_SHA, m_shar, 6); //sender mac
	be_write(ptr + o_ARP_SPA, m_ip, 4); // sender ip
	std::memcpy(ptr + o_ARP_THA, arp + o_ARP_SHA, 10); // dest mac + ip.

	LOGMASKED(LOG_ARP, "Replying to ARP request from %s\n", ip_to_string(be_read(arp + o_ARP_SPA, 4)));
	send_or_queue(frame, FRAME_SIZE);
}

void w5100_device::process_arp_reply(const uint8_t *arp, int length)
{
	if (std::memcmp(m_shar, arp + o_ARP_THA, 6))
		return;

	uint32_t ip = be_read(arp + o_ARP_SPA, 4);
	const uint8_t *mac = arp + o_ARP_SHA;

	LOGMASKED(LOG_ARP, "Received ARP response for %s\n", ip_to_string(ip));

	for (auto socket: m_sockets)
		socket->process_arp_reply(mac, ip);
}

void w5100_device::process_icmp_unreachable(const uint8_t *data, int length)
{
	auto ip = parse_ip(data, length);
	data += ip.header_length;
	length -= ip.header_length;

	if (ip.header_length && ip.proto == IP_PROTOCOL_UDP && length >= 8)
	{
		auto udp = parse_udp(data, length);
		m_uport = udp.dest_port;
		m_uipr = ip.dest_ip;
		m_ir |= IR_UNREACH;
		update_ethernet_irq();

		LOGMASKED(LOG_ICMP, "Received ICMP unreachable for %s\n", ip_to_string(m_uipr, m_uport));
	}


}

void w5100_device::send_icmp_reply(const uint8_t *mac, const wiznet_ip_info &src_ip, const uint8_t *data, int length)
{
	/*
	   hardware bug: if the icmp size is > 119 bytes, all bytes past 119 are (icmp_size - 12) & 0xff
	   hardware bug: icmp reply can be corrupted if second icmp received before the first reply is sent.
	 */

	uint8_t frame[MAX_FRAME_SIZE];
	memset(frame, 0, sizeof(frame));

	wiznet_ip_info ip{};

	ip.proto = IP_PROTOCOL_ICMP;
	ip.dest_ip = src_ip.src_ip;
	ip.fragment = 0x4000;
	ip.ttl = 128;

	uint8_t *icmp = frame + 14 + ip.default_header_length;

	icmp[o_ICMP_TYPE] = ICMP_ECHO_REPLY;
	icmp[o_ICMP_CODE] = 0;
	std::memcpy(icmp + 4, data + 4, length - 4);

	// icmp crc
	uint16_t crc = util::internet_checksum_creator::simple(icmp, length);
	be_write(icmp + o_ICMP_CHECKSUM, crc, 2);

	build_ip_header(frame, mac, ip, length);

	send_or_queue(frame, std::max(60u, length + 14 + ip.default_header_length));

	LOGMASKED(LOG_ICMP, "Sending ICMP reply for %s\n", ip_to_string(src_ip.src_ip));

}

void w5100_device::send_icmp_unreachable(const uint8_t *mac, const wiznet_ip_info &src_ip, const uint8_t *data)
{
	uint8_t frame[MAX_FRAME_SIZE];
	memset(frame, 0, sizeof(frame));

	wiznet_ip_info ip{};

	ip.proto = IP_PROTOCOL_ICMP;
	ip.dest_ip = src_ip.src_ip;
	ip.fragment = 0x4000;
	ip.ttl = 128;

	// payload is original 8 icmp bytes + orginal ip header + original udp header
	int length = 8 + 8 + src_ip.header_length;

	uint8_t *icmp = frame + 14 + ip.default_header_length;
	icmp[o_ICMP_TYPE] = ICMP_DESTINATION_UNREACHABLE;
	icmp[o_ICMP_CODE] = 3; // port unreachable
	memcpy(icmp + 8, data, 8 + src_ip.header_length);

	// icmp crc
	uint16_t crc = util::internet_checksum_creator::simple(icmp, length);
	be_write(icmp + o_ICMP_CHECKSUM, crc, 2);

	build_ip_header(frame, mac, ip, length);

	send_or_queue(frame, std::max(60u, length + 14 + ip.default_header_length));
	LOGMASKED(LOG_ICMP, "Sending ICMP unreachable to %s\n", ip_to_string(src_ip.src_ip));
}

void w5100_device::build_ip_header(uint8_t *frame, const uint8_t *dest_mac, wiznet_ip_info &ip, int data_length)
{
	std::memcpy(frame + o_ETHERNET_DEST, dest_mac, 6);
	std::memcpy(frame + o_ETHERNET_SRC, m_shar, 6);
	be_write(frame + o_ETHERNET_TYPE, ETHERNET_TYPE_IP, 2);

	uint8_t *ptr = frame + 14;

	if (!ip.header_length) ip.header_length = ip.default_header_length;
	ip.src_ip = m_ip;
	ip.total_length = ip.header_length + data_length;

	ptr[o_IP_IHL] = 0x40 | (ip.header_length >> 2);
	ptr[o_IP_TOS] = ip.tos;
	be_write(ptr + o_IP_LENGTH, ip.total_length, 2);
	be_write(ptr + o_IP_IDENTIFICATION, ++m_identification, 2);
	be_write(ptr + o_IP_FLAGS, ip.fragment, 2);
	ptr[o_IP_TTL] = ip.ttl;
	ptr[o_IP_PROTOCOL] = ip.proto;
	be_write(ptr + o_IP_CHECKSUM, 0, 2);
	be_write(ptr + o_IP_SRC_ADDRESS, ip.src_ip ? ip.src_ip : m_ip, 4);
	be_write(ptr + o_IP_DEST_ADDRESS, ip.dest_ip, 4);

	uint16_t crc = util::internet_checksum_creator::simple(ptr, ip.header_length);
	be_write(ptr + o_IP_CHECKSUM, crc, 2);
}

void w5100_device::build_tcp_header(uint8_t *frame, const uint8_t *dest_mac, wiznet_ip_info &ip, wiznet_tcp_info &tcp, int data_length)
{

	ip.proto = IP_PROTOCOL_TCP;
	if (!tcp.header_length) tcp.header_length = tcp.default_header_length;

	build_ip_header(frame, dest_mac, ip, tcp.header_length + data_length);

	uint8_t *ptr = frame + 14 + ip.header_length;

	be_write(ptr + o_TCP_SRC_PORT, tcp.src_port, 2);
	be_write(ptr + o_TCP_DEST_PORT, tcp.dest_port, 2);
	be_write(ptr + o_TCP_SEQ_NUMBER, tcp.sequence_number, 4);
	be_write(ptr + o_TCP_ACK_NUMBER, tcp.ack_number, 4);
	ptr[o_TCP_DATA_OFFSET] = (tcp.header_length >> 2) << 4;
	ptr[o_TCP_FLAGS] = tcp.flags;
	be_write(ptr + o_TCP_WINDOW_SIZE, tcp.window_size, 2);
	be_write(ptr + o_TCP_CHECKSUM, 0, 2);
	be_write(ptr + o_TCP_URGENT, 0, 2);

	// grrrr.... how to handle it?
	#if 0
	if (header_length > 20)
	{
		std::memset(ptr + header_length, 0, header_length - 20);
		if (tcp.option_mss)
		{
			ptr[20] = 2; // mss
			ptr[21] = 4; // length
			be_write(ptr + 22, tcp.option_mss, 2);
		}
	}
	#endif

	auto crc = udp_tcp_checksum(IP_PROTOCOL_TCP, frame + 14, ptr, data_length + tcp.header_length);
	be_write(ptr + o_TCP_CHECKSUM, crc, 2);

	// checksum includes data and pseudo header.
	//uint16_t crc = util::internet_checksum_creator::simple(ptr, header_length);
}

void w5100_device::build_udp_header(uint8_t *frame, const uint8_t *dest_mac, wiznet_ip_info &ip, wiznet_udp_info &udp, int data_length)
{
	ip.proto = IP_PROTOCOL_UDP;

	udp.udp_length = udp.header_length + data_length;

	build_ip_header(frame, dest_mac, ip, udp.udp_length);
	uint8_t *ptr = frame + 14 + ip.header_length;

	be_write(ptr + o_UDP_SRC_PORT, udp.src_port, 2);
	be_write(ptr + o_UDP_DEST_PORT, udp.dest_port, 2);
	be_write(ptr + o_UDP_LENGTH, udp.udp_length, 2);
	be_write(ptr + o_UDP_CHECKSUM, 0, 2);

	auto crc = udp_tcp_checksum(IP_PROTOCOL_UDP, frame + 14, ptr, udp.udp_length);
	if (crc == 0) crc = 0xffff;
	be_write(ptr + o_UDP_CHECKSUM, crc, 2);
}


/* igmp */
void w5100_device::send_igmp_join(const uint8_t *mac, wiznet_ip_info &ip, unsigned version)
{
	static const int FRAME_SIZE = 60;
	uint8_t frame[FRAME_SIZE];
	std::memset(frame, 0, sizeof(frame));

	ip.proto = IP_PROTOCOL_IGMP;
	ip.ttl = 1;
	ip.fragment = 0x4000;
	ip.src_ip = m_ip;
	ip.header_length = ip.default_header_length + 4;

	uint8_t *ptr = frame + 14 + ip.default_header_length;
	// IP Option - Router Alert
	ptr[0] = 0x94;
	ptr[1] = 0x04;
	ptr[2] = 0x00;
	ptr[3] = 0x00;

	build_ip_header(frame, mac, ip, 8);

	ptr += 4;
	// igmp
	ptr[o_IGMP_TYPE] = version == 1 ? IGMP_TYPE_MEMBERSHIP_REPORT_V1 : IGMP_TYPE_MEMBERSHIP_REPORT_V2;
	be_write(ptr + o_IGMP_GROUP_ADDRESS, ip.dest_ip, 4);

	auto crc = util::internet_checksum_creator::simple(ptr, 8);
	be_write(ptr + o_IGMP_CHECKSUM, crc, 2);


	LOGMASKED(LOG_IGMP, "Sending IGMP join to %s\n", ip_to_string(ip.dest_ip));
	send_or_queue(frame, 60);
}
void w5100_device::send_igmp_leave(const uint8_t *mac, wiznet_ip_info &ip, unsigned version)
{

	if (version == 1) return; // v1 doesn't support leave.

	static const int FRAME_SIZE = 60;
	uint8_t frame[FRAME_SIZE];
	std::memset(frame, 0, sizeof(frame));

	uint32_t dest_ip = ip.dest_ip;

	ip.proto = IP_PROTOCOL_IGMP;
	ip.ttl = 1;
	ip.fragment = 0x4000;
	ip.src_ip = m_ip;
	ip.dest_ip = 0xe0000002; // multicast IP
	ip.header_length = ip.default_header_length + 4;

	uint8_t *ptr = frame + 14 + ip.default_header_length;
	// IP Option - Router Alert
	ptr[0] = 0x94;
	ptr[1] = 0x04;
	ptr[2] = 0x00;
	ptr[3] = 0x00;

	build_ip_header(frame, ETHERNET_MULTICAST, ip, 8);

	ptr += 4;
	// igmp
	ptr[o_IGMP_TYPE] = IGMP_TYPE_LEAVE_GROUP;
	be_write(ptr + 4, dest_ip, 4);

	auto crc = util::internet_checksum_creator::simple(ptr, 8);
	be_write(ptr + o_IGMP_CHECKSUM, crc, 2);

	LOGMASKED(LOG_IGMP, "Sending IGMP leave to %s\n", ip_to_string(ip.dest_ip));
	send_or_queue(frame, 60);
}

DEFINE_DEVICE_TYPE(W5100, w5100_device, "w5100", "WIZnet W5100 Ethernet Controller")
