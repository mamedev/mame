// license:BSD-3-Clause
// copyright-holders:Angelo Salese
// thanks-to: David Haywood, Peter Wilhelmsen
/*************************************************************************************************************************

    Metal Freezer (c) 1989 Seibu

    driver by Angelo Salese, based off initial work by David Haywood
    thanks to Peter Wilhelmsen for the decryption

    HW seems the natural evolution of Dark Mist type.

    TODO:
    - A few video register bits still needs sorting out (needs HW tests perhaps?)
    - Nuke legacy video code and re-do it by using tilemap system.
    - sprites are ahead of 1/2 frames, needs sprite DMA fixed;
    - Writes at 0xb800-0xbfff or 0x8000-0x9fff during gameplays? (Check by allowing ROM write)
    - Flip screen support;
    - Why service mode returns all inputs as high? And why sound test doesn't seem to function at all, both BTANBs perhaps?

**************************************************************************************************************************/

#include "emu.h"
#include "t5182.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class metlfrzr_state : public driver_device
{
public:
	metlfrzr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_work_ram(*this, "wram"),
		m_vram(*this, "vram"),
		m_video_regs(*this, "vregs"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_mainbank(*this, "mainbank")
	{ }

	void metlfrzr(machine_config &config);

	void init_metlfrzr();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void legacy_bg_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void legacy_obj_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint32_t screen_update_metlfrzr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void output_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void metlfrzr_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_shared_ptr<uint8_t> m_work_ram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_video_regs;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	required_memory_bank m_mainbank;

	uint8_t m_fg_tilebank = 0;
	bool m_rowscroll_enable = false;
};


void metlfrzr_state::video_start()
{
	// assumes it can make an address mask with m_vram.length() - 1
	assert(!(m_vram.length() & (m_vram.length() - 1)));

	m_fg_tilebank = 0;
	m_rowscroll_enable = false;

	save_item(NAME(m_fg_tilebank));
	save_item(NAME(m_rowscroll_enable));
}

/*
 - video regs format:
    [0x06] ---- --x- used during title screen transition, unknown purpose
    [0x06] ---- ---x X scrolling 8th bit
    [0x15] always 0?
    [0x16] always 0?
    [0x17] xxxx xxxx X scrolling base value
    Notice that it's currently unknown how the game is really supposed to NOT enable scrolling during gameplay.
 */
void metlfrzr_state::legacy_bg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(m_fg_tilebank);
	const uint16_t vram_mask = (m_vram.length() - 1) >> 1;
	uint16_t x_scroll_value = m_video_regs[0x17] + ((m_video_regs[0x06] & 1) << 8);
	int x_scroll_base = (x_scroll_value >> 3) * 32;

	for (int count = 0; count< 32 * 33; count++)
	{
		int tile_base = count;
		int y = (count & 0x1f);
		int x_scroll_shift;
		if (y > 7 || m_rowscroll_enable == false)
		{
			tile_base += x_scroll_base;
			x_scroll_shift = (x_scroll_value & 7);
		}
		else
			x_scroll_shift = 0;
		tile_base &= vram_mask;
		int x = (count >> 5);

		const uint16_t tile = m_vram[tile_base * 2 + 0] + ((m_vram[tile_base * 2 + 1] & 0xf0) << 4);
		const uint8_t color = m_vram[tile_base * 2 + 1] & 0xf;

		gfx->transpen(bitmap, cliprect, tile, color, 0, 0, x * 8 - x_scroll_shift, y * 8, 0xf);
	}
}

/*
 sprite DMA:
    0xfe00-0xffff contains buffer for data to be copied.
    Sprites are currently lagging (noticeable during scrolling) therefore there must be either an automatic or manual trigger.
    Sprite seems to traverse from top to bottom priority-wise, other than that format is almost 1:1 with darkmist.cpp.
 sprite format:
    [0] tttt tttt tile number
    [1] x--- ---- X 8th bit
    [1] -ttt ---- tile bank
    [1] ---- cccc palette number
    [2] yyyy yyyy Y offset
    [3] xxxx xxxx X offset
*/
void metlfrzr_state::legacy_obj_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_2 = m_gfxdecode->gfx(2);
	gfx_element *gfx_3 = m_gfxdecode->gfx(3);
	uint8_t const *const base_spriteram = m_work_ram + 0xe00;

	for (int count = 0x200 - 4; count > -1; count -= 4)
	{
		gfx_element *cur_gfx = base_spriteram[count + 1] & 0x40 ? gfx_3 : gfx_2;
		const uint8_t tile_bank = (base_spriteram[count + 1] & 0x30) >> 4;
		const uint16_t tile = base_spriteram[count] | (tile_bank << 8);
		const uint8_t color = base_spriteram[count + 1] & 0xf;
		int y = base_spriteram[count + 2];
		int x = base_spriteram[count + 3];
		if (BIT(base_spriteram[count + 1], 7))
			x -= 256;

		cur_gfx->transpen(bitmap, cliprect, tile, color, 0, 0, x, y, 0xf);
	}
}

