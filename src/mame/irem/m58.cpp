// license:BSD-3-Clause
// copyright-holders: Lee Taylor
// thanks-to: John Clegg

/****************************************************************************

    Irem M58 hardware

    L Taylor
    J Clegg

    Loosely based on the Kung Fu Master driver.

****************************************************************************/

#include "emu.h"

#include "irem.h"
#include "iremipt.h"

#include "cpu/z80/z80.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class m58_state : public driver_device
{
public:
	m58_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_x_low(*this, "scroll_x_low"),
		m_scroll_x_high(*this, "scroll_x_high"),
		m_scroll_y_low(*this, "scroll_y_low"),
		m_score_panel_disabled(*this, "score_disable"),
		m_dsw2(*this, "DSW2")
	{ }

	void yard(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_x_low;
	required_shared_ptr<uint8_t> m_scroll_x_high;
	required_shared_ptr<uint8_t> m_scroll_y_low;
	required_shared_ptr<uint8_t> m_score_panel_disabled;

	required_ioport m_dsw2;

	tilemap_t* m_bg_tilemap = nullptr;
	bitmap_ind16 m_scroll_panel_bitmap;

	void videoram_w(offs_t offset, uint8_t data);
	void scroll_panel_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);

	void palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_panel(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void yard_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Palette configuration
 *
 *************************************/

void m58_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	uint8_t const *const char_lopal = color_prom + 0x000;
	uint8_t const *const char_hipal = color_prom + 0x100;
	uint8_t const *const sprite_pal = color_prom + 0x200;
	uint8_t const *const sprite_table = color_prom + 0x220;
	uint8_t const *const radar_lopal = color_prom + 0x320;
	uint8_t const *const radar_hipal = color_prom + 0x420;
	static constexpr int resistances_3[3] = { 1000, 470, 220 };
	static constexpr int resistances_2[2]  = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;

	// compute palette information for characters/radar
	scale = compute_resistor_weights(0, 255, -1.0,
			2, resistances_2, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			3, resistances_3, weights_b, 0, 0);

	// character palette
	for (int i = 0; i < 256; i++)
	{
		uint8_t const promval = (char_lopal[i] & 0x0f) | (char_hipal[i] << 4);
		int const r = combine_weights(weights_r, BIT(promval, 6), BIT(promval, 7));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// radar palette
	for (int i = 0; i < 256; i++)
	{
		uint8_t const promval = (radar_lopal[i] & 0x0f) | (radar_hipal[i] << 4);
		int const r = combine_weights(weights_r, BIT(promval, 6), BIT(promval, 7));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));

		palette.set_indirect_color(256 + i, rgb_t(r, g, b));
	}

	// compute palette information for sprites
	scale = compute_resistor_weights(0, 255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	// sprite palette
	for (int i = 0; i < 16; i++)
	{
		uint8_t const promval = sprite_pal[i];
		int const r = combine_weights(weights_r, BIT(promval, 6), BIT(promval, 7));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));

		palette.set_indirect_color(256 + 256 + i, rgb_t(r, g, b));
	}

	// character lookup table
	for (int i = 0; i < 256; i++)
		palette.set_pen_indirect(i, i);

	// radar lookup table
	for (int i = 0; i < 256; i++)
		palette.set_pen_indirect(256 + i, 256 + i);

	// sprite lookup table
	for (int i = 0; i < 256; i++)
	{
		uint8_t const promval = sprite_table[i] & 0x0f;
		palette.set_pen_indirect(256 + 256 + i, 256 + 256 + promval);
	}
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

void m58_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


void m58_state::scroll_panel_w(offs_t offset, uint8_t data)
{
	int sx = (offset % 16);
	int const sy = (offset / 16);

	if (sx < 1 || sx > 14)
		return;

	sx = 4 * (sx - 1);

	for (int i = 0; i < 4; i++)
	{
		int col = (data >> i) & 0x11;
		col = ((col >> 3) | col) & 3;

		m_scroll_panel_bitmap.pix(sy, sx + i) = 0x100 + (sy & 0xfc) + col;
		m_scroll_panel_bitmap.pix(sy, sx + i + 0x2c8) = 0x100 + (sy & 0xfc) + col; // for flipscreen
	}
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m58_state::get_bg_tile_info)
{
	int const offs = tile_index * 2;
	int const attr = m_videoram[offs + 1];
	int const code = m_videoram[offs] + ((attr & 0xc0) << 2);
	int const color = attr & 0x1f;
	int const flags = (attr & 0x20) ? TILE_FLIPX : 0;

	tileinfo.set(0, code, color, flags);
}


