// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Joystick emulation

**********************************************************************/

#include "joystick.h"



/**********************************************************************
 
   Implementation through the 15-pin controller port (used by AES)
 
 **********************************************************************/


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEOGEO_JOY = &device_creator<neogeo_joystick_device>;


static INPUT_PORTS_START( neogeo_joy )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("START_SELECT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor neogeo_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neogeo_joy );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_joystick_device - constructor
//-------------------------------------------------

neogeo_joystick_device::neogeo_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NEOGEO_JOY, "SNK Neo Geo Joystick", tag, owner, clock, "neogeo_joy", __FILE__),
					device_neogeo_control_port_interface(mconfig, *this),
					m_joy(*this, "JOY"),
					m_ss(*this, "START_SELECT")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_joystick_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neogeo_joystick_device::device_reset()
{
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 neogeo_joystick_device::read_ctrl()
{
    return m_joy->read();
}

//-------------------------------------------------
//  read_start_sel
//-------------------------------------------------

UINT8 neogeo_joystick_device::read_start_sel()
{
	return m_ss->read();
}



/**********************************************************************
 
 Implementation through the edge connector (used by MVS) and 
 connecting two controllers
 
 **********************************************************************/

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEOGEO_JOY_AC = &device_creator<neogeo_joy_ac_device>;


static INPUT_PORTS_START( neogeo_joy_ac )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor neogeo_joy_ac_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neogeo_joy_ac );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_joy_ac_device / neogeo_joystick_device - constructor
//-------------------------------------------------

neogeo_joy_ac_device::neogeo_joy_ac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NEOGEO_JOY_AC, "SNK Neo Geo Arcade Joystick", tag, owner, clock, "neogeo_joyac", __FILE__),
					device_neogeo_ctrl_edge_interface(mconfig, *this),
					m_joy1(*this, "JOY1"),
					m_joy2(*this, "JOY2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_joy_ac_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neogeo_joy_ac_device::device_reset()
{
}


//-------------------------------------------------
//  in0_r
//-------------------------------------------------

READ8_MEMBER(neogeo_joy_ac_device::in0_r)
{
	return m_joy1->read();
}

//-------------------------------------------------
//  in1_r
//-------------------------------------------------

READ8_MEMBER(neogeo_joy_ac_device::in1_r)
{
	return m_joy2->read();
}

