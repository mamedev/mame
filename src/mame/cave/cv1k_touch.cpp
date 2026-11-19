// license:BSD-3-Clause
// copyright-holders:buffi
/***************************************************************************

    Serial touch screen used by the CV1000 medal games

    Medal Mahjong Moukari Bancho is the only CV1000 title that has touch screen
    support.  The panel is an unidentified Japanese controller hanging off SCIF
    channel 2 of the SH7709S at 8N1. Everything below was worked out from the
    game code.

    Startup handshake, each step separated by a short sleep.  If any expected
    reply does not turn up the game closes the port and starts over:

        -> 55              <- 06          reset / attention
        -> 05 45           <- 06          enquiry
        -> 15              <- xx xx       identification, value is only shown
        -> 21                             start reporting

    Once reporting the panel sends:

        10                                pen up
        11 xh xl yh yl                    pen down

    Coordinates are 10 bits, the high bytes carry only the top two bits - the
    game rejects a packet whose high byte is greater than 3 and resynchronises.
    Seven consecutive pen up bytes are needed before the game considers the
    touch released.  An idle line doesn't break the run or time anything out.

    The real panel's report rate is unknown.  The game drains the receive FIFO
    one byte per task tick, slower than a continuous stream arrives at 9600
    baud, so this sends a report when the pen goes down or moves, a burst of
    pen up bytes on release, and repeats the current state after a period
    without changes.

***************************************************************************/

#include "emu.h"
#include "cv1k_touch.h"

#define LOG_COMMAND (1U << 1)
#define LOG_REPORT  (1U << 2)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


// The panel is physically larger than the visible picture, so the screen only
// covers the middle ~80% of the reported range. The limits below were measured
// against mmmbanc's factory calibration by walking the on-screen touch marker,
// and make a click land where the pointer is. The full 10 bit range is still
// representable, the game's own calibration screen just never needs the edges.
static INPUT_PORTS_START( cv1k_touchscreen )
	PORT_START("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Touch Screen") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cv1k_touchscreen_device::input_changed), cv1k_touchscreen_device::INPUT_BUTTON)

	PORT_START("TOUCH_X")
	PORT_BIT( 0x3ff, 0x204, IPT_LIGHTGUN_X ) PORT_MINMAX(0x06e, 0x39b) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cv1k_touchscreen_device::input_changed), cv1k_touchscreen_device::INPUT_AXIS)

	PORT_START("TOUCH_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x069, 0x397) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cv1k_touchscreen_device::input_changed), cv1k_touchscreen_device::INPUT_AXIS)
INPUT_PORTS_END


DEFINE_DEVICE_TYPE(CV1K_TOUCHSCREEN, cv1k_touchscreen_device, "cv1k_touchscreen", "CV1000 Serial Touch Screen")

cv1k_touchscreen_device::cv1k_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CV1K_TOUCHSCREEN, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_txd_cb(*this)
	, m_touch(*this, "TOUCH")
	, m_touch_x(*this, "TOUCH_X")
	, m_touch_y(*this, "TOUCH_Y")
	, m_report_timer(nullptr)
	, m_tx_head(0)
	, m_tx_count(0)
	, m_streaming(false)
	, m_got_enq(false)
	, m_report_pending(false)
{
}

ioport_constructor cv1k_touchscreen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cv1k_touchscreen);
}

void cv1k_touchscreen_device::device_start()
{
	m_report_timer = timer_alloc(FUNC(cv1k_touchscreen_device::report), this);

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(clock());

	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_streaming));
	save_item(NAME(m_got_enq));
	save_item(NAME(m_report_pending));
}

void cv1k_touchscreen_device::device_reset()
{
	std::fill(std::begin(m_tx_buffer), std::end(m_tx_buffer), 0);
	m_tx_head = m_tx_count = 0;
	m_streaming = false;
	m_got_enq = false;
	m_report_pending = false;

	m_report_timer->adjust(attotime::never);

	receive_register_reset();
	transmit_register_reset();
	m_txd_cb(1);
}


//**************************************************************************
//  TRANSMIT QUEUE
//**************************************************************************

