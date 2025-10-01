// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Super Fighter Team protected mappers

**************************************************************************************************/

#include "emu.h"
#include "sfteam.h"

#include "bus/generic/slot.h"


/*
 * Xin Qi Gai Wang Zi
 *
 * NVRAM at $40'0000
 * Some writes in ROM space at startup, PC=B4BEA (at least) tests that $100-$102
 * returns SW vectors 0x46fc2700 and resets if unhappy, presumably against copiers.
 *
 * NOTES:
 * - xinqig doesn't work on Teradrive TMSS, no SEGA at $100. Cursory testing was performed.
 * - To quickly check saving: start a new game, skip intro with start button then go to the
 *   bottom-right house, enter into the aura circle and press start to bring inventory, select the
 *   left diary with pen icon (normally disabled).
 * - beggarp (below) reuses aforementioned copier check, expects the original bank to live at $0.
 *   To trigger: after intro, go all the way down then first house on the left (player PoV).
 *   Character will stumble on key shaped tile, go forward a bit then exit the house
 *   (bp b4bea to be sure it's triggering)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_XINQIG, megadrive_unl_xinqig_device, "megadrive_unl_xinqig", "Megadrive Xin Qi Gai Wang Zi cart")

megadrive_unl_xinqig_device::megadrive_unl_xinqig_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, type, tag, owner, clock)
{
}

megadrive_unl_xinqig_device::megadrive_unl_xinqig_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_unl_xinqig_device(mconfig, MEGADRIVE_UNL_XINQIG, tag, owner, clock)
{
}

u16 megadrive_unl_xinqig_device::get_nvram_length()
{
	return 0x4000;
}

void megadrive_unl_xinqig_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0000, 0x40'7fff).rw(FUNC(megadrive_unl_xinqig_device::nvram_r), FUNC(megadrive_unl_xinqig_device::nvram_w));
}

/*
 * beggarp Beggar Prince
 *
 * Super Fighter Team rerelease of xinqig
 *
 * Reuses the original C&E mapper with an extra $e00 write handler.
 *
 * $3c'0030 contains a lengthy message explaining why this is protected to begin with.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_HB_BEGGARP, megadrive_hb_beggarp_device, "megadrive_hb_beggarp", "Megadrive Beggar Prince cart")

megadrive_hb_beggarp_device::megadrive_hb_beggarp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_unl_xinqig_device(mconfig, type, tag, owner, clock)
	, m_sf_rom_bank(*this, "sf_rom_bank")
	, m_sf_rom_view(*this, "sf_rom_view")
	, m_sram_view(*this, "sram_view")
{
}

megadrive_hb_beggarp_device::megadrive_hb_beggarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_hb_beggarp_device(mconfig, MEGADRIVE_HB_BEGGARP, tag, owner, clock)
{
}

void megadrive_hb_beggarp_device::device_start()
{
	megadrive_unl_xinqig_device::device_start();
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x04'0000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_sf_rom_bank->configure_entry(entry, &base[page * page_size]);
			});
}


void megadrive_hb_beggarp_device::device_reset()
{
	megadrive_unl_xinqig_device::device_reset();
	m_sram_view.disable();
	m_sf_rom_view.disable();
	// remaps 0x38'0000 to 0
	m_sf_rom_bank->set_entry(0x0e);
	m_bank_write_lock = false;
}

void megadrive_hb_beggarp_device::bank_write_w(offs_t offset, u16 data, u16 mem_mask)
{
	// Writes 0xa0a0 after the Super Fighter Team logo, that unlocks the NVRAM
	logerror("bank_write_w: %04x & %04x (%d)\n", data, mem_mask, m_bank_write_lock);
	if (ACCESSING_BITS_0_7)
	{
		if (m_bank_write_lock)
			return;
		if (BIT(data, 7))
		{
			m_sram_view.select(0);
			m_sf_rom_view.select(0);
		}
		else
		{
			m_sram_view.disable();
			m_sf_rom_view.disable();
		}

		// TODO: is the game actually using this at all?
		if (BIT(data, 6))
			popmessage("megadrive_hb_beggarp_device: cart locked! %04x & %04x", data, mem_mask);

		// once enabled this lock needs an hard reset to lift
		m_bank_write_lock = !!(BIT(data, 5));
	}
}

void megadrive_hb_beggarp_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x00'0000, 0x03'ffff).view(m_sf_rom_view);
	m_sf_rom_view[0](0x00'0000, 0x03'ffff).bankr(m_sf_rom_bank);
	map(0x00'0e00, 0x00'0e01).w(FUNC(megadrive_hb_beggarp_device::bank_write_w));
	map(0x40'0000, 0x40'7fff).view(m_sram_view);
	m_sram_view[0](0x40'0000, 0x40'7fff).rw(FUNC(megadrive_hb_beggarp_device::nvram_r), FUNC(megadrive_hb_beggarp_device::nvram_w));
}



/*
 * beggarp1 Beggar Prince rev 1
 *
 * Relocates NVRAM, probably to make game compatible with an expansion attached.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_HB_BEGGARP1, megadrive_hb_beggarp1_device, "megadrive_hb_beggarp1", "Megadrive Beggar Prince rev 1 cart")

megadrive_hb_beggarp1_device::megadrive_hb_beggarp1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_hb_beggarp_device(mconfig, MEGADRIVE_HB_BEGGARP1, tag, owner, clock)
{
}

void megadrive_hb_beggarp1_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x00'0000, 0x03'ffff).view(m_sf_rom_view);
	m_sf_rom_view[0](0x00'0000, 0x03'ffff).bankr(m_sf_rom_bank);
	map(0x00'0e00, 0x00'0e01).w(FUNC(megadrive_hb_beggarp1_device::bank_write_w));
	map(0x3c'0000, 0x3f'ffff).view(m_sram_view);
	m_sram_view[0](0x3c'0000, 0x3c'7fff).mirror(0x03'8000).rw(FUNC(megadrive_hb_beggarp1_device::nvram_r), FUNC(megadrive_hb_beggarp1_device::nvram_w));
}

/*
 * wukong Legend of Wukong
 *
 * More $e00 style writes. Swaps the two halves around.
 *
 * TODO:
 * - technically should derive from megadrive_unl_yasech_device
 *
 */


