// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Spectrum Currah MicroSource emulation

**********************************************************************/

#include "emu.h"
#include "usource.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_USOURCE, spectrum_usource_device, "spectrum_usource", "Spectrum Currah \xC2\xB5Source")


//-------------------------------------------------
//  ROM( usource )
//-------------------------------------------------

ROM_START( usource )
	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("microsource.rom", 0x0000, 0x2000, CRC(8e9b67d0) SHA1(fae53678a85ba503b77ed2d877de2635b284eef1))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *spectrum_usource_device::device_rom_region() const
{
	return ROM_NAME( usource );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_usource_device - constructor
//-------------------------------------------------

spectrum_usource_device::spectrum_usource_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SPECTRUM_USOURCE, tag, owner, clock),
	device_spectrum_expansion_interface(mconfig, *this),
	m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_usource_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_usource_device::device_reset()
{
	m_romcs = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_usource_device::romcs)
{
	return m_romcs;
}


READ8_MEMBER(spectrum_usource_device::mreq_r)
{
	uint8_t data;

	if (!machine().side_effects_disabled() && (offset == 0x2bae))
	{
		m_romcs = !m_romcs;
	}

	data = m_rom->base()[offset & 0x1fff];

	return data;
}