void cv1k_touchscreen_device::queue_byte(uint8_t data)
{
	if (m_tx_count >= TX_BUFFER_SIZE)
	{
		LOG("transmit buffer overflow, dropping %02x\n", data);
		return;
	}

	m_tx_buffer[(m_tx_head + m_tx_count) % TX_BUFFER_SIZE] = data;
	m_tx_count++;
}

INPUT_CHANGED_MEMBER(cv1k_touchscreen_device::input_changed)
{
	if (!m_streaming)
		return;

	// Moving the pointer without touching doesn't produce a report.
	if (param == INPUT_AXIS && !BIT(m_touch->read(), 0))
		return;

	// Several fields can change in the same frame, report them once.
	m_report_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(cv1k_touchscreen_device::report)
{
	if (!m_streaming)
		return;

	// Don't let reports pile up behind one that is still being sent,
	// tra_complete() sends it once the queue has drained.
	if (m_tx_count || !is_transmit_register_empty())
	{
		m_report_pending = true;
		return;
	}
	m_report_pending = false;

	if (BIT(m_touch->read(), 0))
	{
		const uint16_t x = m_touch_x->read() & 0x3ff;
		const uint16_t y = m_touch_y->read() & 0x3ff;

		LOGMASKED(LOG_REPORT, "report %03x, %03x\n", x, y);

		queue_byte(RSP_PEN_DWN);
		queue_byte((x >> 8) & 0x03);
		queue_byte(x & 0xff);
		queue_byte((y >> 8) & 0x03);
		queue_byte(y & 0xff);
	}
	else
	{
		LOGMASKED(LOG_REPORT, "pen up\n");
		for (unsigned i = 0; i < RELEASE_PEN_UPS; i++)
			queue_byte(RSP_PEN_UP);
	}
	start_transmit();

	m_report_timer->adjust(attotime::from_msec(IDLE_REPORT_MSEC));
}

void cv1k_touchscreen_device::start_transmit()
{
	if (!is_transmit_register_empty())
		return;

	if (!m_tx_count)
		return;

	const uint8_t data = m_tx_buffer[m_tx_head];
	m_tx_head = (m_tx_head + 1) % TX_BUFFER_SIZE;
	m_tx_count--;

	transmit_register_setup(data);
}


//**************************************************************************
//  SERIAL LINE
//**************************************************************************

void cv1k_touchscreen_device::rxd_w(int state)
{
	device_serial_interface::rx_w(state);
}

void cv1k_touchscreen_device::rcv_complete()
{
	receive_register_extract();

	const uint8_t data = get_received_char();
	const bool got_enq = std::exchange(m_got_enq, false);

	switch (data)
	{
	case CMD_RESET:
		LOGMASKED(LOG_COMMAND, "reset\n");
		m_streaming = false;
		m_report_pending = false;
		m_report_timer->adjust(attotime::never);
		m_tx_head = m_tx_count = 0;
		queue_byte(RSP_ACK);
		break;

	case CMD_ENQ:
		LOGMASKED(LOG_COMMAND, "enquiry\n");
		m_got_enq = true;
		break;

	case CMD_ENQ_E:
		if (got_enq)
		{
			LOGMASKED(LOG_COMMAND, "enquiry ack\n");
			queue_byte(RSP_ACK);
		}
		break;

	case CMD_ID:
		LOGMASKED(LOG_COMMAND, "identify\n");
		// The game only stores these for display, the values are a guess.
		queue_byte(0x01);
		queue_byte(0x00);
		break;

	case CMD_STREAM:
		LOGMASKED(LOG_COMMAND, "start reporting\n");
		m_streaming = true;
		m_report_timer->adjust(attotime::zero);
		break;

	default:
		LOGMASKED(LOG_COMMAND, "unknown command %02x\n", data);
		break;
	}

	start_transmit();
}

void cv1k_touchscreen_device::tra_callback()
{
	m_txd_cb(transmit_register_get_data_bit());
}

void cv1k_touchscreen_device::tra_complete()
{
	start_transmit();

	if (!m_tx_count && m_report_pending)
		m_report_timer->adjust(attotime::zero);
}
