// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************************************************

 V.Smile cart emulation

 We support standard carts and one with on-board NVRAM

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  vsmile_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(VSMILE_ROM_STD,   vsmile_rom_device,       "vsmile_rom",       "V.Smile Cart")
DEFINE_DEVICE_TYPE(VSMILE_ROM_NVRAM, vsmile_rom_nvram_device, "vsmile_rom_nvram", "V.Smile Cart + NVRAM")


vsmile_rom_device::vsmile_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vsmile_cart_interface(mconfig, *this)
{
}

vsmile_rom_device::vsmile_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vsmile_rom_device(mconfig, VSMILE_ROM_STD, tag, owner, clock)
{
}

vsmile_rom_nvram_device::vsmile_rom_nvram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: vsmile_rom_device(mconfig, type, tag, owner, clock)
{
}

vsmile_rom_nvram_device::vsmile_rom_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vsmile_rom_nvram_device(mconfig, VSMILE_ROM_NVRAM, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void vsmile_rom_device::device_start()
{
	save_item(NAME(m_bank_offset));
}

void vsmile_rom_device::device_resolve_objects()
{
	m_bank_offset = 0;
}


/*-------------------------------------------------
 Cart with NVRAM
 -------------------------------------------------*/

uint16_t vsmile_rom_nvram_device::bank2_r(offs_t offset)
{
	if (!m_nvram.empty() && offset < m_nvram.size())
		return m_nvram[offset];
	else    // this cannot actually happen...
		return 0;
}

void vsmile_rom_nvram_device::bank2_w(offs_t offset, uint16_t data)
{
	if (!m_nvram.empty() && offset < m_nvram.size())
		m_nvram[offset] = data;
}

/*-------------------------------------------------
 CS2 bankswitching
 -------------------------------------------------*/

void vsmile_rom_device::set_cs2(bool cs2)
{
	m_bank_offset = cs2 ? 0x400000 : 0x000000;
}
