// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Mikro-Plus - Shadow of the Unicorn

**********************************************************************/

#include "emu.h"
#include "mikroplus.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_MIKROPLUS, spectrum_mikroplus_device, "spectrum_mikroplus", "Mikro-Plus - Shadow of the Unicorn")


//-------------------------------------------------
//  ROM( mikroplus )
//-------------------------------------------------

ROM_START( mikroplus )
	ROM_REGION(0x4000, "rom", 0)
	ROM_LOAD("shadowoftheunicorn.rom", 0x0000, 0x4000, CRC(7085a0fd) SHA1(66cc823587b520af9636eed7a342d69d3dd15123))
ROM_END


//-------------------------------------------------
//  INPUT_PORTS( mikroplus )
//-------------------------------------------------

static INPUT_PORTS_START( mikroplus )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_mikroplus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mikroplus );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *spectrum_mikroplus_device::device_rom_region() const
{
	return ROM_NAME( mikroplus );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_mikroplus_device - constructor
//-------------------------------------------------

spectrum_mikroplus_device::spectrum_mikroplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_MIKROPLUS, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_mikroplus_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_mikroplus_device::device_reset()
{
	io_space().install_read_handler(0xdf, 0xdf, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mikroplus_device::joystick_r), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(spectrum_mikroplus_device::joystick_r)
{
	return m_joy->read() | (0xff ^ 0x1f);
}

READ_LINE_MEMBER(spectrum_mikroplus_device::romcs)
{
	return 1;
}

READ8_MEMBER(spectrum_mikroplus_device::mreq_r)
{
	return m_rom->base()[offset & 0x3fff];
}
