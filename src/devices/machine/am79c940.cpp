// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    AMD Am79C940 Media Access Controller for Ethernet (MACE).
    Emulation by R. Belmont

	Presumably the naming means this was the follow-on to the popular LANCE.

    Manual: https://ardent-tool.com/datasheets/AMD_Am79C940.pdf
 */

#include "emu.h"
#include "am79c940.h"
#include "hashing.h"

#define LOG_REG (1U << 1)
#define LOG_IRQ (1U << 2)
#define LOG_ADDR (1U << 3)
#define LOG_PACKET (1U << 4)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(AM79C940, am79c940_device, "am79c940", "AMD Am79C940 MACE")

am79c940_device::am79c940_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: am79c940_device(mconfig, AM79C940, tag, owner, clock)
{
}

am79c940_device::am79c940_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10)
	, m_revision(revision::C0)
	, m_irq_out(*this)
	, m_tx_drq_out(*this)
	, m_rx_drq_out(*this)
	, m_irq_state(false)
	, m_tx_drq(false)
	, m_rx_drq(false)
	, m_reg{}
	, m_padr{}
	, m_ladrf{}
	, m_address_index(0)
	, m_address_access(true)
	, m_mpc_enabled(false)
	, m_tx_watermark(0)
	, m_rx_watermark(2)
	, m_tx_fifo{}
	, m_tx_flags{}
	, m_tx_controls{}
	, m_tx_head(0)
	, m_tx_tail(0)
	, m_tx_count(0)
	, m_tx_frame_count(0)
	, m_tx_odd_cycle(false)
	, m_tx_packet{}
	, m_tx_length(0)
	, m_tx_assembling(false)
	, m_tx_sending(false)
	, m_tx_loopback(0)
	, m_tx_status{}
	, m_tx_retry{}
	, m_tx_status_head(0)
	, m_tx_status_count(0)
	, m_rx_fifo{}
	, m_rx_flags{}
	, m_rx_head(0)
	, m_rx_tail(0)
	, m_rx_count(0)
	, m_rx_frame_count(0)
	, m_rx_current_words(0)
	, m_rx_packet{}
	, m_rx_length(0)
	, m_rx_position(0)
	, m_rx_wire_length(0)
	, m_rx_wire_position(0)
	, m_rx_error(0)
	, m_rx_runt_snapshot(0)
	, m_rx_collision_snapshot(0)
	, m_rx_busy(false)
	, m_rx_readable(false)
	, m_rx_request_started(false)
	, m_rx_overflow(false)
	, m_rx_overflowed(false)
	, m_rx_overflow_pending(false)
	, m_receive_timer(nullptr)
{
	m_bandwidth = 10'000'000 / 8;
}

void am79c940_device::map(address_map &map)
{
	map(0x00, 0x1f).rw(FUNC(am79c940_device::read), FUNC(am79c940_device::write));
}

void am79c940_device::device_start()
{
	m_receive_timer = timer_alloc(FUNC(am79c940_device::receive_tick), this);

	save_item(NAME(m_irq_state));
	save_item(NAME(m_tx_drq));
	save_item(NAME(m_rx_drq));
	save_item(NAME(m_reg));
	save_item(NAME(m_padr));
	save_item(NAME(m_mac));
	save_item(NAME(m_ladrf));
	save_item(NAME(m_address_index));
	save_item(NAME(m_address_access));
	save_item(NAME(m_mpc_enabled));
	save_item(NAME(m_tx_watermark));
	save_item(NAME(m_rx_watermark));
	save_item(NAME(m_tx_fifo));
	save_item(NAME(m_tx_flags));
	save_item(NAME(m_tx_controls));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_tail));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_frame_count));
	save_item(NAME(m_tx_odd_cycle));
	save_item(NAME(m_tx_packet));
	save_item(NAME(m_tx_length));
	save_item(NAME(m_tx_assembling));
	save_item(NAME(m_tx_sending));
	save_item(NAME(m_tx_loopback));
	save_item(NAME(m_tx_status));
	save_item(NAME(m_tx_retry));
	save_item(NAME(m_tx_status_head));
	save_item(NAME(m_tx_status_count));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_flags));
	save_item(NAME(m_rx_head));
	save_item(NAME(m_rx_tail));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_rx_frame_count));
	save_item(NAME(m_rx_current_words));
	save_item(NAME(m_rx_packet));
	save_item(NAME(m_rx_length));
	save_item(NAME(m_rx_position));
	save_item(NAME(m_rx_wire_length));
	save_item(NAME(m_rx_wire_position));
	save_item(NAME(m_rx_error));
	save_item(NAME(m_rx_runt_snapshot));
	save_item(NAME(m_rx_collision_snapshot));
	save_item(NAME(m_rx_busy));
	save_item(NAME(m_rx_readable));
	save_item(NAME(m_rx_request_started));
	save_item(NAME(m_rx_overflow));
	save_item(NAME(m_rx_overflowed));
	save_item(NAME(m_rx_overflow_pending));
}

