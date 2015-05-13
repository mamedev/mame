// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Joystick Interface

    VTech Laser JS 20
    Dick Smith Electronics X-7315

***************************************************************************/

#include "joystick.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type JOYSTICK_INTERFACE = &device_creator<joystick_interface_device>;

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( joystick )
	PORT_START("joystick_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("joystick_0_arm")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("joystick_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("joystick_1_arm")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor joystick_interface_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( joystick );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  joystick_interface_device - constructor
//-------------------------------------------------

joystick_interface_device::joystick_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, JOYSTICK_INTERFACE, "Laser/VZ Joystick Interface", tag, owner, clock, "joystick", __FILE__),
	device_ioexp_interface(mconfig, *this),
	m_joy0(*this, "joystick_0"),
	m_joy0_arm(*this, "joystick_0_arm"),
	m_joy1(*this, "joystick_1"),
	m_joy1_arm(*this, "joystick_1_arm")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void joystick_interface_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void joystick_interface_device::device_reset()
{
	m_slot->m_io->install_read_handler(0x20, 0x2f, read8_delegate(FUNC(joystick_interface_device::joystick_r), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( joystick_interface_device::joystick_r )
{
	UINT8 data = 0xff;

	if (!BIT(offset, 0)) data &= m_joy0->read();
	if (!BIT(offset, 1)) data &= m_joy0_arm->read();
	if (!BIT(offset, 2)) data &= m_joy1->read();
	if (!BIT(offset, 3)) data &= m_joy1_arm->read();

	return data;
}
