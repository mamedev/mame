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

DEFINE_DEVICE_TYPE(A7800_WHEEL, a7800_wheel_device, "a7800_wheel", "Atari Driving Wheel")


static INPUT_PORTS_START( a7800_wheel )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 )                     // Pin 6
	PORT_BIT( 0x50, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x8C, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_wheel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_wheel );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_wheel_device - constructor
//-------------------------------------------------

a7800_wheel_device::a7800_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_WHEEL, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_wheel(*this, "WHEEL")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_wheel_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - joystick read
//-------------------------------------------------

uint8_t a7800_wheel_device::a7800_joy_r()
{
	static const uint8_t driving_lookup[4] = { 0x00, 0x02, 0x03, 0x01 };

	return m_joy->read() | driving_lookup[ ( m_wheel->read() & 0x18 ) >> 3 ];
}