void am79c940_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	m_reg[XMTFC] = 0x01;
	m_reg[RCVFC] = 0x01;
	m_reg[BIUCC] = 0x20;
	m_reg[FIFOCC] = 0x20;
	m_reg[PHYCC] = 0x80;		// No 10BASE-T link pulses supplied.
	m_address_index = 0;
	m_address_access = true;
	m_mpc_enabled = false;
	m_tx_watermark = 0;
	m_rx_watermark = 2;
	m_tx_status_head = m_tx_status_count = 0;
	m_rx_runt_snapshot = m_rx_collision_snapshot = 0;
	reset_tx_fifo(true);
	reset_rx_fifo();

	set_mac(m_padr);
	set_loopback(false);
	update_irq(true);
	update_requests(true);
}

void am79c940_device::device_post_load()
{
	auto const mac = m_mac;
	set_mac(mac.data());
	update_irq(true);
	update_requests(true);
}

void am79c940_device::update_irq(bool force)
{
	bool const state = bool(m_reg[IR] & ~m_reg[IMR]);
	if (force || (state != m_irq_state))
	{
		m_irq_state = state;
		LOGMASKED(LOG_IRQ, "IRQ %d (IR=%02x IMR=%02x)\n", state, m_reg[IR], m_reg[IMR]);
		m_irq_out(state);
	}
}

void am79c940_device::update_requests(bool force)
{
	static constexpr unsigned tx_watermarks[] = { 8, 16, 32, 32 };
	static constexpr unsigned rx_watermarks[] = { 28, 44, 76, 76 };

	unsigned const free = TX_WORDS - m_tx_count;
	bool const tx = BIT(m_reg[MACCC], 1) && (m_tx_frame_count < 15) &&
		(free > ((BIT(m_reg[FIFOCC], 1) && m_tx_drq) ? 2 : tx_watermarks[m_tx_watermark]));

	if (force || tx != m_tx_drq)
	{
		m_tx_drq = tx;
		m_tx_drq_out(tx);
	}

	bool rx = false;
	if ((m_reg[MACCC] & ENRCV) && m_rx_count && (m_rx_flags[m_rx_head] & FIFO_READY))
	{
		unsigned bytes = 0;
		for (unsigned i = 0; i < m_rx_count; ++i)
		{
			bytes += m_rx_flags[(m_rx_head + i) % RX_WORDS] & 3;
		}
		unsigned const threshold = m_rx_request_started ? rx_watermarks[m_rx_watermark] : std::max(64U, rx_watermarks[m_rx_watermark]);
		rx = m_rx_frame_count || (m_rx_flags[m_rx_head] & FIFO_STATUS) ||
			(BIT(m_reg[RCVFC], 3) ? (bytes > 2) :
			((BIT(m_reg[FIFOCC], 0) && m_rx_drq) ? (bytes > 2) : (bytes >= threshold)));
	}

	if (rx)
	{
		m_rx_request_started = true;
	}

	if (force || rx != m_rx_drq)
	{
		m_rx_drq = rx;
		m_rx_drq_out(rx);
	}
}

void am79c940_device::reset_tx_fifo(bool cancel_wire)
{
	m_tx_head = m_tx_tail = m_tx_count = 0;
	m_tx_odd_cycle = false;
	m_tx_assembling = false;

	if (cancel_wire)
	{
		cancel_send();
		m_tx_sending = false;
	}

	if (!m_tx_sending)
	{
		m_tx_length = 0;
	}

	// Valid completions have a separate lifetime from the data FIFO.
	m_tx_frame_count = m_tx_status_count + (m_tx_sending ? 1 : 0);
}

