// license:BSD-3-Clause
// copyright-holders: Kelvin Sherlock


/*
	WIZnet W5100 (socket implementation)

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
    x400-x7ff, xb00-xfff are socket registers.
    on the w5100, all undocumented socket registers mirror socket register 3.


 */

#include "emu.h"
#include "w5100_socket.h"
#include "util/endianness.h"

// #define LOG_GENERAL (1U << 0)
#define LOG_COMMAND (1U << 1)
#define LOG_TCP     (1U << 2)
#define LOG_SR      (1U << 3)
#define LOG_SEND    (1U << 4)

#define VERBOSE (LOG_GENERAL|LOG_COMMAND|LOG_TCP|LOG_SR|LOG_SEND)
#include "logmacro.h"


ALLOW_SAVE_TYPE(util::tcp_sequence)


/* Socket Registers */
enum : uint8_t {
	Sn_MR = 0x00,
	Sn_CR,
	Sn_IR,
	Sn_SR,
	Sn_PORT0,
	Sn_PORT1,
	Sn_DHAR0,
	Sn_DHAR1,
	Sn_DHAR2,
	Sn_DHAR3,
	Sn_DHAR4,
	Sn_DHAR5,
	Sn_DIPR0,
	Sn_DIPR1,
	Sn_DIPR2,
	Sn_DIPR3,
	Sn_DPORT0,
	Sn_DPORT1,
	Sn_MSSR0,
	Sn_MSSR1,
	Sn_PROTO,
	Sn_TOS,
	Sn_TTL,
	/* 0x17-0x1f reserved */
	Sn_RX_BUF_SIZE = 0x1e,
	Sn_TX_BUF_SIZE,
	Sn_TX_FSR0 = 0x20,
	Sn_TX_FSR1,
	Sn_TX_RD0,
	Sn_TX_RD1,
	Sn_TX_WR0,
	Sn_TX_WR1,
	Sn_RX_RSR0,
	Sn_RX_RSR1,
	Sn_RX_RD0,
	Sn_RX_RD1,
	Sn_RX_WR0,
	Sn_RX_WR1,
	// 0x2c w5100s IMR
	Sn_FRAGR0 = 0x2d, // also on w5100
	Sn_FRAGR1, // also on w5100
};


/* Socket Mode Register */
enum : uint8_t {
	Sn_MR_MULT = 0x80,
	Sn_MR_MF = 0x40,
	Sn_MR_ND = 0x20,
	Sn_MR_MC = 0x20,

	Sn_MR_CLOSED = 0x00,
	Sn_MR_TCP = 0x01,
	Sn_MR_UDP = 0x02,
	Sn_MR_IPRAW = 0x03,
	Sn_MR_MACRAW = 0x04,
	Sn_MR_PPPoE = 0x05
};


/* Socket Command Register */
enum : uint8_t {
	Sn_CR_OPEN = 0x01,
	Sn_CR_LISTEN = 0x02,
	Sn_CR_CONNECT = 0x04,
	Sn_CR_DISCON = 0x08,
	Sn_CR_CLOSE = 0x10,
	Sn_CR_SEND = 0x20,
	Sn_CR_SEND_MAC = 0x21,
	Sn_CR_SEND_KEEP = 0x22,
	Sn_CR_RECV = 0x40,

	// documented in iolib header for w5100s
	Sn_CR_IGMP_JOIN = 0x23,
	Sn_CR_IGMP_LEAVE = 0x24,

};


/* Socket Status Register */
enum : uint8_t {
	Sn_SR_CLOSED = 0x00,
	Sn_SR_INIT = 0x13,
	Sn_SR_LISTEN = 0x14,
	Sn_SR_SYNSENT = 0x15,
	Sn_SR_SYNRECV = 0x16,
	Sn_SR_ESTABLISHED = 0x17,
	Sn_SR_FIN_WAIT = 0x18,
	Sn_SR_CLOSING = 0x1a,
	Sn_SR_TIME_WAIT = 0x1b,
	Sn_SR_CLOSE_WAIT = 0x1c,
	Sn_SR_LAST_ACK = 0x1d,

	Sn_SR_UDP = 0x22,
	Sn_SR_IPRAW = 0x32,
	Sn_SR_MACRAW = 0x42,
	Sn_SR_PPPOE = 0x5f,

	// n.b. SR_ARP only documented for w5100.
	Sn_SR_ARP = 0x01,

};

/* Socket Interrupt Register */
enum : uint8_t {
	Sn_IR_SEND_OK = 0x10,
	Sn_IR_TIMEOUT = 0x08,
	Sn_IR_RECV = 0x04,
	Sn_IR_DISCON = 0x02,
	Sn_IR_CON = 0x01,
};

/* TCP flags */
enum : uint8_t {
	TCP_FIN = 0x01,
	TCP_SYN = 0x02,
	TCP_RST = 0x04,
	TCP_PSH = 0x08,
	TCP_ACK = 0x10,
	TCP_URG = 0x20,
	TCP_ECE = 0x40,
	TCP_CWR = 0x80,
};

/* timer param */
enum {
	TIMER_ARP = 1,
	TIMER_TCP
};


static const int MAX_FRAME_SIZE = 1514;
static const unsigned MAX_MSS_MACRAW = 1514;
static const unsigned MAX_MSS_TCP = 1460;
static const unsigned MAX_MSS_UDP = 1472;
static const unsigned MAX_MSS_IPRAW = 1480;




[[maybe_unused]] static const char *sr_to_cstring(int sr)
{
	switch(sr)
	{
		case Sn_SR_CLOSED: return "SR_CLOSED";
		case Sn_SR_INIT: return "SR_INIT";
		case Sn_SR_LISTEN: return "SR_LISTEN";
		case Sn_SR_SYNSENT: return "SR_SYNSENT";
		case Sn_SR_SYNRECV: return "SR_SYNRECV";
		case Sn_SR_ESTABLISHED: return "SR_ESTABLISHED";
		case Sn_SR_FIN_WAIT: return "SR_FIN_WAIT";
		case Sn_SR_CLOSING: return "SR_CLOSING";
		case Sn_SR_TIME_WAIT: return "SR_TIME_WAIT";
		case Sn_SR_CLOSE_WAIT: return "SR_CLOSE_WAIT";
		case Sn_SR_LAST_ACK: return "SR_LAST_ACK";
		case Sn_SR_UDP: return "SR_UDP";
		case Sn_SR_IPRAW: return "SR_IPRAW";
		case Sn_SR_MACRAW: return "SR_MACRAW";
		case Sn_SR_PPPOE: return "SR_PPPOE";
		case Sn_SR_ARP: return "SR_ARP";
		default: return "???";
	}
}

