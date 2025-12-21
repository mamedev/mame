// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Sega Channel Game no Kanzume "digest" RAM cart, developed by CRI

https://segaretro.org/Game_no_Kanzume_Otokuyou

TODO:
- some unknowns, needs PCB picture

**************************************************************************************************/

#include "emu.h"
#include "seganet.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_SEGANET, megadrive_seganet_device, "megadrive_seganet", "Megadrive Seganet Game no Kanzume RAM cart")


megadrive_seganet_device::megadrive_seganet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_SEGANET, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom(*this, "rom")
	, m_ram_view(*this, "ram_view")
{
}

void megadrive_seganet_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes()),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_rom->configure_entry(0, &base[0]);
			});
}

void megadrive_seganet_device::device_reset()
{
	m_ram_view.disable();
}

void megadrive_seganet_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x2f'ffff).bankr(m_rom);
	// NOTE: menu system writes extra couple writes at $40000,
	// programming mistake?
	map(0x00'0000, 0x03'ffff).view(m_ram_view);
	m_ram_view[0](0x00'0000, 0x03'ffff).ram();
}

void megadrive_seganet_device::time_io_map(address_map &map)
{
//  map(0x01, 0x01) unknown, used in tandem with 0xf1 writes, ram bank select?
	map(0xf1, 0xf1).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// assumed, bclr #$1 at PC=5e282
			if (BIT(data, 1))
				m_ram_view.disable();
			else
				m_ram_view.select(0);
			// bit 0: write protect as per SSF2 mapper?
			if (data & 0xfd)
				popmessage("megadrive_seganet: unhandled $f1 write %02x", data);
		})
	);
}
