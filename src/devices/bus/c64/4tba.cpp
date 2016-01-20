// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Starbyte Software Tie Break Adaptor emulation

    http://hitmen.c02.at/html/hardware.html

**********************************************************************/

#include "4tba.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_4TBA = &device_creator<c64_4tba_device>;


//-------------------------------------------------
//  INPUT_PORTS( c64_4tba )
//-------------------------------------------------

static INPUT_PORTS_START( c64_4tba )
	PORT_START("SP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_7)

	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_c)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_d)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_e)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_f)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_h)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_j)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_k)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_l)

	PORT_START("PA2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_m)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_4tba_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_4tba );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_4tba_device - constructor
//-------------------------------------------------

c64_4tba_device::c64_4tba_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_4TBA, "C64 Tie Break Adapter", tag, owner, clock, "c64_4tba", __FILE__),
	device_pet_user_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_4tba_device::device_start()
{
}