TILEMAP_MAPPER_MEMBER(m58_state::tilemap_scan_rows)
{
	// logical (col,row) -> memory offset
	if (col >= 32)
		return (row + 32) * 32 + col - 32;
	else
		return row * 32 + col;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void m58_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m58_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(m58_state::tilemap_scan_rows)), 8, 8, 64, 32);
	m_bg_tilemap->set_scrolldy(26, 26);

	m_screen->register_screen_bitmap(m_scroll_panel_bitmap);
	save_item(NAME(m_scroll_panel_bitmap));
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

void m58_state::flipscreen_w(uint8_t data)
{
	// screen flip is handled both by software and hardware
	flip_screen_set(BIT(data, 0) ^ BIT(~m_dsw2->read(), 0));

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void m58_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = m_screen->visible_area();

	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int const attr = m_spriteram[offs + 1];
		int const bank = (attr & 0x20) >> 5;
		int code1 = m_spriteram[offs + 2] & 0xbf;
		int code2 = 0;
		int const color = attr & 0x1f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = m_spriteram[offs + 3];
		int sy1 = 210 - m_spriteram[offs];
		int sy2 = 0;

		if (flipy)
		{
			code2 = code1;
			code1 += 0x40;
		}
		else
		{
			code2 = code1 + 0x40;
		}

		if (flip_screen())
		{
			sx = 240 - sx;
			sy2 = 192 - sy1;
			sy1 = sy2 + 0x10;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy2 = sy1 + 0x10;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			code1 + 256 * bank, color,
			flipx, flipy, sx, visarea.min_y + sy1,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 512)
		);
		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			code2 + 256 * bank, color,
			flipx, flipy, sx, visarea.min_y + sy2,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 512)
		);
	}
}



/*************************************
 *
 *  Radar panel rendering
 *
 *************************************/

void m58_state::draw_panel(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!*m_score_panel_disabled)
	{
		const rectangle clippanel(26*8, 32*8-1, 1*8, 31*8-1);
		const rectangle clippanelflip(0*8, 6*8-1, 1*8, 31*8-1);
		rectangle clip = flip_screen() ? clippanelflip : clippanel;
		const rectangle &visarea = m_screen->visible_area();
		int const sx = flip_screen() ? cliprect.min_x - 8 : cliprect.max_x + 1 - 14*4;
		int const yoffs = flip_screen() ? -40 : -16;

		clip.min_y += visarea.min_y + yoffs;
		clip.max_y += visarea.max_y + yoffs;
		clip &= cliprect;

		copybitmap(bitmap, m_scroll_panel_bitmap, flip_screen(), flip_screen(), sx, visarea.min_y + yoffs, clip);
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t m58_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, (*m_scroll_x_high * 0x100) + *m_scroll_x_low);
	m_bg_tilemap->set_scrolly(0, *m_scroll_y_low);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_panel(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void m58_state::yard_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x8fff).ram().w(FUNC(m58_state::videoram_w)).share(m_videoram);
	map(0x9000, 0x9fff).w(FUNC(m58_state::scroll_panel_w));
	map(0xc820, 0xc87f).ram().share(m_spriteram);
	map(0xa000, 0xa000).ram().share(m_scroll_x_low);
	map(0xa200, 0xa200).ram().share(m_scroll_x_high);
	map(0xa400, 0xa400).ram().share(m_scroll_y_low);
	map(0xa800, 0xa800).ram().share(m_score_panel_disabled);
	map(0xd000, 0xd000).w("irem_audio", FUNC(irem_audio_device::cmd_w));
	map(0xd001, 0xd001).w(FUNC(m58_state::flipscreen_w));    // + coin counters
	map(0xd000, 0xd000).portr("IN0");
	map(0xd001, 0xd001).portr("IN1");
	map(0xd002, 0xd002).portr("IN2");
	map(0xd003, 0xd003).portr("DSW1");
	map(0xd004, 0xd004).portr("DSW2");
	map(0xe000, 0xefff).ram();
}



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

// Same as m52, m57 and m62 (IREM Z80 hardware)
static INPUT_PORTS_START( m58 )
	PORT_START("IN0")
	/* Start 1 & 2 also restarts and freezes the game with stop mode on
	   and are used in test mode to enter and exit the various tests */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	// coin input must be active for 19 frames to be consistently recognized
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	// DSW1 is so different from game to game that it isn't included here

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/*************************************
 *
 *  Games port definitions
 *
 *************************************/