[[maybe_unused]] static std::string tcp_flags_to_string(int flags)
{
	std::string rv;

	if (flags & TCP_FIN) rv += "FIN, ";
	if (flags & TCP_SYN) rv += "SYN, ";
	if (flags & TCP_RST) rv += "RST, ";
	if (flags & TCP_PSH) rv += "PSH, ";
	if (flags & TCP_ACK) rv += "ACK, ";
	if (flags & TCP_URG) rv += "URG, ";
	if (flags & TCP_ECE) rv += "ECE, ";
	if (flags & TCP_CWR) rv += "CWR, ";

	if (rv.size()) rv.resize(rv.size() - 2);
	return rv;
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




w5100_socket_device::w5100_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: w5100_socket_device(mconfig, tag, owner, clock, 0)
{
}


w5100_socket_device::w5100_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int socket_number)
	: device_t(mconfig, W5100_SOCKET, tag, owner, clock)
	, m_parent(dynamic_cast<w5100_device *>(owner))
	, m_sn(socket_number)
{
}

void w5100_socket_device::device_start()
{

	m_delayed_ack_timer = timer_alloc(FUNC(w5100_socket_device::delayed_ack_timer), this);
	m_resend_timer = timer_alloc(FUNC(w5100_socket_device::resend_timer), this);

	save_item(NAME(m_mr));
	save_item(NAME(m_ir));
	save_item(NAME(m_sr));
	save_item(NAME(m_port));
	save_item(NAME(m_dhar));
	save_item(NAME(m_dipr));
	save_item(NAME(m_dport));
	save_item(NAME(m_mss));
	save_item(NAME(m_proto));
	save_item(NAME(m_tos));
	save_item(NAME(m_ttl));
	save_item(NAME(m_frag));
	save_item(NAME(m_rx_buf_size));
	save_item(NAME(m_tx_buf_size));
	save_item(NAME(m_tx_rd));
	save_item(NAME(m_tx_wr));
	save_item(NAME(m_rx_rd));
	save_item(NAME(m_rx_wr));

	save_item(NAME(m_active_proto));
	save_item(NAME(m_pending_mss));
	save_item(NAME(m_pending_dipr));
	save_item(NAME(m_pending_dhar));
	save_item(NAME(m_pending_dport));
	save_item(NAME(m_pending_rx_rd));
	save_item(NAME(m_pending_tx_wr));

	save_item(NAME(m_snd_una));
	save_item(NAME(m_snd_nxt));
	save_item(NAME(m_rcv_nxt));
	save_item(NAME(m_irs));
	save_item(NAME(m_iss));
	save_item(NAME(m_snd_wl1));
	save_item(NAME(m_snd_wl2));
	save_item(NAME(m_resend_seq));
	save_item(NAME(m_snd_wnd));

	save_item(NAME(m_current_rcr));
	save_item(NAME(m_current_rtr));
	save_item(NAME(m_arp_valid));
	save_item(NAME(m_send_in_progress));
	save_item(NAME(m_resend_in_progress));
}


TIMER_CALLBACK_MEMBER(w5100_socket_device::delayed_ack_timer)
{
	LOGMASKED(LOG_TCP, "Delayed ack timer\n");
	tcp_send_segment(TCP_ACK, m_snd_nxt, m_rcv_nxt);
}

TIMER_CALLBACK_MEMBER(w5100_socket_device::resend_timer)
{
	// resend for arp query as well as unacked tcp segments.

	// n.b.  an rcr of 0xffff is effectively infinite
	if (++m_current_rcr > m_parent->rcr())
	{
		m_resend_timer->enable(false);
		if (param == TIMER_ARP)
		{
			m_arp_in_progress = false;
			switch (m_active_proto)
			{
				// UDP and IPRAW update tx_rd at this point
				// since it's a failed SEND.
				case Sn_MR_UDP:
					m_tx_rd = m_tx_wr;
					m_sr = Sn_SR_UDP;
					LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
					break;
				case Sn_MR_IPRAW:
					m_tx_rd = m_tx_wr;
					m_sr = Sn_SR_IPRAW;
					LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
					break;
				case Sn_MR_TCP:
					intern_close(0);
					break;
			}
		}
		else if (param == TIMER_TCP)
		{
			intern_close(0);
		}
		m_ir |= Sn_IR_TIMEOUT;
		m_parent->update_ethernet_irq();
		return;
	}

	if (param == TIMER_ARP)
	{
		m_parent->send_arp_request(m_arp_ip);
		arp_update_timer();
	}
	else if (param == TIMER_TCP)
	{
		bool requeue = false;
		switch (m_sr)
		{
			case Sn_SR_SYNSENT:
				tcp_send_segment(TCP_SYN, m_iss, 0);
				requeue = true;
				break;
			case Sn_SR_SYNRECV:
				tcp_send_segment(TCP_ACK | TCP_SYN, m_iss, m_rcv_nxt);
				requeue = true;
				break;

			case Sn_SR_ESTABLISHED:
			case Sn_SR_CLOSE_WAIT:
			case Sn_SR_FIN_WAIT:
				if (m_parent->busy())
					m_resend_in_progress = true;
				else
					tcp_send(true);
				break;

			case Sn_SR_LAST_ACK:
				tcp_send_segment(TCP_FIN | TCP_ACK, m_snd_nxt - 1, m_rcv_nxt);
				requeue = true;
				break;
		}
		if (requeue)
			tcp_update_timer();
	}
}


void w5100_socket_device::device_reset()
{
	m_mr = 0;
	m_ir = 0;
	m_sr = 0;
	m_port = 0;
	memset(m_dhar, 0xff, 6);

	m_dipr = 0;
	m_dport = 0;
	m_mss = 0;
	m_proto = 0;
	m_tos = 0;
	m_ttl = 0x80;
	m_frag = 0x4000;
	m_rx_buf_size = 0x02;
	m_tx_buf_size = 0x02;
	m_tx_rd = 0;
	m_tx_wr = 0;
	m_rx_rd = 0;
	m_rx_wr = 0;

	m_pending_tx_wr = 0;
	m_pending_rx_rd = 0;
	m_pending_mss = 0;
	m_pending_dipr = 0;
	m_pending_dport = 0;
	memset(m_pending_dhar, 0x00, 6); // n.b. doesn't match m_dhar 

	m_snd_una = 0;
	m_snd_nxt = 0;
	m_rcv_nxt = 0;
	m_irs = 0;
	m_iss = 0;
	m_snd_wl1 = 0;
	m_snd_wl2 = 0;
	m_resend_seq = 0;
	m_snd_wnd = 0;

	m_active_proto = 0;
	m_current_rcr = 0;
	m_current_rtr = 0;
	m_arp_valid = false;
	m_send_in_progress = false;
	m_resend_in_progress = false;
	m_arp_in_progress = false;
	m_arp_ip = 0;

}

