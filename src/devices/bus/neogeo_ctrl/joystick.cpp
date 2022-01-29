// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Joystick emulation

**********************************************************************/

#include "emu.h"
#include "joystick.h"



/**********************************************************************

   Implementation through the 15-pin controller port (used by AES)

 **********************************************************************/


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NEOGEO_JOY, neogeo_joystick_device, "neogeo_joy", "SNK Neo Geo Joystick")


static INPUT_PORTS_START( neogeo_joy )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("%p C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("%p D")

	PORT_START("START_SELECT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
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

neogeo_joystick_device::neogeo_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOGEO_JOY, tag, owner, clock),
	device_neogeo_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_ss(*this, "START_SELECT"),
	m_ctrl_sel(0x00)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_joystick_device::device_start()
{
	save_item(NAME(m_ctrl_sel));
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

uint8_t neogeo_joystick_device::read_ctrl()
{
	return m_joy->read() & (BIT(m_ctrl_sel, 2) ? 0x7f : 0xff);
}

//-------------------------------------------------
//  read_start_sel
//-------------------------------------------------

uint8_t neogeo_joystick_device::read_start_sel()
{
	return m_ss->read();
}

//-------------------------------------------------
//  write_ctrlsel
//-------------------------------------------------

void neogeo_joystick_device::write_ctrlsel(uint8_t data)
{
	m_ctrl_sel = data;
}



/**********************************************************************

 Implementation through the edge connector (used by MVS) and
 connecting two controllers

 **********************************************************************/

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NEOGEO_JOY_AC, neogeo_joy_ac_device, "neogeo_joyac", "SNK Neo Geo Arcade Joystick")


static INPUT_PORTS_START( neogeo_joy_ac )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("%p A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("%p B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("%p C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("%p D")

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("%p A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("%p B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("%p C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("%p D")

	PORT_START("START")
	PORT_BIT( 0xfa, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
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

neogeo_joy_ac_device::neogeo_joy_ac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOGEO_JOY_AC, tag, owner, clock),
	device_neogeo_ctrl_edge_interface(mconfig, *this),
	m_joy(*this, "JOY%u", 1U),
	m_ss(*this, "START")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_joy_ac_device::device_start()
{
}


//-------------------------------------------------
//  in0_r
//-------------------------------------------------

uint8_t neogeo_joy_ac_device::in0_r()
{
	return m_joy[0]->read();
}

//-------------------------------------------------
//  in1_r
//-------------------------------------------------

uint8_t neogeo_joy_ac_device::in1_r()
{
	return m_joy[1]->read();
}

//-------------------------------------------------
//  read_start_sel
//-------------------------------------------------

uint8_t neogeo_joy_ac_device::read_start_sel()
{
	return m_ss->read();
}