static INPUT_PORTS_START( yard )
	PORT_INCLUDE(m58)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion (Cheat)" ) PORT_DIPLOCATION("SW2:4") // Listed as "Unused"
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// In stop mode, press 2 to stop and 1 to restart
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:5") // Listed as "Unused"
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Level Select (Cheat)" ) PORT_DIPLOCATION("SW2:6") // Listed as "Unused"
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c, 0x0c, "Time Reduced by Ball Dead" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "x1.3" )
	PORT_DIPSETTING(    0x04, "x1.5" )
	PORT_DIPSETTING(    0x00, "x1.8" )
	IREM_Z80_COINAGE_TYPE_1_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( vs10yarj )
	PORT_INCLUDE(yard)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Allow Continue (Vs. Mode)" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x01, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Defensive Man Pause" ) PORT_DIPLOCATION("SW1:2") // Listed as "Unused"
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vs10yard )
	PORT_INCLUDE(vs10yarj)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // additional test at 0x46e0 on interruption - must be 0
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};


static GFXDECODE_START( gfx_yard )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x3_planar,   0, 32 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     512, 32 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void m58_state::yard(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 3 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &m58_state::yard_map);
	m_maincpu->set_vblank_int("screen", FUNC(m58_state::irq0_line_hold));

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_yard);
	PALETTE(config, m_palette, FUNC(m58_state::palette), 256+256+256, 256+256+16);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(18.432_MHz_XTAL / 3, 384, 0, 256, 282, 42, 266);
	m_screen->set_screen_update(FUNC(m58_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	IREM_M52_LARGE_AUDIO(config, "irem_audio", 0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( 10yard ) // Dumped from an original Irem M52 board. Serial no. 307761/License Seal 09461.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "yf-a-3p-b",    0x0000, 0x2000, CRC(2e205ec2) SHA1(fcfa08f45423b35f2c99d4e6b5474ab1b3a84fec) )
	ROM_LOAD( "yf-a-3n-b",    0x2000, 0x2000, CRC(82fcd980) SHA1(7846705b29961cb95ee1571ee7e16baceea522d4) )
	ROM_LOAD( "yf-a-3m-b",    0x4000, 0x2000, CRC(a8d5c311) SHA1(28edb5cfd943a2262d7e37ef9a7245f7017cbc51) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "yf-a.3e",      0x00000, 0x2000, CRC(77e9e9cc) SHA1(90b0226fc125713dbee2804aeceeb5aa2c8e275e) )
	ROM_LOAD( "yf-a.3d",      0x02000, 0x2000, CRC(854d5ff4) SHA1(9ba09bfabf159facb57faecfe73a6258fa48d152) )
	ROM_LOAD( "yf-a.3c",      0x04000, 0x2000, CRC(0cd8ffad) SHA1(bd1262de3823c34f7394b718477fb5bc58a6e293) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) // chars palette low 4 bits
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) // chars palette high 4 bits
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) // sprites palette
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) // sprites lookup table
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) // radar palette low 4 bits
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) // radar palette high 4 bits
ROM_END

ROM_START( 10yardj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "yf-a.3p",      0x0000, 0x2000, CRC(4586114f) SHA1(a31c68770e7a7eed805c5ba46af302c2895e3cee) )
	ROM_LOAD( "yf-a.3n",      0x2000, 0x2000, CRC(947fa760) SHA1(bd6c2ee6e6800b063b81dbdd9fc929120019439d) )
	ROM_LOAD( "yf-a.3m",      0x4000, 0x2000, CRC(d4975633) SHA1(84a506ae680a9dd26ef6f33880400e965ccf8260) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "yf-a.3e",      0x00000, 0x2000, CRC(77e9e9cc) SHA1(90b0226fc125713dbee2804aeceeb5aa2c8e275e) )
	ROM_LOAD( "yf-a.3d",      0x02000, 0x2000, CRC(854d5ff4) SHA1(9ba09bfabf159facb57faecfe73a6258fa48d152) )
	ROM_LOAD( "yf-a.3c",      0x04000, 0x2000, CRC(0cd8ffad) SHA1(bd1262de3823c34f7394b718477fb5bc58a6e293) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) // chars palette low 4 bits
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) // chars palette high 4 bits
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) // sprites palette
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) // sprites lookup table
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) // radar palette low 4 bits
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) // radar palette high 4 bits
ROM_END

