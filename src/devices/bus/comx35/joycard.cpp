// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 F&M Joycard emulation

**********************************************************************/

#include "joycard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************




//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_JOY = &device_creator<comx_joy_device>;


//-------------------------------------------------
//  INPUT_PORTS( comx_joy )
//-------------------------------------------------

static INPUT_PORTS_START( comx_joy )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor comx_joy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( comx_joy );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_joy_device - constructor
//-------------------------------------------------

comx_joy_device::comx_joy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_JOY, "COMX JoyCard", tag, owner, clock, "comx_joy", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_joy1(*this, "JOY1"),
	m_joy2(*this, "JOY2")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_joy_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_joy_device::device_reset()
{
}


//-------------------------------------------------
//  comx_mrd_r - I/O read
//-------------------------------------------------

UINT8 comx_joy_device::comx_io_r(address_space &space, offs_t offset)
{
	UINT8 data = 0;

	if (offset == 2)
	{
		data = m_joy1->read();
	}
	else if (offset == 4)
	{
		data = m_joy2->read();
	}

	return data;
}
