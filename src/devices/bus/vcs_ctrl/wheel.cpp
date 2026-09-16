// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System Driving Wheel emulation

**********************************************************************/

#include "emu.h"
#include "wheel.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_WHEEL, vcs_wheel_device, "vcs_wheel", "Atari / CBM Driving Wheel")


static INPUT_PORTS_START( vcs_wheel )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_WRITE_LINE_MEMBER(FUNC(vcs_wheel_device::trigger_w)) // Pin 6
	PORT_BIT( 0xdc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vcs_wheel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_wheel );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_wheel_device - constructor
//-------------------------------------------------

vcs_wheel_device::vcs_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_WHEEL, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_wheel(*this, "WHEEL")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_wheel_device::device_start()
{
}


//-------------------------------------------------
//  vcs_joy_r - joystick read
//-------------------------------------------------

uint8_t vcs_wheel_device::vcs_joy_r()
{
	static const uint8_t driving_lookup[4] = { 0x00, 0x02, 0x03, 0x01 };

	return m_joy->read() | driving_lookup[BIT(m_wheel->read(), 3, 2)];
}