void w5100_socket_device::write_register(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case Sn_MR:
			m_mr = data;
			if (m_sn == 0 && (data & Sn_MR_MF) == Sn_MR_MF)
				m_parent->set_promisc(false);
			break;

		case Sn_CR:
			command(data);
			break;

		case Sn_IR:
			m_ir &= ~data;
			m_parent->update_ethernet_irq();
			break;

		case Sn_PORT0:
		case Sn_PORT1:
			util::big_endian_cast<uint8_t>(&m_port)[offset - Sn_PORT0] = data;
			break;

		case Sn_PROTO:
			m_proto = data;
			break;
		case Sn_TOS:
			m_tos = data;
			break;
		case Sn_TTL:
			m_ttl = data;
			break;

		 // only documented for w5100s but exists in w5100.
		case Sn_FRAGR0:
		case Sn_FRAGR1:
			util::big_endian_cast<uint8_t>(&m_frag)[offset - Sn_FRAGR0] = data;
			break;

		 // only documented for w5100s but exists in w5100.
		case Sn_RX_BUF_SIZE:
			m_rx_buf_size = data;
			m_parent->update_socket_rx_bufsize();
			break;
		 // only documented for w5100s but exists in w5100.
		case Sn_TX_BUF_SIZE:
			m_tx_buf_size = data;
			m_parent->update_socket_tx_bufsize();
			break;

		// delayed registers
		case Sn_DHAR0:
		case Sn_DHAR1:
		case Sn_DHAR2:
		case Sn_DHAR3:
		case Sn_DHAR4:
		case Sn_DHAR5:
			m_pending_dhar[offset - Sn_DHAR0] = data;
			break;

		case Sn_DIPR0:
		case Sn_DIPR1:
		case Sn_DIPR2:
		case Sn_DIPR3:
			util::big_endian_cast<uint8_t>(&m_pending_dipr)[offset - Sn_DIPR0] = data;
			break;

		case Sn_DPORT0:
		case Sn_DPORT1:
			util::big_endian_cast<uint8_t>(&m_pending_dport)[offset - Sn_DPORT0] = data;
			break;

		case Sn_MSSR0:
		case Sn_MSSR1:
			util::big_endian_cast<uint8_t>(&m_pending_mss)[offset - Sn_MSSR0] = data;
			break;

		case Sn_TX_WR0:
		case Sn_TX_WR1:
			util::big_endian_cast<uint8_t>(&m_pending_tx_wr)[offset - Sn_TX_WR0] = data;
			break;

		case Sn_RX_RD0:
		case Sn_RX_RD1:
			util::big_endian_cast<uint8_t>(&m_pending_rx_rd)[offset - Sn_RX_RD0] = data;
			break;

		// read-only registers
		case Sn_SR:
		case Sn_TX_FSR0:
		case Sn_TX_FSR1:
		case Sn_TX_RD0:
		case Sn_TX_RD1:
		case Sn_RX_RSR0:
		case Sn_RX_RSR1:
		case Sn_RX_WR0:
		case Sn_RX_WR1:
			break;
	}
}


uint8_t w5100_socket_device::read_register(unsigned offset) {

	uint16_t fsr;

	switch(offset)
	{
		case Sn_MR:
			return m_mr;
		case Sn_IR:
			return m_ir;
		case Sn_SR:
			return m_sr;
		case Sn_PORT0:
		case Sn_PORT1:
			return util::big_endian_cast<uint8_t>(&m_port)[offset - Sn_PORT0];
		case Sn_DHAR0:
		case Sn_DHAR1:
		case Sn_DHAR2:
		case Sn_DHAR3:
		case Sn_DHAR4:
		case Sn_DHAR5:
			return m_dhar[offset - Sn_DHAR0];
		case Sn_DIPR0:
		case Sn_DIPR1:
		case Sn_DIPR2:
		case Sn_DIPR3:
			return util::big_endian_cast<uint8_t>(&m_dipr)[offset - Sn_DIPR0];
		case Sn_DPORT0:
		case Sn_DPORT1:
			return util::big_endian_cast<uint8_t>(&m_dport)[offset - Sn_DPORT0];
		case Sn_MSSR0:
		case Sn_MSSR1:
			return util::big_endian_cast<uint8_t>(&m_mss)[offset - Sn_MSSR0];
		case Sn_PROTO:
			return m_proto;
		case Sn_TOS:
			return m_tos;
		case Sn_TTL:
			return m_ttl;
		case Sn_RX_BUF_SIZE:
			return m_rx_buf_size;
		case Sn_TX_BUF_SIZE:
			return m_tx_buf_size;
		case Sn_TX_FSR0:
		case Sn_TX_FSR1:

			fsr = m_tx_rd - m_tx_wr;

			// the current mr protocol controls if snd_una is considered.
			if ((m_mr & 0x0f) == Sn_MR_TCP && m_snd_una != m_snd_nxt)
				fsr = (uint32_t)m_snd_una - m_tx_wr;

			fsr += get_tx_buffer_size();

			return util::big_endian_cast<uint8_t>(&fsr)[offset - Sn_TX_FSR0];

		case Sn_TX_RD0:
		case Sn_TX_RD1:
			return util::big_endian_cast<uint8_t>(&m_tx_rd)[offset - Sn_TX_RD0];
		case Sn_TX_WR0:
		case Sn_TX_WR1:
			return util::big_endian_cast<uint8_t>(&m_tx_wr)[offset - Sn_TX_WR0];
		case Sn_RX_RSR0:
		case Sn_RX_RSR1:
			fsr = m_rx_wr - m_rx_rd;
			return util::big_endian_cast<uint8_t>(&fsr)[offset - Sn_RX_RSR0];

		case Sn_RX_RD0:
		case Sn_RX_RD1:
			return util::big_endian_cast<uint8_t>(&m_rx_rd)[offset - Sn_RX_RD0];
		case Sn_RX_WR0:
		case Sn_RX_WR1:
			return util::big_endian_cast<uint8_t>(&m_rx_wr)[offset - Sn_RX_WR0];
		case Sn_FRAGR0:
		case Sn_FRAGR1:
			return util::big_endian_cast<uint8_t>(&m_frag)[offset - Sn_FRAGR0];

			// read-only
		case Sn_CR:
		default:
			return 0;
	}
}


void w5100_socket_device::command(unsigned command)
{
	switch(command)
	{
		case Sn_CR_OPEN:
			LOGMASKED(LOG_COMMAND, "Command: Open\n");
			command_open();
			break;
		case Sn_CR_LISTEN:
			LOGMASKED(LOG_COMMAND, "Command: Listen %u\n", m_port);
			command_listen();
			break;
		case Sn_CR_CONNECT:
			LOGMASKED(LOG_COMMAND, "Command: Connect %s\n", ip_to_string(m_pending_dipr, m_pending_dport));
			command_connect();
			break;
		case Sn_CR_DISCON:
			LOGMASKED(LOG_COMMAND, "Command: Disconnect\n");
			command_disconnect();
			break;
		case Sn_CR_CLOSE:
			LOGMASKED(LOG_COMMAND, "Command: Close\n");
			command_close();
			break;
		case Sn_CR_RECV:
			LOGMASKED(LOG_COMMAND, "Command: Receive\n");
			command_recv();
			break;
		case Sn_CR_SEND:
			LOGMASKED(LOG_COMMAND, "Command: Send\n");
			command_send();
			break;
		case Sn_CR_SEND_MAC:
			LOGMASKED(LOG_COMMAND, "Command: Send Mac\n");
			command_send_mac();
			break;
		case Sn_CR_SEND_KEEP:
			LOGMASKED(LOG_COMMAND, "Command: Send Keep Alive\n");
			command_send_keep();
			break;
		default:
			LOGMASKED(LOG_COMMAND, "Unknown command (0x%02x)\n", command);
			break;
	}
}

