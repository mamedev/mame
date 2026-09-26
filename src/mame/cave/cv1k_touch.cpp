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

    Once reporting the panel streams continuously:

        10                                pen up, sent repeatedly
        11 xh xl yh yl                    pen down

    Coordinates are 10 bits, the high bytes carry only the top two bits - the
    game rejects a packet whose high byte is greater than 3 and resynchronises.
    Seven consecutive pen up bytes are needed before the game considers the
    touch released, so the stream must not go idle between reports.

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Touch Screen")

	PORT_START("TOUCH_X")
	PORT_BIT( 0x3ff, 0x204, IPT_LIGHTGUN_X ) PORT_MINMAX(0x06e, 0x39b) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("TOUCH_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x069, 0x397) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END


DEFINE_DEVICE_TYPE(CV1K_TOUCHSCREEN, cv1k_touchscreen_device, "cv1k_touchscreen", "CV1000 Serial Touch Screen")

cv1k_touchscreen_device::cv1k_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CV1K_TOUCHSCREEN, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_txd_cb(*this)
	, m_touch(*this, "TOUCH")
	, m_touch_x(*this, "TOUCH_X")
	, m_touch_y(*this, "TOUCH_Y")
	, m_tx_head(0)
	, m_tx_count(0)
	, m_streaming(false)
	, m_got_enq(false)
{
}

ioport_constructor cv1k_touchscreen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cv1k_touchscreen);
}

void cv1k_touchscreen_device::device_start()
{
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_streaming));
	save_item(NAME(m_got_enq));
}

void cv1k_touchscreen_device::device_reset()
{
	std::fill(std::begin(m_tx_buffer), std::end(m_tx_buffer), 0);
	m_tx_head = m_tx_count = 0;
	m_streaming = false;
	m_got_enq = false;

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(clock());

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

void cv1k_touchscreen_device::queue_report()
{
	if (!BIT(m_touch->read(), 0))
	{
		queue_byte(RSP_PEN_UP);
		return;
	}

	const uint16_t x = m_touch_x->read() & 0x3ff;
	const uint16_t y = m_touch_y->read() & 0x3ff;

	LOGMASKED(LOG_REPORT, "report %03x, %03x\n", x, y);

	queue_byte(RSP_PEN_DWN);
	queue_byte((x >> 8) & 0x03);
	queue_byte(x & 0xff);
	queue_byte((y >> 8) & 0x03);
	queue_byte(y & 0xff);
}

void cv1k_touchscreen_device::start_transmit()
{
	if (!is_transmit_register_empty())
		return;

	// The panel never lets the line go idle once it has been told to report.
	if (!m_tx_count && m_streaming)
		queue_report();

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
}