uint32_t metlfrzr_state::screen_update_metlfrzr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	legacy_bg_draw(bitmap, cliprect);
	legacy_obj_draw(bitmap, cliprect);
	return 0;
}

void metlfrzr_state::output_w(uint8_t data)
{
	// bit 7: flip screen
	// bit 6-5: coin lockouts
	// bit 4: tilemap ROM banking
	// bit 3-2: z80 ROM banking
	// bit 1: enabled during gameplay, rowscroll enable?
	// bit 0: enabled , unknown purpose (lamp?)
	// TODO: bits 1-0 might actually be sprite DMA enable mask/request
	machine().bookkeeping().coin_lockout_w(1, BIT(data, 6));
	machine().bookkeeping().coin_lockout_w(0, BIT(data, 5));
	m_fg_tilebank = (data & 0x10) >> 4;
	m_mainbank->set_entry((data & 0xc) >> 2);
	m_rowscroll_enable = bool(BIT(data, 1));

//  popmessage("%02x %02x",m_fg_tilebank,data & 3);
}

void metlfrzr_state::metlfrzr_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xcfff).ram().share(m_vram);
	map(0xd000, 0xd1ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0xd200, 0xd3ff).ram().w(m_palette, FUNC(palette_device::write_indirect_ext)).share("palette_ext");

	map(0xd400, 0xd47f).rw("t5182", FUNC(t5182_device::sharedram_r), FUNC(t5182_device::sharedram_w));

	map(0xd600, 0xd600).portr("P1");
	map(0xd601, 0xd601).portr("P2");
	map(0xd602, 0xd602).portr("START");
	map(0xd603, 0xd603).portr("DSW1");
	map(0xd604, 0xd604).portr("DSW2");
	map(0xd600, 0xd61f).writeonly().share(m_video_regs);

	map(0xd700, 0xd700).w(FUNC(metlfrzr_state::output_w));
	map(0xd710, 0xd710).w("t5182", FUNC(t5182_device::sound_irq_w));
	map(0xd711, 0xd711).r("t5182", FUNC(t5182_device::sharedram_semaphore_snd_r));
	// following two do swapped access compared to darkmist
	map(0xd712, 0xd712).w("t5182", FUNC(t5182_device::sharedram_semaphore_main_release_w));
	map(0xd713, 0xd713).w("t5182", FUNC(t5182_device::sharedram_semaphore_main_acquire_w));

	map(0xd800, 0xdfff).ram();
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).ram().share(m_work_ram);
}

void metlfrzr_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xf000, 0xffff).ram().share(m_work_ram); // executes code at 0xf5d5
}


static INPUT_PORTS_START( metlfrzr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_DIPNAME( 0x01, 0x01, "2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x40, 0x40, "2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "A" )
	PORT_DIPSETTING(    0x03, "B" )
	PORT_DIPSETTING(    0x01, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	// service mode returns these values divided by 10 (so 02/05/10 effectively means 20k, 50k, 100k)
	// TODO: check if it extends
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "20k, 50k, 100k" )
	PORT_DIPSETTING(    0x0c, "30k, 80k, 150k" )
	PORT_DIPSETTING(    0x04, "50k, 100k, 200k" )
	PORT_DIPSETTING(    0x00, "100k, 200k, 400k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	// disabling following enables intro / how to play screens
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Level_Select ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



void metlfrzr_state::machine_start()
{
	m_mainbank->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void metlfrzr_state::machine_reset()
{
	m_mainbank->set_entry(0);
}


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,4) },
	{ STEP4_INV(16,1), STEP4_INV(0,1) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout sprite_layout =
{
	16, 16,
	RGN_FRAC(1, 1),
	4,
	{ STEP4(0,4) },
	{ STEP4(0,1), STEP4(16,1), STEP4(64*8,1), STEP4(64*8+16,1) },
	{ STEP16(0,32) },
	128 * 8
};


static GFXDECODE_START(gfx_metlfrzr)
	GFXDECODE_ENTRY("tiles1",   0, tile_layout, 0x100, 16)
	GFXDECODE_ENTRY("tiles2",   0, tile_layout, 0x100, 16)
	GFXDECODE_ENTRY("sprites1", 0, sprite_layout,   0, 16)
	GFXDECODE_ENTRY("sprites2", 0, sprite_layout,   0, 16)
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(metlfrzr_state::scanline)
{
	int scanline = param;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h

	// TODO: check this irq.
	if (scanline == 0) // vblank-in irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80 - RST 08h
}

void metlfrzr_state::metlfrzr(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metlfrzr_state::metlfrzr_map);
	m_maincpu->set_addrmap(AS_OPCODES, &metlfrzr_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(metlfrzr_state::scanline), "screen", 0, 1);

	t5182_device &t5182(T5182(config, "t5182", XTAL(14'318'181)/4));
	t5182.ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	t5182.ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x200).set_indirect_entries(256 * 2);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_metlfrzr);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256 - 1, 16, 256 - 16 - 1);
	screen.set_screen_update(FUNC(metlfrzr_state::screen_update_metlfrzr));
	screen.set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181) / 4));    // 3.579545 MHz
	ymsnd.irq_handler().set("t5182", FUNC(t5182_device::ym2151_irq_handler));
	ymsnd.add_route(0, "mono", 0.5);
	ymsnd.add_route(1, "mono", 0.5);
}



