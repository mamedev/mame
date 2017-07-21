// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Saarna
/**********************************************************************

    Atari CX22 trackball controller

**********************************************************************/

#include "emu.h"
#include "trackball.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_TRACKBALL, a7800_trackball_device, "a7800_trackball", "Atari CX22 Trackball")


static INPUT_PORTS_START( a7800_trackball )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                     // Pin 6
	PORT_BIT( 0x5F, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKBALLX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5)

	PORT_START("TRACKBALLY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_trackball_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_trackball );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_trackball_device - constructor
//-------------------------------------------------

a7800_trackball_device::a7800_trackball_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_TRACKBALL, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_trackballx(*this, "TRACKBALLX"),
	m_trackbally(*this, "TRACKBALLY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_trackball_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - joystick read
//-------------------------------------------------

uint8_t a7800_trackball_device::a7800_joy_r()
{

	// The CX22 trakball uses 2 bits of traditional trackball encoding per axis.
	//     b0 = the current trackball spin direction for the axis.
	//     b1 = alternates when motion occurs on the axis
	// The X bits use controller pins 1 and 2.
	// The Y bits use controller pins 3 and 4.

	static uint8_t last_x = 0;
	static uint8_t last_y = 0;

	static uint8_t last_xcode = 0;
	static uint8_t last_ycode = 0;

	uint8_t this_x = m_trackballx->read();
	uint8_t this_y = m_trackbally->read();

	if (this_x>last_x)
		last_xcode=(last_xcode^2) | 0x01;
	if (this_x<last_x)
		last_xcode=(last_xcode^2) & 0xfe;

	if (this_y>last_y)
		last_ycode=(last_ycode^2) | 0x01;
	if (this_y<last_y)
		last_ycode=(last_ycode^2) & 0xfe;

	last_x=this_x;
	last_y=this_y;

	return (m_joy->read() | last_xcode | last_ycode<<2);
}