void am79c940_device::reset_rx_fifo()
{
	cancel_receive();
	m_receive_timer->reset();
	m_rx_head = m_rx_tail = m_rx_count = 0;
	m_rx_frame_count = m_rx_current_words = 0;
	m_rx_length = m_rx_position = m_rx_wire_length = m_rx_wire_position = 0;
	m_rx_busy = m_rx_readable = m_rx_request_started = false;
	m_rx_overflow = m_rx_overflowed = m_rx_overflow_pending = false;
	rx_finish_address_change();
}

bool am79c940_device::tx_dma_w(u16 data, u16 mem_mask, bool eof)
{
	unsigned const bytes = bool(mem_mask & 0xff) + bool(mem_mask & 0xff00);
	unsigned const slots = 1 + (eof && !m_tx_odd_cycle);

	if (!bytes || !BIT(m_reg[MACCC], 1) || m_tx_frame_count == 15 || (m_tx_count + slots > TX_WORDS))
	{
		return false;
	}

	// Each byte write occupies a word; EOF aligns the next packet to four bytes.
	if (BIT(m_reg[BIUCC], 6))
	{
		data = swapendian_int16(data);
	}

	if (!(mem_mask & 0xff))
	{
		data >>= 8;
	}

	m_tx_fifo[m_tx_tail] = data;
	m_tx_flags[m_tx_tail] = bytes | ((eof && slots == 1) ? FIFO_EOF : 0);
	m_tx_controls[m_tx_tail] = m_reg[XMTFC];
	m_tx_tail = (m_tx_tail + 1) % TX_WORDS;
	++m_tx_count;
	m_tx_odd_cycle = !m_tx_odd_cycle;
	if (eof)
	{
		if (slots == 2)
		{
			m_tx_flags[m_tx_tail] = FIFO_EOF;
			m_tx_controls[m_tx_tail] = m_reg[XMTFC];
			m_tx_tail = (m_tx_tail + 1) % TX_WORDS;
			++m_tx_count;
		}
		m_tx_odd_cycle = false;
		++m_tx_frame_count;
	}
	pump_transmit();
	update_requests();
	return true;
}

void am79c940_device::pump_transmit()
{
	if (!BIT(m_reg[MACCC], 1) || m_tx_sending || m_tx_status_count == 2)
	{
		return;
	}
	if (!m_tx_assembling)
	{
		static constexpr unsigned start_points[] = { 4, 16, 64, 112 };
		unsigned bytes = 0;
		bool eof = false;
		for (unsigned i = 0; i < m_tx_count; ++i)
		{
			u8 const flags = m_tx_flags[(m_tx_head + i) % TX_WORDS];
			bytes += flags & 3;
			if (flags & FIFO_EOF)
			{
				eof = true;
				break;
			}
		}
		if (!eof && m_tx_count < TX_WORDS && bytes < start_points[(m_reg[BIUCC] >> 4) & 3])
		{
			return;
		}
		m_tx_assembling = true;
		m_tx_length = 0;
	}

	while (m_tx_count)
	{
		u8 const flags = m_tx_flags[m_tx_head];
		unsigned const bytes = flags & 3;
		// An overlong, unterminated host frame eventually backpressures the host.
		// A FIFO/software reset can discard it; never wrap the assembly buffer.
		if (m_tx_length + bytes > MAX_FRAME)
		{
			return;
		}
		for (unsigned i = 0; i < bytes; ++i)
		{
			m_tx_packet[m_tx_length++] = m_tx_fifo[m_tx_head] >> (8 * i);
		}
		u8 const control = m_tx_controls[m_tx_head];
		m_tx_head = (m_tx_head + 1) % TX_WORDS;
		--m_tx_count;
		if (!(flags & FIFO_EOF))
		{
			continue;
		}

		m_tx_assembling = false;
		m_tx_loopback = (m_reg[UTR] >> 1) & 3;
		bool const receive_fcs = m_tx_loopback && BIT(m_reg[UTR], 3);
		// In receive-FCS loopback the shared CRC unit belongs to RX, and software
		// supplies the FCS. Otherwise APADXMT overrides DXMTFCS.
		if (BIT(control, 0) && !receive_fcs)
		{
			while (m_tx_length < 60)
			{
				m_tx_packet[m_tx_length++] = 0;
			}
		}
		if (!receive_fcs && (BIT(control, 0) || !BIT(control, 3)))
		{
			u32 const crc = util::crc32_creator::simple(m_tx_packet, m_tx_length);
			for (unsigned i = 0; i < 4; ++i)
			{
				m_tx_packet[m_tx_length++] = crc >> (8 * i);
			}
		}
		LOGMASKED(LOG_PACKET, "TX %u bytes, loopback=%u\n", m_tx_length, m_tx_loopback);
		m_tx_sending = true;
		const int sent = send(m_tx_packet, m_tx_length, std::min<unsigned>(4, m_tx_length));
		// In external loopback a connected transceiver returns our transmission
		// while it also goes to the medium. Packet backends do not provide that
		// local echo, so return a successfully sent frame through the timed RX
		// path. FD_TEST instead uses external-loopback mode for full duplex.
		if (m_tx_loopback == 1 && !BIT(m_reg[UTR], 0) && sent == m_tx_length)
		{
			if (!m_rx_busy)
			{
				recv_cb(m_tx_packet, m_tx_length);
			}
			else
			{
				count_missed_packet();
			}
		}
		break;
	}
}

