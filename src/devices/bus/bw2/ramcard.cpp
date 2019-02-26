// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Bondwell 2 RAMCARD emulation

**********************************************************************/

#include "emu.h"
#include "ramcard.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BW2_RAMCARD, bw2_ramcard_device, "bw2_ramcard", "Bondwell 2 RAMCARD")


//-------------------------------------------------
//  ROM( bw2_ramcard )
//-------------------------------------------------

ROM_START( bw2_ramcard )
	ROM_REGION( 0x4000, "ramcard", 0 )
	ROM_LOAD( "ramcard-10.ic10", 0x0000, 0x4000, CRC(68cde1ba) SHA1(a776a27d64f7b857565594beb63aa2cd692dcf04) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bw2_ramcard_device::device_rom_region() const
{
	return ROM_NAME( bw2_ramcard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bw2_ramcard_device - constructor
//-------------------------------------------------

bw2_ramcard_device::bw2_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BW2_RAMCARD, tag, owner, clock),
		device_bw2_expansion_slot_interface(mconfig, *this),
		m_rom(*this, "ramcard"),
		m_ram(*this, "ram"),
		m_en(0),
		m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bw2_ramcard_device::device_start()
{
	// allocate memory
	m_ram.allocate(512 * 1024);

	// state saving
	save_item(NAME(m_en));
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bw2_ramcard_device::device_reset()
{
	m_en = 0;
	m_bank = 0;
}


//-------------------------------------------------
//  bw2_cd_r - cartridge data read
//-------------------------------------------------

uint8_t bw2_ramcard_device::bw2_cd_r(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6)
{
	if (!ram2)
	{
		data = m_rom->base()[offset & 0x3fff];
	}
	else if (m_en && !ram5)
	{
		data = m_ram[(m_bank << 15) | offset];
	}

	return data;
}


//-------------------------------------------------
//  bw2_cd_r - cartridge data write
//-------------------------------------------------

void bw2_ramcard_device::bw2_cd_w(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6)
{
	if (m_en && !ram5)
	{
		m_ram[(m_bank << 15) | offset] = data;
	}
}


//-------------------------------------------------
//  bw2_slot_w - slot write
//-------------------------------------------------

void bw2_ramcard_device::bw2_slot_w(offs_t offset, uint8_t data)
{
	m_en = 1;
	m_bank = data & 0x0f;
}
