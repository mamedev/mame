// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb

#include "emu.h"
#include "pad.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_PAD, vsmile_pad_device, "vsmile_pad", "V.Smile Joystick")


//**************************************************************************
//    V.Smile control pad
//**************************************************************************

DECLARE_ENUM_BITWISE_OPERATORS(vsmile_pad_device::stale_inputs)
ALLOW_SAVE_TYPE(vsmile_pad_device::stale_inputs);

vsmile_pad_device::vsmile_pad_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: vsmile_ctrl_device_base(mconfig, VSMILE_PAD, tag, owner, clock)
	, m_io_joy(*this, "JOY")
	, m_io_colors(*this, "COLORS")
	, m_io_buttons(*this, "BUTTONS")
	, m_idle_timer(nullptr)
	, m_sent_joy(0x00U)
	, m_sent_colors(0x00U)
	, m_sent_buttons(0x00U)
	, m_stale(STALE_ALL)
	, m_active(false)
{
	std::fill(std::begin(m_ctrl_probe_history), std::end(m_ctrl_probe_history), 0U);
}

vsmile_pad_device::~vsmile_pad_device()
{
}

void vsmile_pad_device::device_start()
{
	vsmile_ctrl_device_base::device_start();

	m_idle_timer = machine().scheduler().timer_alloc(
			timer_expired_delegate(FUNC(vsmile_pad_device::handle_idle), this));
	m_idle_timer->adjust(attotime::from_seconds(1));

	m_sent_joy = 0x00U;
	m_sent_colors = 0x00U;
	m_sent_buttons = 0x00U;
	m_stale = STALE_ALL;
	m_active = false;

	save_item(NAME(m_sent_joy));
	save_item(NAME(m_sent_colors));
	save_item(NAME(m_sent_buttons));
	save_item(NAME(m_stale));
	save_item(NAME(m_active));
	save_item(NAME(m_ctrl_probe_history));
}

void vsmile_pad_device::tx_complete()
{
	// update joystick
	if ((m_stale & STALE_JOY) != STALE_NONE)
	{
		m_sent_joy = m_io_joy->read();
		if ((m_stale & STALE_UP_DOWN) != STALE_NONE)
		{
			if (BIT(m_sent_joy, 0))
				uart_tx_fifo_push(0x87); // up
			else if (BIT(m_sent_joy, 1))
				uart_tx_fifo_push(0x8f); // down
			else
				uart_tx_fifo_push(0x80);
		}
		if ((m_stale & STALE_LEFT_RIGHT) != STALE_NONE)
		{
			if (BIT(m_sent_joy, 2))
				uart_tx_fifo_push(0xcf); // left
			else if (BIT(m_sent_joy, 3))
				uart_tx_fifo_push(0xc7); // right
			else
				uart_tx_fifo_push(0xc0);
		}
	}

	// update colors
	if ((m_stale & STALE_COLORS) != STALE_NONE)
	{
		m_sent_colors = m_io_colors->read();
		uart_tx_fifo_push(0x90 | m_sent_colors);
	}

	// update buttons
	if ((m_stale & STALE_BUTTONS) != STALE_NONE)
	{
		m_sent_buttons = m_io_buttons->read();
		if (((m_stale & STALE_OK) != STALE_NONE) && BIT(m_sent_buttons, 0))
			uart_tx_fifo_push(0xa1);
		if (((m_stale & STALE_QUIT) != STALE_NONE) && BIT(m_sent_buttons, 1))
			uart_tx_fifo_push(0xa2);
		if (((m_stale & STALE_HELP) != STALE_NONE) && BIT(m_sent_buttons, 2))
			uart_tx_fifo_push(0xa3);
		if (((m_stale & STALE_ABC) != STALE_NONE) && BIT(m_sent_buttons, 3))
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

void vsmile_pad_device::tx_timeout()
{
	if (m_active)
	{
		m_idle_timer->reset();
		m_active = false;
		m_stale = STALE_ALL;
		std::fill(std::begin(m_ctrl_probe_history), std::end(m_ctrl_probe_history), 0U);
		LOG("left active state\n");
	}
	uart_tx_fifo_push(0x55);
}

void vsmile_pad_device::rx_complete(uint8_t data, bool select)
{
	if (select)
	{
		if (((data & 0xf0) == 0x70) || ((data & 0xf0) == 0xb0))
		{
			m_ctrl_probe_history[0] = ((data & 0xf0) == 0x70) ? 0 : m_ctrl_probe_history[1];
			m_ctrl_probe_history[1] = data;
			uint8_t const response = ((m_ctrl_probe_history[0] + m_ctrl_probe_history[1] + 0x0f) & 0x0f) ^ 0x05;
			LOG(
					"%s: received probe %02X, %02X, sending response %02X\n",
					machine().describe_context(),
					m_ctrl_probe_history[0],
					m_ctrl_probe_history[1],
					0xb0 | response);
			uart_tx_fifo_push(0xb0 | response);
		}
	}
}

void vsmile_pad_device::uart_tx_fifo_push(uint8_t data)
{
	m_idle_timer->reset();
	queue_tx(data);
}

TIMER_CALLBACK_MEMBER(vsmile_pad_device::handle_idle)
{
	LOG("idle timer expired, sending keep-alive 55\n");
	queue_tx(0x55);
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_joy_changed)
{
	if (m_active)
	{
		if (!is_tx_empty())
		{
			LOG("joy changed while transmission in progress, marking stale\n");
			m_stale |= stale_inputs(param);
		}
		else
		{
			uint8_t const joy = m_io_joy->read();
			if ((joy ^ m_sent_joy) & 0x03)
			{
				if (BIT(joy, 0))
					uart_tx_fifo_push(0x87); // up
				else if (BIT(joy, 1))
					uart_tx_fifo_push(0x8f); // down
				else
					uart_tx_fifo_push(0x80);
			}
			if ((joy ^ m_sent_joy) & 0x0c)
			{
				if (BIT(joy, 2))
					uart_tx_fifo_push(0xcf); // left
				else if (BIT(joy, 3))
					uart_tx_fifo_push(0xc7); // right
				else
					uart_tx_fifo_push(0xc0);
			}
			m_sent_joy = joy;
		}
	}
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_color_changed)
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

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_button_changed)
{
	if (m_active)
	{
		if (!is_tx_empty())
		{
			LOG("buttons changed while transmission in progress, marking stale\n");
			m_stale |= stale_inputs(param);
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

static INPUT_PORTS_START( vsmile_pad )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, vsmile_pad_device::STALE_UP_DOWN)    PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, vsmile_pad_device::STALE_UP_DOWN)    PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, vsmile_pad_device::STALE_LEFT_RIGHT) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, vsmile_pad_device::STALE_LEFT_RIGHT) PORT_NAME("Joypad Right")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COLORS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, vsmile_pad_device::STALE_COLORS) PORT_NAME("Green")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, vsmile_pad_device::STALE_COLORS) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, vsmile_pad_device::STALE_COLORS) PORT_NAME("Yellow")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, vsmile_pad_device::STALE_COLORS) PORT_NAME("Red")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, vsmile_pad_device::STALE_OK)   PORT_NAME("OK")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, vsmile_pad_device::STALE_QUIT) PORT_NAME("Quit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, vsmile_pad_device::STALE_HELP) PORT_NAME("Help")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, vsmile_pad_device::STALE_ABC)  PORT_NAME("ABC")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor vsmile_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vsmile_pad );
}
