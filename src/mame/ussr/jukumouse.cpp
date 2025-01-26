// license:BSD-3-Clause
// copyright-holders:Märt Põder

#include "emu.h"
#include "jukumouse.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// measured avg in heavy use 442-520, max ~1400Hz
static constexpr int MOUSE_RATE_HZ = 600;

DEFINE_DEVICE_TYPE(JUKU_MOUSE, juku_mouse_device, "juku_mouse", "Juku E510x mouse")

juku_mouse_device::juku_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JUKU_MOUSE, tag, owner, clock),
	m_mouse_x(*this, "MOUSE_X"),
	m_mouse_y(*this, "MOUSE_Y"),
	m_mouse_b(*this, "BUTTONS"),
	m_int_handler(*this)
{
}

void juku_mouse_device::device_start()
{
	save_item(NAME(m_prev_mouse_y));
	save_item(NAME(m_prev_mouse_x));
	save_item(NAME(m_prev_byte));
	m_mouse_timer = timer_alloc(FUNC(juku_mouse_device::poll_delta), this);
}

void juku_mouse_device::device_reset()
{
	m_prev_mouse_y = 0;
	m_prev_mouse_x = 0;
	m_prev_byte = 0;
	m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(MOUSE_RATE_HZ));
}

/*
 * Calculate positive/negative delta from old and new relative value
 */
inline int delta(int o, int n)
{
	if (o > n) {
		if (o-n < 128) return n - o;
		else return n + 255 - o;
	}
	if (o < n) {
		if (n-o < 128) return n - o;
		else return n - 255 - o;
	}

	return 0;
}

TIMER_CALLBACK_MEMBER( juku_mouse_device::poll_delta )
{
	uint8_t buttons = m_mouse_b->read();
	int x = m_mouse_x->read(), y = m_mouse_y->read();
	int dx = delta(m_prev_mouse_x, x);
	int dy = delta(m_prev_mouse_y, y);

	if (dx != 0 || dy != 0 || (m_prev_byte & 0b11) != buttons) {
		m_int_handler(CLEAR_LINE);
		m_int_handler(ASSERT_LINE);
	}
}

static INPUT_PORTS_START( juku_mouse )
	PORT_START("MOUSE_X")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_CODE(MOUSECODE_X) PORT_SENSITIVITY(23)

	PORT_START("MOUSE_Y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_CODE(MOUSECODE_Y) PORT_SENSITIVITY(23)

	PORT_START("BUTTONS")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Left Button")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Right Button")
INPUT_PORTS_END

uint8_t juku_mouse_device::mouse_port_r()
{
	// 7-------  always 1
	// -6------  always 0
	// --5-----  vertical active
	// ---4----  vertical direction
	// ----3---  horizontal active
	// -----2--  horizontal direction
	// ------1-  left button
	// -------0  right button

	uint8_t data = 0b10000000 | m_mouse_b->read();
	int x = m_mouse_x->read(), y = m_mouse_y->read();
	int dx = delta(m_prev_mouse_x, x);
	int dy = delta(m_prev_mouse_y, y);

	if (dx != 0) {
		data |= (dx > 0) ? 0b00001000 : 0b00001100;
		m_prev_mouse_x = x;
	}

	if (dy != 0) {
		data |= (dy > 0) ? 0b00110000 : 0b00100000;
		m_prev_mouse_y = y;
	}

	m_prev_byte = data;

	return data;
}

ioport_constructor juku_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( juku_mouse );
}
