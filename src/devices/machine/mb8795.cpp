// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "mb8795.h"

#include "hashing.h"
#include "multibyte.h"

#define LOG_REGR   (1U << 1)
#define LOG_REGW   (1U << 2)
#define LOG_TX     (1U << 3)
#define LOG_RX     (1U << 4)
#define LOG_FRAMES (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW|LOG_TX|LOG_RX|LOG_FRAMES)

#include "logmacro.h"

// Lifted from netbsd
enum txs_mask : u8 {
	TXS_READY        = 0x80, // ready for packet
	TXS_BUSY         = 0x40, // receive carrier detect
	TXS_TXRECV       = 0x20, // transmission received
	TXS_SHORTED      = 0x10, // possible coax short
	TXS_UNDERFLOW    = 0x08, // underflow on xmit
	TXS_COLLERR      = 0x04, // collision detected
	TXS_COLLERR16    = 0x02, // 16th collision error
	TXS_PARERR       = 0x01, // parity error in tx data
};
enum rxs_mask : u8 {
	RXS_OK           = 0x80, // packet received ok
	RXS_RESET        = 0x10, // reset packet received
	RXS_SHORT        = 0x08, // < minimum length
	RXS_ALIGNERR     = 0x04, // alignment error
	RXS_CRCERR       = 0x02, // CRC error
	RXS_OVERFLOW     = 0x01, // receiver FIFO overflow
};
enum tmd_mask : u8 {
	TMD_COLLMASK     = 0xf0, // collision count
	TMD_COLLSHIFT    = 4,
	TMD_PARIGNORE    = 0x08, // ignore parity
	TMD_TURBO1       = 0x04,
	TMD_LB_DISABLE   = 0x02, // loop back disabled
	TMD_DISCONTENT   = 0x01, // disable contention (rx carrier)
};
enum rmd_mask : u8 {
	RMD_TEST         = 0x80, // must be zero
	RMD_ADDRSIZE     = 0x10, // reduces NODE match to 5 chars
	RMD_SHORTENABLE  = 0x08, // "rx packets >= 10 bytes" - <?
	RMD_RESETENABLE  = 0x04, // detect "reset" ethernet frames
	RMD_WHATRECV     = 0x03, // controls what packets are received
	RMD_RECV_PROMISC = 0x03, // all packets
	RMD_RECV_MULTI   = 0x02, // accept broad/multicasts
	RMD_RECV_NORMAL  = 0x01, // accept broad/limited multicasts
	RMD_RECV_NONE    = 0x00, // accept no packets
};
enum rst_mask : u8 {
	RST_RESET        = 0x80, // reset interface
};

DEFINE_DEVICE_TYPE(MB8795, mb8795_device, "mb8795", "Fujitsu MB8795")

void mb8795_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(mb8795_device::txstat_r), FUNC(mb8795_device::txstat_w));
	map(0x1, 0x1).rw(FUNC(mb8795_device::txmask_r), FUNC(mb8795_device::txmask_w));
	map(0x2, 0x2).rw(FUNC(mb8795_device::rxstat_r), FUNC(mb8795_device::rxstat_w));
	map(0x3, 0x3).rw(FUNC(mb8795_device::rxmask_r), FUNC(mb8795_device::rxmask_w));
	map(0x4, 0x4).rw(FUNC(mb8795_device::txmode_r), FUNC(mb8795_device::txmode_w));
	map(0x5, 0x5).rw(FUNC(mb8795_device::rxmode_r), FUNC(mb8795_device::rxmode_w));
	map(0x6, 0x6).w(FUNC(mb8795_device::reset_w));
	map(0x7, 0x7).r(FUNC(mb8795_device::tdr1_r));
	map(0x8, 0xd).rw(FUNC(mb8795_device::mac_r), FUNC(mb8795_device::mac_w));
	map(0xf, 0xf).r(FUNC(mb8795_device::tdr2_r));
}

mb8795_device::mb8795_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MB8795, tag, owner, clock),
	device_network_interface(mconfig, *this, 10),
	m_mac{},
	m_txstat(0), m_txmask(0), m_rxstat(0), m_rxmask(0), m_txmode(0), m_rxmode(0), m_txlen(0), m_rxlen(0), m_txcount(0),
	m_drq_tx(false), m_drq_rx(false), m_irq_tx(false), m_irq_rx(false),
	m_timer_tx(nullptr), m_timer_rx(nullptr),
	m_irq_tx_cb(*this),
	m_irq_rx_cb(*this),
	m_drq_tx_cb(*this),
	m_drq_rx_cb(*this)
{
}

void mb8795_device::check_irq()
{
	bool old_irq_tx = m_irq_tx;
	bool old_irq_rx = m_irq_rx;
	m_irq_tx = m_txstat & m_txmask;
	m_irq_rx = m_rxstat & m_rxmask;
	if(m_irq_tx != old_irq_tx)
		m_irq_tx_cb(m_irq_tx);
	if(m_irq_rx != old_irq_rx)
		m_irq_rx_cb(m_irq_rx);
}

