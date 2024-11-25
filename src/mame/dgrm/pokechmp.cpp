// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Poke Champ */

/* This is a Korean hack of Data East's Pocket Gal

   It uses RAM for Palette instead of PROMs
   Samples are played by OKIM6295
   Different Banking
   More Tiles, 8bpp
   Sprites 4bpp instead of 2bpp
   Many code changes

*/

/* README

-The ROMs are labeled as "Unico".
-The CPUs and some other chips are labeled as "SEA HUNTER".
-The chips with the "SEA HUNTER" label all have their
 surfaces scratched out, so I don't know what they are
 (all 40 -pin chips).

ROMs 1 to 4 = GFX?
ROMs 5 to 8 = Program?
ROM 9 = Sound CPU code?
ROM 10 = Sound samples?
ROM 11 = Main CPU code?

-There's a "copyright 1987 data east corp.all rights reserved"
 string inside ROM 11 (because it's a hack of Pocket Gal)

-Sound = Yamaha YM2203C + Y3014B

-Also, there are some GALs on the board (not dumped) a
 8-dips bank and two oscillators (4 MHz and 24 MHz, both near
 the sound parts).

ClawGrip, Jul 2006

*/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pokechmp_state : public driver_device
{
public:
	pokechmp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_okibank(*this, "okibank")
	{ }

	void pokechmp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;
	required_memory_bank m_okibank;

	tilemap_t *m_bg_tilemap = nullptr;

	void main_bank_w(uint8_t data);
	void oki_bank_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	void sound_irq(int state);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void pokechmp_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void pokechmp_state::flipscreen_w(uint8_t data)
{
	if (flip_screen() != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(pokechmp_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index * 2 + 1] + ((m_videoram[tile_index * 2] & 0x3f) << 8);
	int color = m_videoram[tile_index * 2] >> 6;

	tileinfo.set(0, code, color, 0);
}

void pokechmp_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pokechmp_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void pokechmp_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] != 0xf8)
		{
			int sx = 240 - m_spriteram[offs + 2];
			int sy = 240 - m_spriteram[offs];

			int flipx = m_spriteram[offs + 1] & 0x04;
			int flipy = m_spriteram[offs + 1] & 0x02;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				if (flipx) flipx = 0; else flipx = 1;
				if (flipy) flipy = 0; else flipy = 1;
			}

			int tileno = m_spriteram[offs + 3];

			if (m_spriteram[offs + 1] & 0x01) tileno += 0x100;
			if (m_spriteram[offs + 1] & 0x08) tileno += 0x200;

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					tileno,
					(m_spriteram[offs + 1] & 0xf0) >> 4,
					flipx, flipy,
					sx, sy, 0);
		}
	}
}

uint32_t pokechmp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void pokechmp_state::machine_start()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x4000);
	m_mainbank->configure_entries(2, 2, memregion("maincpu")->base() + 0x10000, 0x4000);

	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x8000);
	m_okibank->set_entry(0x08);
}


void pokechmp_state::main_bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x03);
}


void pokechmp_state::oki_bank_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}


void pokechmp_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).ram().w(FUNC(pokechmp_state::videoram_w)).share(m_videoram);
	map(0x1000, 0x11ff).ram().share(m_spriteram);

	map(0x1800, 0x1800).portr("P1");
	map(0x1801, 0x1801).w(FUNC(pokechmp_state::flipscreen_w));
	// 1800 - 0x181f are unused BAC-06 registers
	map(0x1802, 0x181f).nopw();

	map(0x1a00, 0x1a00).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x1c00, 0x1c00).portr("DSW").w(FUNC(pokechmp_state::main_bank_w));

	// Extra on Poke Champ (not on Pocket Gal)
	map(0x2000, 0x23ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x2400, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);
}


/***************************************************************************/

