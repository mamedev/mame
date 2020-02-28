// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    First Byte Switched Joystick Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/FirstByte_JoystickIF.html

**********************************************************************/

#include "emu.h"
#include "fbjoy.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_FBJOY, electron_fbjoy_device, "electron_fbjoy", "First Byte Joystick Interface")


static INPUT_PORTS_START( fbjoy )
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

ioport_constructor electron_fbjoy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( fbjoy );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_fbjoy_device - constructor
//-------------------------------------------------

electron_fbjoy_device::electron_fbjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_FBJOY, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_fbjoy_device::device_start()
{
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_fbjoy_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset == 0xfcc0)
	{
		data = m_joy->read() | 0xe0;
	}

	return data;
}
