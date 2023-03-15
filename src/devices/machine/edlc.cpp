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
 *   - 80c03 inter-packet gap (undocumented)
 */

#include "emu.h"
#include "edlc.h"
#include "hashing.h"

#define LOG_GENERAL (1U << 0)
#define LOG_FRAMES  (1U << 1)
#define LOG_FILTER  (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_FRAMES|LOG_FILTER)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SEEQ8003,  seeq8003_device,  "seeq8003",  "SEEQ 8003 EDLC")
DEFINE_DEVICE_TYPE(SEEQ80C03, seeq80c03_device, "seeq80c03", "SEEQ 80C03 EDLC")

static const u8 ETH_BROADCAST[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

seeq8003_device::seeq8003_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10)
	, m_out_int(*this)
	, m_out_rxrdy(*this)
	, m_out_txrdy(*this)
{
}

seeq8003_device::seeq8003_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: seeq8003_device(mconfig, SEEQ8003, tag, owner, clock)
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

	m_tx_timer = timer_alloc(FUNC(seeq8003_device::transmit), this);
	m_int_timer = timer_alloc(FUNC(seeq8003_device::interrupt), this);

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
	map(0, 5).rw(FUNC(seeq8003_device::unused_r), FUNC(seeq8003_device::station_address_w));
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
	case 0: case 1: case 2:
	case 3: case 4: case 5:
		station_address_w(offset, data);
		break;
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
	// HACK: should be asserted while the last byte is being read from the fifo
	// but doing it when the fifo is empty makes emulation simpler
	return m_rx_fifo.empty();
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

void seeq8003_device::rx_command_w(u8 data)
{
	LOG("rx_command_w 0x%02x (%s)\n", data, machine().describe_context());

	m_rx_command = data;
}

void seeq8003_device::tx_command_w(u8 data)
{
	LOG("tx_command_w 0x%02x (%s)\n", data, machine().describe_context());

	m_tx_command = data;
}

void seeq8003_device::transmit(int param)
{
	if (m_tx_fifo.queue_length())
	{
		u8 buf[MAX_FRAME_SIZE];
		int length = 0;

		// dequeue to buffer
		while (!m_tx_fifo.empty())
			buf[length++] = m_tx_fifo.dequeue();

		// transmit packet autopad
		if (mode_tx_pad() && (length < 60))
			while (length < 60)
				buf[length++] = 0;

		// compute and append fcs
		if (mode_tx_crc())
		{
			u32 const fcs = util::crc32_creator::simple(buf, length);
			buf[length++] = (fcs >> 0) & 0xff;
			buf[length++] = (fcs >> 8) & 0xff;
			buf[length++] = (fcs >> 16) & 0xff;
			buf[length++] = (fcs >> 24) & 0xff;
		}

		LOG("transmitting frame length %d\n", length);
		dump_bytes(buf, length);

		// transmit the frame
		send(buf, length, 4);

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
	unsigned const fcs_bytes = mode_rx_crc() ? 0 : 4;
	for (unsigned i = 0; i < length - fcs_bytes; i++)
		m_rx_fifo.enqueue(buf[i]);

	// enable rx fifo
	m_out_rxrdy(1);

	return length;
}

void seeq8003_device::interrupt(int param)
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
		LOGMASKED(LOG_FILTER, "address_filter accepted: promiscuous mode enabled\n");

		return true;
	}

	// station address
	if (!memcmp(address, m_station_address, 6))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: station address match\n");

		return true;
	}

	// ethernet broadcast
	if (!memcmp(address, ETH_BROADCAST, 6))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: broadcast\n");

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

seeq80c03_device::seeq80c03_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: seeq8003_device(mconfig, SEEQ80C03, tag, owner, clock)
	, m_regbank(*this, "regbank")
{
}

void seeq80c03_device::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_regbank).set_map(&seeq80c03_device::map_reg).set_options(ENDIANNESS_NATIVE, 8, 5, 8);
}

void seeq80c03_device::device_start()
{
	seeq8003_device::device_start();

	save_item(NAME(m_tx_cc));
	save_item(NAME(m_cc));
	save_item(NAME(m_flags));
	save_item(NAME(m_control));
	save_item(NAME(m_config));
	save_item(NAME(m_multicast_filter));
}

void seeq80c03_device::device_reset()
{
	seeq8003_device::device_reset();

	m_tx_cc = 0;
	m_cc = 0;
	m_flags = 0;
	m_control = 0;
	m_config = 0;
	m_multicast_filter = 0;
}

void seeq80c03_device::send_complete_cb(int result)
{
	if (result)
	{
		if (m_control & CTL_SQE)
			m_flags |= FLAGS_SQE;
	}
	else
	{
		// assume transmit failure and no device means loss of carrier
		if ((m_control & CTL_TNC) && !m_dev)
			m_flags |= FLAGS_TNC;
	}
}

void seeq80c03_device::map(address_map &map)
{
	map(0, 7).m(m_regbank, FUNC(address_map_bank_device::amap8));
}

