// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Compact Joystick

**********************************************************************/

#include "emu.h"
#include "joystick.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBCMC_JOYSTICK, bbcmc_joystick_device, "bbcmc_joystick", "BBC Master Compact Joystick")


//-------------------------------------------------
//  INPUT_PORTS( joystick )
//-------------------------------------------------

static INPUT_PORTS_START( joystick )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbcmc_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( joystick );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbcmc_joystick_device - constructor
//-------------------------------------------------

bbcmc_joystick_device::bbcmc_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBCMC_JOYSTICK, tag, owner, clock)
	, device_bbc_joyport_interface(mconfig, *this)
	, m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbcmc_joystick_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbcmc_joystick_device::pb_r()
{
	return m_joy->read();
}
