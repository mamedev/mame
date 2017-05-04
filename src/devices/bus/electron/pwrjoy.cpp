// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Power Software Joystick Interface

**********************************************************************/

#include "emu.h"
#include "pwrjoy.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ELECTRON_PWRJOY = device_creator<electron_pwrjoy_device>;


ROM_START( pwrjoy )
	// Bank 12 Expansion module operating system
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("power_joystick.rom", 0x0000, 0x2000, CRC(44fb9360) SHA1(6d3aa85a436db952906e84839496e681ea115168))
ROM_END


static INPUT_PORTS_START( pwrjoy )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_pwrjoy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pwrjoy );
}

const tiny_rom_entry *electron_pwrjoy_device::device_rom_region() const
{
	return ROM_NAME( pwrjoy );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_pwrjoy_device - constructor
//-------------------------------------------------

electron_pwrjoy_device::electron_pwrjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_PWRJOY, "Power Software Joystick Interface", tag, owner, clock, "electron_pwrjoy", __FILE__),
	device_electron_expansion_interface(mconfig, *this),
	m_exp_rom(*this, "exp_rom"),
	m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_pwrjoy_device::device_start()
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<electron_expansion_slot_device *>(owner());

	space.install_read_handler(0xfcc0, 0xfcc0, READ8_DELEGATE(electron_pwrjoy_device, joystick_r));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_pwrjoy_device::device_reset()
{
	machine().root_device().membank("bank2")->configure_entry(15, memregion("exp_rom")->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(electron_pwrjoy_device::joystick_r)
{
	return m_joy->read() | 0xe0;
}
