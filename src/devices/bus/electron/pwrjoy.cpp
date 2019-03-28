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

DEFINE_DEVICE_TYPE(ELECTRON_PWRJOY, electron_pwrjoy_device, "electron_pwrjoy", "Power Software Joystick Interface")


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
	: device_t(mconfig, ELECTRON_PWRJOY, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_exp_rom(*this, "exp_rom")
	, m_joy(*this, "JOY")
	, m_romsel(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_pwrjoy_device::device_start()
{
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_pwrjoy_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x8000 && offset < 0xc000)
	{
		if (m_romsel == 15)
		{
			data = m_exp_rom->base()[offset & 0x1fff];
		}
	}
	else if (offset == 0xfcc0)
	{
		data = m_joy->read() | 0xe0;
	}

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_pwrjoy_device::expbus_w(offs_t offset, uint8_t data)
{
	if (offset == 0xfe05)
	{
		m_romsel = data & 0x0f;
	}
}
