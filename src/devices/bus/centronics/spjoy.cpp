// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    The Serial Port / Vertical Twist Joystick Interface

    Notes:
    - Test cases for this joystick are Paradroid 2000 (paradr2k) and Blowpipe (blowpipe).

***************************************************************************/

#include "emu.h"
#include "spjoy.h"

DEFINE_DEVICE_TYPE(SERIAL_PORT_JOYSTICK, serial_port_joystick_device, "spjoy", "The Serial Port/Vertical Twist Joystick Interface")

serial_port_joystick_device::serial_port_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_PORT_JOYSTICK, tag, owner, clock)
	, device_centronics_peripheral_interface( mconfig, *this )
	, m_joy(*this, "joy_p%u", 1U)
	, m_data(0xff)
	, m_busy(0)
	, m_ack(0)
{
}

void serial_port_joystick_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_busy));
	save_item(NAME(m_ack));
}

void serial_port_joystick_device::update_busy_ack()
{
	int busy = 0;
	int ack = 0;

	if (~m_data & 0x7f)
	{
		if ((~m_data & 0x5f) & (~m_joy[0]->read() & 0x5f))
		{
			busy = 1;
		}
		if ((~m_data & 0x5f) & (~m_joy[1]->read() & 0x5f))
		{
			ack = 1;
		}
		if (~m_data & 0x20)
		{
			busy = ack = 1;
		}
	}

	if (m_busy != busy)
	{
		m_busy = busy;
		output_busy(busy);
	}

	if (m_ack != ack)
	{
		m_ack = ack;
		output_ack(!ack);
	}
}


static INPUT_PORTS_START( serial_port_joystick )
	PORT_START("joy_p1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("joy_p2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor serial_port_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( serial_port_joystick );
}
