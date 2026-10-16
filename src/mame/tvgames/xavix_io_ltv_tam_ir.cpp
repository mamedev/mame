// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Infrared link peer for the Let's! TV Play Tamagotchi system. Sends a
    predefined Tamagotchi request as receiver pulses, decodes the console's
    transmitted carrier bursts and validates its acknowledgement. This models
    a single request/response exchange.

    TODO:
    - Measure external transmitter polarity and carrier-burst limits.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ltv_tam_ir.h"

#define LOG_PROTOCOL (1U << 1)

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_TAMAGOTCHI_IR, xavix_tamagotchi_ir_device, "xavix_tamagotchi_ir", "XaviX Tamagotchi Infrared Link")

static INPUT_PORTS_START( xavix_tamagotchi_ir )
	PORT_START("SEND")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Send Tamagotchi Data") PORT_CODE(KEYCODE_T)
		PORT_WRITE_LINE_MEMBER(FUNC(xavix_tamagotchi_ir_device::send_request))
INPUT_PORTS_END

xavix_tamagotchi_ir_device::xavix_tamagotchi_ir_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, XAVIX_TAMAGOTCHI_IR, tag, owner, clock)
	, m_event_out_cb(*this)
	, m_receive_phase(receive_phase::IDLE)
	, m_rx_data(1)
	, m_transmit_burst(false)
	, m_transmit_frame(false)
{
}

ioport_constructor xavix_tamagotchi_ir_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xavix_tamagotchi_ir );
}

void xavix_tamagotchi_ir_device::device_start()
{
	assert(clock());
	m_receive_timer = timer_alloc(FUNC(xavix_tamagotchi_ir_device::receive_tick), this);
	m_transmit_timer = timer_alloc(FUNC(xavix_tamagotchi_ir_device::transmit_timeout), this);

	save_item(NAME(m_receive_packet));
	save_item(NAME(m_transmit_packet));
	save_item(NAME(m_receive_phase));
	save_item(NAME(m_receive_bit));
	save_item(NAME(m_transmit_shift));
	save_item(NAME(m_transmit_bit));
	save_item(NAME(m_transmit_bytes));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_tx_pin));
	save_item(NAME(m_transmit_burst));
	save_item(NAME(m_transmit_frame));
	save_item(NAME(m_transmit_mark_pending));
	save_item(NAME(m_acknowledgement_valid));
	save_item(NAME(m_transmit_burst_start));
	save_item(NAME(m_transmit_last_edge));
	save_item(NAME(m_transmit_mark_end));
}

void xavix_tamagotchi_ir_device::device_reset()
{
	m_receive_timer->adjust(attotime::never);
	m_transmit_timer->adjust(attotime::never);
	m_receive_packet.fill(0);
	m_transmit_packet.fill(0);
	m_receive_phase = receive_phase::IDLE;
	m_receive_bit = 0;
	m_transmit_shift = 0;
	m_transmit_bit = 0;
	m_transmit_bytes = 0;
	m_rx_data = 1;
	m_tx_pin = 0;
	m_transmit_burst = false;
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
	m_acknowledgement_valid = false;
	m_transmit_burst_start = attotime::zero;
	m_transmit_last_edge = attotime::zero;
	m_transmit_mark_end = attotime::zero;
}

void xavix_tamagotchi_ir_device::send_request(int state)
{
	// The optical link has no framing for queued or overlapping exchanges.
	if (!state || m_receive_phase != receive_phase::IDLE || m_transmit_burst || m_transmit_frame)
		return;

	m_receive_packet = REQUEST_PACKET;
	m_receive_bit = 0;
	m_acknowledgement_valid = false;
	start_receive_mark(receive_phase::LEADER_MARK, LEADER_MARK_TICKS);
}

