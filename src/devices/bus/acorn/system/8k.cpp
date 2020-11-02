// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 8K Static Memory Board

    Part No. 200,003

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_Memory.html

    The Acorn Extension Memory board provides an extra 8K bytes of
    user RAM space and has sockets for two ROMs which may be 2, 4 or 8K
    devices providing 4 or 8K bytes of firmware.

**********************************************************************/


#include "emu.h"
#include "8k.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_8K, acorn_8k_device, "acorn_8k", "Acorn 8K Static Memory Board")


//-------------------------------------------------
//  INPUT_PORTS( 8k )
//-------------------------------------------------

INPUT_PORTS_START( 8k )
	PORT_START("LINKS")
	PORT_CONFNAME(0x07, 0x01, "Address Selection (RAM)")
	PORT_CONFSETTING(0x00, "&0000-&1FFF")
	PORT_CONFSETTING(0x01, "&2000-&3FFF")
	PORT_CONFSETTING(0x02, "&4000-&5FFF")
	PORT_CONFSETTING(0x03, "&6000-&7FFF")
	PORT_CONFSETTING(0x04, "&8000-&9FFF")
	PORT_CONFSETTING(0x05, "&A000-&BFFF")
	PORT_CONFSETTING(0x06, "&C000-&DFFF")
	PORT_CONFSETTING(0x07, "&E000-&FFFF")
	PORT_CONFNAME(0x70, 0x50, "Address Selection (ROM)")
	PORT_CONFSETTING(0x00, "&0000-&1FFF")
	PORT_CONFSETTING(0x10, "&2000-&3FFF")
	PORT_CONFSETTING(0x20, "&4000-&5FFF")
	PORT_CONFSETTING(0x30, "&6000-&7FFF")
	PORT_CONFSETTING(0x40, "&8000-&9FFF")
	PORT_CONFSETTING(0x50, "&A000-&BFFF")
	PORT_CONFSETTING(0x60, "&C000-&DFFF")
	PORT_CONFSETTING(0x70, "&E000-&FFFF")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor acorn_8k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 8k );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_8k_device::device_add_mconfig(machine_config &config)
{
	/* rom sockets */
	GENERIC_SOCKET(config, "rom0", generic_plain_slot, "acrnsys_rom", "bin,rom").set_device_load(FUNC(acorn_8k_device::rom0_load)); // IC17
	GENERIC_SOCKET(config, "rom1", generic_plain_slot, "acrnsys_rom", "bin,rom").set_device_load(FUNC(acorn_8k_device::rom1_load)); // IC18
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_8k_device - constructor
//-------------------------------------------------

acorn_8k_device::acorn_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_8K, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_rom(*this, "rom%u", 0)
	, m_links(*this, "LINKS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_8k_device::device_start()
{
	save_item(NAME(m_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_8k_device::device_reset()
{
	address_space &space = m_bus->memspace();

	uint16_t ram_addr = (m_links->read() & 0x0f) << 13;

	if (ram_addr == 0x0000) // BLK0
	{
		space.install_ram(0x1000, 0x1fff, m_ram);
	}
	else
	{
		space.install_ram(ram_addr, ram_addr + 0x1fff, m_ram);
	}

	uint16_t rom_addr = (m_links->read() & 0xf0) << 9;

	if (m_rom[0]->exists() && rom_addr != 0x0000) // BLK0
	{
		space.install_rom(rom_addr + 0x0000, rom_addr + 0x0fff, m_rom[0]->get_rom_base());
	}
	if (m_rom[1]->exists() && rom_addr != 0xe000) // don't replace OS in page F
	{
		space.install_rom(rom_addr + 0x1000, rom_addr + 0x1fff, m_rom[1]->get_rom_base());
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result acorn_8k_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 2K and 4K ROM only
	if (size != 0x0800 && size != 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid size: Only 2K/4K is supported");
		return image_init_result::FAIL;
	}

	slot->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 2K ROMs
	uint8_t *rom = slot->get_rom_base();
	if (size <= 0x0800) memcpy(rom + 0x0800, rom, 0x0800);

	return image_init_result::PASS;
}
