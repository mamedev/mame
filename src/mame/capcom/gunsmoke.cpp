// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    Gun.Smoke
    Capcom
    85113-A-1 main PCB + 85113-B-3 video PCB

    driver by Paul Leaman

    Games supported:
        * Gun.Smoke (World)
        * Gun.Smoke (bootleg)
        * Gun.Smoke (Japan)
        * Gun.Smoke (US set 1)
        * Gun.Smoke (US set 2)

****************************************************************************

  GUNSMOKE
  ========

  Driver provided by Paul Leaman


Stephh's notes (based on the games Z80 code and some tests) :

0) all games

  - There is some code that allows you to select your starting level
    (at 0x08dc in 'gunsmoka' and at 0x08d2 in the other sets).
    To do so, once the game has booted (after the "notice" screen),
    turn the "service" mode Dip Switch ON, and change Dip Switches
    DSW 1-0 to 1-3 (which are used by coinage).
  - About the ingame bug at the end of level 2 : enemy's energy
    (stored at 0xf790) is in fact not infinite, but it turns back to
    0xff, so when it reaches 0 again, the boss is dead.


1) 'gunsmoke'

  - World version.
    You can enter 3 chars for your initials.


2) 'gunsmokej'

  - Japan version (but English text though).
    You can enter 8 chars for your initials.


3) 'gunsmokeub'

  - US version licensed to Romstar.
    You can enter 3 chars for your initials.


4) 'gunsmokeu'

  - US version licensed to Romstar.
    You can enter 3 chars for your initials.
  - This is probably a later version of the game because some code
    has been added for the "Lives" Dip Switch that replaces the
    "Demonstration" one (so demonstration is always OFF).
  - Other changes :
      * Year is 1986 instead of 1985.
      * High score is 110000 instead of 100000.
      * Levels 3 and 6 are swapped.


***************************************************************************/

// Notes by Jose Tejada (jotego)
// CPU speed is 3MHz as per PCB measurement
// Vertical speed is 59.63 Hz
// There is no watchdog
// There is a DMA circuit, same as GnG, Commando and other CAPCOM games of the era
// The DMA copies sprites to a video and halts the CPU for ~131us

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class gunsmoke_state : public driver_device
{
public:
	gunsmoke_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_scrollx(*this, "scrollx"),
		m_scrolly(*this, "scrolly"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void gunsmoke(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_chon = 0U;
	uint8_t m_objon = 0U;
	uint8_t m_bgon = 0U;
	uint8_t m_sprite3bank = 0U;

	uint8_t protection_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void c804_w(uint8_t data);
	void d806_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gunsmoke has three 256x4 palette PROMs (one per gun) and a lot of
  256x4 lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void gunsmoke_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0x40-0x4f
	for (int i = 0; i < 0x80; i++)
	{
		uint8_t const ctabentry = color_prom[i] | 0x40;
		palette.set_pen_indirect(i, ctabentry);
	}

	// background tiles use colors 0-0x3f
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] | ((color_prom[i + 0x100] & 0x03) << 4);
		palette.set_pen_indirect(i - 0x80, ctabentry);
	}

	// sprites use colors 0x80-0xff
	for (int i = 0x300; i < 0x400; i++)
	{
		uint8_t const ctabentry = color_prom[i] | ((color_prom[i + 0x100] & 0x07) << 4) | 0x80;
		palette.set_pen_indirect(i - 0x180, ctabentry);
	}
}

void gunsmoke_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gunsmoke_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gunsmoke_state::c804_w(uint8_t data)
{
	// bits 0 and 1 are for coin counters
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
	machine().bookkeeping().coin_counter_w(0, data & 0x02);

	// bits 2 and 3 select the ROM bank
	m_mainbank->set_entry((data & 0x0c) >> 2);

	// bit 5 resets the sound CPU? - we ignore it

	// bit 6 flips screen
	flip_screen_set(data & 0x40);

	// bit 7 enables characters?
	m_chon = data & 0x80;
}

void gunsmoke_state::d806_w(uint8_t data)
{
	// bits 0-2 select the sprite 3 bank
	m_sprite3bank = data & 0x07;

	// bit 4 enables bg 1?
	m_bgon = data & 0x10;

	// bit 5 enables sprites?
	m_objon = data & 0x20;
}

TILE_GET_INFO_MEMBER(gunsmoke_state::get_bg_tile_info)
{
	uint8_t *tilerom = memregion("bgtiles")->base();

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs] + ((attr & 0x01) << 8);
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(gunsmoke_state::get_fg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xe0) << 2);
	int color = attr & 0x1f;

	tileinfo.group = color;

	tileinfo.set(0, code, color, 0);
}

