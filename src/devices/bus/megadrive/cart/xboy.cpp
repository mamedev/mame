// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

X Boy published/developed cart mappers

**************************************************************************************************/


#include "emu.h"
#include "xboy.h"

#include "bus/generic/slot.h"


/*
 * The King of Fighters '99
 * https://segaretro.org/King_of_Fighters_98%27
 *
 * Earlier protection variant
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_KOF98, megadrive_unl_kof98_device, "megadrive_unl_kof98", "Megadrive The King of Fighters '98 cart")

megadrive_unl_kof98_device::megadrive_unl_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_KOF98, tag, owner, clock)
{
}

void megadrive_unl_kof98_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// everytime, trashes stack otherwise
	map(0x48'0000, 0x4b'ffff).lr16(NAME([] () { return 0xaa00; }));
	// when selecting Arcade or VS., pointer to reach above
	map(0x4c'0000, 0x4f'ffff).lr16(NAME([] () { return 0xf000; }));
}


/*
 * A Bug's Life
 * https://segaretro.org/A_Bug%27s_Life_(Mega_Drive)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_BUGSLIFE, megadrive_unl_bugslife_device, "megadrive_unl_bugslife", "Megadrive A Bug's Life cart")

megadrive_unl_bugslife_device::megadrive_unl_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_BUGSLIFE, tag, owner, clock)
{
}

void megadrive_unl_bugslife_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x28; }));
	// those two are patched by SW
	map(0x02, 0x03).lr16(NAME([] () { return 0x01; }));
	map(0x3e, 0x3f).lr16(NAME([] () { return 0x1f; }));
}

/*
 * Pokemon Monster
 * https://segaretro.org/Pocket_Monster
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_POKEMONA, megadrive_unl_pokemona_device, "megadrive_unl_pokemona", "Megadrive Pokemon Monster alt cart")

megadrive_unl_pokemona_device::megadrive_unl_pokemona_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_POKEMONA, tag, owner, clock)
{
}

void megadrive_unl_pokemona_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x14; }));
	// those two are patched by SW
	map(0x02, 0x03).lr16(NAME([] () { return 0x01; }));
	map(0x3e, 0x3f).lr16(NAME([] () { return 0x1f; }));
}

/*
 * The King of Fighters '99
 * https://segaretro.org/The_King_of_Fighters_%2799_(Mega_Drive)
 *
 * Writes "Secondary memory access failure" if $a13000 returns & 0xf != 0
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_KOF99, megadrive_unl_kof99_device, "megadrive_unl_kof99", "Megadrive The King of Fighters '99 cart")

megadrive_unl_kof99_device::megadrive_unl_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_KOF99, tag, owner, clock)
{
}

void megadrive_unl_kof99_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).lr16(NAME([] () { return 0x00; }));
	map(0x02, 0x03).lr16(NAME([] () { return 0x01; }));
	map(0x3e, 0x3f).lr16(NAME([] () { return 0x1f; }));
}

/*
 * pokestad          Pokemon Stadium
 * https://segaretro.org/Pokemon_Stadium
 *
 * lionkin3           Lion King 3
 * mulan              Hua Mu Lan - Mulan
 * pokemon2           Pocket Monsters 2
 * souledge           Soul Edge vs Samurai Spirits
 * sdkong99/skkong99  Super Donkey Kong 99
 * topf               Top Fighter 2000 MK VIII
 * https://segaretro.org/Top_Fighter_2000_MK_VIII
 *
 * gunfight Gunfight 3 in 1
 * https://segaretro.org/Gunfight_3_in_1
 *
 * Obfuscated bankswitch mechanism + a bitswap based protection device.
 *
 * TODO:
 * - pokestad is the only game that doesn't access protection, verify on HW if it has it anyway.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_TOPF, megadrive_unl_topf_device, "megadrive_unl_topf", "Megadrive Top Fighter cart")

megadrive_unl_topf_device::megadrive_unl_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_TOPF, tag, owner, clock)
	, m_rom_bank(*this, "rom_bank_%u", 0U)
	, m_rom_view(*this, "rom_view")
{
}

void megadrive_unl_topf_device::device_start()
{
	megadrive_rom_device::device_start();
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x00'8000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 32; i++)
					m_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
			});


	save_item(NAME(m_prot_latch));
	save_item(NAME(m_prot_mode));
}

void megadrive_unl_topf_device::device_reset()
{
	m_prot_latch = 0;
	m_prot_mode = 0;
	m_rom_view.select(0);
}

void megadrive_unl_topf_device::prot_latch_w(u8 data)
{
	m_prot_latch = data;
}

void megadrive_unl_topf_device::prot_mode_w(u8 data)
{
	m_prot_mode = data & 3;
}

u8 megadrive_unl_topf_device::prot_data_r()
{
	u8 res = 0;
	switch(m_prot_mode)
	{
		case 0: res = (m_prot_latch << 1); break;
		case 1: res = (m_prot_latch >> 1); break;
		case 2: res = bitswap<8>(m_prot_latch, 3, 2, 1, 0, 7, 6, 5, 4); break;
		case 3: res = bitswap<8>(m_prot_latch, 0, 1, 2, 3, 4, 5, 6, 7); break;
	}

	return res;
}

void megadrive_unl_topf_device::cart_map(address_map &map)
{
	// TODO: just 0x0f'ffff ?
	map(0x00'0000, 0x1f'ffff).view(m_rom_view);
	m_rom_view[0](0x00'0000, 0x1f'ffff).bankr(m_rom);
	for (int bank = 0; bank < 16; bank++)
	{
		u32 bank_base = bank * 0x1'0000;
		m_rom_view[1](0x00'0000 | bank_base, 0x00'7fff | bank_base).bankr(m_rom_bank[bank * 2 + 0]);
		m_rom_view[1](0x00'8000 | bank_base, 0x00'ffff | bank_base).bankr(m_rom_bank[bank * 2 + 1]);
	}

	map(0x60'0001, 0x60'0001).mirror(0x0f'fff8).w(FUNC(megadrive_unl_topf_device::prot_latch_w));
	map(0x60'0003, 0x60'0003).mirror(0x0f'fff8).w(FUNC(megadrive_unl_topf_device::prot_mode_w));
	map(0x60'0005, 0x60'0005).mirror(0x0f'fff8).r(FUNC(megadrive_unl_topf_device::prot_data_r));

	map(0x70'0000, 0x70'0000).mirror(0x0f'ffff).lw8(NAME([this] (u8 data) {
		if (data)
		{
			m_rom_view.select(1);
			for (int i = 0; i < 16; i++)
			{
				m_rom_bank[i * 2 + 0]->set_entry((i * 2 | data) & 0x3f);
				m_rom_bank[i * 2 + 1]->set_entry((i * 2 | (data | 1)) & 0x3f);
			}
		}
		else
		{
			m_rom_view.select(0);
		}
	}));
}
