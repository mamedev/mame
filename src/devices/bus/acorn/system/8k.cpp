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

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


namespace {

class acorn_8k_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_8K, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_rom(*this, "rom%u", 0)
		, m_ram(*this, "ram", 0x2000, ENDIANNESS_LITTLE)
		, m_links(*this, "LINKS")
	{
	}

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device_array<generic_slot_device, 2> m_rom;
	memory_share_creator<uint8_t> m_ram;
	required_ioport m_links;

	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom0) { return load_rom(image, m_rom[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1) { return load_rom(image, m_rom[1]); }
};


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
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "acrnsys_rom", "bin,rom").set_device_load(FUNC(acorn_8k_device::rom0)); // IC17
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "acrnsys_rom", "bin,rom").set_device_load(FUNC(acorn_8k_device::rom1)); // IC18
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_8k_device::device_start()
{
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

std::pair<std::error_condition, std::string> acorn_8k_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t const size = slot->common_get_size("rom");

	// socket accepts 2K and 4K ROM only
	if (size != 0x0800 && size != 0x1000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size: Only 2K/4K is supported");

	slot->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 2K ROMs
	uint8_t *rom = slot->get_rom_base();
	if (size <= 0x0800) memcpy(rom + 0x0800, rom, 0x0800);

	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_8K, device_acorn_bus_interface, acorn_8k_device, "acorn_8k", "Acorn 8K Static Memory Board")