void am79c940_device::send_complete_cb(int result)
{
	if (!m_tx_sending)
	{
		return;
	}
	unsigned const tail = (m_tx_status_head + m_tx_status_count) & 1;
	m_tx_status[tail] = 0x80 | ((result != m_tx_length && m_tx_loopback < 2) ? 0x02 : 0);
	m_tx_retry[tail] = 0;
	++m_tx_status_count;
	m_reg[IR] |= 0x01 | ((m_tx_length > 1518) ? 0x40 : 0);
	m_tx_sending = false;
	m_tx_length = 0;
	// External echo starts after send(), so its receive completion can follow
	// this callback at the same timestamp. Finish it before starting another
	// looped frame, or the new receive would see the preceding one as busy.
	if (m_tx_loopback != 1 || !m_rx_busy)
	{
		pump_transmit();
	}
	update_requests();
	update_irq();
}

u8 am79c940_device::transmit_status_r()
{
	if (!m_tx_status_count)
	{
		return 0;
	}
	u8 const data = m_tx_status[m_tx_status_head];
	if (!machine().side_effects_disabled())
	{
		m_tx_status_head ^= 1;
		--m_tx_status_count;
		--m_tx_frame_count;
		pump_transmit();
		update_requests();
	}
	return data;
}

bool am79c940_device::receive_filter(const u8 *buf) const
{
	if (BIT(m_reg[MACCC], 7))
	{
		return true;
	}
	if (std::all_of(buf, buf + 6, [](u8 b)
		{
			return b == 0xff;
		}))
	{
		return !BIT(m_reg[MACCC], 2);
	}
	if (buf[0] & 1)
	{
		if ((m_reg[UTR] & 0x06) && !BIT(m_reg[UTR], 3))
		{
			return false; // The CRC unit must belong to RX for the logical filter.
		}
		unsigned const hash = (~u32(util::crc32_creator::simple(buf, 6))) >> 26;
		return BIT(m_ladrf[hash >> 3], hash & 7);
	}
	return !BIT(m_reg[MACCC], 3) && std::equal(buf, buf + 6, m_padr);
}

int am79c940_device::recv_start_cb(u8 *buf, int len)
{
	// The FCS contract is explicit: never guess its presence from CRC validity.
	if (len < 8 || len > int(MAX_FRAME) || !receive_filter(buf))
	{
		return 0;
	}
	if (len < 64)
	{
		count_runt_packet();
		if (!(m_reg[UTR] & 0x20) && (!(m_reg[RCVFC] & 0x08) || len < 12))
		{
			return 0;
		}
	}
	if (!(m_reg[MACCC] & ENRCV) || m_rx_busy || m_rx_overflow || m_rx_frame_count == 15)
	{
		count_missed_packet();
		return 0;
	}
	m_rx_error = 0;
	if ((!(m_reg[UTR] & 0x06) || BIT(m_reg[UTR], 3)) &&
		u32(util::crc32_creator::simple(buf, len)) != 0x2144df1c)
	{
		m_rx_error = 0x10;
	}
	m_rx_wire_length = len;
	m_rx_length = len;
	// Strip pad AND FCS only for short 802.3 length fields. Ethernet-II and
	// 802.3 lengths >=46 pass through unchanged, including their FCS.
	if (BIT(m_reg[RCVFC], 0) && len >= 18)
	{
		unsigned const length = (unsigned(buf[12]) << 8) | buf[13];
		if (length < 46)
		{
			m_rx_length = std::min<unsigned>(14 + length, len - 4);
		}
	}
	std::copy_n(buf, m_rx_length, m_rx_packet);
	m_rx_position = m_rx_wire_position = 0;
	m_rx_current_words = 0;
	m_rx_busy = true;
	m_rx_readable = m_rx_overflowed = false;
	m_receive_timer->adjust(attotime::from_ticks(8, m_bandwidth));
	LOGMASKED(LOG_PACKET, "RX %d wire bytes, %u delivered bytes, status=%02x\n", len, m_rx_length, m_rx_error);
	return len;
}

