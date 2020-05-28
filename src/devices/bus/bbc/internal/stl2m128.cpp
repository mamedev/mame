// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk Twomeg 128K Shadow and Sideways ROM/RAM board

**********************************************************************/


#include "emu.h"
#include "stl2m128.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_STL2M128, bbc_stl2m128_device, "bbc_stl2m128", "Solidisk Twomeg 128K RAM/ROM Expansion");


//-------------------------------------------------
//  INPUT_PORTS( stl2m128 )
//-------------------------------------------------

static INPUT_PORTS_START(stl2m128)
	PORT_START("wp")
	PORT_CONFNAME(0x01, 0x00, "Write Protect Sideways RAM")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_stl2m128_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(stl2m128);
}

//-------------------------------------------------
//  ROM( stl2m128 )
//-------------------------------------------------

ROM_START(stl2m128)
	ROM_REGION(0x4000, "rom", ROMREGION_ERASEFF)
	// ROM image usually distributed on floppy, to load into RAM
	ROM_LOAD("manger128-1.00.rom", 0x0000, 0x102a, CRC(dbe84acb) SHA1(d170632b88d0ab276a919b0076dca305094a39b2))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_stl2m128_device::device_add_mconfig(machine_config &config)
{
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_stl2m128_device::device_rom_region() const
{
	return ROM_NAME(stl2m128);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_stl2m128_device - constructor
//-------------------------------------------------

bbc_stl2m128_device::bbc_stl2m128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_STL2M128, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
	, m_wp(*this, "wp")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_stl2m128_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x20000);

	/* copy rom image into ram bank c */
	memcpy(&m_ram[0x10000], memregion("rom")->base(), 0x4000);

	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x20000, m_region_swr->base(), 0x10000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x20000);
	save_item(NAME(m_romsel));
	save_item(NAME(m_shadow));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_stl2m128_device::romsel_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x0c)
	{
	case 0x00:
		m_romsel = data & 0x0f;
		break;
	case 0x04:
		m_shadow = data;
		break;
	}
}

uint8_t bbc_stl2m128_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (BIT(m_shadow, 7) && offset >= 0x2000)
	{
		if (BIT(m_shadow, 1))
		{
			/* banks E/F */
			data = m_ram[offset | 0x18000];
		}
		else
		{
			/* banks C/D */
			data = m_ram[offset | 0x10000];
		}
	}
	else
	{
		data = m_mb_ram->pointer()[offset];
	}

	return data;
}

void bbc_stl2m128_device::ram_w(offs_t offset, uint8_t data)
{
	if (BIT(m_shadow, 7) && offset >= 0x2000)
	{
		if (BIT(m_shadow, 1))
		{
			/* banks E/F */
			m_ram[offset | 0x18000] = data;
		}
		else
		{
			/* banks C/D */
			m_ram[offset | 0x10000] = data;
		}
	}
	else
	{
		m_mb_ram->pointer()[offset] = data;
	}
}

uint8_t bbc_stl2m128_device::paged_r(offs_t offset)
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
	case 8: case 9: case 10: case 11:
		/* expansion board rom sockets */
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	case 4: case 5: case 6: case 7: case 12: case 13: case 14: case 15:
		/* expansion board ram sockets */
		data = m_ram[offset | ((m_romsel & 0x03) << 14) | ((m_romsel & 0x08) << 13)];
		break;
	}


	return data;
}

void bbc_stl2m128_device::paged_w(offs_t offset, uint8_t data)
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
	case 8: case 9: case 10: case 11:
		/* expansion board rom sockets */
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	case 4: case 5: case 6: case 7: case 12: case 13: case 14: case 15:
		/* expansion board ram sockets */
		if (!m_wp->read())
		{
			m_ram[offset | ((m_romsel & 0x03) << 14) | ((m_romsel & 0x08) << 13)] = data;
		}
		break;
	}
}
