// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Saarna
/**********************************************************************

    Atari ST mouse controller emulation

**********************************************************************/

#include "emu.h"
#include "stmouse.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_STMOUSE, a7800_stmouse_device, "a7800_stmouse", "Atari ST mouse")


	// Only 1 button is presently supported. It's not clear at this time if the ST
	// mouse has pull-ups on the second button, which would be required for it to 
	// be reliably used with the 7800 paddle lines.

static INPUT_PORTS_START( a7800_stmouse )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) 	//pin 6
	PORT_BIT( 0x5F, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_stmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_stmouse );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_stmouse_device - constructor
//-------------------------------------------------

a7800_stmouse_device::a7800_stmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_STMOUSE, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_stmousex(*this, "MOUSEX"),
	m_stmousey(*this, "MOUSEY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_stmouse_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - joystick read
//-------------------------------------------------

uint8_t a7800_stmouse_device::a7800_joy_r()
{
	// The ST mouse uses 2 bits of quadrature encoding per axis.
	// The X bits use pins 1 and 2, so encoding uses 0x01 and 0x02.
	// The Y bits use pins 3 and 4, so encoding uses 0x04 and 0x08.

	static const uint8_t axis_lookup_x[4] = { 0x00, 0x02, 0x03, 0x01 };
	static const uint8_t axis_lookup_y[4] = { 0x00, 0x08, 0x0C, 0x04 };

	return m_joy->read() | axis_lookup_x[ m_stmousex->read() & 0x3 ] |
		axis_lookup_y[ ( m_stmousey->read() & 0x3 )]  ;

}