void am79c940_device::rx_push(u16 data, u8 flags)
{
	assert(m_rx_count < RX_WORDS);
	m_rx_fifo[m_rx_tail] = data;
	m_rx_flags[m_rx_tail] = flags;
	m_rx_tail = (m_rx_tail + 1) % RX_WORDS;
	++m_rx_count;
	if (flags & FIFO_ACTIVE)
	{
		++m_rx_current_words;
	}
}

void am79c940_device::rx_mark_ready()
{
	m_rx_readable = true;
	for (unsigned i = 0; i < m_rx_current_words; ++i)
	{
		m_rx_flags[(m_rx_tail + RX_WORDS - 1 - i) % RX_WORDS] |= FIFO_READY;
	}
}

TIMER_CALLBACK_MEMBER(am79c940_device::receive_tick)
{
	if (!m_rx_busy || m_rx_overflowed)
	{
		return;
	}
	m_rx_wire_position = std::min<unsigned>(m_rx_wire_length, m_rx_wire_position + 8);
	// Hold the last word for EOF and status publication at wire completion.
	unsigned const limit = std::min<unsigned>(m_rx_wire_position, (m_rx_length - 1) & ~1U);
	while (m_rx_position < limit)
	{
		if (m_rx_count == RX_WORDS)
		{
			rx_overflow();
			return;
		}
		rx_push(m_rx_packet[m_rx_position] | (u16(m_rx_packet[m_rx_position + 1]) << 8),
			2 | FIFO_ACTIVE | (m_rx_readable ? FIFO_READY : 0));
		m_rx_position += 2;
	}
	if (m_rx_wire_position >= (BIT(m_reg[RCVFC], 3) ? 12 : 64))
	{
		rx_mark_ready();
	}
	if (m_rx_wire_position < m_rx_wire_length)
	{
		m_receive_timer->adjust(attotime::from_ticks(std::min<unsigned>(8, m_rx_wire_length - m_rx_wire_position), m_bandwidth));
	}
	update_requests();
}

void am79c940_device::rx_append_status(u16 length, u8 flags)
{
	rx_push(length & 0xff, 1 | FIFO_STATUS | FIFO_READY);
	rx_push((length >> 8) | flags, 1 | FIFO_STATUS | FIFO_READY);
	rx_push(m_rx_runt_snapshot, 1 | FIFO_STATUS | FIFO_READY);
	rx_push(m_rx_collision_snapshot, 1 | FIFO_STATUS | FIFO_LAST | FIFO_READY | ((flags & 0x80) ? FIFO_OVERFLOW : 0));
	m_rx_runt_snapshot = m_rx_collision_snapshot = 0;
	++m_rx_frame_count;
}

void am79c940_device::rx_overflow_status()
{
	if (m_rx_overflow_pending && m_rx_count + 5 <= RX_WORDS)
	{
		// Preserve older completed packets. This zero-byte EOF represents the
		// failed frame after its unread fragment has been discarded.
		rx_push(0, FIFO_EOF | FIFO_READY);
		rx_append_status(0, 0x80);
		m_rx_overflow_pending = false;
	}
}

void am79c940_device::rx_overflow()
{
	m_receive_timer->reset();
	m_rx_tail = (m_rx_tail + RX_WORDS - m_rx_current_words) % RX_WORDS;
	m_rx_count -= m_rx_current_words;
	m_rx_current_words = 0;
	m_rx_overflow = m_rx_overflowed = m_rx_overflow_pending = true;
	count_missed_packet();
	rx_overflow_status();
	update_requests();
}

void am79c940_device::rx_finish_address_change()
{
	if (m_reg[IAC] & ADDRCHG)
	{
		m_reg[IAC] &= ~ADDRCHG;
		m_address_access = true;
	}
}

