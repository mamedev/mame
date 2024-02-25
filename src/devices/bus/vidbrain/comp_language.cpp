// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/**********************************************************************

    VideoBrain The Computational Language cartridge emulation
    14KB PROM (7*Intel B2616), 1KB RAM (2*Intel P2114L)

**********************************************************************/

#include "emu.h"
#include "comp_language.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VB_COMP_LANGUAGE, videobrain_comp_language_cartridge_device, "vb_comp_language", "VideoBrain The Computational Language cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_comp_language_cartridge_device - constructor
//-------------------------------------------------

videobrain_comp_language_cartridge_device::videobrain_comp_language_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VB_COMP_LANGUAGE, tag, owner, clock),
	device_videobrain_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_comp_language_cartridge_device::device_start()
{
	m_bank = 0;
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  videobrain_bo_r - cartridge data read
//-------------------------------------------------

uint8_t videobrain_comp_language_cartridge_device::videobrain_bo_r(offs_t offset, int cs1, int cs2)
{
	uint8_t data = 0;

	if (!cs1 || !cs2)
	{
		// fixed ROM A/B and RAM
		if (offset >= 0x1c00)
			data = m_ram[offset & m_ram_mask];
		else
			data = m_rom[offset & 0xfff & m_rom_mask];
	}
	else if (offset >= 0x3000)
	{
		// banked ROM
		const int lut_roms[8] = { 2, 3, 4, 6, 4, 5 }; // C/D, E/G, E/F
		const int bank = lut_roms[(m_bank << 1 | BIT(offset, 11)) % 6];
		data = m_rom[((bank * 0x800) | (offset & 0x7ff)) & m_rom_mask];
	}

	return data;
}


//-------------------------------------------------
//  videobrain_bo_w - cartridge data write
//-------------------------------------------------

void videobrain_comp_language_cartridge_device::videobrain_bo_w(offs_t offset, uint8_t data, int cs1, int cs2)
{
	if (!cs1)
	{
		// bus conflict issue
		const int lut_bank[4][4] =
		{
			// 1000 1001 100A 100F
			{  0,   1,   2,   0  }, // 00
			{  2,   1,   2,   1  }, // 55
			{  0,   2,   2,   0  }, // AA
			{  2,   2,   2,   2  }  // FF
		};
		m_bank = lut_bank[data & 3][offset & 3];
	}
	else if (!cs2 && offset >= 0x1c00)
	{
		m_ram[offset & m_ram_mask] = data;
	}
}