void w5100_socket_device::command_open()
{

	uint16_t max_mss = 0;
	auto proto = m_mr & 0x0f;


	switch (proto)
	{
		case Sn_MR_TCP:
			m_sr = Sn_SR_INIT;
			LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
			max_mss = MAX_MSS_TCP;
			break;
		case Sn_MR_UDP:
			m_sr = Sn_SR_UDP;
			LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
			max_mss = 1472;

			if (m_mr & Sn_MR_MULT)
			{
				// dest har, ip, and port are updated at this point.
				std::memcpy(m_dhar, m_pending_dhar, 6);
				m_dport = m_pending_dport;
				m_dipr = m_pending_dipr;

				// generate an igmp message
				wiznet_ip_info ip;
				memset(&ip, 0, sizeof(ip));
				ip.dest_ip = m_dipr;
				ip.tos = m_tos;
				m_parent->send_igmp_join(m_dhar, ip, m_mr & Sn_MR_MC ? 1 : 2);
			}
			break;
		case Sn_MR_IPRAW:
			m_sr = Sn_SR_IPRAW;
			LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
			max_mss = MAX_MSS_IPRAW;
			break;
		case Sn_MR_MACRAW:
			if (m_sn == 0)
			{
				m_sr = Sn_SR_MACRAW;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				max_mss = MAX_MSS_MACRAW;
				if ((m_mr & Sn_MR_MF) == 0)
					m_parent->set_promisc(true);
			}
			else
				proto = 0;
			break;
		case Sn_MR_CLOSED:
		case Sn_MR_PPPoE:
			// unsupported
		default:
			return;
	}


	// reset timers.
	m_delayed_ack_timer->enable(false);
	m_resend_timer->enable(false);
	m_arp_valid = false;
	m_arp_in_progress = false;
	m_send_in_progress = false;
	m_resend_in_progress = false;

	// reset read/write pointers.
	m_rx_rd = 0;
	m_rx_wr = 0;
	m_tx_rd = 0;
	m_tx_wr = 0;
	m_pending_tx_wr = 0;

	m_snd_una = 0;
	m_snd_nxt = 0;
	m_rcv_nxt = 0;
	m_irs = 0;
	m_iss = 0;
	m_snd_wl1 = 0;
	m_snd_wl2 = 0;
	m_resend_seq = 0;
	m_snd_wnd = 0;

	m_active_proto = proto;
	m_mss = m_pending_mss;
	if (m_mss == 0 || m_mss > max_mss)
		m_mss = max_mss;
}

void w5100_socket_device::command_close()
{
	if (m_sr == Sn_SR_CLOSED) return;
	intern_close(0);
}


void w5100_socket_device::intern_close(unsigned irqs)
{
	if (m_sr == Sn_SR_UDP && (m_mr & Sn_MR_MULT))
	{
		wiznet_ip_info ip;
		memset(&ip, 0, sizeof(ip));
		ip.dest_ip = m_dipr;
		ip.tos = m_tos;
		m_parent->send_igmp_leave(m_dhar, ip, m_mr & Sn_MR_MC ? 1 : 2);
	}

	// reset timers
	m_delayed_ack_timer->enable(false);
	m_resend_timer->enable(false);

	m_sr = Sn_SR_CLOSED;
	LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
	m_active_proto = 0;

	if (irqs)
	{
		m_ir |= irqs;
		m_parent->update_ethernet_irq();
	}
}

void w5100_socket_device::command_listen()
{
	if (m_sr != Sn_SR_INIT) return;

	m_sr = Sn_SR_LISTEN;
	LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
}

void w5100_socket_device::command_connect()
{
	if (m_sr != Sn_SR_INIT) return;

	if (!before_send()) return;
	intern_connect();
}

void w5100_socket_device::intern_connect()
{
	m_iss = tcp_generate_iss();
	m_snd_una = m_iss;
	m_snd_nxt = m_iss + 1;
	m_tx_rd = (uint32_t)m_snd_nxt;

	tcp_send_segment(TCP_SYN, m_iss, 0);
	m_sr = Sn_SR_SYNSENT;
	LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
	tcp_start_timer();
}

void w5100_socket_device::command_disconnect()
{
	switch(m_sr)
	{
	case Sn_SR_ESTABLISHED:
	case Sn_SR_CLOSE_WAIT:
		// if a send is in progress, it will handle the fin.
		// otherwise, send it now.
		if (!m_send_in_progress)
		{
			tcp_send_segment(TCP_FIN | TCP_ACK, m_snd_nxt, m_rcv_nxt);
			++m_snd_nxt;
			tcp_start_timer();
		}
		m_sr++;
		break;
	}
}

void w5100_socket_device::command_send_keep()
{
	if (m_sr != Sn_SR_ESTABLISHED && m_sr != Sn_SR_CLOSE_WAIT)
		return;

	// only valid if data sent
	if (m_snd_nxt == m_iss + 1)
		return;

	// and all pending data has been sent.
	if (m_snd_nxt != m_snd_una)
		return;

	--m_snd_una;
	// initiate re-send.
	tcp_start_timer();
}

void w5100_socket_device::command_recv()
{

	m_rx_rd = m_pending_rx_rd;

	uint16_t size = m_rx_wr - m_rx_rd;

	if (m_sr == Sn_SR_ESTABLISHED || m_sr == Sn_SR_CLOSE_WAIT)
	{
		bool send_ack = false;
		if ((m_mr & Sn_MR_ND) || recv_window_size() < m_mss)
			send_ack = true;

		if (send_ack)
		{
			m_delayed_ack_timer->enable(false);
			tcp_send_segment(TCP_ACK, m_snd_nxt, m_rcv_nxt);
		}
	}

	if (size)
	{
		// re-trigger irq if data is still pending.
		m_ir |= Sn_IR_RECV;
		m_parent->update_ethernet_irq();
	}
}


void w5100_socket_device::command_send()
{
	switch (m_sr)
	{
		case Sn_SR_UDP:
		case Sn_SR_IPRAW:
		case Sn_SR_MACRAW:
		case Sn_SR_ESTABLISHED:
		case Sn_SR_CLOSE_WAIT:
			break;
		default:
			return;
	}

	m_tx_wr = m_pending_tx_wr;

	// UDP or IPRAW may been an arp
	if (m_sr == Sn_SR_UDP || m_sr == Sn_SR_IPRAW)
	{
		if (!before_send()) return;
	}
	intern_send();
}

void w5100_socket_device::command_send_mac()
{
	if (m_sr != Sn_SR_UDP && m_sr != Sn_SR_IPRAW)
		return;

	m_tx_wr = m_pending_tx_wr;

	// update the registers.
	std::memcpy(m_dhar, m_pending_dhar, 6);
	m_dipr = m_pending_dipr;
	m_dport = m_pending_dport;
	m_arp_valid = true;

	intern_send();
}

static unsigned standard_header_size(unsigned proto)
{
	switch(proto)
	{
		case Sn_MR_TCP:
			return 14 + 20 + 20;
		case Sn_MR_UDP:
			return 14 + 20 + 8;
		case Sn_MR_IPRAW:
			return 14 + 20;
		default:
			return 0;
	}
}