ROM_START( vs10yard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.3p",         0x0000, 0x2000, CRC(1edac08f) SHA1(c6a3290e9dba663dccf0613853abfab8e912477d) )
	ROM_LOAD( "vyf-a.3m",     0x2000, 0x2000, CRC(3b9330f8) SHA1(b35fe72cf724cfb887906060bbcf40b0c896ccf0) )
	ROM_LOAD( "a.3m",         0x4000, 0x2000, CRC(cf783dad) SHA1(0b1b875ac65ba90c92ca06d0aa01c477b7427322) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "vyf-a.3a",     0x00000, 0x2000, CRC(354d7330) SHA1(0dac87e502d5e9089c4e5ca87c7626940a17e9b2) )
	ROM_LOAD( "vyf-a.3c",     0x02000, 0x2000, CRC(f48eedca) SHA1(6aef3208de8b1dd4078de20c0b5ce96219c79d40) )
	ROM_LOAD( "vyf-a.3d",     0x04000, 0x2000, CRC(7d1b4d93) SHA1(9389de1230b93f529c492af6fb911c00280cae8a) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) // chars palette low 4 bits
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) // chars palette high 4 bits
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) // sprites palette
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) // sprites lookup table
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) // radar palette low 4 bits
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) // radar palette high 4 bits
ROM_END

ROM_START( vs10yardj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vyf-a.3n",     0x0000, 0x2000, CRC(418e01fc) SHA1(56a6515735cd88ec803e24574a28aef823a5d36b) )
	ROM_LOAD( "vyf-a.3m",     0x2000, 0x2000, CRC(3b9330f8) SHA1(b35fe72cf724cfb887906060bbcf40b0c896ccf0) )
	ROM_LOAD( "vyf-a.3k",     0x4000, 0x2000, CRC(a0ec15bb) SHA1(a5ce9341e9d05e33c025ac62a27faf738c88326e) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "vyf-a.3a",     0x00000, 0x2000, CRC(354d7330) SHA1(0dac87e502d5e9089c4e5ca87c7626940a17e9b2) )
	ROM_LOAD( "vyf-a.3c",     0x02000, 0x2000, CRC(f48eedca) SHA1(6aef3208de8b1dd4078de20c0b5ce96219c79d40) )
	ROM_LOAD( "vyf-a.3d",     0x04000, 0x2000, CRC(7d1b4d93) SHA1(9389de1230b93f529c492af6fb911c00280cae8a) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) // chars palette low 4 bits
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) // chars palette high 4 bits
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) // sprites palette
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) // sprites lookup table
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) // radar palette low 4 bits
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) // radar palette high 4 bits
ROM_END

