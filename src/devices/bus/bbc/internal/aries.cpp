// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aries B12 Sideways ROM Expansion
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Aries_B12.html

    Aries B20 20K RAM expansion
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Aries_B20.html

    Aries-B32 32K RAM expansion
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Aries_B32.html

**********************************************************************/


#include "emu.h"
#include "aries.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ARIESB12, bbc_ariesb12_device, "bbc_ariesb12", "Aries-B12 ROM Expansion");
DEFINE_DEVICE_TYPE(BBC_ARIESB20, bbc_ariesb20_device, "bbc_ariesb20", "Aries-B20 RAM expansion (w/ Aries-B12)");
DEFINE_DEVICE_TYPE(BBC_ARIESB32, bbc_ariesb32_device, "bbc_ariesb32", "Aries-B32 RAM expansion (w/ Aries-B12)");


//-------------------------------------------------
//  ROM( aries )
//-------------------------------------------------

ROM_START(ariesb20)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "24", "v2.4")
	ROMX_LOAD("aries-b20_v2.4.rom", 0x0000, 0x2000, CRC(47a43a03) SHA1(2590596e0864b97b30050c33b21aa89bb8956204), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "22", "v2.2")
	ROMX_LOAD("aries-b20_v2.2.rom", 0x0000, 0x2000, CRC(c2f334a9) SHA1(d04bfd332d63f6f23dca2ef81a1d2074e3d88ee9), ROM_BIOS(1))
ROM_END

ROM_START(ariesb32)
	ROM_REGION(0x4000, "aries_rom", 0)
	ROM_SYSTEM_BIOS(0, "22", "v2.2")
	ROMX_LOAD("aries-b32_v2.2.rom", 0x0000, 0x4000, CRC(2cb3443d) SHA1(466e3b68f1a8e0287ae095cb3c16dac6afc89dcb), ROM_BIOS(0))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_ariesb12_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot0-3 are unavailable when board is fitted, replaced by sockets on board */;
	//config.device_remove(":romslot0");
	//config.device_remove(":romslot1");
	//config.device_remove(":romslot2");
	//config.device_remove(":romslot3");

	/* rom/ram sockets */
	BBC_ROMSLOT16(config, m_rom[0], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[1], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[3], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, nullptr);
}

void bbc_ariesb32_device::device_add_mconfig(machine_config &config)
{
	bbc_ariesb12_device::device_add_mconfig(config);

	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, nullptr);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_ariesb20_device::device_rom_region() const
{
	return ROM_NAME(ariesb20);
}

const tiny_rom_entry *bbc_ariesb32_device::device_rom_region() const
{
	return ROM_NAME(ariesb32);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_aries_device - constructor
//-------------------------------------------------

bbc_ariesb12_device::bbc_ariesb12_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}

bbc_ariesb12_device::bbc_ariesb12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_ariesb12_device(mconfig, BBC_ARIESB12, tag, owner, clock)
{
}

bbc_ariesb20_device::bbc_ariesb20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_ariesb12_device(mconfig, BBC_ARIESB20, tag, owner, clock)
{
}

bbc_ariesb32_device::bbc_ariesb32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_ariesb12_device(mconfig, BBC_ARIESB32, tag, owner, clock)
	, m_aries_rom(*this, "aries_rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ariesb12_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x24000, m_region_swr->base(), 0x10000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
}

void bbc_ariesb20_device::device_start()
{
	bbc_ariesb12_device::device_start();

	m_shadow = false;
	m_ram = std::make_unique<uint8_t[]>(0x5000);

	/* register for save states */
	save_item(NAME(m_shadow));
	save_pointer(NAME(m_ram), 0x5000);
}

void bbc_ariesb32_device::device_start()
{
	bbc_ariesb12_device::device_start();

	m_shadow = false;
	m_ram = std::make_unique<uint8_t[]>(0x8000);

	/* move internal roms to board */
	memcpy(m_region_swr->base() + 0x3c000, m_aries_rom->base(), 0x4000);

	/* register for save states */
	save_item(NAME(m_ramsel));
	save_item(NAME(m_shadow));
	save_pointer(NAME(m_ram), 0x8000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_ariesb20_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_write_handler(0xfffe, 0xffff, write8sm_delegate(*this, FUNC(bbc_ariesb20_device::control_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_ariesb12_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_rom[m_romsel] && m_rom[m_romsel]->present())
	{
		data = m_rom[m_romsel]->read(offset);
	}
	else
	{
		data = m_region_swr->base()[offset + (m_romsel << 14)];
	}

	return data;
}

void bbc_ariesb12_device::paged_w(offs_t offset, uint8_t data)
{
	if (m_rom[m_romsel] && m_rom[m_romsel]->present())
	{
		m_rom[m_romsel]->write(offset, data);
	}

	/* always write to sideways ram */
	m_rom[0x00]->write(offset, data);
}


void bbc_ariesb20_device::control_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		/* &fffe - select BBC ram */
		m_shadow = false;
		break;
	case 1:
		/* &ffff - select Aries ram */
		m_shadow = true;
		break;
	}
}

uint8_t bbc_ariesb20_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_shadow && offset >= 0x3000)
		data = m_ram[offset - 0x3000];
	else
		data = m_mb_ram->pointer()[offset];

	return data;
}

void bbc_ariesb20_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_shadow && offset >= 0x3000)
		m_ram[offset - 0x3000] = data;
	else
		m_mb_ram->pointer()[offset] = data;
}


void bbc_ariesb32_device::romsel_w(offs_t offset, uint8_t data)
{
	//logerror("romsel_w: %04x = %02x\n", offset | 0xfe30, data);

	switch (offset)
	{
	case 0x00:
		m_romsel = data & 0x0f;
		break;
	case 0x02:
		m_ramsel = data & 0x0f;
		break;
	case 0x04:
		break;
	case 0x06:
		// switch
		break;
	case 0x08:
		break;
	case 0x0a:
		// switch
		break;
	case 0x0e:
		m_shadow = false;
		break;
	case 0x0f:
		m_shadow = true;
		break;
	}
}

uint8_t bbc_ariesb32_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_shadow && offset >= 0x3000)
		data = m_ram[offset - 0x3000];
	else
		data = m_mb_ram->pointer()[offset];

	return data;
}

void bbc_ariesb32_device::ram_w(offs_t offset, uint8_t data)
{
	//if ((offset & 0xff) == 0x00)
	//	logerror("ram_w: %04x = %02x\n", offset, data);
	if (m_shadow && offset >= 0x3000)
		m_ram[offset - 0x3000] = data;
	else
		m_mb_ram->pointer()[offset] = data;
}

uint8_t bbc_ariesb32_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_rom[m_romsel] && m_rom[m_romsel]->present())
	{
		data = m_rom[m_romsel]->read(offset);
	}
	else if (m_romsel == 13 || m_romsel == 14)
	{
		data = m_ram[offset | (m_romsel & 0x02) << 13];
	}
	else
	{
		data = m_region_swr->base()[offset + (m_romsel << 14)];
	}

	return data;
}

void bbc_ariesb32_device::paged_w(offs_t offset, uint8_t data)
{
	//if ((offset & 0xff) == 0x00)
	//	logerror("paged_w: %04x = %02x\n", offset | 0x8000, data);
	if (m_rom[m_romsel])
	{
		m_rom[m_romsel]->write(offset, data);
	}

	if (m_ramsel == 13 || m_ramsel == 14)
	{
		m_ram[offset | (m_ramsel & 0x02) << 13] = data;
	}
}
