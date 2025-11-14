// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Gamtec/Chuanpu/Never Ending Soft cart mappers

Using a fixed data (discrete?) chip at $40'0000 as protection device

**************************************************************************************************/

#include "emu.h"
#include "gamtec.h"

#include "bus/generic/slot.h"


/*
 * 16 Zhang Mahjong II
 * https://segaretro.org/16_Zhang_Mahjong_II
 *
 * TODO:
 * - unknown purpose for ports $400002 and $400006 (spotted thru DASM code analysis)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_TILESMJ2, megadrive_unl_tilesmj2_device, "megadrive_unl_tilesmj2", "Megadrive 16 Zhang Mahjong II cart")

megadrive_unl_tilesmj2_device::megadrive_unl_tilesmj2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_TILESMJ2, tag, owner, clock)
{
}

void megadrive_unl_tilesmj2_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
//  map(0x40'0000, 0x40'0000) unused?
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x98; })); // PC=193c, <unknown>
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xc9; })); // PC=3ec, startup
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0x18; })); // PC=1626, attract
}

/*
 * Líng Huàn Dàoshi - Super Magician
 * https://segaretro.org/Ling_Huan_Daoshi
 *
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_ELFWOR, megadrive_unl_elfwor_device, "megadrive_unl_elfwor", "Megadrive Ling Huan Daoshi Super Magician cart")

megadrive_unl_elfwor_device::megadrive_unl_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_ELFWOR, tag, owner, clock)
{
}

void megadrive_unl_elfwor_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x55; }));
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x0f; }));
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xc9; }));
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0x18; }));
}

/*
 * Huan Le Tao Qi Shu: Smart Mouse (huanle original set, not the Piko Interactive 2017 re-release)
 * https://segaretro.org/Huan_Le_Tao_Qi_Shu:_Smart_Mouse
 *
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SMOUSE, megadrive_unl_smouse_device, "megadrive_unl_smouse", "Megadrive Huan Le Tao Qi Shu Smart Mouse cart")

megadrive_unl_smouse_device::megadrive_unl_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SMOUSE, tag, owner, clock)
{
}

void megadrive_unl_smouse_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x55; }));
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x0f; }));
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xaa; }));
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0xf0; }));
}

/*
 * Ya Se Chuan Shuo / Wu Kong Wai Zhuan
 * https://segaretro.org/Ya_Se_Chuan_Shuo
 * https://segaretro.org/Wu_Kong_Wai_Zhuan
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_YASECH, megadrive_unl_yasech_device, "megadrive_unl_yasech", "Megadrive Ya Se Chuan Shuo cart")

megadrive_unl_yasech_device::megadrive_unl_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_YASECH, tag, owner, clock)
	, m_nvram(*this, "nvram")
{
}

void megadrive_unl_yasech_device::device_add_mconfig(machine_config &config)
{
	// init doesn't really matter: game specifically looks for a specific header pattern at tail
	// (0x06, 0x09, 0x01, 0x08) in both banks, flushing to 0 if not found.
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void megadrive_unl_yasech_device::device_start()
{
	megadrive_rom_device::device_start();
	const u32 nvram_size = 0x1000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

u16 megadrive_unl_yasech_device::nvram_r(offs_t offset)
{
	const u32 nvram_offset = offset & 0xfff;
	return 0xff00 | m_nvram_ptr[nvram_offset];
}

void megadrive_unl_yasech_device::nvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		const u32 nvram_offset = offset & 0xfff;
		m_nvram_ptr[nvram_offset] = data & 0xff;
	}
}


void megadrive_unl_yasech_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x20'0000, 0x20'1fff).rw(FUNC(megadrive_unl_yasech_device::nvram_r), FUNC(megadrive_unl_yasech_device::nvram_w));
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x63; }));
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x98; }));
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xc9; }));
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0x18; }));
}

/*
 * Meng Huan Shui Guo Pan: 777 Casino
 * https://segaretro.org/Meng_Huan_Shui_Guo_Pan:_777_Casino
 *
 * Protection check here is subtle: it's done from Z80 space when new game is selected.
 * If fails, going in a parlor and talk with a cashier twice will crash the game by Z80 trashing
 * work RAM.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_777CASINO, megadrive_unl_777casino_device, "megadrive_unl_777casino", "Megadrive 777 Casino cart")

megadrive_unl_777casino_device::megadrive_unl_777casino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_777CASINO, tag, owner, clock)
{
}

void megadrive_unl_777casino_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// TODO: unconfirmed values (carried over from GPGX)
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x63; }));
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x98; }));
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xc9; }));
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0x18; }));
}

/*
 * Soul Blade
 * https://segaretro.org/Soul_Blade
 *
 * Game also implements self-modifying code at PC=28026 .. PC=28066, presumably against copiers
 *
 * Shi San Zhang Ma Jiang: Zhong Guo Mei Nv Pian
 * (assumed, only $40'0004 checked on title to gameplay transition)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SOULBLADE, megadrive_unl_soulblade_device, "megadrive_unl_soulblade", "Megadrive Soul Blade cart")

megadrive_unl_soulblade_device::megadrive_unl_soulblade_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SOULBLADE, tag, owner, clock)
{
}

void megadrive_unl_soulblade_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// unknown, assuming same as other entries
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x63; }));
	// alt value on fight startup (PC=28B62 #-$68), unused?
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x98; }));
	// after set amount of blocks/damage during fights (PC=2844C #-$56 or #-$37), causes GFX garbage
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xc9; }));
	// on fight startup, twice (PC=28B58 #-$10), locks up if unhappy
	map(0x40'0006, 0x40'0006).lr8(NAME([] () { return 0xf0; }));
}

/*
 * Super Bubble Bobble MD
 * https://segaretro.org/Super_Bubble_Bobble_MD
 * https://bootleggames.fandom.com/wiki/Super_Bubble_Bobble_MD
 *
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SUPRBUBL, megadrive_unl_suprbubl_device, "megadrive_unl_suprbubl", "Megadrive Super Bubble Bobble cart")

megadrive_unl_suprbubl_device::megadrive_unl_suprbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SUPRBUBL, tag, owner, clock)
{
}

void megadrive_unl_suprbubl_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// just checked once, on startup
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x55; }));
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0x0f; }));
}

/*
 * Chao Ji Mahjong Club
 * https://segaretro.org/Chao_Ji_Mahjong_Club
 *
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_CJMJCLUB, megadrive_unl_cjmjclub_device, "megadrive_unl_cjmjclub", "Megadrive Chao Ji Mahjong Club cart")

megadrive_unl_cjmjclub_device::megadrive_unl_cjmjclub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_CJMJCLUB, tag, owner, clock)
{
}

void megadrive_unl_cjmjclub_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// at startup, passed as dword to $ffff08
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x90; }));
	map(0x40'0002, 0x40'0002).lr8(NAME([] () { return 0xd3; }));
}

/*
 * Creaton Softech published games (beyond the one above)
 *
 * majianqr https://segaretro.org/Ma_Jiang_Qing_Ren:_Ji_Ma_Jiang_Zhi
 * fengkuan https://segaretro.org/Feng_Kuang_Tao_Hua_Yuan
 * mhpoker  https://segaretro.org/Du_Shen_Zhi_Meng_Huan_Poker
 * btlchess https://segaretro.org/Zhan_Qi_Chinese_Battle_Chess
 *
 * Uses $400000, $401000 instead of the usual linear mapping (just hooked up differently?)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_MJLOV, megadrive_unl_mjlov_device, "megadrive_unl_mjlov", "Megadrive Creaton Softec cart")

megadrive_unl_mjlov_device::megadrive_unl_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_MJLOV, tag, owner, clock)
{
}

void megadrive_unl_mjlov_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x90; }));
	map(0x40'1000, 0x40'1000).lr8(NAME([] () { return 0xd3; }));
}

/*
 * San Guo Yan Yi: Huo Shao Chi Bi / The Battle of Red Cliffs
 * https://segaretro.org/San_Guo_Yan_Yi:_Huo_Shao_Chi_Bi
 *
 * Slight ROM encryption
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_REDCLIFF, megadrive_unl_redcliff_device, "megadrive_unl_redcliff", "Megadrive Battle of Red Cliffs cart")


megadrive_unl_redcliff_device::megadrive_unl_redcliff_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_UNL_REDCLIFF, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_rom(*this, "rom")
{
}

void megadrive_unl_redcliff_device::device_start()
{
	memory_region *const romregion(cart_rom_region());
	m_decrypted_rom.resize(0x40'0000);

	auto enc = &romregion->as_u8();
	int i;

	std::fill(m_decrypted_rom.begin(), m_decrypted_rom.end(), 0xff ^ 0x40);

	// NOTE: dump is oversized, original in .mdx format?
	for (i = 4; i < romregion->bytes(); i++)
		m_decrypted_rom[i - 4] = enc[i] ^ 0x40;

	m_rom->configure_entry(0, &m_decrypted_rom[0]);
}

void megadrive_unl_redcliff_device::device_reset()
{
}

void megadrive_unl_redcliff_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	// NOTE: may not even use the same "chip" described in this file
	// gameplay
	map(0x40'0000, 0x40'0000).lr8(NAME([] () { return 0x55; }));
	// startup
	map(0x40'0004, 0x40'0004).lr8(NAME([] () { return 0xaa; }));
}

/*
 * Squirrel King
 * https://segaretro.org/Squirrel_King
 *
 * Uses area $400000-$400006 as r/w latch
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SQUIRRELK, megadrive_unl_squirrelk_device, "megadrive_unl_squirrelk", "Megadrive Squirrel King cart")

megadrive_unl_squirrelk_device::megadrive_unl_squirrelk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SQUIRRELK, tag, owner, clock)
{
}

void megadrive_unl_squirrelk_device::device_start()
{
	megadrive_rom_device::device_start();
	save_item(NAME(m_latch));
}

void megadrive_unl_squirrelk_device::device_reset()
{
	megadrive_rom_device::device_reset();
	// irrelevant, initialized at startup
	m_latch = 0xff;
}

void megadrive_unl_squirrelk_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0000, 0x40'0000).select(6).lrw8(
		NAME([this] (offs_t offset) {
			return m_latch;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_latch = data;
		})
	);
}

/*
 * The Lion King II
 * https://segaretro.org/The_Lion_King_II
 *
 * Similar to above except two latches instead of one
 * Game also implements self-modifying code at PC=808DA .. PC=8095A, presumably against copiers
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_LIONKING2, megadrive_unl_lionking2_device, "megadrive_unl_lionking2", "Megadrive The Lion King II cart")

megadrive_unl_lionking2_device::megadrive_unl_lionking2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_LIONKING2, tag, owner, clock)
{
}

void megadrive_unl_lionking2_device::device_start()
{
	megadrive_rom_device::device_start();
	save_pointer(NAME(m_latch), 2);
}

void megadrive_unl_lionking2_device::device_reset()
{
	megadrive_rom_device::device_reset();
	// irrelevant, initialized at startup
	m_latch[0] = m_latch[1] = 0xff;
}

void megadrive_unl_lionking2_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x40'0000, 0x40'0000).select(4).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_latch[offset] = data;
		})
	);
	map(0x40'0002, 0x40'0002).select(4).lr8(
		NAME([this] (offs_t offset, u8 data) {
			return m_latch[offset];
		})
	);
}

/*
 * Tenchi o Kurau III: Sangoku Gaiden / Tun Shi Tian Di 3: San Guo Wai Chuan / Chinese Fighter III
 * https://segaretro.org/Tenchi_o_Kurau_III:_Sangoku_Gaiden
 *
 * Pseudo-banking scheme looks similar to Top Fighter, otherwise using Squirrel King latching
 * obfuscated by address lanes.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_CHINF3, megadrive_unl_chinf3_device, "megadrive_unl_chinf3", "Megadrive Chinese Fighter III cart")

megadrive_unl_chinf3_device::megadrive_unl_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_CHINF3, tag, owner, clock)
	, m_page_rom(*this, "page_rom")
	, m_page_view(*this, "page_view")
{
}

void megadrive_unl_chinf3_device::device_start()
{
	megadrive_rom_device::device_start();
	memory_region *const romregion(cart_rom_region());
	const u32 page_size = 0x01'0000;
	device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / page_size),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				m_page_rom->configure_entry(entry, &base[page * page_size]);
			});

	save_pointer(NAME(m_prot_latch), 4);
}

void megadrive_unl_chinf3_device::device_reset()
{
	megadrive_rom_device::device_reset();
	m_page_view.select(0);
	std::fill_n(&m_prot_latch[0], 4, 0xff);
}

void megadrive_unl_chinf3_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).view(m_page_view);
	m_page_view[0](0x00'0000, 0x1f'ffff).bankr(m_rom);
	m_page_view[1](0x00'0000, 0x00'ffff).mirror(0x1f'0000).bankr(m_page_rom);
	map(0x40'0000, 0x40'0000).select(0xc).mirror(0x0f'fff3).lrw8(
		NAME([this] (offs_t offset) {
			return m_prot_latch[offset >> 2];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_prot_latch[offset >> 2] = data;
		})
	);
	map(0x60'0000, 0x60'0000).mirror(0x0f'ffff).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (data)
			{
				m_page_rom->set_entry(data & 0xf);
				m_page_view.select(1);
			}
			else
			{
				m_page_view.select(0);
			}
		})
	);
}