void w5100_socket_device::intern_send()
{
	uint8_t frame[MAX_FRAME_SIZE];

	if (m_parent->busy())
	{
		m_send_in_progress = true;
		return;
	}

	if (m_sr == Sn_SR_ESTABLISHED || m_sr == Sn_SR_CLOSE_WAIT)
	{
		tcp_send(false);
		return;
	}

	const unsigned size = (m_tx_wr - m_tx_rd) & 0xffff;

	const unsigned header_size = standard_header_size(m_active_proto);
	memset(frame, 0, header_size);

	unsigned msize = std::min(size, (unsigned)m_mss);
	m_parent->copy_from_tx_buffer(m_sn, m_tx_rd, frame + header_size, msize);


	if (m_sr == Sn_SR_UDP)
	{
		// UDP is split into multiple packets if > mss
		// w5100 locks ip on 0-size; w5100s sends a 0-sized UDP packet.

		auto ip = socket_ip_info();
		auto udp = socket_udp_info();

		m_parent->build_udp_header(frame, m_dhar, ip, udp, msize);

		m_tx_rd += msize;
		m_parent->send(frame, msize + header_size);
		LOGMASKED(LOG_SEND, "Sending %u bytes to %s (UDP)\n", msize + header_size, ip_to_string(m_dipr, m_dport));
		if (size == msize)
		{
			m_ir |= Sn_IR_SEND_OK;
			m_parent->update_ethernet_irq();
		}
		else
			m_send_in_progress = true;
	}

	if (m_sr == Sn_SR_MACRAW || m_sr == Sn_SR_IPRAW)
	{
		// packet is dropped if > mss.
		// but send ok irq is still triggered.
		if (size == msize)
		{
			if (m_sr == Sn_SR_IPRAW)
			{
				auto ip = socket_ip_info();
				m_parent->build_ip_header(frame, m_dhar, ip, msize);
				LOGMASKED(LOG_SEND, "Sending %u bytes to %s (IPRAW)\n", msize + header_size, ip_to_string(m_dipr));
			}
			else
			{
				LOGMASKED(LOG_SEND, "Sending %u bytes (MACRAW)\n", msize + header_size);
			}

			m_parent->send(frame, msize + header_size);

		}
		m_tx_rd = m_tx_wr;
		m_ir |= Sn_IR_SEND_OK;
		m_parent->update_ethernet_irq();
	}
}

/*
 before connect/send processing.
 updates port, ip, dhar.
 returns true if dhar is already valid.
 returns false (and initiates arp request) otherwise
*/

bool w5100_socket_device::before_send()
{
	if (m_dipr != m_pending_dipr) m_arp_valid = false;
	m_dipr = m_pending_dipr;
	m_dport = m_pending_dport;

	if (m_sr == Sn_SR_UDP && (m_mr & Sn_MR_MULT))
	{
		// use provided dhar for multicast
		std::memcpy(m_dhar, m_pending_dhar, 6);
		m_arp_valid = true;
		return true;
	}

	if (m_sr == Sn_SR_UDP || m_sr == Sn_SR_IPRAW)
	{
		if (m_parent->is_broadcast_ip(m_dipr, m_dhar))
		{
			m_arp_valid = true;
			return true;
		}
	}

	if (m_arp_valid)
		return true;

	// w5100s doesn't have an ARP status.
	m_sr = Sn_SR_ARP;
	LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));

	m_arp_ip = m_parent->arp_ip(m_dipr);
	m_parent->send_arp_request(m_arp_ip);

	m_arp_in_progress = true;
	arp_start_timer();
	return false;
}

void w5100_socket_device::arp_start_timer()
{
	m_current_rcr = 0;
	m_current_rtr = m_parent->rtr();
	attotime tm = attotime::from_usec(m_current_rtr * 100);
	m_resend_timer->adjust(tm, TIMER_ARP, tm);
}

void w5100_socket_device::arp_update_timer()
{
	// if parent rtr has changed, reschedule.

	const auto rtr = m_parent->rtr();
	if (rtr != m_current_rtr)
	{
		m_current_rtr = rtr;
		attotime tm = attotime::from_usec(m_current_rtr * 100);
		m_resend_timer->adjust(tm, TIMER_ARP, tm);
	}
}

// tcp timer uses a back-off (and takes time to resend) so it's one-shot
void w5100_socket_device::tcp_start_timer()
{
	m_current_rcr = 0;
	m_current_rtr = m_parent->rtr();
	attotime tm = attotime::from_usec(m_current_rtr * 100);
	m_resend_timer->adjust(tm, TIMER_TCP);
	m_resend_seq = m_snd_una;
}

void w5100_socket_device::tcp_update_timer()
{
	auto rtr = m_parent->rtr();

	const auto max_shift = count_leading_zeros_32(rtr) - 16;

	rtr <<= std::min((int)m_current_rcr, max_shift);

	attotime tm = attotime::from_usec(rtr * 100);
	m_resend_timer->adjust(tm, TIMER_TCP);
	m_resend_seq = m_snd_una;
}

void w5100_socket_device::process_arp_reply(const uint8_t *mac, uint32_t ip)
{
	if (m_arp_in_progress && m_arp_ip == ip)
	{
		std::memcpy(m_dhar, mac, 6);
		m_arp_valid = true;
		m_arp_in_progress = false;
		m_resend_timer->enable(false);

		switch(m_active_proto)
		{
			case Sn_MR_IPRAW:
				m_sr = Sn_SR_IPRAW;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				intern_send();
				break;
			case Sn_MR_UDP:
				m_sr = Sn_SR_UDP;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				intern_send();
				break;
			case Sn_MR_TCP:
				m_sr = Sn_SR_INIT;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				intern_connect();
				break;
		}
	}
}

bool w5100_socket_device::intern_recv(const uint8_t *header, int header_size, const uint8_t *payload, int payload_size)
{

	const int avail = recv_window_size();


	if (payload_size + header_size == 0)
		return false;

	if (payload_size + header_size > avail)
		return false;

	if (header_size)
	{
		m_parent->copy_to_rx_buffer(m_sn, m_rx_wr, header, header_size);
		m_rx_wr += header_size;
	}
	if (payload_size)
	{
		m_parent->copy_to_rx_buffer(m_sn, m_rx_wr, payload, payload_size);
		m_rx_wr += payload_size;
	}

	if (m_active_proto == Sn_MR_TCP)
	{
		m_rcv_nxt += payload_size;
	}

	m_ir |= Sn_IR_RECV;
	m_parent->update_ethernet_irq();

	return true;
}

// callback from parent from send_complete_cb to space out multi-frame sends.
void w5100_socket_device::process_pending()
{
	if (m_send_in_progress)
	{
		m_send_in_progress = false;
		intern_send();
		return;
	}

	if (m_resend_in_progress)
	{
		m_resend_in_progress = false;
		tcp_send(true);
		return;
	}
}

bool w5100_socket_device::process_macraw(const uint8_t *buffer, int length)
{
	if (m_sr != Sn_SR_MACRAW) return false;

	// { uint16_t length_with_header }
	uint8_t header[2];

	header[0] = (length + 2) >> 8;
	header[1] = length + 2;

	intern_recv(header, 2, buffer, length);
	return true;
}