ROM_START( vs10yardu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "yf-a-3p-vu.3p",      0x0000, 0x2000, CRC(f5243513) SHA1(de3d2bbc07dd5532248e8335493e8d35d1de8003) )
	ROM_LOAD( "yf-a-3n-h-vs.3n",    0x2000, 0x2000, CRC(a14d7a14) SHA1(1b900ed276dd2d918f82613399416cf399362405) )
	ROM_LOAD( "yf-a-3m-h-vs.3m",    0x4000, 0x2000, CRC(dc4bb0ce) SHA1(9d9c960744720ffeddc7c9f1db4981fb6a0006d7) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "yf-s-3b.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s-1b.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s-3a.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s-1a.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "yf-a-3e-h-vs.3e",     0x00000, 0x2000, CRC(354d7330) SHA1(0dac87e502d5e9089c4e5ca87c7626940a17e9b2) )
	ROM_LOAD( "yf-a-3c-h-vs.3c",     0x02000, 0x2000, CRC(f48eedca) SHA1(6aef3208de8b1dd4078de20c0b5ce96219c79d40) )
	ROM_LOAD( "yf-a-3d-h-vs.3d",     0x04000, 0x2000, CRC(7d1b4d93) SHA1(9389de1230b93f529c492af6fb911c00280cae8a) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "yf-b-5b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )
	ROM_LOAD( "yf-b-5c.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b-5f.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b-5e.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b-5j.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b-5k.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, "proms", 0 ) // on these sets the content of the sprite color PROM needs reversing - are the PROMs on the other sets from bootleg boards, or hand modified?
	ROM_LOAD( "yf-a-5c.5c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) // chars palette low 4 bits
	ROM_LOAD( "yf-a-5d.5d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) // chars palette high 4 bits
	ROMX_LOAD( "yf-b-2b.2b",     0x0200, 0x0020, CRC(fcd283ea) SHA1(6ebc3e966bb920685250f38edab5fe1f8a27c316), ROM_GROUPSIZE(16) | ROM_REVERSE ) // sprites palette
	ROM_LOAD( "yf-b-3l.3l",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) // sprites lookup table
	ROM_LOAD( "yf-b-2r.2r",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) // radar palette low 4 bits
	ROM_LOAD( "yf-b-2p.2p",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) // radar palette high 4 bits
ROM_END

ROM_START( 10yard85 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "yf-a-3p-h.3p",       0x0000, 0x2000, CRC(c83da5e3) SHA1(7f2dd11483158e389f5c39ee8de64be406501451) )
	ROM_LOAD( "yf-a-3n-h.3n",       0x2000, 0x2000, CRC(8dc5f32f) SHA1(f550ed326711d1103711b99777f302f0d48e8eaf) )
	ROM_LOAD( "yf-a-3m-h.3m",       0x4000, 0x2000, CRC(7d5d0c20) SHA1(38ada7a53881f7f812b02514d13fbf0fa013c0f1) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "yf-s-3b.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s-1b.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s-3a.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s-1a.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "yf-a-3e-h.3e",     0x00000, 0x2000, CRC(5fba9074) SHA1(aa9881315850e86b49712a4afb551778ee57ae75) ) // this ROM changes to give the '85 text
	ROM_LOAD( "yf-a-3c-h.3c",     0x02000, 0x2000, CRC(f48eedca) SHA1(6aef3208de8b1dd4078de20c0b5ce96219c79d40) )
	ROM_LOAD( "yf-a-3d-h.3d",     0x04000, 0x2000, CRC(7d1b4d93) SHA1(9389de1230b93f529c492af6fb911c00280cae8a) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "yf-b-5b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )
	ROM_LOAD( "yf-b-5c.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b-5f.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b-5e.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b-5j.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b-5k.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, "proms", 0 ) // on these sets the content of the sprite color PROM needs reversing - are the PROMs on the other sets from bootleg boards, or hand modified?
	ROM_LOAD( "yf-a-5c.5c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) // chars palette low 4 bits
	ROM_LOAD( "yf-a-5d.5d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) // chars palette high 4 bits
	ROMX_LOAD( "yf-b-2b.2b",     0x0200, 0x0020, CRC(fcd283ea) SHA1(6ebc3e966bb920685250f38edab5fe1f8a27c316), ROM_GROUPSIZE(16) | ROM_REVERSE ) // sprites palette
	ROM_LOAD( "yf-b-3l.3l",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) // sprites lookup table
	ROM_LOAD( "yf-b-2r.2r",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) // radar palette low 4 bits
	ROM_LOAD( "yf-b-2p.2p",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) // radar palette high 4 bits
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE  INPUT     STATE      INIT        MONITOR  COMPANY                 FULLNAME                                 FLAGS
GAME( 1983, 10yard,    0,        yard,    yard,     m58_state, empty_init, ROT0,    "Irem",                 "10-Yard Fight (World, set 1)",          MACHINE_SUPPORTS_SAVE ) // no copyright
GAME( 1983, 10yardj,   10yard,   yard,    yard,     m58_state, empty_init, ROT0,    "Irem",                 "10-Yard Fight (Japan)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1984, vs10yard,  10yard,   yard,    vs10yard, m58_state, empty_init, ROT0,    "Irem",                 "Vs 10-Yard Fight (World, 11/05/84)",    MACHINE_SUPPORTS_SAVE )
GAME( 1984, vs10yardj, 10yard,   yard,    vs10yarj, m58_state, empty_init, ROT0,    "Irem",                 "Vs 10-Yard Fight (Japan)",              MACHINE_SUPPORTS_SAVE )
GAME( 1984, vs10yardu, 10yard,   yard,    vs10yard, m58_state, empty_init, ROT0,    "Irem (Taito license)", "Vs 10-Yard Fight (US, Taito license)",  MACHINE_SUPPORTS_SAVE ) // had '85 stickers, but doesn't have '85 on the title screen like the set below
GAME( 1985, 10yard85,  10yard,   yard,    yard,     m58_state, empty_init, ROT0,    "Irem (Taito license)", "10-Yard Fight '85 (US, Taito license)", MACHINE_SUPPORTS_SAVE )
