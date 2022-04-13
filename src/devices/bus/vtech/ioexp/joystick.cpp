// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Joystick Interface

    VTech Laser JS 20
    Dick Smith Electronics X-7315

***************************************************************************/

#include "emu.h"
#include "joystick.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_JOYSTICK_INTERFACE, vtech_joystick_interface_device, "vtech_joystick", "Laser/VZ Joystick Interface")

//-------------------------------------------------
//  io_map - memory space address map
//-------------------------------------------------

void vtech_joystick_interface_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x20, 0x2f).r(FUNC(vtech_joystick_interface_device::joystick_r));
}

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

ioport_constructor vtech_joystick_interface_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( joystick );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_joystick_interface_device - constructor
//-------------------------------------------------

vtech_joystick_interface_device::vtech_joystick_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_ioexp_device(mconfig, VTECH_JOYSTICK_INTERFACE, tag, owner, clock),
	m_joy0(*this, "joystick_0"),
	m_joy0_arm(*this, "joystick_0_arm"),
	m_joy1(*this, "joystick_1"),
	m_joy1_arm(*this, "joystick_1_arm")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_joystick_interface_device::device_start()
{
	vtech_ioexp_device::device_start();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t vtech_joystick_interface_device::joystick_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (!BIT(offset, 0)) data &= m_joy0->read();
	if (!BIT(offset, 1)) data &= m_joy0_arm->read();
	if (!BIT(offset, 2)) data &= m_joy1->read();
	if (!BIT(offset, 3)) data &= m_joy1_arm->read();

	return data;
}
