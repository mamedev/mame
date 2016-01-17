// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System Driving Wheel emulation

**********************************************************************/

#include "wheel.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VCS_WHEEL = &device_creator<vcs_wheel_device>;


static INPUT_PORTS_START( vcs_wheel )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                     // Pin 6
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

vcs_wheel_device::vcs_wheel_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VCS_WHEEL, "Atari / CBM Driving Wheel", tag, owner, clock, "vcs_wheel", __FILE__),
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

UINT8 vcs_wheel_device::vcs_joy_r()
{
	static const UINT8 driving_lookup[4] = { 0x00, 0x02, 0x03, 0x01 };

	return m_joy->read() | driving_lookup[ ( m_wheel->read() & 0x18 ) >> 3 ];
}
