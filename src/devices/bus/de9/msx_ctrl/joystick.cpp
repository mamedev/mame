// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

   MSX Digital Joystick emulation

**********************************************************************/

#include "emu.h"
#include "joystick.h"


DEFINE_DEVICE_TYPE(MSX_JOYSTICK, msx_joystick_device, "msx_joystick", "MSX Digital Joystick")


static INPUT_PORTS_START(msx_joystick)
	PORT_START("JOY")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0xff80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor msx_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_joystick);
}

msx_joystick_device::msx_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_JOYSTICK, tag, owner, clock)
	, device_de9_port_interface(mconfig, *this)
	, m_joy(*this, "JOY")
{
}

u16 msx_joystick_device::read()
{
	return m_joy->read();
}
