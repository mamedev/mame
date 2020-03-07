// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Pull Down RAM - Micro User Jul/Aug 1990

**********************************************************************/


#include "emu.h"
#include "pdram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_PDRAM, bbc_pdram_device, "bbc_pdram", "Micro User Pull Down RAM (DIY)");


//-------------------------------------------------
//  ROM( pdram )
//-------------------------------------------------

ROM_START(pdram)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("ramdvr.rom", 0x0000, 0x2000, CRC(56f61728) SHA1(265359d1fb9c0adca53912aa6a42a995b8225a3e))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry* bbc_pdram_device::device_rom_region() const
{
	return ROM_NAME(pdram);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_pdram_device - constructor
//-------------------------------------------------

bbc_pdram_device::bbc_pdram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_PDRAM, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_ram_page(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_pdram_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x8000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x8000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_pdram_device::fred_w(offs_t offset, uint8_t data)
{
	if (offset == 0xff)
	{
		m_ram_page = data;
	}
}

uint8_t bbc_pdram_device::jim_r(offs_t offset)
{
	return m_ram[(m_ram_page << 8) | offset];
}

void bbc_pdram_device::jim_w(offs_t offset, uint8_t data)
{
	m_ram[(m_ram_page << 8) | offset] = data;
}
