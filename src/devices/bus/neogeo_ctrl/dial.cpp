// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Dial Controller emulation

**********************************************************************/

#include "emu.h"
#include "dial.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NEOGEO_DIAL, neogeo_dial_device, "neogeo_dial", "SNK Neo Geo Dial Controller")


static INPUT_PORTS_START( neogeo_dial )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("START")
	PORT_BIT( 0xfa, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
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

neogeo_dial_device::neogeo_dial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOGEO_DIAL, tag, owner, clock),
	device_neogeo_ctrl_edge_interface(mconfig, *this),
	m_joy1(*this, "JOY1"),
	m_joy2(*this, "JOY2"),
	m_dial1(*this, "DIAL1"),
	m_dial2(*this, "DIAL2"),
	m_ss(*this, "START")
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
//  in0_r
//-------------------------------------------------

READ8_MEMBER(neogeo_dial_device::in0_r)
{
	uint8_t res = 0;
	if (m_ctrl_sel & 0x01)
		res = m_joy1->read();
	else
		res = m_dial1->read();

	return res;
}

//-------------------------------------------------
//  in1_r
//-------------------------------------------------

READ8_MEMBER(neogeo_dial_device::in1_r)
{
	uint8_t res = 0;
	if (m_ctrl_sel & 0x01)
		res = m_joy2->read();
	else
		res = m_dial2->read();

	return res;
}

//-------------------------------------------------
//  write_ctrlsel
//-------------------------------------------------

void neogeo_dial_device::write_ctrlsel(uint8_t data)
{
	m_ctrl_sel = data;
}

//-------------------------------------------------
//  read_start_sel
//-------------------------------------------------

uint8_t neogeo_dial_device::read_start_sel()
{
	return m_ss->read();
}