void gunsmoke_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gunsmoke_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 32, 32, 2048, 8);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gunsmoke_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(  6,   6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(  6,   6);

	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x4f);
}

void gunsmoke_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 32; offs >= 0; offs -= 32)
	{
		int attr = m_spriteram[offs + 1];
		int bank = (attr & 0xc0) >> 6;
		int code = m_spriteram[offs];
		int color = attr & 0x0f;
		int flipx = 0;
		int flipy = attr & 0x10;
		int sx = m_spriteram[offs + 3] - ((attr & 0x20) << 3);
		int sy = m_spriteram[offs + 2];

		if (bank == 3)
			bank += m_sprite3bank;

		code += 256 * bank;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx + 128, sy + 6, 0);
	}
}

uint32_t gunsmoke_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx[0] + 256 * m_scrollx[1]);
	m_bg_tilemap->set_scrolly(0, m_scrolly[0]);

	if (m_bgon)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_objon)
		draw_sprites(bitmap, cliprect);

	if (m_chon)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


// Read/Write Handlers

uint8_t gunsmoke_state::protection_r(offs_t offset)
{
	/*
	    The routine at 0x0e69 tries to read data starting at 0xc4c9.
	    If this value is zero, it interprets the next two bytes as a
	    jump address.

	    This was resulting in a reboot which happens at the end of level 3
	    if you go too far to the right of the screen when fighting the level boss.

	    A non-zero for the first byte seems to be harmless
	    (although it may not be the correct behaviour).

	    This could be some devious protection or it could be a bug in the
	    arcade game.  It's hard to tell without pulling the code apart.
	*/

	static const uint8_t fixed_data[] = { 0xff, 0x00, 0x00 };
	return fixed_data[offset];
}

// Memory Maps

void gunsmoke_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc4c9, 0xc4cb).r(FUNC(gunsmoke_state::protection_r));
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc804, 0xc804).w(FUNC(gunsmoke_state::c804_w));  // ROM bank switch, screen flip
	// 0xc806 DMA trigger (not emulated)
	map(0xd000, 0xd3ff).ram().w(FUNC(gunsmoke_state::videoram_w)).share(m_videoram);
	map(0xd400, 0xd7ff).ram().w(FUNC(gunsmoke_state::colorram_w)).share(m_colorram);
	map(0xd800, 0xd801).ram().share(m_scrollx);
	map(0xd802, 0xd802).ram().share(m_scrolly);
	map(0xd806, 0xd806).w(FUNC(gunsmoke_state::d806_w));  // sprites and bg enable
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).ram().share(m_spriteram);
}

void gunsmoke_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc800).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe001).w("ym1", FUNC(ym2203_device::write));
	map(0xe002, 0xe003).w("ym2", FUNC(ym2203_device::write));
}

// Input Ports

static INPUT_PORTS_START( gunsmoke )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )    // VBLANK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "30K 80K 80K+" )
	PORT_DIPSETTING(    0x03, "30K 100K 100K+" )
	PORT_DIPSETTING(    0x00, "30K 100K 150K+" )
	PORT_DIPSETTING(    0x02, "30K 100K")
	PORT_DIPNAME( 0x04, 0x04, "Demo" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW ) // Also "debug mode"

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gunsmokeu )
	PORT_INCLUDE(gunsmoke)

	// Same as 'gunsmoke', but "Lives" Dip Switch instead of "Demonstration" Dip Switch
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END