void xavix_tamagotchi_ir_device::start_receive_mark(receive_phase phase, u32 ticks)
{
	m_receive_phase = phase;
	m_rx_data = 0;
	// The demodulator is idle high; event 1 records only each carrier-burst falling edge.
	m_event_out_cb(1);
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
		if (++m_receive_bit == PACKET_BITS)
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
	state = bool(state);
	if (state == m_tx_pin)
		return;

	m_tx_pin = state;
	attotime const now = machine().time();
	if (!m_transmit_burst)
	{
		begin_transmit_burst();
	}
	else
	{
		m_transmit_last_edge = now;
	}

	m_transmit_timer->adjust(attotime::from_ticks(TX_SILENCE_TICKS, clock()));
}

void xavix_tamagotchi_ir_device::begin_transmit_burst()
{
	attotime const now = machine().time();
	// Abandon a partial response after more than the longest valid inter-burst gap.
	if (m_transmit_frame && (now - m_transmit_mark_end).as_ticks(clock()) > TX_FRAME_GAP_MAX_TICKS)
	{
		LOGMASKED(LOG_PROTOCOL, "%s: discarded truncated Tamagotchi IR response\n", machine().describe_context());
		m_transmit_frame = false;
		m_transmit_mark_pending = false;
	}

	if (m_transmit_frame && m_transmit_mark_pending)
	{
		u64 const space_ticks = (now - m_transmit_mark_end).as_ticks(clock());
		append_transmit_bit(space_ticks >= TX_BIT_THRESHOLD_TICKS);
		m_transmit_mark_pending = false;
	}

	m_transmit_burst = true;
	m_transmit_burst_start = now;
	m_transmit_last_edge = now;
}

TIMER_CALLBACK_MEMBER(xavix_tamagotchi_ir_device::transmit_timeout)
{
	if (m_transmit_burst)
	{
		finish_transmit_burst();
	}
	else if (m_transmit_frame)
	{
		LOGMASKED(LOG_PROTOCOL, "%s: discarded truncated Tamagotchi IR response\n", machine().describe_context());
		m_transmit_frame = false;
		m_transmit_mark_pending = false;
	}
}

void xavix_tamagotchi_ir_device::finish_transmit_burst()
{
	if (!m_transmit_burst)
		return;

	m_transmit_burst = false;
	m_transmit_timer->adjust(attotime::never);

	// External transistor polarity and the timer terminal half-cycle have not been measured.
	u64 const mark_ticks = (m_transmit_last_edge - m_transmit_burst_start).as_ticks(clock()) + CARRIER_HALF_TICKS;
	if (!m_transmit_frame)
	{
		if (mark_ticks >= TX_LEADER_MIN_TICKS && mark_ticks <= TX_LEADER_MAX_TICKS)
		{
			m_transmit_packet.fill(0);
			m_transmit_shift = 0;
			m_transmit_bit = 0;
			m_transmit_bytes = 0;
			m_transmit_frame = true;
			m_transmit_mark_end = m_transmit_last_edge;
			m_transmit_timer->adjust(attotime::from_ticks(TX_FRAME_GAP_MAX_TICKS, clock()));
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
		m_transmit_mark_end = m_transmit_last_edge;
		m_transmit_timer->adjust(attotime::from_ticks(TX_FRAME_GAP_MAX_TICKS, clock()));
		return;
	}

	LOGMASKED(LOG_PROTOCOL, "%s: discarded malformed Tamagotchi IR mark (%llu ticks)\n", machine().describe_context(), mark_ticks);
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
}

void xavix_tamagotchi_ir_device::append_transmit_bit(bool bit)
{
	if (m_transmit_bytes >= PACKET_BYTES)
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
	m_acknowledgement_valid = validate_acknowledgement();
	LOGMASKED(LOG_PROTOCOL, "%s: Tamagotchi IR response %s (%u bytes, %u partial bits)\n",
		machine().describe_context(), m_acknowledgement_valid ? "accepted" : "rejected", m_transmit_bytes, m_transmit_bit);
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
}

bool xavix_tamagotchi_ir_device::validate_acknowledgement() const
{
	if (m_transmit_bytes != PACKET_BYTES || m_transmit_bit)
		return false;

	std::array<u8, PACKET_BYTES> expected = REQUEST_PACKET;
	expected[1] = 0x41;
	++expected[PACKET_BYTES - 1];
	return m_transmit_packet == expected;
}
