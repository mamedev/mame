// license:BSD-3-Clause
// copyright-holders: Mirko Buffoni

/***************************************************************************

Vulgus memory map (preliminary)

driver by Mirko Buffoni

MAIN CPU
0000-9fff ROM
cc00-cc7f Sprites
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff background video RAM
dc00-dfff background color RAM
e000-efff RAM

read:
c000      IN0
c001      IN1
c002      IN2
c003      DSW1
c004      DSW2

write:
c802      background y scroll low 8 bits
c803      background x scroll low 8 bits
c805      background palette bank selector
c902      background y scroll high bit
c903      background x scroll high bit

SOUND CPU
0000-3fff ROM
4000-47ff RAM

write:
8000      AY-3-8910 #1 control
8001      AY-3-8910 #1 write
c000      AY-3-8910 #2 control
c001      AY-3-8910 #2 write

All Clocks and Vsync verified by Corrado Tomaselli (August 2012)

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class vulgus_state : public driver_device
{
public:
	vulgus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_scroll_low(*this, "scroll_low"),
		m_scroll_high(*this, "scroll_high"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram")
	{ }

	void vulgus(machine_config &config) ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;

	virtual void video_start() override ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_scroll_low;
	required_shared_ptr<uint8_t> m_scroll_high;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;

	uint8_t m_palette_bank = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void c804_w(uint8_t data);
	void palette_bank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void palette(palette_device &palette) const ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	void sound_map(address_map &map) ATTR_COLD;
};

class _1942iti_state : public vulgus_state
{
public:
	_1942iti_state(const machine_config &mconfig, device_type type, const char *tag) :
		vulgus_state(mconfig, type, tag),
		m_rombank(*this, "rombank")
	{ }

	void _1942iti(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank m_rombank;

	void _1942iti_main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void vulgus_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[256] >> 0) & 0x01;
		bit1 = (color_prom[256] >> 1) & 0x01;
		bit2 = (color_prom[256] >> 2) & 0x01;
		bit3 = (color_prom[256] >> 3) & 0x01;
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*256] >> 0) & 0x01;
		bit1 = (color_prom[2*256] >> 1) & 0x01;
		bit2 = (color_prom[2*256] >> 2) & 0x01;
		bit3 = (color_prom[2*256] >> 3) & 0x01;
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
		color_prom++;
	}

	color_prom += 2 * 256;
	// color_prom now points to the beginning of the lookup table

	// characters use colors 32-47 (?)
	for (int i = 0; i < m_gfxdecode->gfx(0)->colors() * m_gfxdecode->gfx(0)->granularity(); i++)
		palette.set_pen_indirect(m_gfxdecode->gfx(0)->colorbase() + i, 32 + *color_prom++);

	// sprites use colors 16-31
	for (int i = 0; i < m_gfxdecode->gfx(2)->colors() * m_gfxdecode->gfx(2)->granularity(); i++)
		palette.set_pen_indirect(m_gfxdecode->gfx(2)->colorbase() + i, 16 + *color_prom++);

	// background tiles use colors 0-15, 64-79, 128-143, 192-207 in four banks
	for (int i = 0; i < m_gfxdecode->gfx(1)->colors() * m_gfxdecode->gfx(1)->granularity() / 4; i++)
	{
		palette.set_pen_indirect(m_gfxdecode->gfx(1)->colorbase() + 0 * 32 * 8 + i, *color_prom);
		palette.set_pen_indirect(m_gfxdecode->gfx(1)->colorbase() + 1 * 32 * 8 + i, *color_prom + 64);
		palette.set_pen_indirect(m_gfxdecode->gfx(1)->colorbase() + 2 * 32 * 8 + i, *color_prom + 128);
		palette.set_pen_indirect(m_gfxdecode->gfx(1)->colorbase() + 3 * 32 * 8 + i, *color_prom + 192);
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(vulgus_state::get_fg_tile_info)
{
	int const code = m_fgvideoram[tile_index];
	int const color = m_fgvideoram[tile_index + 0x400];
	tileinfo.set(0,
			code + ((color & 0x80) << 1),
			color & 0x3f,
			0);
	tileinfo.group = color & 0x3f;
}

TILE_GET_INFO_MEMBER(vulgus_state::get_bg_tile_info)
{
	int const code = m_bgvideoram[tile_index];
	int const color = m_bgvideoram[tile_index + 0x400];
	tileinfo.set(1,
			code + ((color & 0x80) << 1),
			(color & 0x1f) + (0x20 * m_palette_bank),
			TILE_FLIPYX((color & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void vulgus_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(vulgus_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(vulgus_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);

	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 47);

	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(6, 6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(6, 6);

	save_item(NAME(m_palette_bank));
}

void _1942iti_state::machine_start()
{
	vulgus_state::machine_start();

	m_rombank->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void vulgus_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void vulgus_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


void vulgus_state::c804_w(uint8_t data)
{
	// bits 0 and 1 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 7 flips screen
	flip_screen_set(data & 0x80);
}


void vulgus_state::palette_bank_w(uint8_t data)
{
	if (m_palette_bank != (data & 3))
	{
		m_palette_bank = data & 3;
		m_bg_tilemap->mark_all_dirty();
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

void vulgus_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(2);

	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int const code = m_spriteram[offs];
		int const color = m_spriteram[offs + 1] & 0x0f;
		int sy = m_spriteram[offs + 2];
		int sx = m_spriteram[offs + 3];
		bool const flip = flip_screen() ? true : false;
		int dir = 1;

		if (sy == 0)
			continue;

		if (flip)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			dir = -1;
		}

		// draw sprite rows (16*16, 16*32, or 16*64)
		int row = (m_spriteram[offs + 1] & 0xc0) >> 6;
		if (row == 2) row = 3;

		for (; row >= 0; row--)
			gfx->transpen(bitmap, cliprect, code + row, color, flip, flip, sx + 128, sy + 6 + 16 * row * dir, 15);
	}
}

uint32_t vulgus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scroll_low[1] + 256 * m_scroll_high[1]);
	m_bg_tilemap->set_scrolly(0, m_scroll_low[0] + 256 * m_scroll_high[0]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


INTERRUPT_GEN_MEMBER(vulgus_state::vblank_irq)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h - vblank
}

void vulgus_state::main_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc801, 0xc801).nopw(); // ?
	map(0xc802, 0xc803).ram().share(m_scroll_low);
	map(0xc804, 0xc804).w(FUNC(vulgus_state::c804_w));
	map(0xc805, 0xc805).w(FUNC(vulgus_state::palette_bank_w));
	map(0xc902, 0xc903).ram().share(m_scroll_high);
	map(0xcc00, 0xcc7f).ram().share(m_spriteram);
	map(0xd000, 0xd7ff).ram().w(FUNC(vulgus_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xd800, 0xdfff).ram().w(FUNC(vulgus_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xe000, 0xefff).ram();
}

void _1942iti_state::_1942iti_main_map(address_map &map)
{
	main_map(map);

	map(0x8000, 0xbfff).bankr(m_rombank);
	map(0xc806, 0xc806).lw8(NAME([this] (uint8_t data) { m_rombank->set_entry(data & 0x03); }));
}

void vulgus_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xc000, 0xc001).w("ay2", FUNC(ay8910_device::address_data_w));
}


static INPUT_PORTS_START( vulgus )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	// Only the parent set seems to use/see the second coin slot even if set to Cocktail mode
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, "Invalid" ) // disables both coins
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:8" ) // Shown as "Unused" in the manual, are 7 & 8 undocumented Difficulty??
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" ) // Shown as "Unused" in the manual, Code performs a read then (& 0x03)
	PORT_DIPNAME( 0x04, 0x04, "Demo Music" )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,3,2")
	PORT_DIPSETTING(    0x30, "10000 50000" )
	PORT_DIPSETTING(    0x50, "10000 60000" )
	PORT_DIPSETTING(    0x10, "10000 70000" )
	PORT_DIPSETTING(    0x70, "20000 60000" )
	PORT_DIPSETTING(    0x60, "20000 70000" )
	PORT_DIPSETTING(    0x20, "20000 80000" )
	PORT_DIPSETTING(    0x40, "30000 70000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};
static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};



static GFXDECODE_START( gfx_vulgus )
	GFXDECODE_ENTRY( "chars",   0, charlayout,           0, 64 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,  64*4+16*16, 32*4 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,      64*4, 16 )
GFXDECODE_END



void vulgus_state::vulgus(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 4);  // 3 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &vulgus_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(vulgus_state::vblank_irq));

	Z80(config, m_audiocpu, XTAL(12'000'000) / 4); // 3 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &vulgus_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(vulgus_state::irq0_line_hold), attotime::from_hz(8*60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000) / 2, 384, 128, 0, 262, 22, 246);  // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(vulgus_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vulgus);

	PALETTE(config, m_palette, FUNC(vulgus_state::palette), 64*4+16*16+4*32*8, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.25); // 1.5 MHz

	AY8910(config, "ay2", XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.25); // 1.5 MHz
}

void _1942iti_state::_1942iti(machine_config &config)
{
	vulgus(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_1942iti_state::_1942iti_main_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vulgus ) // Board ID# 84602-01A-1
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "vulgus.002",   0x0000, 0x2000, CRC(e49d6c5d) SHA1(48072aaa1f2603b6301d7542cc3df10ead2847bb) )
	ROM_LOAD( "vulgus.003",   0x2000, 0x2000, CRC(51acef76) SHA1(14dda82b90f9c3a309561a73c300cb54b5fca77d) )
	ROM_LOAD( "vulgus.004",   0x4000, 0x2000, CRC(489e7f60) SHA1(f3f685955fc42f238909dcdb5edc4c117e5543db) )
	ROM_LOAD( "vulgus.005",   0x6000, 0x2000, CRC(de3a24a8) SHA1(6bc9dda7dbbbef82e9f61c9d5cf1555e5290b249) )
	ROM_LOAD( "1-8n.bin",     0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1-11c.bin",    0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "1-3d.bin",     0x00000, 0x2000, CRC(8bc5d7a5) SHA1(c572b4a26f12013f5f6463b79ba9cbee4c474bbe) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "2-2a.bin",     0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )
	ROM_LOAD( "2-3a.bin",     0x02000, 0x2000, CRC(8da520da) SHA1(c4c633a909526308de4ad83e8ca449fa71eb3cb5) )
	ROM_LOAD( "2-4a.bin",     0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "2-5a.bin",     0x06000, 0x2000, CRC(b6d81984) SHA1(c935176f8a9bce0f74ff466e10c23ff6557f85ec) )
	ROM_LOAD( "2-6a.bin",     0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "2-7a.bin",     0x0a000, 0x2000, CRC(1e1ca773) SHA1(dbced07d4a886ed9ad3302aaa37bc02c599ee132) )

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "2-2n.bin",     0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )
	ROM_LOAD( "2-3n.bin",     0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "2-4n.bin",     0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "2-5n.bin",     0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "e8.bin",       0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    // red component
	ROM_LOAD( "e9.bin",       0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    // green component
	ROM_LOAD( "e10.bin",      0x0200, 0x0100, CRC(de1fb621) SHA1(c719892f0c6d8c82ee2ff41bfe74b67648f5b4f5) )    // blue component
	ROM_LOAD( "d1.bin",       0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    // char lookup table
	ROM_LOAD( "j2.bin",       0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    // sprite lookup table
	ROM_LOAD( "c9.bin",       0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    // tile lookup table
	ROM_LOAD( "82s126.9k",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    // interrupt timing? (not used)
	ROM_LOAD( "82s129.8n",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    // video timing? (not used)
ROM_END

ROM_START( vulgusa )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "v2",           0x0000, 0x2000, CRC(3e18ff62) SHA1(03f61cc25b4c258effac2172f25641b668a1ae97) )
	ROM_LOAD( "v3",           0x2000, 0x2000, CRC(b4650d82) SHA1(4567dfe2b12c59f8c75f5198a136a9afe4975e09) )
	ROM_LOAD( "v4",           0x4000, 0x2000, CRC(5b26355c) SHA1(4220b70ad2bdfe269d4ac4e957114dbd3cea0975) )
	ROM_LOAD( "v5",           0x6000, 0x2000, CRC(4ca7f10e) SHA1(a3c278aecbb63063b660854ccef6fbaff7e58e32) )
	ROM_LOAD( "1-8n.bin",     0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1-11c.bin",    0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "1-3d.bin",     0x00000, 0x2000, CRC(8bc5d7a5) SHA1(c572b4a26f12013f5f6463b79ba9cbee4c474bbe) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "2-2a.bin",     0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )
	ROM_LOAD( "2-3a.bin",     0x02000, 0x2000, CRC(8da520da) SHA1(c4c633a909526308de4ad83e8ca449fa71eb3cb5) )
	ROM_LOAD( "2-4a.bin",     0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "2-5a.bin",     0x06000, 0x2000, CRC(b6d81984) SHA1(c935176f8a9bce0f74ff466e10c23ff6557f85ec) )
	ROM_LOAD( "2-6a.bin",     0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "2-7a.bin",     0x0a000, 0x2000, CRC(1e1ca773) SHA1(dbced07d4a886ed9ad3302aaa37bc02c599ee132) )

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "2-2n.bin",     0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )
	ROM_LOAD( "2-3n.bin",     0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "2-4n.bin",     0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "2-5n.bin",     0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "e8.bin",       0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    // red component
	ROM_LOAD( "e9.bin",       0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    // green component
	ROM_LOAD( "e10.bin",      0x0200, 0x0100, CRC(de1fb621) SHA1(c719892f0c6d8c82ee2ff41bfe74b67648f5b4f5) )    // blue component
	ROM_LOAD( "d1.bin",       0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    // char lookup table
	ROM_LOAD( "j2.bin",       0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    // sprite lookup table
	ROM_LOAD( "c9.bin",       0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    // tile lookup table
	ROM_LOAD( "82s126.9k",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    // interrupt timing? (not used)
	ROM_LOAD( "82s129.8n",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    // video timing? (not used)
ROM_END

ROM_START( vulgusj )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "1-4n.bin",     0x0000, 0x2000, CRC(fe5a5ca5) SHA1(bb1b5ba5ce54f5e329d23fb8ad1357f65f10d6bf) )
	ROM_LOAD( "1-5n.bin",     0x2000, 0x2000, CRC(847e437f) SHA1(1d45ca0b92e7aa3099b8a61c27629d9bec3f25b8) )
	ROM_LOAD( "1-6n.bin",     0x4000, 0x2000, CRC(4666c436) SHA1(a2c921f30f91fead59c4d85d4a5ea8acbcfbf424) )
	ROM_LOAD( "1-7n.bin",     0x6000, 0x2000, CRC(ff2097f9) SHA1(49789c26a2adde043e5dba9d45a89509a211f05c) )
	ROM_LOAD( "1-8n.bin",     0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1-11c.bin",    0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "1-3d.bin",     0x00000, 0x2000, CRC(8bc5d7a5) SHA1(c572b4a26f12013f5f6463b79ba9cbee4c474bbe) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "2-2a.bin",     0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )
	ROM_LOAD( "2-3a.bin",     0x02000, 0x2000, CRC(8da520da) SHA1(c4c633a909526308de4ad83e8ca449fa71eb3cb5) )
	ROM_LOAD( "2-4a.bin",     0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "2-5a.bin",     0x06000, 0x2000, CRC(b6d81984) SHA1(c935176f8a9bce0f74ff466e10c23ff6557f85ec) )
	ROM_LOAD( "2-6a.bin",     0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "2-7a.bin",     0x0a000, 0x2000, CRC(1e1ca773) SHA1(dbced07d4a886ed9ad3302aaa37bc02c599ee132) )

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "2-2n.bin",     0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )
	ROM_LOAD( "2-3n.bin",     0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "2-4n.bin",     0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "2-5n.bin",     0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "e8.bin",       0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    // red component
	ROM_LOAD( "e9.bin",       0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    // green component
	ROM_LOAD( "e10.bin",      0x0200, 0x0100, CRC(de1fb621) SHA1(c719892f0c6d8c82ee2ff41bfe74b67648f5b4f5) )    // blue component
	ROM_LOAD( "d1.bin",       0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    // char lookup table
	ROM_LOAD( "j2.bin",       0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    // sprite lookup table
	ROM_LOAD( "c9.bin",       0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    // tile lookup table
	ROM_LOAD( "82s126.9k",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    // interrupt timing? (not used)
	ROM_LOAD( "82s129.8n",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    // video timing? (not used)
ROM_END

ROM_START( mach9 )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "02_4n.bin",   0x0000, 0x2000, CRC(b3310b0c) SHA1(f083d8633da69acaa03e7566f28e87ef0482927d) )
	ROM_LOAD( "03_5n.bin",   0x2000, 0x2000, CRC(51acef76) SHA1(14dda82b90f9c3a309561a73c300cb54b5fca77d) )
	ROM_LOAD( "04_6n.bin",   0x4000, 0x2000, CRC(489e7f60) SHA1(f3f685955fc42f238909dcdb5edc4c117e5543db) )
	ROM_LOAD( "05_7n.bin",   0x6000, 0x2000, CRC(ef3e4278) SHA1(eb3433827a53b2e9c2b09add7376982e84b29558) )
	ROM_LOAD( "06_8n.bin",   0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "07_11c.bin",  0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "01_3d.bin",   0x00000, 0x2000, CRC(be556775) SHA1(16a4e746ea2462689b7a0e9f01c88d7edf06092d) )

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "08_2a.bin",   0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )
	ROM_LOAD( "09_3a.bin",   0x02000, 0x2000, CRC(9193f2f1) SHA1(de1ee725627baeabec6823f6ecc4e7f6df152ce3) )
	ROM_LOAD( "10_4a.bin",   0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "11_5a.bin",   0x06000, 0x2000, CRC(d729b5b7) SHA1(e9af9bc7f627e313ec070dc9e41ce6f2cddc5b38) )
	ROM_LOAD( "12_6a.bin",   0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "13_7a.bin",   0x0a000, 0x2000, CRC(8033cd4f) SHA1(5eb2e5931e44ca6bf64117dd34e9b6072e6b0ffc) )

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "14_2n.bin",   0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )
	ROM_LOAD( "15_3n.bin",   0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "16_4n.bin",   0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "17_5n.bin",   0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129_8e.bin",    0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    // red component
	ROM_LOAD( "82s129_9e.bin",    0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    // green component
	ROM_LOAD( "82s129_10e.bin",   0x0200, 0x0100, CRC(8404067c) SHA1(6e8826f56267007e2adf02dc03dd96bd40e64935) )    // blue component, only PROM slightly different from original?
	ROM_LOAD( "82s129_1d.bin",    0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    // char lookup table
	ROM_LOAD( "82s129_2j.bin",    0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    // sprite lookup table
	ROM_LOAD( "82s129_9c.bin",    0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    // tile lookup table
	ROM_LOAD( "82s129_9k.bin",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    // interrupt timing? (not used)
	ROM_LOAD( "82s129_8n.bin",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    // video timing? (not used)
ROM_END

// Same PCB as 'mach9'
ROM_START( 1942iti )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2764.n4",             0x00000, 0x2000, CRC(0720ef77) SHA1(59466c22f8c37b80762c95049521fc5d31cf0932) )
	ROM_LOAD( "2764.n5",             0x02000, 0x2000, CRC(9353a860) SHA1(4ed19fc1f4f87e95bcad988b2f9851ed7604e586) )
        ROM_LOAD( "2764.n6",             0x04000, 0x2000, CRC(2b2faee6) SHA1(bcd2e5675b863df8be8bc813e25f4aa65a969359) )
	ROM_LOAD( "2764.n7",             0x06000, 0x2000, CRC(bd3cbb4c) SHA1(9da177d68d39b56375975b8700cc0cc8b48211fe) )
	ROM_LOAD( "daugther_27128.3",    0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "daugther_2764.2",     0x14000, 0x2000, CRC(9eca91e1) SHA1(48ccb608519debb681fa4f78985a074e05040edc) )
	ROM_LOAD( "daugther_27128.1",    0x18000, 0x4000, CRC(c661c8eb) SHA1(d5acf045d5773b01430bb54bc92ccd291318d2d7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "27128.c11",           0x00000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "2764.d3",             0x00000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "bottom_2764.a2",      0x00000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )
	ROM_LOAD( "bottom_2764.a3",      0x02000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "bottom_2764.a4",      0x04000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "bottom_2764.a5",      0x06000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "bottom_2764.a6",      0x08000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "bottom_2764.a7",      0x0a000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "bottom_27128.n2",     0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )
	ROM_LOAD( "bottom_27128.n3",     0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "bottom_27128.n4",     0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "bottom_27128.n5",     0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "bottom_tbp24s10n.e8", 0x00000, 0x0100, CRC(6dbdf73c) SHA1(d335c9a0aa5ada2cd75f6e7125956536afde1b4c) )   // red component
	ROM_LOAD( "bottom_tbp24s10n.e9", 0x00100, 0x0100, CRC(e5c2e1d0) SHA1(a9bd1e0cc330e8174b9a064fba45a5a4c9ecb5c0) )   // green component
	ROM_LOAD( "tbp24s10n.e10",       0x00200, 0x0100, CRC(dc7312a1) SHA1(810174ba1266de83fb238be786a6244229fc30f5) )   // blue component
	ROM_LOAD( "tbp24s10n.d1",        0x00300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )   // char lookup table
	ROM_LOAD( "bottom_tbp24s10n.j2", 0x00400, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )   // sprite lookup table
	ROM_LOAD( "bottom_tbp24s10n.c9", 0x00500, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )   // tile lookup table
	ROM_LOAD( "tbp24s10n.j9",        0x00600, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )   // interrupt timing? (not used)
	ROM_LOAD( "bottom_82s129.n8",    0x00700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )   // video timing? (not used)
ROM_END

} // anonymous namespace


GAME( 1984, vulgus,  0,      vulgus,   vulgus, vulgus_state,   empty_init,   ROT270, "Capcom",          "Vulgus (set 1)",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, vulgusa, vulgus, vulgus,   vulgus, vulgus_state,   empty_init,   ROT90,  "Capcom",          "Vulgus (set 2)",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, vulgusj, vulgus, vulgus,   vulgus, vulgus_state,   empty_init,   ROT270, "Capcom",          "Vulgus (Japan?)",            MACHINE_SUPPORTS_SAVE )
GAME( 1984, mach9,   vulgus, vulgus,   vulgus, vulgus_state,   empty_init,   ROT270, "bootleg (Itisa)", "Mach-9 (bootleg of Vulgus)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, 1942iti, 1942,   _1942iti, vulgus, _1942iti_state, empty_init,   ROT270, "bootleg (Itisa)", "1942 (Itisa bootleg)",       MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
