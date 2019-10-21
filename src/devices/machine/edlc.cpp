// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the SEEQ 8003 Ethernet Data Link Controller.
 *
 * This implementation uses transmit/receive fifos which hold entire frames,
 * rather than the 16-byte fifos of the real device to simplify logic. In
 * hardware, RxTxEOF is effectively the 9th bit of the data bus, however to
 * simplify emulation is implemented as two separate read/write line handlers
 * which must be used strictly as follows:
 *
 *   - rxeof_r() must be read before fifo_r()
 *   - txeof_w() must be written after fifo_w()
 *
 * Sources:
 *   - http://www.bitsavers.org/components/seeq/_dataBooks/1985_SEEQ_Data_Book.pdf
 *
 * TODO:
 *   - RxDC (discard) and TxRET (retransmit) logic
 *   - 80c03 support
 *   - testing
 */

#include "emu.h"
#include "edlc.h"
#include "hashing.h"

#define LOG_GENERAL (1U << 0)
#define LOG_FRAMES  (1U << 1)
#define LOG_FILTER  (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_FRAMES|LOG_FILTER)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SEEQ8003, seeq8003_device, "seeq8003", "SEEQ 8003 EDLC")

static const u8 ETH_BROADCAST[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

seeq8003_device::seeq8003_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEEQ8003, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10.0f)
	, m_out_int(*this)
	, m_out_rxrdy(*this)
	, m_out_txrdy(*this)
{
}

void seeq8003_device::device_start()
{
	m_out_int.resolve_safe();
	m_out_rxrdy.resolve_safe();
	m_out_txrdy.resolve_safe();

	save_item(NAME(m_int_state));
	save_item(NAME(m_reset_state));
	save_item(NAME(m_station_address));
	save_item(NAME(m_rx_status));
	save_item(NAME(m_tx_status));
	save_item(NAME(m_rx_command));
	save_item(NAME(m_tx_command));
	//save_item(NAME(m_rx_fifo));
	//save_item(NAME(m_tx_fifo));

	m_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(seeq8003_device::transmit), this));
	m_int_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(seeq8003_device::interrupt), this));

	m_int_state = 0;
	m_reset_state = 1;
}

void seeq8003_device::device_reset()
{
	m_rx_status = RXS_O;
	m_tx_status = TXS_O;
	m_rx_command = 0;
	m_tx_command = 0;

	m_rx_fifo.clear();
	m_tx_fifo.clear();
	m_out_rxrdy(0);

	// TODO: deassert RxDC and TxRET

	if (m_dev)
		m_out_txrdy(1);

	interrupt();
}

int seeq8003_device::recv_start_cb(u8 *buf, int length)
{
	// check receiver disabled
	if (!m_reset_state || ((m_rx_command & RXC_M) == RXC_M0))
		return 0;

	if (address_filter(buf))
	{
		LOG("receiving frame length %d\n", length);
		dump_bytes(buf, length);

		return receive(buf, length);
	}

	return 0;
}

void seeq8003_device::map(address_map &map)
{
	map.unmap_value_high();

	map(0, 0).w(FUNC(seeq8003_device::station_address_w<0>));
	map(1, 1).w(FUNC(seeq8003_device::station_address_w<1>));
	map(2, 2).w(FUNC(seeq8003_device::station_address_w<2>));
	map(3, 3).w(FUNC(seeq8003_device::station_address_w<3>));
	map(4, 4).w(FUNC(seeq8003_device::station_address_w<4>));
	map(5, 5).w(FUNC(seeq8003_device::station_address_w<5>));
	map(6, 6).rw(FUNC(seeq8003_device::rx_status_r), FUNC(seeq8003_device::rx_command_w));
	map(7, 7).rw(FUNC(seeq8003_device::tx_status_r), FUNC(seeq8003_device::tx_command_w));
}

u8 seeq8003_device::read(offs_t offset)
{
	u8 data = 0xff;

	switch (offset)
	{
	case 6: data = rx_status_r(); break;
	case 7: data = tx_status_r(); break;
	}

	return data;
}

void seeq8003_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: station_address_w<0>(data); break;
	case 1: station_address_w<1>(data); break;
	case 2: station_address_w<2>(data); break;
	case 3: station_address_w<3>(data); break;
	case 4: station_address_w<4>(data); break;
	case 5: station_address_w<5>(data); break;
	case 6: rx_command_w(data); break;
	case 7: tx_command_w(data); break;
	}
}

void seeq8003_device::reset_w(int state)
{
	if (m_reset_state && !state)
	{
		// enter reset state
		m_out_txrdy(0);

		// TODO: assert RxDC and TxRET
	}
	else if (!m_reset_state && state)
	{
		// leave reset state
		device_reset();
	}

	m_reset_state = state;
}

u8 seeq8003_device::fifo_r()
{
	if (!m_reset_state)
		return 0xff;

	if (m_rx_fifo.empty())
		fatalerror("seeq8003_device::fifo_r: fifo empty\n");

	u8 const data = m_rx_fifo.dequeue();

	if (m_rx_fifo.empty())
	{
		// disable rx fifo
		m_out_rxrdy(0);

		// schedule interrupt
		m_int_timer->adjust(attotime::zero);
	}

	return data;
}

