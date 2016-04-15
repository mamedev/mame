// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Kizuna 4Players Controller emulation

**********************************************************************/

#include "kizuna4p.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEOGEO_KIZ4P = &device_creator<neogeo_kizuna4p_device>;


static INPUT_PORTS_START( neogeo_kiz4p )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Joy A - Up") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Joy A - Down") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Joy A - Left") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Joy A - Right") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Joy A - Button A") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Joy A - Button B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Joy A - Button C") PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Joy A - Button D") PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Joy B - Up") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Joy B - Down") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Joy B - Left") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Joy B - Right") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Joy B - Button A") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Joy B - Button B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Joy B - Button C") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Joy B - Button D") PORT_PLAYER(2)

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor neogeo_kizuna4p_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neogeo_kiz4p );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_kizuna4p_device - constructor
//-------------------------------------------------

neogeo_kizuna4p_device::neogeo_kizuna4p_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NEOGEO_KIZ4P, "SNK Neo Geo Kizuna 4P Controller", tag, owner, clock, "neogeo_kiz4p", __FILE__),
					device_neogeo_control_port_interface(mconfig, *this),
					m_joy1(*this, "JOY1"),
					m_joy2(*this, "JOY2"),
					m_ss(*this, "START")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_kizuna4p_device::device_start()
{
	save_item(NAME(m_ctrl_sel));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neogeo_kizuna4p_device::device_reset()
{
	m_ctrl_sel = 0;
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 neogeo_kizuna4p_device::read_ctrl()
{
	UINT8 res = 0;

	if (m_ctrl_sel & 0x01)
		res = m_joy2->read();
	else
		res = m_joy1->read();
	
	if (m_ctrl_sel & 0x04) res &= ((m_ctrl_sel & 0x01) ? ~0x20 : ~0x10);

	return res;
}


//-------------------------------------------------
//  read_start_sel
//-------------------------------------------------

UINT8 neogeo_kizuna4p_device::read_start_sel()
{
	return BIT(m_ss->read(), m_ctrl_sel & 0x01);
}


//-------------------------------------------------
//  write_ctrlsel
//-------------------------------------------------

void neogeo_kizuna4p_device::write_ctrlsel(UINT8 data)
{
	m_ctrl_sel = data;
}
