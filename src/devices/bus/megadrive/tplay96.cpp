// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Electronic Arts Triple Play 96/Gold mapper

8-bit NVRAM with NO write protection control lanes

At title screen hold A+B+C then press start, SFX plays, release all to get prompted for NVRAM reinitialize.

TODO:
- find out if any other cart needs this mapping (from header start with $200001?);

**************************************************************************************************/


#include "emu.h"
#include "tplay96.h"

DEFINE_DEVICE_TYPE(MD_ROM_TPLAY96, md_rom_tplay96_device, "md_rom_tplay96", "MD EA Triple Play 96 (8-bit SRAM)")

md_rom_tplay96_device::md_rom_tplay96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MD_ROM_TPLAY96, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
{
}

void md_rom_tplay96_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void md_rom_tplay96_device::device_start()
{
	const u32 nvram_size = 0x8000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

uint16_t md_rom_tplay96_device::read(offs_t offset)
{
	if (offset >= 0x200000/2 && offset < 0x210000/2)
	{
		const u32 nvram_offset = offset & 0x7fff;
		return 0xff00 | m_nvram_ptr[nvram_offset];
	}

	if (offset < 0x400000/2)
	{
		return m_rom[offset];
	}

	return 0xffff;
}

void md_rom_tplay96_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x200000/2 && offset < 0x210000/2 && ACCESSING_BITS_0_7)
	{
		const u32 nvram_offset = offset & 0x7fff;
		m_nvram_ptr[nvram_offset] = data & 0xff;
	}
}
