/**********************************************************************

    Starbyte Software Tie Break Adaptor emulation

    http://hitmen.c02.at/html/hardware.html

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_4tba.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_4TBA = &device_creator<c64_4tba_device>;


INPUT_CHANGED_MEMBER( c64_4tba_device::fire3 )
{
	m_slot->sp2_w(newval);
}

static INPUT_PORTS_START( c64_4player )
	PORT_START("SP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, c64_4tba_device, fire3, 0)

	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_4tba_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_4player );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_4tba_device - constructor
//-------------------------------------------------

c64_4tba_device::c64_4tba_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_4TBA, "C64 Tie Break Adapter", tag, owner, clock),
	device_c64_user_port_interface(mconfig, *this),
	m_pb(*this, "PB"),
	m_pa2(*this, "PA2")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_4tba_device::device_start()
{
}


//-------------------------------------------------
//  c64_pb_r - port B read
//-------------------------------------------------

UINT8 c64_4tba_device::c64_pb_r(address_space &space, offs_t offset)
{
	return m_pb->read();
}


//-------------------------------------------------
//  c64_pb_w - port B write
//-------------------------------------------------

int c64_4tba_device::c64_pa2_r()
{
	return BIT(m_pa2->read(), 0);
}


//-------------------------------------------------
//  c64_cnt1_w - CNT 1 write
//-------------------------------------------------

void c64_4tba_device::c64_cnt1_w(int level)
{
	m_slot->cnt2_w(level);
}
