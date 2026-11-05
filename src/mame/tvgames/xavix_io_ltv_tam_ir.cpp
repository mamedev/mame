// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Infrared transceiver for the Let's! TV Play Tamagotchi system. A bitbanger
    image connects an external peer using frames with a two-byte big-endian
    length and 1-256 payload bytes. The device converts frames to timed receiver
    pulses and transmitted carrier bursts back to frames. Application payloads,
    checksums and acknowledgements belong to the ROM and external peer.

    TODO:
    - Measure external transmitter polarity and carrier-burst limits.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ltv_tam_ir.h"

#define LOG_PROTOCOL (1U << 1)

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_TAMAGOTCHI_IR, xavix_tamagotchi_ir_device, "xavix_tamagotchi_ir", "XaviX Tamagotchi Infrared Link")

xavix_tamagotchi_ir_device::xavix_tamagotchi_ir_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, XAVIX_TAMAGOTCHI_IR, tag, owner, clock)
	, xavix_ir_receiver_interface(*this, TX_SILENCE_TICKS, false)
	, m_stream(*this, "stream")
	, m_event_out_cb(*this)
	, m_receive_phase(receive_phase::IDLE)
	, m_rx_data(1)
	, m_transmit_frame(false)
{
}

void xavix_tamagotchi_ir_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_stream);
}

void xavix_tamagotchi_ir_device::device_start()
{
	m_event_out_cb.resolve_safe();

	assert(clock());
	m_poll_timer = timer_alloc(FUNC(xavix_tamagotchi_ir_device::poll_stream), this);
	m_receive_timer = timer_alloc(FUNC(xavix_tamagotchi_ir_device::receive_tick), this);
	m_media_notifier = m_stream->add_media_change_notifier(
		[this] (device_image_interface::media_change_event) { reset_link(); });

	save_item(NAME(m_receive_packet));
	save_item(NAME(m_transmit_packet));
	save_item(NAME(m_stream_length));
	save_item(NAME(m_stream_bytes));
	save_item(NAME(m_stream_header_bytes));
	save_item(NAME(m_receive_phase));
	save_item(NAME(m_receive_length));
	save_item(NAME(m_receive_bit));
	save_item(NAME(m_transmit_shift));
	save_item(NAME(m_transmit_bit));
	save_item(NAME(m_transmit_bytes));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_transmit_frame));
	save_item(NAME(m_transmit_mark_pending));
}

void xavix_tamagotchi_ir_device::device_reset()
{
	reset_link();
	ir_receiver_reset(true);
}

void xavix_tamagotchi_ir_device::reset_link()
{
	m_poll_timer->adjust(attotime::zero);
	m_receive_timer->adjust(attotime::never);
	ir_receiver_reset(false);
	m_receive_packet.fill(0);
	m_transmit_packet.fill(0);
	m_stream_length = 0;
	m_stream_bytes = 0;
	m_stream_header_bytes = 0;
	m_receive_phase = receive_phase::IDLE;
	m_receive_length = 0;
	m_receive_bit = 0;
	m_transmit_shift = 0;
	m_transmit_bit = 0;
	m_transmit_bytes = 0;
	m_rx_data = 1;
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
}

TIMER_CALLBACK_MEMBER(xavix_tamagotchi_ir_device::poll_stream)
{
	// Polling assembles one complete frame; socket arrival times do not time optical edges.
	m_poll_timer->adjust(attotime::from_msec(1));
	if (m_receive_phase != receive_phase::IDLE || ir_receiver_active() || m_transmit_frame)
		return;

	// Bound work per poll, including when discarding an oversized frame.
	for (unsigned count = 0; count < MAX_PACKET_BYTES + 2; count++)
	{
		u8 data;
		if (!m_stream->input(&data, 1))
			return;

		if (m_stream_header_bytes < 2)
		{
			m_stream_length = (m_stream_length << 8) | data;
			if (++m_stream_header_bytes == 2 && !m_stream_length)
				m_stream_header_bytes = 0;
			continue;
		}

		if (m_stream_length <= m_receive_packet.size())
			m_receive_packet[m_stream_bytes] = data;
		if (++m_stream_bytes != m_stream_length)
			continue;

		m_stream_header_bytes = 0;
		m_stream_bytes = 0;
		if (m_stream_length > m_receive_packet.size())
		{
			LOGMASKED(LOG_PROTOCOL, "Discarded oversized infrared stream frame (%u bytes)\n", m_stream_length);
			m_stream_length = 0;
			continue;
		}

		m_receive_length = m_stream_length;
		m_stream_length = 0;
		m_receive_bit = 0;
		start_receive_mark(receive_phase::LEADER_MARK, LEADER_MARK_TICKS);
		return;
	}
}

void xavix_tamagotchi_ir_device::start_receive_mark(receive_phase phase, u32 ticks)
{
	m_receive_phase = phase;
	m_rx_data = 0;
	// The demodulator is idle high; event 1 records only each carrier-burst falling edge.
	m_event_out_cb();
	m_receive_timer->adjust(attotime::from_ticks(ticks, clock()));
}