void mb8795_device::device_start()
{
	m_timer_tx = timer_alloc(FUNC(mb8795_device::tx_update), this);
	m_timer_rx = timer_alloc(FUNC(mb8795_device::rx_update), this);
}

void mb8795_device::device_reset()
{
	m_txstat = TXS_READY;
	m_txmask = 0x00;
	m_rxstat = 0x00;
	m_rxmask = 0x00;
	m_txmode = 0x00;
	m_rxmode = 0x00;

	m_drq_tx = m_drq_rx = false;
	m_irq_tx = m_irq_rx = false;

	m_txlen = m_rxlen = m_txcount = 0;

	// TODO: verify if LBC is asserted after reset
	set_loopback(true);

	start_send();
}

int mb8795_device::recv_start_cb(u8 *buf, int len)
{
	if(!m_rxlen) {
		memcpy(m_rxbuf, buf, len);
		m_rxlen = len;
		return len;
	} else
		return 0;
}

void mb8795_device::recv_complete_cb(int result)
{
	if(result > 0)
		receive();
}

u8 mb8795_device::txstat_r()
{
	LOGMASKED(LOG_REGR, "txstat_r %02x %s\n", m_txstat, machine().describe_context());
	return m_txstat;
}

void mb8795_device::txstat_w(u8 data)
{
	m_txstat = m_txstat & (0xf0 | ~data);
	check_irq();
	LOGMASKED(LOG_REGW, "txstat_w %02x %s\n", m_txstat, machine().describe_context());
}

u8 mb8795_device::txmask_r()
{
	LOGMASKED(LOG_REGR, "txmask_r %02x %s\n", m_txmask, machine().describe_context());
	return m_txmask;
}

void mb8795_device::txmask_w(u8 data)
{
	m_txmask = data & 0xaf;
	check_irq();
	LOGMASKED(LOG_REGW, "txmask_w %02x %s\n", m_txmask, machine().describe_context());
}

u8 mb8795_device::rxstat_r()
{
	LOGMASKED(LOG_REGR, "rxstat_r %02x %s\n", m_rxstat, machine().describe_context());
	return m_rxstat;
}

void mb8795_device::rxstat_w(u8 data)
{
	m_rxstat = m_rxstat & (0x70 | ~data);
	check_irq();
	LOGMASKED(LOG_REGW, "rxstat_w %02x %s\n", m_rxstat, machine().describe_context());
}

u8 mb8795_device::rxmask_r()
{
	LOGMASKED(LOG_REGR, "rxmask_r %02x %s\n", m_rxmask, machine().describe_context());
	return m_rxmask;
}

void mb8795_device::rxmask_w(u8 data)
{
	m_rxmask = data & 0x9f;
	check_irq();
	LOGMASKED(LOG_REGW, "rxmask_w %02x %s\n", m_rxmask, machine().describe_context());
}

u8 mb8795_device::txmode_r()
{
	LOGMASKED(LOG_REGR, "txmode_r %02x %s\n", m_txmode, machine().describe_context());
	return m_txmode;
}

void mb8795_device::txmode_w(u8 data)
{
	set_loopback(!(data & TMD_LB_DISABLE));
	m_txmode = data;
	LOGMASKED(LOG_REGW, "txmode_w %02x %s\n", m_txmode, machine().describe_context());
}

u8 mb8795_device::rxmode_r()
{
	LOGMASKED(LOG_REGR, "rxmode_r %02x %s\n", m_rxmode, machine().describe_context());
	return m_rxmode;
}

void mb8795_device::rxmode_w(u8 data)
{
	m_rxmode = data;
	LOGMASKED(LOG_REGW, "rxmode_w %02x %s\n", m_rxmode, machine().describe_context());
}

void mb8795_device::reset_w(u8 data)
{
	if(data & RST_RESET)
		reset();
}

u8 mb8795_device::tdr1_r()
{
	LOGMASKED(LOG_REGR, "tdr1_r %02x %s\n", m_txcount & 0xff, machine().describe_context());
	return m_txcount;
}

u8 mb8795_device::mac_r(offs_t offset)
{
	return m_mac[offset];
}

void mb8795_device::mac_w(offs_t offset, u8 data)
{
	m_mac[offset] = data;
	set_mac(m_mac);
}

u8 mb8795_device::tdr2_r()
{
	LOGMASKED(LOG_REGR, "tdr2_r %02x %s\n", m_txcount >> 8, machine().describe_context());
	return (m_txcount >> 8) & 0x3f;
}

void mb8795_device::start_send()
{
	m_timer_tx->adjust(attotime::zero);
}

