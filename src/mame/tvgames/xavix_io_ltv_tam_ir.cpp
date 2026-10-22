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
	// The implemented packet is a factory request with no identified retail profile.
	// PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Send Tamagotchi Data") PORT_CODE(KEYCODE_T)
	//     PORT_WRITE_LINE_MEMBER(FUNC(xavix_tamagotchi_ir_device::send_request))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

xavix_tamagotchi_ir_device::xavix_tamagotchi_ir_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, XAVIX_TAMAGOTCHI_IR, tag, owner, clock)
	, xavix_ir_receiver_interface(*this, TX_SILENCE_TICKS, false)
	, m_event_out_cb(*this)
	, m_receive_phase(receive_phase::IDLE)
	, m_rx_data(1)
	, m_transmit_frame(false)
{
}

ioport_constructor xavix_tamagotchi_ir_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xavix_tamagotchi_ir );
}

void xavix_tamagotchi_ir_device::device_start()
{
	m_event_out_cb.resolve_safe();

	assert(clock());
	m_receive_timer = timer_alloc(FUNC(xavix_tamagotchi_ir_device::receive_tick), this);

	save_item(NAME(m_receive_packet));
	save_item(NAME(m_transmit_packet));
	save_item(NAME(m_receive_phase));
	save_item(NAME(m_receive_bit));
	save_item(NAME(m_transmit_shift));
	save_item(NAME(m_transmit_bit));
	save_item(NAME(m_transmit_bytes));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_transmit_frame));
	save_item(NAME(m_transmit_mark_pending));
	save_item(NAME(m_acknowledgement_valid));
}

void xavix_tamagotchi_ir_device::device_reset()
{
	m_receive_timer->adjust(attotime::never);
	ir_receiver_reset(true);
	m_receive_packet.fill(0);
	m_transmit_packet.fill(0);
	m_receive_phase = receive_phase::IDLE;
	m_receive_bit = 0;
	m_transmit_shift = 0;
	m_transmit_bit = 0;
	m_transmit_bytes = 0;
	m_rx_data = 1;
	m_transmit_frame = false;
	m_transmit_mark_pending = false;
	m_acknowledgement_valid = false;
}

void xavix_tamagotchi_ir_device::send_request(int state)
{
	// The optical link has no framing for queued or overlapping exchanges.
	if (!state || m_receive_phase != receive_phase::IDLE || ir_receiver_active() || m_transmit_frame)
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
	ir_receiver_w(state);
}

void xavix_tamagotchi_ir_device::ir_burst_started(u64 gap_ticks)
{
	// Abandon a partial response after more than the longest valid inter-burst gap.
	if (m_transmit_frame && gap_ticks > TX_FRAME_GAP_MAX_TICKS)
	{
		LOGMASKED(LOG_PROTOCOL, "%s: discarded truncated Tamagotchi IR response\n", machine().describe_context());
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
		LOGMASKED(LOG_PROTOCOL, "%s: discarded truncated Tamagotchi IR response\n", machine().describe_context());
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