ROM_START(metlfrzr)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("1.15j", 0x00000, 0x08000, CRC(f59b5fa2) SHA1(6033967dad5e64f45afbcb1b45c8eb79e0787afb))
	ROM_LOAD("2.14j", 0x10000, 0x10000, CRC(21ecc248) SHA1(2fccf7db73890faf7c489bfc43c88ded54d5052d))

	ROM_REGION(0x8000, "t5182:external", 0) // Toshiba T5182 external data ROM
	ROM_LOAD("3.4h", 0x0000, 0x8000, CRC(36f88e54) SHA1(5cbea56c7e547c353ae2f9256caaceb20e5e8503))

	ROM_REGION(0x20000, "tiles1", 0)
	ROM_LOAD16_BYTE("10.5a", 0x00001, 0x10000, CRC(3313e74a) SHA1(8622dfb5c013173d5bb037254f4c23b1282404e1))
	ROM_LOAD16_BYTE("12.7a", 0x00000, 0x10000, CRC(6da5fda9) SHA1(9d7b0b26598f31da589fece3535a4d1405b03fc2))

	ROM_REGION(0x20000, "tiles2", 0)
	ROM_LOAD16_BYTE("11.6a", 0x00001, 0x10000, CRC(fa6490b8) SHA1(9a4c1e09b9e8fb256fec0a5ed120fece8a12e1c8))
	ROM_LOAD16_BYTE("13.9a", 0x00000, 0x10000, CRC(a4f689ec) SHA1(e58bfede3fabf4cfca76c20aafb3e9fb604777c9))

	ROM_REGION(0x20000, "sprites1", 0)
	ROM_LOAD16_BYTE("14.13a", 0x00001, 0x10000, CRC(a9cd5225) SHA1(f3d5e29ee08fb563fdc1af3c64128f2cd2feb987))
	ROM_LOAD16_BYTE("16.11a", 0x00000, 0x10000, CRC(92f2cb49) SHA1(498021d94b0fde216207076491702af2324a2dcc))

	ROM_REGION(0x20000, "sprites2", 0)
	ROM_LOAD16_BYTE("15.12a", 0x00001, 0x10000, CRC(ce5c4c8b) SHA1(2351d66ba51e80097ce53bfd448ac24901844cda))
	ROM_LOAD16_BYTE("17.10a", 0x00000, 0x10000, CRC(3fec33f7) SHA1(af086ba30fc4521a0114da2824f5baa04d225a89))

	ROM_REGION(0x20000, "proms", 0)
	ROM_LOAD("n8s129a.7f",  0x000, 0x100, CRC(c849d60b) SHA1(0022fb71b3d777cadac7005e6156725df9bcaf90))
	ROM_LOAD("n82s135n.9c", 0x000, 0x100, CRC(7bbd52db) SHA1(b9bab5fb515579d0270aea8b992a16eeb878f242))

	ROM_REGION(0x20000, "plds", 0)
	ROM_LOAD("pld3.14h.bin", 0x000, 0x149, CRC(8183f7f0) SHA1(3cec53838120064374ecf4ebee048409c6f34081))
	ROM_LOAD("pld8.4d.bin",  0x000, 0x149, CRC(f1e35034) SHA1(527faddbf2ac905fa59ebda8ea327e6e6a7c1fb6))
ROM_END



void metlfrzr_state::init_metlfrzr()
{
	// same as seibu/airraid.cpp
	uint8_t *rom = memregion("maincpu")->base();

	for (int A = 0x0000; A < 0x8000; A++)
	{
		// decode the opcodes
		m_decrypted_opcodes[A] = rom[A];

		if (BIT(A, 5) && !BIT(A, 3))
			m_decrypted_opcodes[A] ^= 0x40;

		if (BIT(A, 10) && !BIT(A, 9) && BIT(A, 3))
			m_decrypted_opcodes[A] ^= 0x20;

		if ((BIT(A, 10) ^ BIT(A, 9)) && BIT(A, 1))
			m_decrypted_opcodes[A] ^= 0x02;

		if (BIT(A, 9) || !BIT(A, 5) || BIT(A, 3))
			m_decrypted_opcodes[A] = bitswap<8>(m_decrypted_opcodes[A], 7, 6, 1, 4, 3, 2, 5, 0);

		// decode the data
		if (BIT(A, 5))
			rom[A] ^= 0x40;

		if (BIT(A, 9) || !BIT(A, 5))
			rom[A] = bitswap<8>(rom[A], 7, 6, 1, 4, 3, 2, 5, 0);
	}
}

} // Anonymous namespace


GAME( 1989, metlfrzr,  0,    metlfrzr, metlfrzr, metlfrzr_state, init_metlfrzr, ROT270, "Seibu Kaihatsu", "Metal Freezer (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
