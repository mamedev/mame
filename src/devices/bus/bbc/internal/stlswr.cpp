// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk Sideways RAM

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_128KSWE.html

**********************************************************************/


#include "emu.h"
#include "stlswr.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_STLSWR16, bbc_stlswr16_device, "bbc_stlswr16", "Solidisk SWR16 - 16K Sideways RAM");
DEFINE_DEVICE_TYPE(BBC_STLSWR32, bbc_stlswr32_device, "bbc_stlswr32", "Solidisk SWR32 - 32K Sideways RAM");
DEFINE_DEVICE_TYPE(BBC_STLSWR64, bbc_stlswr64_device, "bbc_stlswr64", "Solidisk SWR64 - 64K Sideways RAM");
DEFINE_DEVICE_TYPE(BBC_STLSWR128, bbc_stlswr128_device, "bbc_stlswr128", "Solidisk SWR128 - 128K Sideways RAM");


//-------------------------------------------------
//  ROM( stlswr )
//-------------------------------------------------

ROM_START(stlswr)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "13", "Toolkit 1.3 03/03/86")
	ROMX_LOAD("toolkit-1.3-16k-128k-03-03-86.rom", 0x0000, 0x4000, CRC(a1745d10) SHA1(1821e74feca60b95766ead06df20f6db22e2b4bb), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_stlswr16_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot3 is unavailable when board is fitted */
	//config.device_remove(":romslot3");
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

void bbc_stlswr32_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot3 is unavailable when board is fitted */
	//config.device_remove(":romslot3");
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

void bbc_stlswr64_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot3 is unavailable when board is fitted */
	//config.device_remove(":romslot3");
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

void bbc_stlswr128_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot3 is unavailable when board is fitted */
	//config.device_remove(":romslot3");
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_stlswr_device::device_rom_region() const
{
	return ROM_NAME(stlswr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_stlswr_device - constructor
//-------------------------------------------------

bbc_stlswr_device::bbc_stlswr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}

bbc_stlswr16_device::bbc_stlswr16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stlswr_device(mconfig, BBC_STLSWR16, tag, owner, clock)
{
}

bbc_stlswr32_device::bbc_stlswr32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stlswr_device(mconfig, BBC_STLSWR32, tag, owner, clock)
{
}

bbc_stlswr64_device::bbc_stlswr64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stlswr_device(mconfig, BBC_STLSWR64, tag, owner, clock)
{
}

bbc_stlswr128_device::bbc_stlswr128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stlswr_device(mconfig, BBC_STLSWR128, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_stlswr_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_romsel));
	save_item(NAME(m_ramsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_stlswr_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 0: case 1: case 2: case 3:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel] && m_mb_rom[m_romsel]->present())
		{
			data = m_mb_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	default:
		/* expansion board sockets */
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbc_stlswr_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 0: case 1: case 2: case 3:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel])
		{
			m_mb_rom[m_romsel]->write(offset, data);
		}
		break;
	}

	/* expansion board sockets */
	if (m_rom[m_ramsel])
	{
		m_rom[m_ramsel]->write(offset, data);
	}
}
