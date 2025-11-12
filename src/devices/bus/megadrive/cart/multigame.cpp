// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Multigame cart mappers, with CMOS latching on power/reset

TODO:
- sportg mirroring is unconfirmed (may just rotate starting address instead);

**************************************************************************************************/

#include "emu.h"
#include "multigame.h"

#include "bus/generic/slot.h"

/*
 * TecToy Sport Games
 * https://segaretro.org/Sport_Games
 *
 * Sega MPR-19945-MX ROM with a 74HC74N and a 74HC00N attached.
 * Games bankswitch also changes with power switch according to manual,
 * for simplicity we just use /VRES only.
 *
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_TECTOY_SPORTS, megadrive_tectoy_sports_device, "megadrive_tectoy_sports", "Megadrive TecToy Sport Games cart")

megadrive_tectoy_sports_device::megadrive_tectoy_sports_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_TECTOY_SPORTS, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom(*this, "rom")
{
}

void megadrive_tectoy_sports_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x10'0000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_rom->configure_entry(entry, &base[page * page_size]);
			});
	// m_rom_mirror = 0x30'0000;

	// No info about what is the default game on cold boot, assume Super Volley Ball
	m_game_sel = 2;
	save_item(NAME(m_game_sel));
}

void megadrive_tectoy_sports_device::device_reset()
{
	m_game_sel ++;
	m_game_sel %= 3;

	const std::string game_names[] = { "Super Volley Ball", "World Championship Soccer II", "Super Real Basketball" };

	logerror("Game mounted: %s\n", game_names[m_game_sel]);
	m_rom->set_entry(m_game_sel);
}

void megadrive_tectoy_sports_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x0f'ffff).mirror(0x30'0000).bankr(m_rom);
}

/*
 * Codemasters 2-in-1
 * https://segaretro.org/2_Games_on_One_Cart:_Fantastic_Dizzy_and_Cosmic_Spacehead
 * https://segaretro.org/Double_Hits:_Micro_Machines_/_Psycho_Pinball
 *
 * "One of the games will begin: for example Cosmic Spacehead", again suggesting that cold boot
 * may still have preserved (static) state.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_CM2IN1, megadrive_cm2in1_device, "megadrive_cm2in1", "Megadrive Codemasters 2 games on 1 cart")

megadrive_cm2in1_device::megadrive_cm2in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_CM2IN1, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom(*this, "rom")
{
}

void megadrive_cm2in1_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x20'0000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_rom->configure_entry(entry, &base[page * page_size]);
			});

	// start in a predictable manner
	m_game_sel = 1;
	save_item(NAME(m_game_sel));
}

void megadrive_cm2in1_device::device_reset()
{
	m_game_sel ++;
	m_game_sel &= 1;

	m_rom->set_entry(m_game_sel);
}

void megadrive_cm2in1_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).mirror(0x20'0000).bankr(m_rom);
}

/*
 * 3 in 1 Flashback - World Championship Soccer - Tecmo World Cup 92
 * 3 in 1 Road Rash - Ms. Pac-Man - Block Out
 *
 * Unknown source (pirate but with copyright text on back cover!?)
 *
 * Uneven bank scheme, the two soccer games lies at $18'0000 and $1c'0000 so we simplify with a
 * memory_view. Road Rash cart is half total size/page size, so we need to capture ROM length at
 * init time.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_3IN1FWT, megadrive_3in1fwt_device, "megadrive_3in1fwt", "Megadrive Flashback 3-in-1 cart")

megadrive_3in1fwt_device::megadrive_3in1fwt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_3IN1FWT, tag, owner, clock)
	, m_rom_bank(*this, "rom_bank")
	, m_rom_view(*this, "rom_view")
{
}

void megadrive_3in1fwt_device::device_start()
{
	megadrive_rom_device::device_start();
	memory_region *const romregion(cart_rom_region());

	m_rom_mask = romregion->bytes() - 1;
	const u32 page_size = romregion->bytes() / 8;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8(), pg_mult = page_size] (unsigned entry, unsigned page)
			{
				m_rom_bank->configure_entry(entry, &base[page * pg_mult]);
			});

	m_page_mask = page_size - 1;

	// start in a predictable manner
	m_game_sel = 2;
	save_item(NAME(m_game_sel));
}

void megadrive_3in1fwt_device::device_reset()
{
	megadrive_rom_device::device_reset();
	m_game_sel ++;
	m_game_sel %= 3;

	m_rom_view.select(m_game_sel);
	// unknown order
	if (m_game_sel != 0)
	{
		u8 page_sel = 6 + ((m_game_sel & 1) ^ 1);
		logerror("Page selection %02x\n", page_sel);
		m_rom_bank->set_entry(page_sel);
	}
}

void megadrive_3in1fwt_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).view(m_rom_view);
	m_rom_view[0](0x00'0000, m_rom_mask ).bankr(m_rom);
	m_rom_view[1](0x00'0000, m_page_mask).bankr(m_rom_bank);
	m_rom_view[2](0x00'0000, m_page_mask).bankr(m_rom_bank);
}