int seeq8003_device::rxeof_r()
{
	return m_rx_fifo.queue_length() == 1;
}

void seeq8003_device::fifo_w(u8 data)
{
	if (!m_reset_state)
		return;

	if (m_tx_fifo.full())
		fatalerror("seeq8003_device::fifo_w: fifo full\n");

	m_tx_fifo.enqueue(data);

	if (m_tx_fifo.full())
		m_out_txrdy(0);
}

void seeq8003_device::txeof_w(int state)
{
	if (m_reset_state && state)
	{
		// disable tx fifo
		m_out_txrdy(0);

		// schedule transmit
		m_tx_timer->adjust(attotime::zero);
	}
}

u8 seeq8003_device::rx_status_r()
{
	u8 const data = m_rx_status;

	// clear interrupt
	if (m_reset_state && !machine().side_effects_disabled())
	{
		m_rx_status |= RXS_O;
		m_int_timer->adjust(attotime::zero);
	}

	return data;
}

u8 seeq8003_device::tx_status_r()
{
	u8 const data = m_tx_status;

	// clear interrupt
	if (m_reset_state && !machine().side_effects_disabled())
	{
		m_tx_status |= TXS_O;
		m_int_timer->adjust(attotime::zero);
	}

	return data;
}

void seeq8003_device::transmit(void *ptr, int param)
{
	if (m_tx_fifo.queue_length())
	{
		u8 buf[MAX_FRAME_SIZE];
		int length = 0;

		// dequeue to buffer
		while (!m_tx_fifo.empty())
			buf[length++] = m_tx_fifo.dequeue();

		// compute and append fcs
		u32 const fcs = util::crc32_creator::simple(buf, length);
		buf[length++] = (fcs >> 0) & 0xff;
		buf[length++] = (fcs >> 8) & 0xff;
		buf[length++] = (fcs >> 16) & 0xff;
		buf[length++] = (fcs >> 24) & 0xff;

		LOG("transmitting frame length %d\n", length);
		dump_bytes(buf, length);

		// transmit the frame
		send(buf, length);

		// TODO: transmit errors/TxRET

		// update status
		m_tx_status = TXS_S;
	}
	else
		m_tx_status = TXS_U;

	// enable tx fifo
	m_out_txrdy(1);

	interrupt();
}

int seeq8003_device::receive(u8 *buf, int length)
{
	// discard if rx status has not been read
	// TODO: RxDC
	if (!(m_rx_status & RXS_O))
		return 0;

	m_rx_status = RXS_E;

	// check for errors
	u32 const fcs = util::crc32_creator::simple(buf, length);
	if (length < 64)
		m_rx_status |= RXS_S;
	else if (~fcs != FCS_RESIDUE)
		m_rx_status |= RXS_C;
	else
		m_rx_status |= RXS_G;

	// enqueue from buffer
	for (unsigned i = 0; i < length - 4; i++)
		m_rx_fifo.enqueue(buf[i]);

	// enable rx fifo
	m_out_rxrdy(1);

	return length;
}

void seeq8003_device::interrupt(void *ptr, int param)
{
	int const state =
		(!(m_tx_status & TXS_O) && (m_tx_status & m_tx_command & TXS_M)) ||
		(!(m_rx_status & RXS_O) && (m_rx_status & m_rx_command & RXS_M));

	// TODO: assert RxDC for masked rx crc or short frame errors

	if (state != m_int_state)
	{
		m_int_state = state;
		m_out_int(state);
	}
}

bool seeq8003_device::address_filter(u8 *address)
{
	LOGMASKED(LOG_FILTER, "address_filter testing destination address %02x:%02x:%02x:%02x:%02x:%02x\n",
		address[0], address[1], address[2], address[3], address[4], address[5]);

	if ((m_rx_command & RXC_M) == RXC_M1)
	{
		LOG("address_filter accepted: promiscuous mode enabled\n");

		return true;
	}

	// ethernet broadcast
	if (!memcmp(address, ETH_BROADCAST, 6))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: broadcast\n");

		return true;
	}

	// station address
	if (!memcmp(address, m_station_address, 6))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: station address match\n");

		return true;
	}

	// ethernet multicast
	if (((m_rx_command & RXC_M) == RXC_M3) && (address[0] & 0x1))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: multicast address match\n");

		return true;
	}

	return false;
}

void seeq8003_device::dump_bytes(u8 *buf, int length)
{
	if (VERBOSE & LOG_FRAMES)
	{
		// pad frame with zeros to 8-byte boundary
		for (int i = 0; i < 8 - (length % 8); i++)
			buf[length + i] = 0;

		// dump length / 8 (rounded up) groups of 8 bytes
		for (int i = 0; i < (length + 7) / 8; i++)
			LOGMASKED(LOG_FRAMES, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
				buf[i * 8 + 0], buf[i * 8 + 1], buf[i * 8 + 2], buf[i * 8 + 3],
				buf[i * 8 + 4], buf[i * 8 + 5], buf[i * 8 + 6], buf[i * 8 + 7]);
	}
}
