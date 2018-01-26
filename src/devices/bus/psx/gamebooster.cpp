// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

    Datel Game Booster for Playstation 1
	
	Gameboy emulator with Gameboy cartridge slot

**********************************************************************/

#include "emu.h"
#include "gamebooster.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSX_GAMEBOOSTER, psx_gamebooster_device, "psxgboost", "Datel Game Booster for Playstation")

//-------------------------------------------------
//  ROM( psxgboost )
//-------------------------------------------------

ROM_START( psxgboost )
	ROM_REGION(0x40000, "rom", 0)
	ROM_LOAD("Game Booster.rom", 0x0000, 0x40000, CRC(c8e459b8) SHA1(c20ab073f61242f37665f12199b95cfa3a83e9fc) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *psx_gamebooster_device::device_rom_region() const
{
	return ROM_NAME( psxgboost );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psx_gamebooster_device - constructor
//-------------------------------------------------

psx_gamebooster_device::psx_gamebooster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSX_GAMEBOOSTER, tag, owner, clock)
	, psx_parallel_interface(mconfig, *this)
	, m_rom(*this, "rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psx_gamebooster_device::device_start()
{
	m_slot = dynamic_cast<psx_parallel_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psx_gamebooster_device::device_reset()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ16_MEMBER(psx_gamebooster_device::exp_r)
{
	return m_rom->base()[offset & 0x1ffff];
}
