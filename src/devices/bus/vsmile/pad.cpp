// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "pad.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_PAD, vsmile_pad_device, "vsmile_pad", "V.Smile Control Pad")


//**************************************************************************
//    V.Smile control pad
//**************************************************************************

vsmile_pad_device::vsmile_pad_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: vsmile_ctrl_device_base(mconfig, VSMILE_PAD, tag, owner, clock)
	, m_io_joy(*this, "JOY")
	, m_io_colors(*this, "COLORS")
	, m_io_buttons(*this, "BUTTONS")
	, m_idle_timer(nullptr)
	, m_ctrl_probe_count(0U)
{
}

vsmile_pad_device::~vsmile_pad_device()
{
}

void vsmile_pad_device::device_start()
{
	vsmile_ctrl_device_base::device_start();

	m_idle_timer = machine().scheduler().timer_alloc(
			timer_expired_delegate(FUNC(vsmile_pad_device::handle_idle), this));
	m_idle_timer->adjust(attotime::from_hz(1));

	save_item(NAME(m_ctrl_probe_history));
	save_item(NAME(m_ctrl_probe_count));
}

void vsmile_pad_device::tx_complete()
{
	m_idle_timer->adjust(attotime::from_hz(1));
}

void vsmile_pad_device::rx_complete(uint8_t data, bool select)
{
	if (select)
	{
		//printf("%s Receiving: %02x\n", tag(), data);
		if ((data >> 4) == 7 || (data >> 4) == 11)
		{
			m_ctrl_probe_history[0] = m_ctrl_probe_history[1];
			m_ctrl_probe_history[1] = data;
			const uint8_t response = ((m_ctrl_probe_history[0] + m_ctrl_probe_history[1] + 0x0f) & 0x0f) ^ 0x05;
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
	queue_tx(0x55);
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_joy_changed)
{
	const uint8_t value = m_io_joy->read();

	if (BIT(value, 2))
		uart_tx_fifo_push(0xcf);
	else if (BIT(value, 3))
		uart_tx_fifo_push(0xc7);
	else
		uart_tx_fifo_push(0xc0);

	if (BIT(value, 0))
		uart_tx_fifo_push(0x87);
	else if (BIT(value, 1))
		uart_tx_fifo_push(0x8f);
	else
		uart_tx_fifo_push(0x80);
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_color_changed)
{
	uart_tx_fifo_push(0x90 | m_io_colors->read());
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_button_changed)
{
	const uint8_t value = m_io_buttons->read();
	const size_t bit = reinterpret_cast<size_t>(param);
	if (BIT(value, bit))
	{
		uart_tx_fifo_push(0xa1 + (uint8_t)bit);
	}
	else
	{
		uart_tx_fifo_push(0xa0);
	}
}

static INPUT_PORTS_START( vsmile_pad )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Right")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COLORS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Green")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Yellow")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Red")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 0) PORT_NAME("OK")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 1) PORT_NAME("Quit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 2) PORT_NAME("Help")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 3) PORT_NAME("ABC")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor vsmile_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vsmile_pad );
}
