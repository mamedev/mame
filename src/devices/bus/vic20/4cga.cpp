// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Classical Games/Protovision 4 Player Interface emulation

    http://www.protovision-online.com/hardw/4_player.htm
    http://hitmen.c02.at/html/hardware.html

**********************************************************************/

#include "4cga.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_4CGA = &device_creator<c64_4cga_device>;


static INPUT_PORTS_START( c64_4player )
	PORT_START("JOY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy3_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy3_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy3_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy3_3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_h)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy4_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy4_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy4_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, c64_4cga_device, write_joy4_3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_j)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
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
	device_t(mconfig, C64_4CGA, "C64 Protovision 4 Player Interface", tag, owner, clock, "c64_4cga", __FILE__),
	device_pet_user_port_interface(mconfig, *this),
	m_port(0),
	m_joy3(0xf),
	m_joy4(0xf)
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
//  update_output
//-------------------------------------------------

void c64_4cga_device::update_output()
{
	UINT8 data;

	if (m_port)
	{
		data = m_joy3;
	}
	else
	{
		data = m_joy4;
	}

	output_c((data>>0)&1);
	output_d((data>>1)&1);
	output_e((data>>2)&1);
	output_f((data>>3)&1);
}


//-------------------------------------------------
//  c64_pb_w - port B write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_4cga_device::input_l )
{
	m_port = state;
	update_output();
}
