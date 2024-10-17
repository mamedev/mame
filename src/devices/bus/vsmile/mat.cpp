// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb

#include "emu.h"
#include "mat.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_MAT, vsmile_mat_device, "vsmile_mat", "V.Smile Gym Mat")


//**************************************************************************
//    V.Smile gym mat
//**************************************************************************

vsmile_mat_device::vsmile_mat_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: vsmile_pad_device(mconfig, VSMILE_MAT, tag, owner, clock)
	, m_io_joy(*this, "JOY")
	, m_io_colors(*this, "COLORS")
	, m_io_buttons(*this, "BUTTONS")
{
}

vsmile_mat_device::~vsmile_mat_device()
{
}

void vsmile_mat_device::tx_complete()
{
	// update "joystick"-mapped mat pads
	if ((m_stale & STALE_JOY) != STALE_NONE)
	{
		m_sent_joy = m_io_joy->read();
		if ((m_stale & STALE_YELLOW_RIGHT) != STALE_NONE)
		{
			if (BIT(m_sent_joy, 0))
				uart_tx_fifo_push(0xcb); // yellow
			else if (BIT(m_sent_joy, 1))
				uart_tx_fifo_push(0xcd); // right
			else
				uart_tx_fifo_push(0xc0);
		}
		if ((m_stale & STALE_RED_LEFT) != STALE_NONE)
		{
			if (BIT(m_sent_joy, 2))
				uart_tx_fifo_push(0x8b); // red
			else if (BIT(m_sent_joy, 3))
				uart_tx_fifo_push(0x8d); // left
			else
				uart_tx_fifo_push(0x80);
		}
	}

	// update "color"-mapped mat pads
	if ((m_stale & STALE_COLORS) != STALE_NONE)
	{
		m_sent_colors = m_io_colors->read();
		uart_tx_fifo_push(0x90 | m_sent_colors);
	}

	// update "button"-mapped mat pads
	if ((m_stale & STALE_BUTTONS) != STALE_NONE)
	{
		m_sent_buttons = m_io_buttons->read();
		if (((m_stale & STALE_OK) != STALE_NONE) && BIT(m_sent_buttons, 0))
			uart_tx_fifo_push(0xa1);
		if (((m_stale & STALE_QUIT) != STALE_NONE) && BIT(m_sent_buttons, 1))
			uart_tx_fifo_push(0xa2);
		if (((m_stale & STALE_HELP) != STALE_NONE) && BIT(m_sent_buttons, 2))
			uart_tx_fifo_push(0xa3);
		if (((m_stale & STALE_BLUE) != STALE_NONE) && BIT(m_sent_buttons, 3))
			uart_tx_fifo_push(0xa4);
		if (!m_sent_buttons)
			uart_tx_fifo_push(0xa0);
	}

	// if nothing happens in the next second we'll queue a keep-alive
	if (!m_active)
		LOG("entered active state\n");
	m_idle_timer->adjust(attotime::from_seconds(1));
	m_active = true;
	m_stale = STALE_NONE;
}

INPUT_CHANGED_MEMBER(vsmile_mat_device::mat_joy_changed)
{
	if (m_active)
	{
		if (!is_tx_empty())
		{
			LOG("joy changed while transmission in progress, marking stale\n");
			m_stale |= stale_mat_inputs(param);
		}
		else
		{
			uint8_t const joy = m_io_joy->read();
			if ((joy ^ m_sent_joy) & 0x03)
			{
				if (BIT(joy, 0))
					uart_tx_fifo_push(0xcb); // yellow
				else if (BIT(joy, 1))
					uart_tx_fifo_push(0xcd); // right
				else
					uart_tx_fifo_push(0xc0);
			}
			if ((joy ^ m_sent_joy) & 0x0c)
			{
				if (BIT(joy, 2))
					uart_tx_fifo_push(0x8b); // red
				else if (BIT(joy, 3))
					uart_tx_fifo_push(0x8d); // left
				else
					uart_tx_fifo_push(0x80);
			}
			m_sent_joy = joy;
		}
	}
}

INPUT_CHANGED_MEMBER(vsmile_mat_device::mat_color_changed)
{
	if (m_active)
	{
		if (!is_tx_empty())
		{
			LOG("colors changed while transmission in progress, marking stale\n");
			m_stale |= STALE_COLORS;
		}
		else
		{
			m_sent_colors = m_io_colors->read();
			uart_tx_fifo_push(0x90 | m_sent_colors);
		}
	}
}

INPUT_CHANGED_MEMBER(vsmile_mat_device::mat_button_changed)
{
	if (m_active)
	{
		if (!is_tx_empty())
		{
			LOG("buttons changed while transmission in progress, marking stale\n");
			m_stale |= stale_mat_inputs(param);
		}
		else
		{
			uint8_t const buttons = m_io_buttons->read();
			if (BIT((m_sent_buttons ^ buttons) & buttons, 0))
				uart_tx_fifo_push(0xa1);
			if (BIT((m_sent_buttons ^ buttons) & buttons, 1))
				uart_tx_fifo_push(0xa2);
			if (BIT((m_sent_buttons ^ buttons) & buttons, 2))
				uart_tx_fifo_push(0xa3);
			if (BIT((m_sent_buttons ^ buttons) & buttons, 3))
				uart_tx_fifo_push(0xa4);
			if (!buttons)
				uart_tx_fifo_push(0xa0);
			m_sent_buttons = buttons;
		}
	}
}

static INPUT_PORTS_START( vsmile_mat )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_joy_changed), vsmile_mat_device::STALE_YELLOW_RIGHT) PORT_NAME("Yellow")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_joy_changed), vsmile_mat_device::STALE_YELLOW_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(JOYCODE_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_joy_changed), vsmile_mat_device::STALE_RED_LEFT)     PORT_NAME("Red")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_BUTTON4) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_joy_changed), vsmile_mat_device::STALE_RED_LEFT)     PORT_NAME("Left")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COLORS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(JOYCODE_BUTTON5) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_color_changed), vsmile_mat_device::STALE_COLORS) PORT_NAME("Center")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_BUTTON6) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_color_changed), vsmile_mat_device::STALE_COLORS) PORT_NAME("Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_BUTTON7) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_color_changed), vsmile_mat_device::STALE_COLORS) PORT_NAME("Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(JOYCODE_BUTTON8) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_color_changed), vsmile_mat_device::STALE_COLORS) PORT_NAME("Green")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 )  PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(JOYCODE_BUTTON9)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_button_changed), vsmile_mat_device::STALE_OK)   PORT_NAME("OK")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_CODE(KEYCODE_0_PAD)     PORT_CODE(JOYCODE_BUTTON10) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_button_changed), vsmile_mat_device::STALE_QUIT) PORT_NAME("Quit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(JOYCODE_BUTTON11) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_button_changed), vsmile_mat_device::STALE_HELP) PORT_NAME("Help")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_CODE(KEYCODE_1_PAD)     PORT_CODE(JOYCODE_BUTTON12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vsmile_mat_device::mat_button_changed), vsmile_mat_device::STALE_BLUE) PORT_NAME("Blue")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor vsmile_mat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vsmile_mat );
}