// Graphics Layouts

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	1024,   // 1024 characters
	2,  // 2 bits per pixel
	{ 4, 0 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout tilelayout =
{
	32,32,  // 32*32 tiles
	512,    // 512 tiles
	4,      // 4 bits per pixel
	{ 512*256*8+4, 512*256*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			64*8+0, 64*8+1, 64*8+2, 64*8+3, 65*8+0, 65*8+1, 65*8+2, 65*8+3,
			128*8+0, 128*8+1, 128*8+2, 128*8+3, 129*8+0, 129*8+1, 129*8+2, 129*8+3,
			192*8+0, 192*8+1, 192*8+2, 192*8+3, 193*8+0, 193*8+1, 193*8+2, 193*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
			24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	256*8   // every tile takes 256 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	2048,   // 2048 sprites
	4,      // 4 bits per pixel
	{ 2048*64*8+4, 2048*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    // every sprite takes 64 consecutive bytes
};

// Graphics Decode Info

static GFXDECODE_START( gfx_gunsmoke )
	GFXDECODE_ENTRY( "chars",   0, charlayout,            0, 32 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,         32*4, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 32*4+16*16, 16 )
GFXDECODE_END

// Machine Driver

void gunsmoke_state::machine_start()
{
	uint8_t *rombase = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 4, &rombase[0x8000], 0x4000);

	save_item(NAME(m_chon));
	save_item(NAME(m_objon));
	save_item(NAME(m_bgon));
	save_item(NAME(m_sprite3bank));
}

void gunsmoke_state::machine_reset()
{
	m_chon = 0;
	m_objon = 0;
	m_bgon = 0;
	m_sprite3bank = 0;
}

void gunsmoke_state::gunsmoke(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 4);   // 3 MHz Verified on PCB by jotego
	m_maincpu->set_addrmap(AS_PROGRAM, &gunsmoke_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(gunsmoke_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 12_MHz_XTAL / 4));  // 3 MHz, actually inside a 85H001 CAPCOM custom
	audiocpu.set_addrmap(AS_PROGRAM, &gunsmoke_state::sound_map);
	audiocpu.set_periodic_int(FUNC(gunsmoke_state::irq0_line_hold), attotime::from_ticks(384*262/4, 12_MHz_XTAL / 2));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL / 2, 384, 128, 0, 262, 22, 246); // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(gunsmoke_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gunsmoke);

	PALETTE(config, m_palette, FUNC(gunsmoke_state::palette), 32*4 + 16*16 + 16*16, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8));
	ym1.add_route(0, "mono", 0.22);
	ym1.add_route(1, "mono", 0.22);
	ym1.add_route(2, "mono", 0.22);
	ym1.add_route(3, "mono", 0.14);

	ym2203_device &ym2(YM2203(config, "ym2", 12_MHz_XTAL / 8));
	ym2.add_route(0, "mono", 0.22);
	ym2.add_route(1, "mono", 0.22);
	ym2.add_route(2, "mono", 0.22);
	ym2.add_route(3, "mono", 0.14);
}

// ROMs

