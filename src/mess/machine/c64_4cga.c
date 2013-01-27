/**********************************************************************

    Classical Games/Protovision 4 Player Interface emulation

    http://www.protovision-online.com/hardw/4_player.htm
    http://hitmen.c02.at/html/hardware.html

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_4cga.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_4CGA = &device_creator<c64_4cga_device>;


static INPUT_PORTS_START( c64_4player )
	PORT_START("JOY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FIRE")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_4cga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_4player );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_4cga_device - constructor
//-------------------------------------------------

c64_4cga_device::c64_4cga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_4CGA, "C64 Protovision 4 Player Interface", tag, owner, clock),
	device_c64_user_port_interface(mconfig, *this),
	m_fire(*this, "FIRE"),
	m_joy3(*this, "JOY3"),
	m_joy4(*this, "JOY4"),
	m_port(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_4cga_device::device_start()
{
	// state saving
	save_item(NAME(m_port));
}


//-------------------------------------------------
//  c64_pb_r - port B read
//-------------------------------------------------

UINT8 c64_4cga_device::c64_pb_r(address_space &space, offs_t offset)
{
	UINT8 data = m_fire->read();

	if (m_port)
	{
		data &= m_joy3->read();
	}
	else
	{
		data &= m_joy4->read();
	}

	return data;
}


//-------------------------------------------------
//  c64_pb_w - port B write
//-------------------------------------------------

void c64_4cga_device::c64_pb_w(address_space &space, offs_t offset, UINT8 data)
{
	m_port = BIT(data, 7);
}