void pokechmp_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3812_device::write));
	map(0x1800, 0x1800).nopw();    // MSM5205 chip on Pocket Gal, not connected here?
	map(0x2000, 0x2000).w(FUNC(pokechmp_state::oki_bank_w)); // sound ROM bank seems to be replaced with OKI bank
	map(0x2800, 0x2800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // extra
	map(0x3000, 0x3000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom().region("audiocpu", 0x8000);
}


void pokechmp_state::oki_map(address_map &map)
{
	map(0x00000, 0x37fff).rom();
	map(0x38000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( pokechmp )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow 2 Players Game" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3") // Affects Time: Normal=120 & Hardest=100
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2") // Listed as "Number of Balls" in the manual
	PORT_DIPSETTING(    0x00, "3" ) // Manual shows 2
	PORT_DIPSETTING(    0x40, "4" ) // Manual shows 3
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") // Not shown or listed in the manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout pokechmp_charlayout =
{
	8,8, // 8*8 characters
	RGN_FRAC(1,8),
	8,
	// bizarre order, but it seems to be correct?
	{ RGN_FRAC(1,8), RGN_FRAC(3,8),RGN_FRAC(0,8),RGN_FRAC(5,8),RGN_FRAC(2,8),RGN_FRAC(7,8),RGN_FRAC(4,8),RGN_FRAC(6,8) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8  // every char takes 8 consecutive bytes
};



static const gfx_layout pokechmp_spritelayout =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,4),   // 1024 sprites
	4,
	{RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4)},
	{ 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0, 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every char takes 8 consecutive bytes
};


static GFXDECODE_START( gfx_pokechmp )
	GFXDECODE_ENTRY( "bgs", 0x00000, pokechmp_charlayout,   0x100, 4 )
	GFXDECODE_ENTRY( "sprites", 0x00000, pokechmp_spritelayout,   0,  32 )
GFXDECODE_END

/*

OSCs: 24MHz & 4MHz

Clocks - from Billiard List PCB
Main 6502: 1mhz
Sound 6502: 1mhz
YM2203: 1mhz
YM3014: 1.5mhz
OKI M6295 (an AD65 on this board, note pin 7 is low): 1.5mhz

*/

void pokechmp_state::sound_irq(int state)
{
	// VBLANK is probably not the source of this interrupt
	if (state)
		m_audiocpu->set_input_line(0, HOLD_LINE);
}

void pokechmp_state::pokechmp(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 4_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pokechmp_state::main_map);

	M6502(config, m_audiocpu, 4_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &pokechmp_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pokechmp_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);
	screen.screen_vblank().append(FUNC(pokechmp_state::sound_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pokechmp);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2203(config, "ym1", 4_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.60);

	YM3812(config, "ym2", 24_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", 24_MHz_XTAL / 16, okim6295_device::PIN7_LOW));
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);
	oki.set_addrmap(0, &pokechmp_state::oki_map);
}


ROM_START( pokechmp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pokechamp_11_27010.bin", 0x00000, 0x20000, CRC(9afb6912) SHA1(e45da9524e3bb6f64a68200b70d0f83afe6e4379) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pokechamp_09_27c512.bin", 0x00000, 0x10000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21) ) // 0xxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "bgs", 0 )
	ROM_LOAD( "pokechamp_05_27c020.bin",       0x00000, 0x40000, CRC(554cfa42) SHA1(862d0dd83697da7bd52dc640c34926c62691afea) )
	ROM_LOAD( "pokechamp_06_27c020.bin",       0x40000, 0x40000, CRC(00bb9536) SHA1(1a5584297ebb425d6ce331955e0c6a4f467cd1e6) )
	ROM_LOAD( "pokechamp_07_27c020.bin",       0x80000, 0x40000, CRC(4b15ab5e) SHA1(5523134853b9ea1c81fd5aeb58061376d94e9298) )
	ROM_LOAD( "pokechamp_08_27c020.bin",       0xc0000, 0x40000, CRC(e9db54d6) SHA1(ac3b7c06d0f61847bf9bc6147f2f88d712f2b4b3) )

	ROM_REGION( 0x20000, "sprites", 0 )
	// the first half of all these ROMs is identical.  For ROM 3 both halves match.  Correct decode is to ignore the first half
	ROM_LOAD( "pokechamp_02_27c512.bin",       0x00000, 0x08000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87) ) ROM_CONTINUE( 0x00000, 0x8000 )
	ROM_LOAD( "pokechamp_01_27c512.bin",       0x08000, 0x08000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c) ) ROM_CONTINUE( 0x08000, 0x8000 )
	ROM_LOAD( "pokechamp_04_27c512.bin",       0x10000, 0x08000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038) ) ROM_CONTINUE( 0x10000, 0x8000 )
	ROM_LOAD( "pokechamp_03_27c512.bin",       0x18000, 0x08000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5) ) ROM_CONTINUE( 0x18000, 0x8000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pokechamp_10_27c040.bin",       0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

// only the 'maincpu' and 'bgs' regions were dumped for this set, others assumed to be the same
ROM_START( pokechmpa ) // PCB: DGRM NO 1342
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1", 0x00000, 0x20000, CRC(7d051c36) SHA1(8c2329f863ad677f4398a7dab7476c9492ad4f24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("pokechamp_09_27c512.bin", 0x00000, 0x10000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21)) // 0xxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "bgs", 0 )
	ROM_LOAD( "6",       0x00000, 0x40000, CRC(1aec1de2) SHA1(f42db2445dcf1fb0957bf8a4414c3266ae47fae1) )
	ROM_LOAD( "5",       0x40000, 0x40000, CRC(79823f7a) SHA1(1059b4baf4d4d3c49d4de4194f29f8601e75972b) )
	ROM_LOAD( "4",       0x80000, 0x40000, CRC(e76f7596) SHA1(bb4c55bad2693da3f76d33fdf0f7f32c44dfd3e0) )
	ROM_LOAD( "3",       0xc0000, 0x40000, CRC(a22946b8) SHA1(d77fb5bfe00349753a9e6ea9de82c1eefca090f7) )

	ROM_REGION( 0x20000, "sprites", 0 )
	// the first half of all these ROMs is identical.  For ROM 3 both halves match.  Correct decode is to ignore the first half
	ROM_LOAD( "pokechamp_02_27c512.bin",       0x00000, 0x08000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87) ) ROM_CONTINUE( 0x00000, 0x8000 )
	ROM_LOAD( "pokechamp_01_27c512.bin",       0x08000, 0x08000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c) ) ROM_CONTINUE( 0x08000, 0x8000 )
	ROM_LOAD( "pokechamp_04_27c512.bin",       0x10000, 0x08000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038) ) ROM_CONTINUE( 0x10000, 0x8000 )
	ROM_LOAD( "pokechamp_03_27c512.bin",       0x18000, 0x08000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5) ) ROM_CONTINUE( 0x18000, 0x8000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pokechamp_10_27c040.bin",       0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

// only the 'maincpu' and 'bgs' and 'oki' regions were dumped for this set, others assumed to be the same
ROM_START(billlist)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("billiard_list.1", 0x00000, 0x20000, CRC(4ef416f7) SHA1(e995410e2c79a3fbd2ac76a80dc6c412eb454e52) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("pokechamp_09_27c512.bin", 0x00000, 0x10000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21)) // 0xxxxxxxxxxxxxxx = 0x00

	ROM_REGION(0x100000, "bgs", 0)
	ROM_LOAD("billiard_list.6", 0x00000, 0x40000, CRC(e674f7c0) SHA1(8a610a92ae141f3004497dc3ce102d07a178683f) )
	ROM_LOAD("billiard_list.5", 0x40000, 0x40000, CRC(f50d1510) SHA1(e09c6b1d0a5bdb44b3b5ae6bd54b169abd978af5) )
	ROM_LOAD("billiard_list.4", 0x80000, 0x40000, CRC(7466c0be) SHA1(6e82d9d8ec5bdeca2f6c7b9ca0f31aabf3faacea) )
	ROM_LOAD("billiard_list.3", 0xc0000, 0x40000, CRC(1ac4fa42) SHA1(2da5c6aa7e6b34ad1a2f052a41a2e607e2f904c2) )

	ROM_REGION(0x20000, "sprites", 0)
	// the first half of all these ROMs is identical.  For ROM 3 both halves match.  Correct decode is to ignore the first half
	ROM_LOAD("pokechamp_02_27c512.bin", 0x00000, 0x08000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87)) ROM_CONTINUE(0x00000, 0x8000)
	ROM_LOAD("pokechamp_01_27c512.bin", 0x08000, 0x08000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c)) ROM_CONTINUE(0x08000, 0x8000)
	ROM_LOAD("pokechamp_04_27c512.bin", 0x10000, 0x08000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038)) ROM_CONTINUE(0x10000, 0x8000)
	ROM_LOAD("pokechamp_03_27c512.bin", 0x18000, 0x08000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5)) ROM_CONTINUE(0x18000, 0x8000)

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("billiard_list.x", 0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

} // anonymous namespace


GAME( 1995, pokechmp,  0,        pokechmp, pokechmp, pokechmp_state, empty_init, ROT0, "D.G.R.M.", "Poke Champ (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, pokechmpa, pokechmp, pokechmp, pokechmp, pokechmp_state, empty_init, ROT0, "D.G.R.M.", "Poke Champ (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, billlist,  pokechmp, pokechmp, pokechmp, pokechmp_state, empty_init, ROT0, "D.G.R.M.", "Billard List",       MACHINE_SUPPORTS_SAVE )
