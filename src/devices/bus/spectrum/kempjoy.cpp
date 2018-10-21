// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Kempston Joystick Interface

**********************************************************************/

#include "emu.h"
#include "kempjoy.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_KEMPJOY, spectrum_kempjoy_device, "spectrum_kempjoy", "Kempston Joystick Interface")


//-------------------------------------------------
//  INPUT_PORTS( kempjoy )
//-------------------------------------------------

static INPUT_PORTS_START( kempjoy )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_kempjoy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kempjoy );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_kempjoy_device - constructor
//-------------------------------------------------

spectrum_kempjoy_device::spectrum_kempjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_KEMPJOY, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_kempjoy_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_kempjoy_device::device_reset()
{
	io_space().install_read_handler(0x1f, 0x1f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_kempjoy_device::joystick_r), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(spectrum_kempjoy_device::joystick_r)
{
	return m_joy->read() & 0x1f;
}