void seeq80c03_device::map_reg(address_map &map)
{
	map(0x00, 0x05).w(FUNC(seeq80c03_device::station_address_w));

	map(0x08, 0x11).w(FUNC(seeq80c03_device::multicast_filter_w));
	map(0x12, 0x12).nopw(); // TODO: inter-packet gap
	map(0x13, 0x13).w(FUNC(seeq80c03_device::control_w));
	map(0x14, 0x14).w(FUNC(seeq80c03_device::config_w));

	map(0x00, 0x00).r(FUNC(seeq80c03_device::tx_ccl_r)).mirror(0x18);
	map(0x01, 0x01).r(FUNC(seeq80c03_device::tx_cch_r)).mirror(0x18);
	map(0x02, 0x02).r(FUNC(seeq80c03_device::ccl_r)).mirror(0x18);
	map(0x03, 0x03).r(FUNC(seeq80c03_device::cch_r)).mirror(0x18);
	map(0x04, 0x04).r(FUNC(seeq80c03_device::test_r)).mirror(0x18);
	map(0x05, 0x05).r(FUNC(seeq80c03_device::flags_r)).mirror(0x18);

	map(0x06, 0x06).rw(FUNC(seeq80c03_device::rx_status_r), FUNC(seeq80c03_device::rx_command_w)).mirror(0x18);
	map(0x07, 0x07).rw(FUNC(seeq80c03_device::tx_status_r), FUNC(seeq80c03_device::tx_command_w)).mirror(0x18);
}

u8 seeq80c03_device::read(offs_t offset)
{
	u8 data = 0xff;

	switch (offset)
	{
	case 0: data = tx_ccl_r(); break;
	case 1: data = tx_cch_r(); break;
	case 2: data = ccl_r(); break;
	case 3: data = cch_r(); break;
	case 4: data = test_r(); break;
	case 5: data = flags_r(); break;
	case 6: data = rx_status_r(); break;
	case 7: data = tx_status_r(); break;
	}

	return data;
}

void seeq80c03_device::write(offs_t offset, u8 data)
{
	switch (m_tx_command & TXC_B)
	{
	case 0x00:
		switch (offset)
		{
		case 0: case 1: case 2:
		case 3: case 4: case 5:
			station_address_w(offset, data);
			break;
		case 6: rx_command_w(data); break;
		case 7: tx_command_w(data); break;
		}
		break;
	case 0x20:
		switch (offset)
		{
		case 0: case 1: case 2:
		case 3: case 4: case 5:
			multicast_filter_w(offset, data);
			break;
		case 6: rx_command_w(data); break;
		case 7: tx_command_w(data); break;
		}
		break;
	case 0x40:
		switch (offset)
		{
		case 0: case 1:
			multicast_filter_w(offset + 8, data);
			break;
		case 2: break; // TODO: inter-packet gap
		case 3: control_w(data); break;
		case 4: config_w(data); break;
		case 6: rx_command_w(data); break;
		case 7: tx_command_w(data); break;
		}
		break;
	case 0x60:
		switch (offset)
		{
		case 6: rx_command_w(data); break;
		case 7: tx_command_w(data); break;
		}
		break;
	}
}

void seeq80c03_device::multicast_filter_w(offs_t offset, u8 data)
{
	unsigned const shift = (offset < 6) ? (offset * 8) : ((offset - 2) * 8);

	m_multicast_filter &= ~(u64(0xff) << shift);
	m_multicast_filter |= u64(data) << shift;
}

void seeq80c03_device::tx_command_w(u8 data)
{
	seeq8003_device::tx_command_w(data);

	m_regbank->set_bank((data >> 5) & 3);
}

void seeq80c03_device::control_w(u8 data)
{
	LOG("control_w 0x%02x (%s)\n", data, machine().describe_context());

	if ((m_control & CTL_TCC) && !(data & CTL_TCC))
		m_tx_cc = 0;
	if ((m_control & CTL_CC) && !(data & CTL_CC))
		m_cc = 0;
	if ((m_control & CTL_SQE) && !(data & CTL_SQE))
		m_flags &= ~FLAGS_SQE;
	if ((m_control & CTL_TNC) && !(data & CTL_TNC))
		m_flags &= ~FLAGS_TNC;

	m_control = data;
}

void seeq80c03_device::config_w(u8 data)
{
	LOG("config_w 0x%02x (%s)\n", data, machine().describe_context());

	m_config = data;
}

bool seeq80c03_device::address_filter(u8 *address)
{
	LOGMASKED(LOG_FILTER, "address_filter testing destination address %02x:%02x:%02x:%02x:%02x:%02x\n",
		address[0], address[1], address[2], address[3], address[4], address[5]);

	if ((m_rx_command & RXC_M) == RXC_M1)
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: promiscuous mode enabled\n");

		return true;
	}

	// station address
	if (m_config & CFG_GAM)
	{
		if (!memcmp(address, m_station_address, 5) && !(((address[5] ^ m_station_address[5]) & 0xf0)))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted: station address match\n");

			return true;
		}
	}
	else if (!memcmp(address, m_station_address, 6))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: station address match\n");

		return true;
	}

	// ethernet broadcast
	if (!memcmp(address, ETH_BROADCAST, 6))
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted: broadcast\n");

		return true;
	}

	// ethernet multicast
	if (((m_rx_command & RXC_M) == RXC_M3) && (address[0] & 0x1))
	{
		// multicast hash filter
		if (m_control & CTL_MHF)
		{
			u32 const crc = util::crc32_creator::simple(address, 6);

			if (!BIT(m_multicast_filter, crc & 63))
				return false;
		}

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