void mb8795_device::tx_dma_w(u8 data, bool eof)
{
	m_txbuf[m_txlen++] = data;
	if(m_txstat & TXS_READY) {
		m_txstat &= ~TXS_READY;
		check_irq();
	}

	m_drq_tx = false;
	m_drq_tx_cb(m_drq_tx);

	if(eof) {
		LOGMASKED(LOG_TX, "send packet, dest=%02x.%02x.%02x.%02x.%02x.%02x len=%04x\n",
			m_txbuf[0], m_txbuf[1], m_txbuf[2], m_txbuf[3], m_txbuf[4], m_txbuf[5], m_txlen);

		if(m_txlen > 1500)
			m_txlen = 1500; // Weird packet send on loopback test in the next

		// append frame check sequence
		put_u32le(&m_txbuf[m_txlen], util::crc32_creator::simple(m_txbuf, m_txlen));
		m_txlen += 4;

		if(VERBOSE & LOG_FRAMES)
			log_bytes(m_txbuf, m_txlen);

		// count number of bits transmitted
		m_txcount = send(m_txbuf, m_txlen, 4) * 8;

		// transmitted frames are seen by the receiver
		// TODO: TXS_TXRECV
		if(m_txmode & TMD_LB_DISABLE) {
			memcpy(m_rxbuf, m_txbuf, m_txlen);
			m_rxlen = m_txlen;
			receive();
		}

		m_txlen = 0;
		m_txstat |= TXS_READY;
		start_send();
	} else
		m_timer_tx->adjust(attotime::from_nsec(800));
}

void mb8795_device::rx_dma_r(u8 &data, bool &eof)
{
	m_drq_rx = false;
	m_drq_rx_cb(m_drq_rx);

	if(m_rxlen) {
		data = m_rxbuf[0];
		m_rxlen--;
		memmove(m_rxbuf, m_rxbuf+1, m_rxlen);
	} else
		data = 0;

	if(m_rxlen) {
		m_timer_rx->adjust(attotime::from_nsec(800));
		eof = false;
	} else
		eof = true;
}

void mb8795_device::receive()
{
	bool keep = false;
	switch(m_rxmode & RMD_WHATRECV) {
	case RMD_RECV_NONE:
		keep = false;
		break;
	case RMD_RECV_NORMAL:
		keep = recv_is_broadcast() || recv_is_me() || recv_is_local_multicast();
		break;
	case RMD_RECV_MULTI:
		keep = recv_is_broadcast() || recv_is_me() || recv_is_multicast();
		break;
	case RMD_RECV_PROMISC:
		keep = true;
		break;
	}
	LOGMASKED(LOG_RX, "received packet for %02x.%02x.%02x.%02x.%02x.%02x len=%04x, mode=%d -> %s\n",
			m_rxbuf[0], m_rxbuf[1], m_rxbuf[2], m_rxbuf[3], m_rxbuf[4], m_rxbuf[5],
			m_rxlen, m_rxmode & 3, keep ? "kept" : "dropped");
	if(!keep)
		m_rxlen = 0;
	else {
		if(VERBOSE & LOG_FRAMES)
			log_bytes(m_rxbuf, m_rxlen);

		m_rxstat = 0;
		if(m_rxlen < 64) {
			if(!(m_rxmode & RMD_SHORTENABLE) || (m_rxlen < 10)) {
				LOGMASKED(LOG_RX, "short packet discarded\n");
				m_rxlen = 0;
				return;
			} else
				m_rxstat |= RXS_SHORT;
		}

		// TODO: RMD_TEST
		if(util::crc32_creator::simple(m_rxbuf, m_rxlen).m_raw != get_u32le(&m_rxbuf[m_rxlen - 4]))
			m_rxstat |= RXS_CRCERR;

		// frame check sequence is not added to fifo
		m_rxlen -= 4;

		if(!(m_rxstat & (RXS_SHORT | RXS_CRCERR)))
			m_rxstat |= RXS_OK;

		check_irq();
		m_timer_rx->adjust(attotime::zero);
	}
}

bool mb8795_device::recv_is_broadcast()
{
	return
		m_rxbuf[0] == 0xff &&
		m_rxbuf[1] == 0xff &&
		m_rxbuf[2] == 0xff &&
		m_rxbuf[3] == 0xff &&
		m_rxbuf[4] == 0xff &&
		m_rxbuf[5] == 0xff;
}

bool mb8795_device::recv_is_me()
{
	return
		m_rxbuf[0] == m_mac[0] &&
		m_rxbuf[1] == m_mac[1] &&
		m_rxbuf[2] == m_mac[2] &&
		m_rxbuf[3] == m_mac[3] &&
		m_rxbuf[4] == m_mac[4] &&
		((m_rxmode & RMD_ADDRSIZE) || m_rxbuf[5] == m_mac[5]);
}

bool mb8795_device::recv_is_local_multicast()
{
	return
		(m_rxbuf[0] & 0x01) &&
		(m_rxbuf[0] & 0xfe) == m_mac[0] &&
		m_rxbuf[1] == m_mac[1] &&
		m_rxbuf[2] == m_mac[2];
}

bool mb8795_device::recv_is_multicast()
{
	return m_rxbuf[0] & 0x01;
}

TIMER_CALLBACK_MEMBER(mb8795_device::tx_update)
{
	m_drq_tx = true;
	m_drq_tx_cb(m_drq_tx);
}

TIMER_CALLBACK_MEMBER(mb8795_device::rx_update)
{
	if(m_rxlen) {
		m_drq_rx = true;
		m_drq_rx_cb(m_drq_rx);
	}
}
