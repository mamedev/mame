// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Saarna
/**********************************************************************

    Atari 7800 Prosystem joystick emulation

**********************************************************************/

#include "emu.h"
#include "joyproline.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_JOYSTICK_PROLINE, a7800_joystick_proline_device, "a7800_joystick_proline", "Atari 7800 Proline joystick")


static INPUT_PORTS_START( a7800_joystick_proline )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) //PROLINE A
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) //PROLINE B
	PORT_BIT( 0xA0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_joystick_proline_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_joystick_proline );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_joystick_proline_device - constructor
//-------------------------------------------------

a7800_joystick_proline_device::a7800_joystick_proline_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_JOYSTICK_PROLINE, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_joystick_proline_device::device_start()
{
}


//-------------------------------------------------
//  a7800_joy_r - joystick_proline read
//-------------------------------------------------

uint8_t a7800_joystick_proline_device::a7800_joy_r()
{
	return m_joy->read();
}
