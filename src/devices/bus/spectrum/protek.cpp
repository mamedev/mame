// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Protek Joystick Interface

**********************************************************************/

#include "emu.h"
#include "protek.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_PROTEK, spectrum_protek_device, "spectrum_protek", "Protek Joystick Interface")


//-------------------------------------------------
//  INPUT_PORTS( protek )
//-------------------------------------------------

static INPUT_PORTS_START( protek )
	PORT_START("LINE3") /* 0xF7FE */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY

	PORT_START("LINE4") /* 0xEFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_protek_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( protek );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_protek_device - constructor
//-------------------------------------------------

spectrum_protek_device::spectrum_protek_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_PROTEK, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_exp_line3(*this, "LINE3")
	, m_exp_line4(*this, "LINE4")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_protek_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t spectrum_protek_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xff)
	{
	case 0xfe:
		if (((offset >> 8) & 8) == 0)
			data = m_exp_line3->read() | (0xff ^ 0x10);

		if (((offset >> 8) & 16) == 0)
			data = m_exp_line4->read() | (0xff ^ 0x1d);
		break;
	}

	return data;
}