DEFINE_DEVICE_TYPE(MEGADRIVE_HB_WUKONG, megadrive_hb_wukong_device, "megadrive_hb_wukong", "Megadrive Legend of Wukong cart")

megadrive_hb_wukong_device::megadrive_hb_wukong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, MEGADRIVE_HB_WUKONG, tag, owner, clock)
	, m_sf_rom_bank(*this, "sf_rom_bank")
	, m_sram_view(*this, "sram_view")
{
}

void megadrive_hb_wukong_device::device_start()
{
	megadrive_rom_tplay96_device::device_start();
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x20'0000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_sf_rom_bank->configure_entry(entry, &base[page * page_size]);
			});

}

void megadrive_hb_wukong_device::device_reset()
{
	megadrive_rom_tplay96_device::device_reset();
	m_sram_view.disable();
	m_sf_rom_bank->set_entry(1);
}

u16 megadrive_hb_wukong_device::get_nvram_length()
{
	// unverified, game just use up to $3c'095f (0x800)
	return 0x2000;
}

void megadrive_hb_wukong_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).bankr(m_rom);
	map(0x20'0000, 0x3f'ffff).bankr(m_sf_rom_bank);
	map(0x00'0e00, 0x00'0e01).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			// 0x00 on Super Fighter Team logo
			// 0x81 afterwards (where it checks that SRAM works)
			logerror("bank_write_w: %04x & %04x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_sf_rom_bank->set_entry(!BIT(data, 7));
				if (BIT(data, 0))
					m_sram_view.select(0);
				else
					m_sram_view.disable();
			}
		})
	);
	map(0x3c'0000, 0x3f'ffff).view(m_sram_view);
	m_sram_view[0](0x3c'0000, 0x3c'3fff).mirror(0x03'c000).rw(FUNC(megadrive_hb_wukong_device::nvram_r), FUNC(megadrive_hb_wukong_device::nvram_w));
}

/*
 * starodys Star Odyssey
 * https://segaretro.org/Star_Odyssey_(Super_Fighter_Team)
 *
 *
 *
 */


DEFINE_DEVICE_TYPE(MEGADRIVE_HB_STARODYS, megadrive_hb_starodys_device, "megadrive_hb_starodys", "Megadrive Star Odyssey cart")