u32 xavix_tamagotchi_ir_device::receive_space_ticks() const
{
	return BIT(m_receive_packet[m_receive_bit >> 3], m_receive_bit & 7) ? ONE_SPACE_TICKS : ZERO_SPACE_TICKS;
}

TIMER_CALLBACK_MEMBER(xavix_tamagotchi_ir_device::receive_tick)
{
	switch (m_receive_phase)
	{
	case receive_phase::LEADER_MARK:
		m_rx_data = 1;
		m_receive_phase = receive_phase::LEADER_SPACE;
		m_receive_timer->adjust(attotime::from_ticks(LEADER_SPACE_TICKS, clock()));
		break;

	case receive_phase::LEADER_SPACE:
		start_receive_mark(receive_phase::DATA_MARK, DATA_MARK_TICKS);
		break;

	case receive_phase::DATA_MARK:
		m_rx_data = 1;
		m_receive_phase = receive_phase::DATA_SPACE;
		m_receive_timer->adjust(attotime::from_ticks(receive_space_ticks(), clock()));
		break;

	case receive_phase::DATA_SPACE:
		if (++m_receive_bit == m_receive_length * 8)
			start_receive_mark(receive_phase::TRAILER_MARK, TRAILER_MARK_TICKS);
		else
			start_receive_mark(receive_phase::DATA_MARK, DATA_MARK_TICKS);
		break;

	case receive_phase::TRAILER_MARK:
		m_rx_data = 1;
		m_receive_phase = receive_phase::IDLE;
		m_receive_timer->adjust(attotime::never);
		break;

	case receive_phase::IDLE:
		m_receive_timer->adjust(attotime::never);
		break;
	}
}

void xavix_tamagotchi_ir_device::tx_w(int state)
{
	ir_receiver_w(state);
}

void xavix_tamagotchi_ir_device::ir_burst_started(u64 gap_ticks)
{
	// Abandon a partial frame after more than the longest valid inter-burst gap.
	if (m_transmit_frame && gap_ticks > TX_FRAME_GAP_MAX_TICKS)
	{
		LOGMASKED(LOG_PROTOCOL, "%s: discarded truncated Tamagotchi IR frame\n", machine().describe_context());
		m_transmit_frame = false;
		m_transmit_mark_pending = false;
	}

	if (m_transmit_frame && m_transmit_mark_pending)
	{
		append_transmit_bit(gap_ticks >= TX_BIT_THRESHOLD_TICKS);
		m_transmit_mark_pending = false;
	}
}

void xavix_tamagotchi_ir_device::ir_idle_timeout()
{
	if (m_transmit_frame)
	{
		LOGMASKED(LOG_PROTOCOL, "%s: discarded truncated Tamagotchi IR frame\n", machine().describe_context());
		m_transmit_frame = false;
		m_transmit_mark_pending = false;
	}
}

void xavix_tamagotchi_ir_device::ir_burst_finished(u64 width_ticks)
{
	// External transistor polarity and the timer terminal half-cycle have not been measured.
	u64 const mark_ticks = width_ticks + CARRIER_HALF_TICKS;
	if (!m_transmit_frame)
	{
		if (mark_ticks >= TX_LEADER_MIN_TICKS && mark_ticks <= TX_LEADER_MAX_TICKS)
		{
			m_transmit_packet.fill(0);
			m_transmit_shift = 0;
			m_transmit_bit = 0;
			m_transmit_bytes = 0;
			m_transmit_frame = true;
			ir_receiver_wait(TX_FRAME_GAP_MAX_TICKS);
		}
		return;
	}

	if (mark_ticks >= TX_TRAILER_MIN_TICKS)
	{
		finish_transmit_packet();
		return;
	}

	if (mark_ticks >= TX_DATA_MARK_MIN_TICKS && mark_ticks <= TX_DATA_MARK_MAX_TICKS)
	{
		m_transmit_mark_pending = true;
		ir_receiver_wait(TX_FRAME_GAP_MAX_TICKS);
		return;
	}

	LOGMASKED(LOG_PROTOCOL, "%s: discarded malformed Tamagotchi IR mark (%llu ticks)\n", machine().describe_context(), mark_ticks);
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
}

void xavix_tamagotchi_ir_device::append_transmit_bit(bool bit)
{
	if (m_transmit_bytes >= m_transmit_packet.size())
	{
		m_transmit_frame = false;
		return;
	}

	if (bit)
		m_transmit_shift |= u8(1U << m_transmit_bit);

	if (++m_transmit_bit == 8)
	{
		m_transmit_packet[m_transmit_bytes++] = m_transmit_shift;
		m_transmit_shift = 0;
		m_transmit_bit = 0;
	}
}

void xavix_tamagotchi_ir_device::finish_transmit_packet()
{
	if (m_transmit_bytes && !m_transmit_bit)
	{
		m_stream->output(u8(m_transmit_bytes >> 8));
		m_stream->output(u8(m_transmit_bytes));
		for (unsigned index = 0; index < m_transmit_bytes; index++)
			m_stream->output(m_transmit_packet[index]);
	}
	LOGMASKED(LOG_PROTOCOL, "%s: Tamagotchi IR frame (%u bytes, %u partial bits)\n",
		machine().describe_context(), m_transmit_bytes, m_transmit_bit);
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
}
