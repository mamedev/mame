// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Spectrum +2 Test Software

**********************************************************************/

#include "emu.h"
#include "plus2test.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_PLUS2TEST, spectrum_plus2test_device, "spectrum_plus2test", "Spectrum +2 Test Software")


//-------------------------------------------------
//  ROM( plus2test )
//-------------------------------------------------

ROM_START( plus2test )
	ROM_REGION(0x4000, "rom", 0)
	ROM_LOAD("plus2test.rom", 0x0000, 0x2000, CRC(6bbe1079) SHA1(2ed54f8ccf1ef26045bb409bb86c859ee23e83d1))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *spectrum_plus2test_device::device_rom_region() const
{
	return ROM_NAME( plus2test );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_plus2test_device - constructor
//-------------------------------------------------

spectrum_plus2test_device::spectrum_plus2test_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_PLUS2TEST, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_plus2test_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_plus2test_device::romcs()
{
	return 1;
}

uint8_t spectrum_plus2test_device::mreq_r(offs_t offset)
{
	return m_rom->base()[offset & 0x3fff];
}