megadrive_hb_starodys_device::megadrive_hb_starodys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, MEGADRIVE_HB_STARODYS, tag, owner, clock)
	, m_sf_rom_bank(*this, "sf_rom_bank_%u", 0U)
	, m_sf_rom_view(*this, "sf_rom_view")
	, m_sram_view(*this, "sram_view")
{
}

void megadrive_hb_starodys_device::device_start()
{
	megadrive_rom_tplay96_device::device_start();
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x04'0000;
	device_generic_cart_interface::map_non_power_of_two(
		unsigned(romregion->bytes() / page_size),
		[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
		{
			for (int i = 0; i < 5; i++)
				m_sf_rom_bank[i]->configure_entry(entry, &base[page * page_size]);
		});
}

void megadrive_hb_starodys_device::device_reset()
{
	megadrive_rom_tplay96_device::device_reset();
	m_sram_view.disable();
	for (int i = 0; i < 5; i++)
		m_sf_rom_bank[i]->set_entry(i);
	m_sf_rom_view.select(0);
	m_bank_num = 0;
	m_bank_write_lock = false;
}

u16 megadrive_hb_starodys_device::get_nvram_length()
{
	// checks up to $20'3fff
	return 0x8000;
}

void megadrive_hb_starodys_device::cart_map(address_map &map)
{
	//map(0x00'0000, 0x1f'ffff).mirror(0x20'0000).bankr(m_rom);
	map(0x00'0000, 0x1f'ffff).view(m_sf_rom_view);
	m_sf_rom_view[0](0x00'0000, 0x03'ffff).mirror(0x1c'0000).bankr(m_sf_rom_bank[0]);
	m_sf_rom_view[1](0x00'0000, 0x03'ffff).bankr(m_sf_rom_bank[0]);
	m_sf_rom_view[1](0x04'0000, 0x07'ffff).bankr(m_sf_rom_bank[1]);
	m_sf_rom_view[1](0x08'0000, 0x0b'ffff).bankr(m_sf_rom_bank[2]);
	m_sf_rom_view[1](0x0c'0000, 0x0f'ffff).bankr(m_sf_rom_bank[3]);
	m_sf_rom_view[1](0x10'0000, 0x13'ffff).bankr(m_sf_rom_bank[4]);
	map(0x00'0d00, 0x00'0d01).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask)
		{
			logerror("$d00: %04x & %04x (%d)\n", data, mem_mask, m_bank_write_lock);
			if (m_bank_write_lock)
				return;

			if (BIT(data, 7))
				m_sram_view.select(0);
			else
				m_sram_view.disable();
		})
	);
	map(0x00'0e00, 0x00'0e01).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask)
		{
			logerror("$e00: %04x & %04x (%d)\n", data, mem_mask, m_bank_write_lock);
			if (m_bank_write_lock)
				return;

			m_bank_write_lock = !!(BIT(~data, 7));
			m_sf_rom_view.select(BIT(data, 6));
			// TODO: is the game actually using this at all?
			if (BIT(data, 5))
				popmessage("megadrive_hb_starodys_device: cart locked! %04x & %04x", data, mem_mask);
		})
	);
	map(0x00'0f00, 0x00'0f01).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask)
		{
			logerror("$e00: %04x & %04x (%d)\n", data, mem_mask, m_bank_write_lock);
			if (m_bank_write_lock)
				return;

			m_bank_num = (data >> 4) & 7;
			for (int i = 0; i < 5; i++)
				m_sf_rom_bank[i]->set_entry((m_bank_num + i) & 7);
		})
	);
	map(0x20'0000, 0x2f'ffff).view(m_sram_view);
	m_sram_view[0](0x20'0000, 0x2f'ffff).rw(FUNC(megadrive_hb_starodys_device::nvram_r), FUNC(megadrive_hb_starodys_device::nvram_w));
}

void megadrive_hb_starodys_device::time_io_map(address_map &map)
{
	// $a13'001, $a13'005, $a13'009 checked at startup
	// $a13'00f after loading a game
	map(0x01, 0x01).mirror(0xfe).lr8(
		NAME([this] (offs_t offset) {
			return m_bank_num << 4;
		})
	);
}
