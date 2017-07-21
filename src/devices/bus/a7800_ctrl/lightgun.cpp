// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari lightgun emulation

**********************************************************************/

#include "emu.h"
#include "lightgun.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_LIGHTGUN, a7800_lightgun_device, "a7800_lightgun", "Atari Light Gun")


static INPUT_PORTS_START( a7800_lightgun )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) 
	PORT_BIT( 0xAE, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x50, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_lightgun_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_lightgun );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_lightgun_device - constructor
//-------------------------------------------------

a7800_lightgun_device::a7800_lightgun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_LIGHTGUN, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_lightx(*this, "LIGHTX"),
	m_lighty(*this, "LIGHTY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_lightgun_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - lightgun read
//-------------------------------------------------

uint8_t a7800_lightgun_device::a7800_joy_r()
{
	return m_joy->read();
}

//-------------------------------------------------
//  a7800_lightgun_x_r - lightgun X read
//-------------------------------------------------

uint8_t a7800_lightgun_device::a7800_light_x_r()
{
        return m_lightx->read();
}


//-------------------------------------------------
//  a7800_lightgun_y_r - lightgun Y read
//-------------------------------------------------

uint8_t a7800_lightgun_device::a7800_light_y_r()
{
        return m_lighty->read();
}

