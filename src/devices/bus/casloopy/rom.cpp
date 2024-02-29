// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************************************************

    Casio Loopy cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  casloopy_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(CASLOOPY_ROM_STD,     casloopy_rom_device,    "casloopy_rom",    "Casio Loopy Standard Cart")
DEFINE_DEVICE_TYPE(CASLOOPY_ROM_ADPCM,   casloopy_adpcm_device,  "casloopy_adpcm",  "Casio Loopy ADPCM Cart")


casloopy_rom_device::casloopy_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock), device_casloopy_cart_interface(mconfig, *this)
	, m_rom_base(nullptr)
	, m_nvram_base(nullptr)
	, m_rom_size(0)
	, m_nvram_size(0)
{
}

casloopy_rom_device::casloopy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: casloopy_rom_device(mconfig, CASLOOPY_ROM_STD, tag, owner, clock)
{
}

casloopy_adpcm_device::casloopy_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: casloopy_rom_device(mconfig, CASLOOPY_ROM_ADPCM, tag, owner, clock)
{
}


/*-------------------------------------------------
 device_t implementation
 -------------------------------------------------*/

void casloopy_rom_device::device_start()
{
}

void casloopy_adpcm_device::device_add_mconfig(machine_config &config)
{
	casloopy_rom_device::device_add_mconfig(config);

	// TODO: Add support for the MSM6653A ADPCM chip, or samples
	// and route to the speakers
}


/*-------------------------------------------------
 load/unload
 -------------------------------------------------*/

std::error_condition casloopy_rom_device::load()
{
	memory_region *const romregion = memregion("^rom");
	m_rom_base = reinterpret_cast<const u16 *>(romregion->base());
	m_rom_size = romregion->bytes() / 2;

	memory_region *const nvramregion = memregion("^nvram");
	if (nvramregion)
	{
		m_nvram_base = reinterpret_cast<u8 *>(nvramregion->base());
		m_nvram_size = nvramregion->bytes();

		if (m_nvram_size & (m_nvram_size - 1))
			return image_error::BADSOFTWARE;

		save_pointer(NAME(m_nvram_base), m_nvram_size);
		battery_load(m_nvram_base, m_nvram_size, nullptr);
	}

	return std::error_condition();
}


void casloopy_rom_device::unload()
{
	if (m_nvram_base)
		battery_save(m_nvram_base, m_nvram_size);
}


/*-------------------------------------------------
 read/write
 -------------------------------------------------*/

u16 casloopy_rom_device::rom_r(offs_t offset)
{
	if (offset < m_rom_size)
		return m_rom_base[offset];
	else
		return 0xffff;
}

u8 casloopy_rom_device::ram_r(offs_t offset)
{
	if (m_nvram_base)
		return m_nvram_base[offset & (m_nvram_size - 1)];
	else
		return 0xff;
}

void casloopy_rom_device::ram_w(offs_t offset, u8 data)
{
	if (m_nvram_base)
		m_nvram_base[offset & (m_nvram_size - 1)] = data;
}