// only need remote_ip, proto from ip_header....
bool w5100_socket_device::process_ipraw(const uint8_t *mac, const wiznet_ip_info &ip, const uint8_t *data, int data_length)
{
	if (m_active_proto != Sn_MR_IPRAW) return false;

	if (ip.proto != m_proto) return false;

	// { uint32_t ip; uint16_t length }
	uint8_t header[6];
	header[0] = ip.src_ip >> 24;
	header[1] = ip.src_ip >> 16;
	header[2] = ip.src_ip >> 8;
	header[3] = ip.src_ip;
	header[4] = data_length >> 8;
	header[5] = data_length;

	intern_recv(header, 6, data, data_length);
	return true;
}

bool w5100_socket_device::process_udp(const uint8_t *mac, const wiznet_ip_info &ip, const wiznet_udp_info &udp, const uint8_t *data, int data_length)
{
	if (m_active_proto != Sn_MR_UDP) return false;

	const bool is_multicast = (ip.src_ip & 0xf00000000) == 0xe00000000;

	if (is_multicast && (m_mr & Sn_MR_MULT) == 0) return false;

	if (m_port != udp.dest_port) return false;

	// {uint32_t ip; uint16_t port; uint16_t length }
	uint8_t header[8];
	header[0] = ip.src_ip >> 24;
	header[1] = ip.src_ip >> 16;
	header[2] = ip.src_ip >> 8;
	header[3] = ip.src_ip;
	header[4] = udp.src_port >> 8;
	header[5] = udp.src_port;
	header[6] = data_length >> 8;
	header[7] = data_length;

	intern_recv(header, 8, data, data_length);

	return true;
}

// only need remote_ip from ip_header....
bool w5100_socket_device::process_tcp(const uint8_t *mac, const wiznet_ip_info &ip, const wiznet_tcp_info &tcp, const uint8_t *seg_data, int seg_len)
{
	if (m_active_proto != Sn_MR_TCP) return false;

	if (tcp.dest_port != m_port) return false;

	unsigned flags = tcp.flags;
	util::tcp_sequence seg_seq = tcp.sequence_number;

	if (m_sr == Sn_SR_LISTEN)
	{

		// 3.10.7.2 pp 60
		if (flags & TCP_RST) return false; // parent will RST
		if (flags & TCP_ACK) return false; // parent will RST

		if (flags & TCP_SYN)
		{
			// load the mss
			if (tcp.option_mss && tcp.option_mss <= MAX_MSS_TCP)
				m_mss = tcp.option_mss;

			m_rcv_nxt = seg_seq + 1;
			m_irs = seg_seq;
			m_iss = tcp_generate_iss();

			m_snd_nxt = m_iss + 1;
			m_snd_una = m_iss;

			m_tx_rd = (uint32_t)m_snd_nxt;
			m_dport = tcp.src_port;
			m_dipr = ip.src_ip;
			std::memcpy(m_dhar, mac, 6);

			//  data in the syn segment is supported.
			if (seg_len)
			{
				intern_recv(nullptr, 0, seg_data, seg_len);
			}

			tcp_send_segment(TCP_SYN|TCP_ACK, m_iss, m_rcv_nxt);
			m_sr = Sn_SR_SYNRECV;
			LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
			tcp_start_timer(); // activate the resend timer
			return true;
		}
		return false; // parent will RST;
	}


	if (tcp.src_port != m_dport || ip.src_ip != m_dipr) return false;

	tcp_process_segment(tcp, seg_data, seg_len);
	return true;
}

