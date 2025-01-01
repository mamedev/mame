// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBS Electronics Booster-Grip Joystick Adaptor

    This peripheral device, which adds two extra buttons to an
    ordinary single-button joystick, was packaged with the Atari 2600
    version of Omega Race. CBS Electronics also advertised it as a
    pack-in for their unreleased 2600 title Wings (it does not work
    with the known prototype). The homebrew Thrust games optionally
    support the Booster-Grip as well.

**********************************************************************/

#include "emu.h"
#include "joybooster.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_JOYSTICK_BOOSTER, vcs_joystick_booster_device, "vcs_joystick_booster", "CBS Electronics Booster-Grip Joystick Adaptor")


static INPUT_PORTS_START( vcs_joystick_booster )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY       // Pin 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY     // Pin 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY     // Pin 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY    // Pin 4
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_WRITE_LINE_MEMBER(FUNC(vcs_joystick_booster_device::trigger_w)) // Pin 6
	PORT_BIT( 0xd0, IP_ACTIVE_LOW, IPT_UNUSED )

	// Pin 5
	PORT_START("POTX")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_BUTTON2 )

	// Pin 9
	PORT_START("POTY")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_BUTTON3 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vcs_joystick_booster_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_joystick_booster );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_joystick_booster_device - constructor
//-------------------------------------------------

vcs_joystick_booster_device::vcs_joystick_booster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_JOYSTICK_BOOSTER, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_potx(*this, "POTX"),
	m_poty(*this, "POTY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_joystick_booster_device::device_start()
{
}


//-------------------------------------------------
//  vcs_joy_r - joystick read
//-------------------------------------------------

uint8_t vcs_joystick_booster_device::vcs_joy_r()
{
	return m_joy->read();
}

uint8_t vcs_joystick_booster_device::vcs_pot_x_r()
{
	return m_potx->read();
}

uint8_t vcs_joystick_booster_device::vcs_pot_y_r()
{
	return m_poty->read();
}