ROM_START( gunsmoke )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gs03.09n", 0x00000, 0x8000, CRC(40a06cef) SHA1(3e2a52d476298b7252f0adaefdb42090351e921c) ) // Code 0000-7fff // gse_03
	ROM_LOAD( "gs04.10n", 0x08000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) // Paged code
	ROM_LOAD( "gs05.12n", 0x10000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )

	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokeb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "3.ic85", 0x00000, 0x8000, CRC(ae6f4b75) SHA1(f4ee4f7a7d507ceaef9ce8165704fd80c8c1e8ba) ) // Code 0000-7fff
	ROM_LOAD( "4.ic86", 0x08000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) // Paged code
	ROM_LOAD( "5.ic87", 0x10000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.ic41", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "1.ic39", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "13.ic21", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "12.ic20", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "11.ic19", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "10.ic18", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "9.ic04",  0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "8.ic03",  0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "7.ic02",  0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "6.ic01",  0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "22.ic134", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "21.ic133", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "20.ic132", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "19.ic131", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "18.ic115", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "17.ic114", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "16.ic113", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "15.ic112", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "14.ic25", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	/* The names of the PROMs starting with "g-" do not yet reflect their position in the PCB layout of this bootleg.
	   As the ICs are not socketed, but directly soldered to the PCB, it is harder to identify which is which.
	   But it would be good to figure this out at some point, for the sake of documenting this specific board layout. */
	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "prom.ic3", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "prom.ic4", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "prom.ic5", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokej )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gsj_03.09n", 0x00000, 0x8000, CRC(b56b5df6) SHA1(0295a3ef491b6b8ee9c198fd08dddc29d88bbef6) ) // Code 0000-7fff
	ROM_LOAD( "gs04.10n",   0x08000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) // Paged code
	ROM_LOAD( "gs05.12n",   0x10000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokeu )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gsa_03.9n", 0x00000, 0x8000, CRC(51dc3f76) SHA1(2a188fee73c3662b665b56a825eb908b7b42dcd0) ) // Code 0000-7fff
	ROM_LOAD( "gs04.10n",  0x08000, 0x8000, CRC(5ecf31b8) SHA1(34ec9727330821a45b497c78c970a1a4f14ff4ee) ) // Paged code
	ROM_LOAD( "gs05.12n",  0x10000, 0x8000, CRC(1c9aca13) SHA1(eb92c373d2241aea4c59248e1b82717733105ac0) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokeua )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gsr_03y.09n", 0x00000, 0x8000, CRC(1b42423f) SHA1(495f6d477ce791e296dd8a170ba28f0b8d2e66d2) ) // Code 0000-7fff
	ROM_LOAD( "gsr_04y.10n", 0x08000, 0x8000, CRC(a5ee595b) SHA1(10cd25134f4b19ca32bef63d54c11461b31d3438) ) // Paged code
	ROM_LOAD( "gsr_05y.12n", 0x10000, 0x8000, CRC(1c9aca13) SHA1(eb92c373d2241aea4c59248e1b82717733105ac0) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokeub )
	ROM_REGION( 0x18000, "maincpu", 0 ) // has a small extra piece of code at 0x2f00 and a jump to it at 0x297b, otherwise the same as gunsmokeub including the datecode, chip had an 'A' stamped on it, bugfix?
	ROM_LOAD( "gsr_03a.9n", 0x00000, 0x8000, CRC(2f6e6ad7) SHA1(e9e4a367c240a35a1ba2eeaec9458996f7926f16) ) // Code 0000-7fff
	ROM_LOAD( "gs04.10n",   0x08000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) // Paged code
	ROM_LOAD( "gs05.12n",   0x10000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokeuc )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gsr_03.9n", 0x00000, 0x8000, CRC(592f211b) SHA1(8de44b3cafa3d2ce9aba515cf3ec4bac0bcdeb5b) ) // Code 0000-7fff
	ROM_LOAD( "gs04.10n",  0x08000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) // Paged code
	ROM_LOAD( "gs05.12n",  0x10000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h", 0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f", 0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c", 0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c", 0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c", 0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gs10.02c", 0x18000, 0x8000, CRC(f469c13c) SHA1(54eda52d6fce58771c0adfe2c88292a41d5a9b99) )
	ROM_LOAD( "gs09.06a", 0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a", 0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a", 0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gs06.02a", 0x38000, 0x8000, CRC(4cafe7a6) SHA1(fe501f3a5e9ce9e82e9708f1cd297f4c94ef0f81) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gs22.06n", 0x00000, 0x8000, CRC(dc9c508c) SHA1(920505dd4c63b177918feb4e54cca8a7948ec9d9) ) // Sprites planes 2-3
	ROM_LOAD( "gs21.04n", 0x08000, 0x8000, CRC(68883749) SHA1(c7bf2bf49c53feddf8f30b4001dc2d59b52b1c28) ) // Sprites planes 2-3
	ROM_LOAD( "gs20.03n", 0x10000, 0x8000, CRC(0be932ed) SHA1(1c5af5884a23112dbc36579515d1cb497992da2f) ) // Sprites planes 2-3
	ROM_LOAD( "gs19.01n", 0x18000, 0x8000, CRC(63072f93) SHA1(cb3a2729782cf2855558d081fe92d28366228b8e) ) // Sprites planes 2-3
	ROM_LOAD( "gs18.06l", 0x20000, 0x8000, CRC(f69a3c7c) SHA1(e9eb9dfa7d53aa7b728150f91d05bfc3bf6f1e75) ) // Sprites planes 0-1
	ROM_LOAD( "gs17.04l", 0x28000, 0x8000, CRC(4e98562a) SHA1(0341b8a79be1d71a57d0d76ed890e15f9f92259e) ) // Sprites planes 0-1
	ROM_LOAD( "gs16.03l", 0x30000, 0x8000, CRC(0d99c3b3) SHA1(436c566b76f632242448671e3b6319f7d9f65322) ) // Sprites planes 0-1
	ROM_LOAD( "gs15.01l", 0x38000, 0x8000, CRC(7f14270e) SHA1(dd06c333c2ea097e25185a1423cd61e1b7afc42b) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b", 0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) )    // red component
	ROM_LOAD( "g-02.04b", 0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) )    // green component
	ROM_LOAD( "g-03.05b", 0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) )    // blue component
	ROM_LOAD( "g-04.09d", 0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) )    // char lookup table
	ROM_LOAD( "g-06.14a", 0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) )    // tile lookup table
	ROM_LOAD( "g-07.15a", 0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) )    // tile palette bank
	ROM_LOAD( "g-09.09f", 0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) )    // sprite lookup table
	ROM_LOAD( "g-08.08f", 0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) )    // sprite palette bank
	ROM_LOAD( "g-10.02j", 0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
	ROM_LOAD( "g-05.01f", 0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) )    // priority? (not used)