void am79c940_device::recv_complete_cb(int result)
{
	if (!m_rx_busy)
	{
		return;
	}
	m_receive_timer->reset();
	if (!m_rx_overflowed)
	{
		unsigned const remaining = m_rx_length - m_rx_position;
		if (m_rx_count + (remaining + 1) / 2 + 4 > RX_WORDS)
		{
			rx_overflow();
		}
		else
		{
			rx_mark_ready();
			for (unsigned i = 0; i < m_rx_current_words; ++i)
			{
				m_rx_flags[(m_rx_tail + RX_WORDS - 1 - i) % RX_WORDS] &= ~FIFO_ACTIVE;
			}
			while (m_rx_position < m_rx_length)
			{
				unsigned const bytes = std::min<unsigned>(2, m_rx_length - m_rx_position);
				u16 data = m_rx_packet[m_rx_position++];
				if (bytes == 2)
				{
					data |= u16(m_rx_packet[m_rx_position++]) << 8;
				}
				rx_push(data, bytes | FIFO_READY | ((m_rx_position == m_rx_length) ? FIFO_EOF : 0));
			}
			rx_append_status(m_rx_length, m_rx_error);
		}
	}
	m_rx_current_words = 0;
	m_rx_busy = false;
	rx_finish_address_change();
	if (m_tx_loopback == 1)
	{
		pump_transmit();
	}
	update_requests();
}

am79c940_device::fifo_read_result am79c940_device::receive_r(u16 mem_mask, bool direct, bool status_only)
{
	fifo_read_result result;
	unsigned const wanted = bool(mem_mask & 0xff) + bool(mem_mask & 0xff00);
	if (!wanted || !m_rx_count)
	{
		return result;
	}
	u8 const flags = m_rx_flags[m_rx_head];
	bool const status = bool(flags & FIFO_STATUS);
	if (!(flags & FIFO_READY) || (status_only && !status) || (status && !direct && !status_only))
	{
		return result;
	}
	result.valid = true;
	result.status = status;
	result.frame_done = bool(flags & FIFO_LAST);
	if (status)
	{
		result.bytes = 1;
		result.data = m_rx_fifo[m_rx_head];
	}
	else
	{
		// A word read following a byte read may straddle two internal words,
		// but must never straddle data EOF and the first status access.
		for (unsigned i = 0; i < m_rx_count && result.bytes < wanted; ++i)
		{
			unsigned const index = (m_rx_head + i) % RX_WORDS;
			u8 const entry = m_rx_flags[index];
			if (!(entry & FIFO_READY) || (entry & FIFO_STATUS))
			{
				break;
			}
			unsigned const bytes = std::min<unsigned>(entry & 3, wanted - result.bytes);
			for (unsigned j = 0; j < bytes; ++j)
			{
				result.data |= u16(u8(m_rx_fifo[index] >> (8 * j))) << (8 * result.bytes++);
			}
			if (bytes == (entry & 3) && (entry & FIFO_EOF))
			{
				result.eof = true;
				break;
			}
		}
		// An incomplete word at a temporary FIFO boundary cannot be delivered
		// as an odd final transfer: wait for the second byte unless this is EOF.
		if (result.bytes < wanted && !result.eof)
		{
			return fifo_read_result{};
		}
	}
	if (status || result.bytes <= 1)
	{
		result.data = (result.data & 0xff) * 0x0101;
	}
	else if (BIT(m_reg[BIUCC], 6))
	{
		result.data = swapendian_int16(result.data);
	}
	result.valid_mask = mem_mask & 0xffff;
	if (!status && wanted == 2 && result.bytes == 1)
	{
		result.valid_mask = BIT(m_reg[BIUCC], 6) ? 0xff00 : 0x00ff;
	}
	if (!result.bytes)
	{
		result.valid_mask = 0;
	}

	if (!machine().side_effects_disabled())
	{
		unsigned left = result.bytes;
		do
		{
			u8 const entry = m_rx_flags[m_rx_head];
			unsigned const bytes = std::min<unsigned>(left, entry & 3);
			if (status || bytes == (entry & 3))
			{
				m_rx_head = (m_rx_head + 1) % RX_WORDS;
				--m_rx_count;
				if (entry & FIFO_ACTIVE)
				{
					--m_rx_current_words;
				}
			}
			else
			{
				m_rx_fifo[m_rx_head] >>= 8;
				--m_rx_flags[m_rx_head];
			}
			left -= bytes;
		}
		while (left);
		if (result.eof)
		{
			--m_rx_frame_count;
			m_rx_request_started = false;
			m_reg[IR] |= 0x02;
		}
		if (flags & FIFO_OVERFLOW)
		{
			m_rx_overflow = false;
		}
		rx_overflow_status();
		update_requests();
		update_irq();
	}
	return result;
}