void w5100_socket_device::tcp_process_segment(const wiznet_tcp_info &tcp, const uint8_t *seg_data, int seg_len)
{
	unsigned flags = tcp.flags;
	util::tcp_sequence seg_seq = tcp.sequence_number;
	util::tcp_sequence seg_ack = tcp.ack_number;


	if (m_sr == Sn_SR_SYNSENT)
	{
		// 3.10.7.3 pp 61-62

		if (flags & TCP_ACK)
		{
			// If SEG.ACK =< ISS or SEG.ACK > SND.NXT, send a reset (unless
            // the RST bit is set, if so drop the segment and return)
            // and discard the segment.  Return.
            //
			// If SND.UNA < SEG.ACK =< SND.NXT, then the ACK is acceptable.
            // Some deployed TCP code has used the check SEG.ACK == SND.NXT
            // (using "==" rather than "=<"), but this is not appropriate
            // when the stack is capable of sending data on the SYN because
            // the TCP peer may not accept and acknowledge all of the data
            // on the SYN.

			// n.b. snd_una = iss, snd_nxt = iss +1

			if (seg_ack <= m_iss || seg_ack > m_snd_nxt)
			{
				if (flags & TCP_RST) return;
				tcp_send_segment(TCP_RST, seg_ack, 0);
				return;
			}
		}
		if (flags & TCP_RST)
		{
			if (flags & TCP_ACK)
			{
				intern_close(Sn_IR_DISCON);
			}
			return;
		}

		if (flags & TCP_SYN)
		{
			m_rcv_nxt = seg_seq + 1;
			m_irs = seg_seq;
			if (flags & TCP_ACK)
				m_snd_una = seg_ack;

			// If SND.UNA > ISS (our SYN has been ACKed)
			if (m_snd_una > m_iss)
			{
				m_snd_wnd = tcp.window_size;
				m_snd_wl1 = seg_seq;
				m_snd_wl2 = seg_ack;

				m_tx_wr = (uint32_t)m_snd_nxt;

				if (seg_len)
					intern_recv(seg_data, seg_len);

				tcp_send_segment(TCP_ACK, m_snd_nxt, m_rcv_nxt);
				m_sr = Sn_SR_ESTABLISHED;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				m_ir |= Sn_IR_CON;
				m_parent->update_ethernet_irq();

				m_resend_timer->enable(false);
				// w5100s keep-alive timer would kick off here
			}
			else
			{
				// n.b. - actual hardware will transition from SYN_SENT to SYN_RECV but will not establish
				// a connection after receiving an ack.
				tcp_send_segment(TCP_SYN|TCP_ACK, m_iss, m_rcv_nxt);
				m_sr = Sn_SR_SYNRECV;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				tcp_start_timer();
			}
		}
		return;
	}


	// 3.10.74 pp 63

	if (flags & TCP_RST)
	{
		// pp 64-65

		// rst ignored unless seg_seq == rcv_nxt or 0
		if (seg_seq != 0 && seg_seq != m_rcv_nxt)
			return;

		intern_close(Sn_IR_DISCON);
		return;
	}



	// out-of order segments are dropped.
	if (seg_seq != m_rcv_nxt)
	{
		m_delayed_ack_timer->enable(false);
		tcp_send_segment(TCP_ACK, m_snd_nxt, m_rcv_nxt);
		return;
	}



	if (flags & TCP_SYN)
	{
		// pp 66
		// n.b. - w5100 goes to a closed state instead of a listen state.
		tcp_send_segment(TCP_RST, m_snd_nxt, 0);
		intern_close(Sn_IR_DISCON);
		return;
	}

	if (!(flags & TCP_ACK)) return;


	bool update_resend = false;
	switch(m_sr)
	{
		case Sn_SR_SYNRECV:
			// pp 67
			if (m_snd_una < seg_ack && seg_ack <= m_snd_nxt)
			{
				m_snd_wnd = tcp.window_size;
				m_snd_wl1 = seg_seq;
				m_snd_wl2 = seg_ack;

				m_snd_una = seg_ack;

				m_sr = Sn_SR_ESTABLISHED;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				m_ir |= Sn_IR_CON;
				m_parent->update_ethernet_irq();
			}
			else
			{
				tcp_send_segment(TCP_RST, seg_ack, 0);
				return;
			}
			break;

		case Sn_SR_ESTABLISHED:
		case Sn_SR_FIN_WAIT:
		case Sn_SR_CLOSE_WAIT:
		case Sn_SR_CLOSING:

			// pp 67

			if (seg_ack > m_snd_nxt)
			{
				// ack for something not sent. drop segment and return.
				m_delayed_ack_timer->enable(false);
				tcp_send_segment(TCP_ACK, m_snd_nxt, m_rcv_nxt);
				return;
			}

			if (m_snd_una < seg_ack && seg_ack <= m_snd_nxt)
			{
				m_snd_una = seg_ack;

				// if a re-send timer is in effect, re-set the retry counter.
				update_resend = true;

				if (m_snd_una == m_snd_nxt && m_sr == Sn_SR_CLOSING)
				{
					m_sr = Sn_SR_TIME_WAIT;
					LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
					update_resend = false;
				}
			}


			// If SND.UNA =< SEG.ACK =< SND.NXT, the send window should be updated.
			// If (SND.WL1 < SEG.SEQ or (SND.WL1 = SEG.SEQ and SND.WL2 =< SEG.ACK)),
			// set SND.WND <- SEG.WND, set SND.WL1 <- SEG.SEQ, and set SND.WL2 <- SEG.ACK.

			if (m_snd_una <= seg_ack && seg_ack <= m_snd_nxt)
			{
				if (m_snd_wl1 < seg_seq || (m_snd_wl1 == seg_seq && m_snd_wl2 <= seg_ack))
				{
					if (tcp.window_size > m_snd_wnd && m_snd_wnd < m_mss) update_resend = true;

					m_snd_wnd = tcp.window_size;
					m_snd_wl1 = seg_seq;
					m_snd_wl2 = seg_ack;
				}
			}

			break;

		case Sn_SR_LAST_ACK:
			// pp 68
			if (seg_ack == m_snd_nxt)
			{
				// if FIN acknowledged, go to closed state.
				m_snd_una = seg_ack;
				intern_close(Sn_IR_DISCON);
				return;
			}
			break;

		case Sn_SR_TIME_WAIT:
			break;

		default:
			break;
	}

	if (m_snd_una == m_snd_nxt)
	{
		// all data acknowledged, kill the re-send timer.
		m_resend_timer->enable(false);
		m_resend_in_progress = false;
	}
	else if (update_resend)
	{
		// data acked or window updated so re-trigger the resend timer.
		tcp_start_timer();
	}


	// segment text
	int tcp_flags = 0;
	if (seg_len)
	{
		// pp 68
		switch(m_sr)
		{
			case Sn_SR_ESTABLISHED:
			case Sn_SR_FIN_WAIT:

				if (!intern_recv(seg_data, seg_len))
				{
					// no partial stores
					m_delayed_ack_timer->enable(false);
					tcp_send_segment(TCP_ACK, m_snd_nxt, m_rcv_nxt);
					return;
				}
				tcp_flags |= TCP_ACK;
				break;

			case Sn_SR_CLOSE_WAIT:
			case Sn_SR_CLOSING:
			case Sn_SR_LAST_ACK:
			case Sn_SR_TIME_WAIT:
				seg_len = 0;
				break;
			default:
				break;
		}

	}

	if (flags & TCP_FIN)
	{
		// pp 69
		switch(m_sr)
		{
			case Sn_SR_SYNRECV:
			case Sn_SR_ESTABLISHED:

				++m_rcv_nxt;
				tcp_flags |= TCP_ACK;

				m_sr = Sn_SR_CLOSE_WAIT;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				m_ir |= Sn_IR_DISCON;
				m_parent->update_ethernet_irq();
				break;

			case Sn_SR_FIN_WAIT:
				// tcp specifies SR_CLOSING state, actual hardware goes to a LAST ACK state.
				if (m_snd_una == m_snd_nxt)
				{
					// fin wait 2
					m_sr = Sn_SR_TIME_WAIT;
				}
				else
				{
					// fin wait 1
					m_sr = Sn_SR_LAST_ACK;
				}
				tcp_flags |= TCP_ACK;
				LOGMASKED(LOG_SR, "Socket -> %s\n", sr_to_cstring(m_sr));
				break;

			case Sn_SR_CLOSE_WAIT:
			case Sn_SR_CLOSING:
			case Sn_SR_LAST_ACK:
				break;
			case Sn_SR_TIME_WAIT:
				break;

			default: break;
		}
	}

	// delayed ack handling:
	// if this is a data segment and ND == 0, wait RTR to ack
	// exceptions: if delay timer already active, cancel and send immediate ack.
	// exception: if connection is closing, don't delay (handled above since seg_len == 0)

	if (seg_len && (m_mr & Sn_MR_ND) == 0 && !m_delayed_ack_timer->enabled())
	{
		auto rtr = m_parent->rtr();
		attotime tm = attotime::from_usec(rtr * 100);
		m_delayed_ack_timer->reset(tm);
		return;
	}

	if (tcp_flags)
	{
		m_delayed_ack_timer->enable(false);
		tcp_send_segment(tcp_flags, m_snd_nxt, m_rcv_nxt);
	}

	if (m_sr == Sn_SR_TIME_WAIT)
	{
		// There is no 2msl timer; close immediately.
		intern_close(Sn_IR_DISCON);
	}
}

