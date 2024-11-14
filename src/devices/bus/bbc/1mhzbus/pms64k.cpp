// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    PMS 64K Non-Volatile Ram Module

**********************************************************************/


#include "emu.h"
#include "pms64k.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_PMS64K, bbc_pms64k_device, "bbc_pms64k", "PMS 64K Non-Volatile Ram Module");


//-------------------------------------------------
//  ROM( pms64k )
//-------------------------------------------------

ROM_START(pms64k)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("pms_utility_12d.rom", 0x0000, 0x4000, CRC(c630990e) SHA1(da9abe3b1b0bf34ee5d6ed7ee032686403436289))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_pms64k_device::device_rom_region() const
{
	return ROM_NAME(pms64k);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_pms64k_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_pms64k_device - constructor
//-------------------------------------------------

bbc_pms64k_device::bbc_pms64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_PMS64K, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_ram_page(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_pms64k_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);
	m_nvram->set_base(m_ram.get(), 0x10000);

	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_ram_page));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_pms64k_device::fred_w(offs_t offset, uint8_t data)
{
	if (offset == 0xff)
	{
		m_ram_page = data;
	}
}

uint8_t bbc_pms64k_device::jim_r(offs_t offset)
{
	return m_ram[(m_ram_page << 8) | offset];
}

void bbc_pms64k_device::jim_w(offs_t offset, uint8_t data)
{
	m_ram[(m_ram_page << 8) | offset] = data;
}