am79c940_device::fifo_read_result am79c940_device::rx_dma_r(u16 mem_mask)
{
	return receive_r(mem_mask, true);
}

am79c940_device::fifo_read_result am79c940_device::bus_r(offs_t offset, u16 mem_mask)
{
	offset &= 0x1f;
	if (offset == RCVFIFO)
	{
		return receive_r(mem_mask, false);
	}
	if (offset == RCVFS)
	{
		return receive_r(0xffff, false, true); // Byte registers ignore BE.
	}
	fifo_read_result result;
	if (offset == XMTFIFO)
	{
		return result;
	}
	result.valid = (offset != PADR && offset != LADRF) || address_selected(offset == PADR);
	result.data = u16(read(offset)) * 0x0101;
	result.valid_mask = result.valid ? 0xffff : 0;
	result.bytes = result.valid ? 1 : 0;
	return result;
}

bool am79c940_device::bus_w(offs_t offset, u16 data, u16 mem_mask, bool eof)
{
	offset &= 0x1f;
	if (offset == XMTFIFO)
	{
		return tx_dma_w(data, mem_mask, eof);
	}
	if (offset == RCVFIFO)
	{
		return false;
	}
	bool const valid = (offset != PADR && offset != LADRF) || address_selected(offset == PADR);
	write(offset, BIT(m_reg[BIUCC], 6) ? (data >> 8) : data);
	return valid;
}

void am79c940_device::increment_counter(unsigned reg, u8 event)
{
	if (++m_reg[reg] == 0)
	{
		m_reg[IR] |= event;
		update_irq();
	}
}

void am79c940_device::count_missed_packet()
{
	// Monitoring starts on ENRCV and continues even if software clears ENRCV.
	// Reset/ADDRCHG stop it until ENRCV is set again.
	if (m_mpc_enabled)
	{
		increment_counter(MPC, 0x04);
	}
}

void am79c940_device::count_runt_packet()
{
	increment_counter(RNTPC, 0x08);
	if (m_rx_runt_snapshot != 0xff)
	{
		++m_rx_runt_snapshot;
	}
}

void am79c940_device::count_receive_collision()
{
	increment_counter(RCVCC, 0x10);
	if (m_rx_collision_snapshot != 0xff)
	{
		++m_rx_collision_snapshot;
	}
}

bool am79c940_device::address_selected(bool physical) const
{
	return m_address_access && bool(m_reg[IAC] & (physical ? PHYADDR : LOGADDR));
}

void am79c940_device::advance_address(bool physical)
{
	if (++m_address_index == (physical ? 6 : 8))
	{
		m_address_index = 0;
		m_reg[IAC] &= ~(physical ? PHYADDR : LOGADDR);
		if (physical)
		{
			set_mac(m_padr);
		}
	}
}

u8 am79c940_device::address_r(bool physical)
{
	if (!address_selected(physical))
	{
		return 0; // No DTV; the byte register interface represents this as zero.
	}

	u8 const data = physical ? m_padr[m_address_index] : m_ladrf[m_address_index];
	if (!machine().side_effects_disabled())
	{
		advance_address(physical);
	}
	return data;
}

void am79c940_device::address_w(bool physical, u8 data)
{
	if (address_selected(physical))
	{
		LOGMASKED(LOG_ADDR, "%s[%u] <- %02x\n", physical ? "PADR" : "LADRF", m_address_index, data);
		(physical ? m_padr[m_address_index] : m_ladrf[m_address_index]) = data;
		advance_address(physical);
	}
}

