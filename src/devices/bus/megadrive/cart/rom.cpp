// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Megadrive generic ROM cart emulation

**************************************************************************************************/

#include "emu.h"
#include "rom.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM, megadrive_rom_device, "megadrive_rom", "Megadrive ROM cart")

megadrive_rom_device::megadrive_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom(*this, "rom")
{
}

megadrive_rom_device::megadrive_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_ROM, tag, owner, clock)
{
}

std::error_condition megadrive_rom_device::load()
{
	// if there's no ROM region, there's nothing to do
	//printf("load()\n");
	memory_region *const romregion(cart_rom_region());
	if (!romregion || !romregion->bytes())
		return std::error_condition();

	auto const bytes(romregion->bytes());
	if (0x400000 < bytes)
	{
		osd_printf_error("Unsupported cartridge ROM size");
		return image_error::INVALIDLENGTH;
	}

	m_rom_mask = device_generic_cart_interface::map_non_power_of_two(
		unsigned(romregion->bytes()),
		[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
		{
			m_rom->configure_entry(0, &base[0]);
		});
	m_rom_mirror = 0x3f'ffff ^ m_rom_mask;

	logerror("Map linear rom with mask: %08x mirror: %08x\n", m_rom_mask, m_rom_mirror);

	return std::error_condition();
}

void megadrive_rom_device::device_start()
{
}

void megadrive_rom_device::device_reset()
{
}

void megadrive_rom_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
}

/*
 * Super Street Fighter II
 *
 * Sega 315-5709 or 315-5779 mapper
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_SSF2, megadrive_rom_ssf2_device, "megadrive_rom_ssf2", "Megadrive SSF2 ROM cart")

megadrive_rom_ssf2_device::megadrive_rom_ssf2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom_bank(*this, "rom_bank_%u", 0U)
	, m_sram_view(*this, "sram_view")
{
}

megadrive_rom_ssf2_device::megadrive_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_ssf2_device(mconfig, MEGADRIVE_ROM_SSF2, tag, owner, clock)
{
}

void megadrive_rom_ssf2_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x08'0000;
	device_generic_cart_interface::map_non_power_of_two(
		unsigned(romregion->bytes() / page_size),
		[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
		{
			for (int i = 0; i < 8; i++)
				m_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
		});
}

void megadrive_rom_ssf2_device::device_reset()
{
	for (int i = 0; i < 8; i++)
		m_rom_bank[i]->set_entry(i);
	// Starts in SRAM disabled mode, thru /VRES connected
	// (game initializes $f1 after checking memory banks at $30'0000)
	m_sram_view.disable();
}

// need this for subclassing ("A memory_view can be present in only one address map")
void megadrive_rom_ssf2_device::cart_bank_map(address_map &map)
{
	for (int i = 0; i < 8; i++)
	{
		const u32 page_size = 0x08'0000;

		map(0x00'0000 | (page_size * i), 0x07'ffff | (page_size * i)).bankr(m_rom_bank[i]);
	}
}

void megadrive_rom_ssf2_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).m(*this, FUNC(megadrive_rom_ssf2_device::cart_bank_map));
	map(0x20'0000, 0x3f'ffff).view(m_sram_view);
	m_sram_view[0](0x20'0000, 0x3f'ffff).unmaprw();
}

void megadrive_rom_ssf2_device::time_io_map(address_map &map)
{
	map(0xf1, 0xf1).lw8(NAME([this] (offs_t offset, u8 data) {
		if (BIT(data, 1))
			m_sram_view.disable();
		else
			m_sram_view.select(0);
	}));
	map(0xf3, 0xf3).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[1]->set_entry(data & 0xf); }));
	map(0xf5, 0xf5).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[2]->set_entry(data & 0xf); }));
	map(0xf7, 0xf7).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[3]->set_entry(data & 0xf); }));
	map(0xf9, 0xf9).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[4]->set_entry(data & 0xf); }));
	map(0xfb, 0xfb).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[5]->set_entry(data & 0xf); }));
	map(0xfd, 0xfd).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[6]->set_entry(data & 0xf); }));
	map(0xff, 0xff).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[7]->set_entry(data & 0xf); }));
}
