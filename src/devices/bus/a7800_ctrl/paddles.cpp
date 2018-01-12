// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari analog paddles emulation

**********************************************************************/

#include "emu.h"
#include "paddles.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_PADDLES, a7800_paddles_device, "a7800_paddles", "Atari paddles")


static INPUT_PORTS_START( a7800_paddles )
	PORT_START("JOY")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("POTX")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(1) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)

	PORT_START("POTY")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(2) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_paddles_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_paddles );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_paddles_device - constructor
//-------------------------------------------------

a7800_paddles_device::a7800_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_PADDLES, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_potx(*this, "POTX"),
	m_poty(*this, "POTY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_paddles_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - joystick read
//-------------------------------------------------

uint8_t a7800_paddles_device::a7800_joy_r()
{
	return m_joy->read();
}


//-------------------------------------------------
//  a7800_pot_x_r - potentiometer X read
//-------------------------------------------------

uint8_t a7800_paddles_device::a7800_pot_x_r()
{
	return m_potx->read();
}


//-------------------------------------------------
//  a7800_pot_y_r - potentiometer Y read
//-------------------------------------------------

uint8_t a7800_paddles_device::a7800_pot_y_r()
{
	return m_poty->read();
}