u8 am79c940_device::read(offs_t offset)
{
	u8 data = 0;
	switch (offset & 0x1f)
	{
	case RCVFIFO:
		data = receive_r(0x00ff, false).data;
		break;
	case RCVFS:
		data = receive_r(0x00ff, false, true).data;
		break;
	case XMTFS:
		data = transmit_status_r();
		break;
	case XMTRC:
		data = m_tx_status_count ? m_tx_retry[m_tx_status_head] : 0;
		break;
	case FIFOFC:
		data = (m_rx_frame_count << 4) | m_tx_frame_count;
		break;
	case PR:
		data = (m_tx_status_count ? 0x80 : 0) | (m_tx_drq ? 0x40 : 0) | (m_rx_drq ? 0x20 : 0);
		break;
	case XMTFC: case RCVFC: case IMR: case BIUCC: case FIFOCC:
	case MACCC: case PLSCC: case PHYCC: case IAC: case UTR:
		data = m_reg[offset & 0x1f];
		break;
	case CHIPID_LO:
		data = u16(m_revision) & 0xff;
		break;
	case CHIPID_HI:
		data = u16(m_revision) >> 8;
		break;
	case IR: case MPC: case RNTPC: case RCVCC:
		data = m_reg[offset & 0x1f];
		if (!machine().side_effects_disabled())
		{
			m_reg[offset & 0x1f] = 0;
			if ((offset & 0x1f) == IR)
			{
				update_irq();
			}
		}
		break;
	case PADR:
		data = address_r(true);
		break;
	case LADRF:
		data = address_r(false);
		break;
	default:
		// Reserved and unimplemented production test registers.
		break;
	}
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REG, "read %02x -> %02x\n", offset & 0x1f, data);
	}
	return data;
}

void am79c940_device::write(offs_t offset, u8 data)
{
	LOGMASKED(LOG_REG, "write %02x <- %02x\n", offset & 0x1f, data);
	switch (offset & 0x1f)
	{
	case XMTFIFO:
		// This handler takes a logical byte, unlike the pin-level bus/DMA API.
		tx_dma_w(u16(data) * 0x0101, 0x00ff, false);
		break;
	case XMTFC:
		m_reg[XMTFC] = data & 0x89;
		break;
	case RCVFC:
		m_reg[RCVFC] = data & 0x0d;
		update_requests();
		break;
	case IMR:
		m_reg[IMR] = data;
		update_irq();
		break;
	case BIUCC:
		if (BIT(data, 0))
		{
			device_reset();
		}
		else
		{
			m_reg[BIUCC] = data & 0x70;
			pump_transmit();
			update_requests();
		}
		break;
	case FIFOCC:
		// Programmed watermarks read back immediately, but become active only on
		// the update strobes, which reset the corresponding data FIFO.
		if (BIT(data, 3))
		{
			m_tx_watermark = data >> 6;
			reset_tx_fifo(true);
		}
		if (BIT(data, 2))
		{
			m_rx_watermark = (data >> 4) & 3;
			reset_rx_fifo();
		}
		m_reg[FIFOCC] = data & 0xf3;
		update_requests();
		break;
	case MACCC:
		if (!BIT(data, 1) && BIT(m_reg[MACCC], 1))
		{
			reset_tx_fifo(false); // A fully submitted wire frame finishes normally.
		}
		m_reg[MACCC] = data & 0xef;
		if (m_reg[IAC] & ADDRCHG)
		{
			m_reg[MACCC] &= ~ENRCV;
		}
		if (m_reg[MACCC] & ENRCV)
		{
			m_address_access = false;
			m_mpc_enabled = true;
		}
		pump_transmit();
		update_requests();
		break;
	case PLSCC:
		m_reg[PLSCC] = data & 0x0f;
		break;
	case PHYCC:
		// No external link/polarity inputs yet. Disabling the link test forces
		// link pass; writes cannot directly set LNKFL or REVPOL.
		m_reg[PHYCC] = (data & 0x5f) | (BIT(data, 6) ? 0 : 0x80);
		break;
	case IAC:
		if (data & ADDRCHG)
		{
			m_reg[MACCC] &= ~ENRCV;
			m_mpc_enabled = false;
			m_address_access = !m_rx_busy;
			m_reg[IAC] = m_rx_busy ? ADDRCHG : 0;
		}
		m_reg[IAC] = (m_reg[IAC] & ADDRCHG) | ((data & LOGADDR) ? LOGADDR : (data & PHYADDR));
		m_address_index = 0;
		update_requests();
		break;
	case LADRF:
		address_w(false, data);
		break;
	case PADR:
		address_w(true, data);
		break;
	case UTR:
		// RTRD is sticky, but does not prevent updates to other UTR bits (even
		// RTRE). Reserved test access stays unavailable regardless of RTRE.
		m_reg[UTR] = (m_reg[UTR] & 0x40) | (data & ((m_revision == revision::C0) ? 0xff : 0xfe));
		set_loopback(BIT(data, 2)); // Both internal paths are equivalent at MAC level.
		break;
	default:
		// Ignore read-only/reserved registers.
		break;
	}
}
