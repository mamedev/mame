// license:BSD-3-Clause
// copyright-holders: Ken White
/**********************************************************************

    Commodore PET user port dual joystick and sound device emulation

    BEST OF THE PET GAZETTE magazine documents attaching Atari-style joysticks
    with DB-9S connectors to a DB-25P connector plugged into the user port.
    page 42, PET AND THE DUAL JOYSTICKS by Chuck Johnson
    page 47, STANDARD #1 (CB2 Sound)

**********************************************************************/

/*

Connections
----------
    User      Joysticks
    Port      ____
             /    |
    F ----- |   1 | joy 1 up
      > --- | 6   | joy 1 button
    E ----- |   2 | joy 1 down
            | 7   |
    D ----- |   3 | joy 1 left
    A ----- | 8   | ground
    C ----- |   4 | joy 1 right
            | 9   |
            |   5 |
             \____|
              ____
             /    |
    L ----- |   1 | joy 2 up
      > --- | 6   | joy 2 button
    K ----- |   2 | joy 2 down
            | 7   |
    J ----- |   3 | joy 2 left
    N ----- | 8   | ground
    H ----- |   4 | joy 2 right
            | 9   |
            |   5 |
             \____|

    M ----- / audio
    N ----- \ amplifier

*/

#include "2joysnd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PET_USERPORT_JOYSTICK_AND_SOUND_DEVICE = &device_creator<pet_userport_joystick_and_sound_device>;

#define DAC_TAG         "dac"

MACHINE_CONFIG_FRAGMENT( 2joysnd )
	MCFG_SPEAKER_STANDARD_MONO("cb2spkr")
	MCFG_SOUND_ADD(DAC_TAG, DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cb2spkr", 1.00)
MACHINE_CONFIG_END

//-------------------------------------------------
//  INPUT_PORTS( 2joysnd )
//-------------------------------------------------

static INPUT_PORTS_START( 2joysnd )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_c)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_d)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_and_sound_device, write_down1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_and_sound_device, write_up1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_h)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, device_pet_user_port_interface, output_j)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_and_sound_device, write_down2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_and_sound_device, write_up2)

	PORT_START("FIRE")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_and_sound_device, write_fire1)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, pet_userport_joystick_and_sound_device, write_fire2)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pet_userport_joystick_and_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 2joysnd );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pet_userport_joystick_and_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( 2joysnd );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_user_port_dual_joystick_and_sound_device - constructor
//-------------------------------------------------

pet_userport_joystick_and_sound_device::pet_userport_joystick_and_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PET_USERPORT_JOYSTICK_AND_SOUND_DEVICE, "Dual Joysticks and Sound", tag, owner, clock, "2joysnd", __FILE__),
	device_pet_user_port_interface(mconfig, *this),
	m_dac(*this, DAC_TAG),
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

void pet_userport_joystick_and_sound_device::device_start()
{
}


//-------------------------------------------------
//  update_port1
//-------------------------------------------------

void pet_userport_joystick_and_sound_device::update_port1()
{
	output_f(m_up1 && m_fire1);
	output_e(m_down1 && m_fire1);
}


//-------------------------------------------------
//  update_port2
//-------------------------------------------------

void pet_userport_joystick_and_sound_device::update_port2()
{
	output_l(m_up2 && m_fire2);
	output_k(m_down2 && m_fire2);
}


DECLARE_WRITE_LINE_MEMBER( pet_userport_joystick_and_sound_device::input_m )
{
	m_dac->write_unsigned8(state ? 0xff : 0x00);
}