ROM_END

ROM_START( gunsmokeg )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gsg03.09n", 0x00000, 0x8000, CRC(8ad2754e) SHA1(221309d4d76e49f9b80849630b2846fc2e3d72a1) ) // Code 0000-7fff
	ROM_LOAD( "gs04.10n",  0x08000, 0x8000, CRC(8d4b423f) SHA1(149274c2ed1526ca1f419fdf8a24059ff138f7f2) ) // Paged code
	ROM_LOAD( "gs05.12n",  0x10000, 0x8000, CRC(2b5667fb) SHA1(5b689bca1e76d803b4cae22feaa7744fa528e93f) ) // Paged code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs02.14h",  0x00000, 0x8000, CRC(cd7a2c38) SHA1(c76c471f694b76015370f0eacf5350e652f526ff) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "gs01.11f",  0x00000, 0x4000, CRC(b61ece9b) SHA1(eb3fc62644cc5b5a2b9cbe67c393d4a0e2a59ca9) ) // Characters

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "gs13.06c",  0x00000, 0x8000, CRC(f6769fc5) SHA1(d192ec176425327ca4b7e25fc8432fc47837ba29) ) // 32x32 tiles planes 2-3
	ROM_LOAD( "gs12.05c",  0x08000, 0x8000, CRC(d997b78c) SHA1(3b4a9b6f9e57ecfb4ab9734379bd0ee765fd6daa) )
	ROM_LOAD( "gs11.04c",  0x10000, 0x8000, CRC(125ba58e) SHA1(cf6931653cebd051564bed8121ab8713a55095c5) )
	ROM_LOAD( "gsg10.02c", 0x18000, 0x8000, CRC(0674ff4d) SHA1(cceeb3dfe8cad01115d337bce703d446cfd499ad) )

	ROM_LOAD( "gs09.06a",  0x20000, 0x8000, CRC(539f182d) SHA1(4190c0adbecc57b92f4d002e121acb77e8c5d8d8) ) // 32x32 tiles planes 0-1
	ROM_LOAD( "gs08.05a",  0x28000, 0x8000, CRC(e87e526d) SHA1(d10068addf30322424a85bbc6382cb762ae3fbe2) )
	ROM_LOAD( "gs07.04a",  0x30000, 0x8000, CRC(4382c0d2) SHA1(8615e62bc57b40d082f6ca211d64f22185bed1fd) )
	ROM_LOAD( "gsg06.02a", 0x38000, 0x8000, CRC(5cb850a7) SHA1(5b4fec3fae4d1947778c832c41f757414652f34a) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gsg22.06n", 0x00000, 0x8000, CRC(96779c38) SHA1(83f5811b7674e39fac6d127bcf2741a5ba111ec1) ) // Sprites planes 2-3
	ROM_LOAD( "gsg21.04n", 0x08000, 0x8000, CRC(6e8a02c7) SHA1(8db5792ded8c6360e5a07f96b7799eae7591d719) ) // Sprites planes 2-3
	ROM_LOAD( "gsg20.03n", 0x10000, 0x8000, CRC(139bf927) SHA1(f58bf8cffbc4e9e1a48c905d341f92a22df6cf37) ) // Sprites planes 2-3
	ROM_LOAD( "gsg19.01n", 0x18000, 0x8000, CRC(8f249573) SHA1(390103b100ec944c249759383d505df6292f5405) ) // Sprites planes 2-3
	ROM_LOAD( "gsg18.06l", 0x20000, 0x8000, CRC(b290451c) SHA1(c31d93efd9c1b8d71d4875d7a095a5de2011d2b8) ) // Sprites planes 0-1
	ROM_LOAD( "gsg17.04l", 0x28000, 0x8000, CRC(61c9bd10) SHA1(ca3116c657b2cc8d7d5f0ac602a294deb92a4e8c) ) // Sprites planes 0-1
	ROM_LOAD( "gsg16.03l", 0x30000, 0x8000, CRC(6620103b) SHA1(ef1260c05c958c115f54ddaffa213d320508ff11) ) // Sprites planes 0-1
	ROM_LOAD( "gsg15.01l", 0x38000, 0x8000, CRC(ccc1c1b6) SHA1(feb480195bc4157d7be385b055bdd47505de6bc6) ) // Sprites planes 0-1

	ROM_REGION( 0x8000, "bgtiles", 0 )
	ROM_LOAD( "gs14.11c", 0x00000, 0x8000, CRC(0af4f7eb) SHA1(24a98fdeedeeaf1035b4af52d5a8dd5e47a5e62d) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "g-01.03b",  0x0000, 0x0100, CRC(02f55589) SHA1(8a3f98304aedf3aba1c08b615bf457752a480edc) ) // Red component
	ROM_LOAD( "g-02.04b",  0x0100, 0x0100, CRC(e1e36dd9) SHA1(5bd88a35898a2d973045bdde8311aac3a12826de) ) // Green component
	ROM_LOAD( "g-03.05b",  0x0200, 0x0100, CRC(989399c0) SHA1(e408e391f49ed0c7b9e16479fea44b809440fefc) ) // Blue component
	ROM_LOAD( "g-04.09d",  0x0300, 0x0100, CRC(906612b5) SHA1(7b727a6200c088538180758320ede84aa7e5b96d) ) // Char lookup table
	ROM_LOAD( "g-06.14a",  0x0400, 0x0100, CRC(4a9da18b) SHA1(fed3b81b56aab2ed0a21ed1fcebe3f1ae095a13b) ) // Tile lookup table
	ROM_LOAD( "g-07.15a",  0x0500, 0x0100, CRC(cb9394fc) SHA1(8ad0fde6a8ef8326d2da4b6dbf3b51f5f6c668c8) ) // Tile palette bank
	ROM_LOAD( "g-09.09f",  0x0600, 0x0100, CRC(3cee181e) SHA1(3f95bdb12391cb9b3673191bda8d09c84b36b4d3) ) // Sprite lookup table
	ROM_LOAD( "g-08.08f",  0x0700, 0x0100, CRC(ef91cdd2) SHA1(90b9191c9f10a153d64055a4238eb6e15b8c12bc) ) // Sprite palette bank
	ROM_LOAD( "g-10.02j",  0x0800, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) ) // Video timing (not used)
	ROM_LOAD( "g-05.01f",  0x0900, 0x0100, CRC(25c90c2a) SHA1(42893572bab757ec01e181fc418cb911638d37e0) ) // Priority? (not used)
