// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Saarna
/**********************************************************************

    Amiga mouse controller

**********************************************************************/

#include "emu.h"
#include "amigamouse.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_AMIGAMOUSE, a7800_amigamouse_device, "a7800_amigamouse", "Amiga mouse")

	// Only 1 button is presently supported. It's not clear at this time if the Amiga
	// mouse has pull-ups on the second and third buttons, which would be required for
	// them to be reliably used with the 7800 paddle lines.

static INPUT_PORTS_START( a7800_amigamouse )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) //pin 6
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

ioport_constructor a7800_amigamouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_amigamouse );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_amigamouse_device - constructor
//-------------------------------------------------

a7800_amigamouse_device::a7800_amigamouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_AMIGAMOUSE, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_amigamousex(*this, "MOUSEX"),
	m_amigamousey(*this, "MOUSEY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_amigamouse_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - joystick read
//-------------------------------------------------

uint8_t a7800_amigamouse_device::a7800_joy_r()
{
	// The Amiga mouse uses 2 bits of quadrature encoding per axis.
	// The X bits use pins 2 and 4, so encoding uses 0x02 and 0x08.
	// The Y bits use pins 1 and 3, so encoding uses 0x01 and 0x04.
	
	static const uint8_t axis_lookup_x[4] = { 0x00, 0x02, 0x0a, 0x08 };
	static const uint8_t axis_lookup_y[4] = { 0x00, 0x01, 0x05, 0x04 };

	return m_joy->read() | axis_lookup_x[ m_amigamousex->read() & 0x3 ] |
		axis_lookup_y[ m_amigamousey->read() & 0x3 ];
}


