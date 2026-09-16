// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk Fourmeg RAM/ROM Expansions

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_4Meg.html

**********************************************************************/


#include "emu.h"
#include "stl4m32.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_STL4M32, bbc_stl4m32_device, "bbc_stl4m32", "Solidisk Fourmeg 32K RAM/ROM Expansion");


//-------------------------------------------------
//  INPUT_PORTS( stl4m )
//-------------------------------------------------

INPUT_PORTS_START(stl4m32)
	PORT_START("stl4m_clock")
	PORT_CONFNAME(0x01, 0x01, "4MHz Clock") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_stl4m32_device::clock_switch), 0)
	PORT_CONFSETTING(0x00, "2MHz")
	PORT_CONFSETTING(0x01, "4MHz")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_stl4m32_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(stl4m32);
}


//-------------------------------------------------
//  ROM( stl4m32 )
//-------------------------------------------------

ROM_START(stl4m32)
	ROM_REGION(0x4000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "30", "Manager 3.00 (16.6.86)")
	ROMX_LOAD("manager_4meg32k-16.6.86.rom", 0x0000, 0x4000, CRC(2024cd0d) SHA1(cf45217ac88480a6963cbf75e8cf684cd81c7b09), ROM_BIOS(0))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_stl4m32_device::device_add_mconfig(machine_config &config)
{
	//G65SC02(config.replace(), m_maincpu, DERIVED_CLOCK(1, 4));

	/* 5 x 32K rom sockets */
	BBC_ROMSLOT32(config, m_rom[4], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[6], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT32(config, m_rom[12], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "ram").set_fixed_ram(true);
}


const tiny_rom_entry *bbc_stl4m32_device::device_rom_region() const
{
	return ROM_NAME(stl4m32);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_stl4m32_device - constructor
//-------------------------------------------------

bbc_stl4m32_device::bbc_stl4m32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_STL4M32, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_stl4m32_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x8000);

	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x30000, m_region_swr->base() + 0xc000, 0x4000);
	memcpy(m_region_swr->base() + 0x28000, memregion("rom")->base(), 0x4000);
	memset(m_region_swr->base() + 0x0c000, 0xff, 0x4000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x8000);
	save_item(NAME(m_romsel));
	save_item(NAME(m_ramsel));
	save_item(NAME(m_shadow));
}


//-------------------------------------------------
//  device_reset_after_children - reset after child devices
//-------------------------------------------------

void bbc_stl4m32_device::device_reset() //_after_children()
{
	/* set cpu clock to 4mhz */
	//m_maincpu->set_clock(DERIVED_CLOCK(1, 4));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER(bbc_stl4m32_device::clock_switch)
{
	switch (newval)
	{
	case 0:
		//m_maincpu->set_clock(DERIVED_CLOCK(1, 8));
		break;
	case 1:
		//m_maincpu->set_clock(DERIVED_CLOCK(1, 4));
		break;
	}
}

void bbc_stl4m32_device::romsel_w(offs_t offset, uint8_t data)
{
	logerror("romsel_w: %04x = %02x\n", offset | 0xfe30, data);

	switch (offset & 0x0e)
	{
	case 0x00:
		m_romsel = data & 0x0f;
		break;
	case 0x02:
		m_ramsel = data;
		break;
	case 0x04:
		m_shadow = data;
		break;
	}
}

uint8_t bbc_stl4m32_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (BIT(m_shadow, 7) && offset >= 0x2000)
	{
		data = m_ram[offset];
	}
	else
	{
		data = m_mb_ram->pointer()[offset];
	}

	return data;
}

void bbc_stl4m32_device::ram_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x00)
		logerror("ram_w: %04x = %02x\n", offset, data);

	if (BIT(m_romsel, 7) && offset >= 0x2000)
	{
		m_ram[offset] = data;
	}
	else
	{
		m_mb_ram->pointer()[offset] = data;
	}
}


uint8_t bbc_stl4m32_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (BIT(m_romsel, 7) && offset < 0x2000)
	{
		data = m_ram[offset];
	}
	else
	{
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
		case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
			/* expansion board rom sockets */
			if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
			{
				data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
			}
			else
			{
				data = m_region_swr->base()[offset + (m_romsel << 14)];
			}
			break;
		case 14: case 15:
			/* expansion board ram sockets */
			data = m_ram[offset | ((m_romsel & 0x01) << 14)];
			break;
		}
	}

	return data;
}

void bbc_stl4m32_device::paged_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x00)
		logerror("paged_w: %04x = %02x\n", offset | 0x8000, data);

	if (BIT(m_romsel, 7) && offset < 0x2000)
	{
		m_ram[offset] = data;
	}
	else
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
		case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
			/* expansion board rom sockets */
			if (m_rom[m_romsel & 0x0e])
			{
				m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
			}
			break;
		}

		if (m_ramsel == 14 || m_ramsel == 15)
		{
			/* expansion board ram sockets */
			m_ram[offset | ((m_ramsel & 0x01) << 14)] = data;
		}
	}
}


uint8_t bbc_stl4m32_device::mos_r(offs_t offset)
{
	uint8_t data = 0xff;

	//if (BIT(m_shadow, 7) && offset >= 0x2000)
	//{
	//  data = m_ram[offset];
	//}
	//else
	//{
		data = m_region_mos->base()[offset];
	//}

	return data;
}

void bbc_stl4m32_device::mos_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x00)
		logerror("mos_w: %04x = %02x\n", offset | 0xc000, data);

	//if (BIT(m_romsel, 7) && offset >= 0x2000)
	//{
	//  m_ram[offset] = data;
	//}
}
