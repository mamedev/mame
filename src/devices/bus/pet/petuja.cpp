// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore PET userport joystick adapter emulation

    http://zimmers.net/cbmpics/cbm/PETx/petfaq.html

**********************************************************************/

#include "petuja.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PET_USERPORT_JOYSTICK_ADAPTER = &device_creator<pet_userport_joystick_adapter_device>;


//-------------------------------------------------
//  INPUT_PORTS( petuja )
//-------------------------------------------------

static INPUT_PORTS_START( petuja )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_adapter_device, write_up1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_adapter_device, write_down1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_e)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_f)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_adapter_device, write_up2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_adapter_device, write_down2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_k)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_l)

	PORT_START("FIRE")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_adapter_device, write_fire1)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_adapter_device, write_fire2)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pet_userport_joystick_adapter_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( petuja );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_userport_joystick_adapter_device - constructor
//-------------------------------------------------

pet_userport_joystick_adapter_device::pet_userport_joystick_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PET_USERPORT_JOYSTICK_ADAPTER, "PET Userport Joystick Adapter", tag, owner, clock, "petuja", __FILE__),
	device_pet_user_port_interface(mconfig, *this),
	m_up1(1),
	m_down1(1),
	m_fire1(1),
	m_up2(1),
	m_down2(1),
	m_fire2(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_userport_joystick_adapter_device::device_start()
{
}


//-------------------------------------------------
//  update_port1
//-------------------------------------------------

void pet_userport_joystick_adapter_device::update_port1()
{
	printf( "update port1\n" );
	output_c(m_up1 && m_fire1);
	output_d(m_down1 && m_fire1);
}


//-------------------------------------------------
//  update_port2
//-------------------------------------------------

void pet_userport_joystick_adapter_device::update_port2()
{
	printf( "update port2\n" );
	output_h(m_up2 && m_fire2);
	output_j(m_down2 && m_fire2);
}
