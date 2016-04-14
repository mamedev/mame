// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Dial Controller emulation

**********************************************************************/

#include "dial.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEOGEO_DIAL = &device_creator<neogeo_dial_device>;


static INPUT_PORTS_START( neogeo_dial )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor neogeo_dial_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neogeo_dial );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_dial_device - constructor
//-------------------------------------------------

neogeo_dial_device::neogeo_dial_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NEOGEO_DIAL, "SNK Neo Geo Dial Controller", tag, owner, clock, "neogeo_dial", __FILE__),
					device_neogeo_control_port_interface(mconfig, *this),
					m_joy(*this, "JOY"),
					m_dial(*this, "DIAL")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_dial_device::device_start()
{
	save_item(NAME(m_ctrl_sel));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neogeo_dial_device::device_reset()
{
	m_ctrl_sel = 0;
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 neogeo_dial_device::read_ctrl()
{
	UINT8 res = 0;
	if (m_ctrl_sel & 0x01)
		res = m_joy->read();
	else
		res = m_dial->read();
	
	return res;
}

//-------------------------------------------------
//  write_ctrlsel
//-------------------------------------------------

void neogeo_dial_device::write_ctrlsel(UINT8 data)
{
	m_ctrl_sel = data;
}