void w5100_socket_device::tcp_send(bool retransmit)
{
	uint8_t frame[MAX_FRAME_SIZE];

	uint16_t read_ptr = retransmit ? (uint32_t)m_resend_seq : m_tx_rd;
	uint16_t write_ptr = m_tx_wr;

	const int header_size = 54;

    //                    tx_rd .... tx_wr
	// | ................|...............|
	// snd_una ... snd_nxt

	auto seq = retransmit ? m_resend_seq : m_snd_nxt;

	const int total_size = (write_ptr - (uint32_t)seq) & 0xffff;

	bool fin = false;
	bool zwp = false;
	if (m_sr == Sn_SR_FIN_WAIT || m_sr == Sn_SR_TIME_WAIT)
		fin = true;

	if (total_size == 0 && retransmit && !fin)
	{
		// shouldn't happen but just in case...
		if (m_snd_una != m_snd_nxt)
			tcp_update_timer();
		return;
	}

	// account for snd_wnd
	int window_size = m_snd_una + m_snd_wnd - seq;

	if (window_size <= 0)
	{
		window_size = 1; // 1-byte for zero-window probe.
		zwp = true;
	}

	int msize = std::min({total_size, window_size, (int)m_mss});

	// w5100 always includes a push.
	int flags = TCP_ACK | TCP_PSH;


	// hardware bug: FIN will be sent with all segments when the socket disconnects while data is still pending.
	if (fin && total_size == msize)
	{
		flags |= TCP_FIN;
		flags &= ~TCP_PSH;
	}

	auto ip = socket_ip_info();
	auto tcp = socket_tcp_info(flags, seq, m_rcv_nxt);

	m_parent->copy_from_tx_buffer(m_sn, read_ptr, frame + header_size, msize);
	m_parent->build_tcp_header(frame, m_dhar, ip, tcp, msize);

	LOGMASKED(LOG_TCP, "%s %u/%u bytes (window = %u)\n", retransmit ? "Resending" : "Sending", msize, total_size, window_size);


	m_parent->send(frame, msize + header_size);
	LOGMASKED(LOG_SEND, "Sending %u bytes to %s (TCP)\n", msize + header_size, ip_to_string(m_dipr, m_dport));

	if (retransmit)
	{
		if (!zwp)
			m_resend_seq += msize;

		if (msize == total_size || msize == window_size)
		{
			tcp_update_timer();
		}
		else
		{
			m_resend_in_progress = true;
		}
	}
	else
	{
		if (msize == total_size || msize == window_size || zwp)
		{
			m_snd_nxt += total_size;
			if (fin) ++m_snd_nxt;

			m_tx_rd = m_tx_wr;
			m_ir |= Sn_IR_SEND_OK;
			m_parent->update_ethernet_irq();

			// don't re-send if this was an empty segment.
			if (m_snd_una != m_snd_nxt)
				tcp_start_timer();
		}
		else
		{
			m_snd_nxt += msize;
			m_send_in_progress = true;
		}
	}


}

void w5100_socket_device::tcp_send_segment(unsigned flags, util::tcp_sequence seq, util::tcp_sequence ack)
{
	static const int FRAME_SIZE = 60;
	uint8_t frame[FRAME_SIZE];
	std::memset(frame, 0, sizeof(frame));

	auto ip = socket_ip_info();
	auto tcp = socket_tcp_info(flags, seq, ack);

	tcp.header_length = tcp.default_header_length;
	if (flags & TCP_SYN)
	{
		// include mss option
		tcp.header_length += 4;
		frame[14 + 20 + 20] = 2; // mss option
		frame[14 + 20 + 21] = 4; // length = 4
		frame[14 + 20 + 22] = m_mss >> 8;
		frame[14 + 20 + 23] = m_mss & 0xff;
	}

	m_parent->build_tcp_header(frame, m_dhar, ip, tcp, 0);
	m_parent->send_or_queue(frame, FRAME_SIZE);
}

// called from parent to reset a tcp segment that wasn't handled.
// this particular socket may not be tcp or even open.
void w5100_socket_device::tcp_send_reset(const uint8_t *mac, const wiznet_ip_info &src_ip, const wiznet_tcp_info &src_tcp)
{
	static const int FRAME_SIZE = 60;
	uint8_t frame[FRAME_SIZE];
	std::memset(frame, 0, sizeof(frame));

	wiznet_ip_info ip;
	wiznet_tcp_info tcp;

	std::memset(&ip, 0, sizeof(ip));
	std::memset(&tcp, 0, sizeof(tcp));

	ip.dest_ip = src_ip.src_ip;
	ip.ttl = 0x80;
	ip.fragment = 0x4000;
	ip.header_length = ip.default_header_length;
	tcp.header_length = tcp.default_header_length;
	tcp.src_port = src_tcp.dest_port;
	tcp.dest_port = src_tcp.src_port;
	tcp.flags = TCP_RST;

	// w5100 has a bug where the sequence #/ack # is not sent
	// (which means the RST doesn't actually work)
	// this is fixed on the W5100s.

	m_parent->build_tcp_header(frame, mac, ip, tcp, 0);
	m_parent->send_or_queue(frame, FRAME_SIZE);
}

uint32_t w5100_socket_device::tcp_generate_iss() const
{
	// W5100 uses the current cycle count

	return attotime_to_clocks(machine().time());
}

uint16_t w5100_socket_device::recv_window_size() const
{
	return get_rx_buffer_size() - (m_rx_wr - m_rx_rd);
}


void w5100_socket_device::process_igmp_query(uint32_t ip_address)
{
	if (m_active_proto != Sn_MR_UDP) return;
	if ((m_mr & Sn_MR_MULT) == 0) return;
	if (ip_address != 0 && ip_address != m_dipr) return;

	wiznet_ip_info ip;
	std::memset(&ip, 0, sizeof(ip));
	ip.tos = m_tos;
	ip.dest_ip = m_dipr;
	// all other fields handled by parent

	m_parent->send_igmp_join(m_dhar, ip, m_mr & Sn_MR_MC ? 0 : 1);
}



// buffer size management.


// called from parent when tmsr is set. value will be 1k, 2k, 4k, or 8k.
void w5100_socket_device::set_tx_buffer_size(unsigned value)
{
	m_tx_buf_size = value >> 10;
}

unsigned w5100_socket_device::get_tx_buffer_size() const
{
	switch (m_tx_buf_size)
	{
	case 0:
	case 1:
	case 2:
	case 4:
		return m_tx_buf_size << 10;
	default:
		// w5100s invalid value size is 1.
		return 0x2000;
	}
}

void w5100_socket_device::set_rx_buffer_size(unsigned value)
{
	m_rx_buf_size = value >> 10;
}

unsigned w5100_socket_device::get_rx_buffer_size() const
{
	switch (m_rx_buf_size)
	{
	case 0:
	case 1:
	case 2:
	case 4:
		return m_rx_buf_size << 10;
	default:
		// w5100s invalid value size is 1.
		return 0x2000;
	}
}


wiznet_ip_info w5100_socket_device::socket_ip_info() const
{
	wiznet_ip_info ip;
	std::memset(&ip, 0, sizeof(ip));

	switch (m_active_proto)
	{
		case Sn_MR_UDP:
			ip.proto = 17;
			break;
		case Sn_MR_TCP:
			ip.proto = 6;
			break;
		default:
			ip.proto = m_proto;
	}
	ip.ttl = m_ttl;
	ip.tos = m_tos;
	ip.fragment = m_frag;
	ip.dest_ip = m_dipr;
	return ip;
}

wiznet_tcp_info w5100_socket_device::socket_tcp_info(unsigned flags, util::tcp_sequence seq, util::tcp_sequence ack) const
{
	wiznet_tcp_info tcp;
	std::memset(&tcp, 0, sizeof(tcp));

	tcp.src_port = m_port;
	tcp.dest_port = m_dport;
	tcp.window_size = recv_window_size();
	tcp.sequence_number = (uint32_t)seq;
	tcp.ack_number = (uint32_t)ack;
	tcp.flags = flags;
	return tcp;
}

wiznet_udp_info w5100_socket_device::socket_udp_info(unsigned data_length) const
{
	wiznet_udp_info udp;
	std::memset(&udp, 0, sizeof(udp));

	udp.src_port = m_port;
	udp.dest_port = m_dport;
	udp.udp_length = udp.header_length + data_length;
	return udp;
}


DEFINE_DEVICE_TYPE(W5100_SOCKET, w5100_socket_device, "w5100_socket", "WIZnet W5100 Ethernet Controller Socket")