ROM_END

}  // anonymous namespace


// Game Drivers

// At 0x7E50 in the first ROM is 85113 (project ident code) and the project codename 'Gunman' both stored as ASCII.
// Following that at (stored as raw data) is the build date in yyyymmdd format.  After that a ROM identification string(?) which I've
// left in the comment after each set.

// this information is not displayed onscreen

GAME( 1985, gunsmoke,   0,        gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "Capcom",                   "Gun.Smoke (World, 1985-11-15)",                 MACHINE_SUPPORTS_SAVE ) // GSE_03
GAME( 1985, gunsmokeb,  gunsmoke, gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "bootleg",                  "Gun.Smoke (World, 1985-11-15) (bootleg)",       MACHINE_SUPPORTS_SAVE ) // based on above version, warning message patched out
GAME( 1985, gunsmokej,  gunsmoke, gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "Capcom",                   "Gun.Smoke (Japan, 1985-11-15)",                 MACHINE_SUPPORTS_SAVE ) // GSJ_03
GAME( 1986, gunsmokeu,  gunsmoke, gunsmoke, gunsmokeu, gunsmoke_state, empty_init, ROT270, "Capcom (Romstar license)", "Gun.Smoke (USA and Canada, 1986-04-08)",        MACHINE_SUPPORTS_SAVE ) // GSA_03
GAME( 1985, gunsmokeua, gunsmoke, gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "Capcom (Romstar license)", "Gun.Smoke (USA and Canada, 1986-01-20)",        MACHINE_SUPPORTS_SAVE ) // GSA_03Y (GSR 03Y on the chip)
GAME( 1985, gunsmokeub, gunsmoke, gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "Capcom (Romstar license)", "Gun.Smoke (USA and Canada, 1985-11-15, set 1)", MACHINE_SUPPORTS_SAVE ) // GSR_03 (03A on the chip)
GAME( 1985, gunsmokeuc, gunsmoke, gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "Capcom (Romstar license)", "Gun.Smoke (USA and Canada, 1985-11-15, set 2)", MACHINE_SUPPORTS_SAVE ) // GSR_03
GAME( 1985, gunsmokeg,  gunsmoke, gunsmoke, gunsmoke,  gunsmoke_state, empty_init, ROT270, "Capcom",                   "Gun.Smoke (Germany, 1985-11-15, censored)",     MACHINE_SUPPORTS_SAVE ) // GSG_03, has 'World' regional warning, but game censored for German market
