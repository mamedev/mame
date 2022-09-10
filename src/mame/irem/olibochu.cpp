// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Oli-Boo-Chu (aka Punching Kid)

driver by Nicola Salmoria

TODO:
- verify CPU speed/XTAL, currently guessed
- verify AY8910 irq freq and XTAL, both are approximated from video recording
- Figure out (ad)pcm format or chip? see samples roms, 16.1m is definitely death sound.
  Sample triggers are already hooked up preliminary.
- How is the first half of the palette used? (the colors, not the clut).
  The title logo looks better with it, but not much else does. Or maybe it's a
  region change? The Japanese Irem flyer (Punching Kid) shows the purple/yellow
  maze colors. The USA licensed version has a black background like in MAME,
  and it matches a video of the cabinet.

============================================================================
PSG table (from sound test):
data0 |data1   |sample            | possible data trigger
----------------------------------------------------------
0x80  |0x00    | coin             | 0x.1
0x08  |0x00    | opening melody   | 0x.5
0x40  |0x00    | bgm              | 0x.2
0x04  |0x00    | hi-score melody  | 0x.6
0x02  |0x00    | ending melody    | 0x.7
0x01  |0x00    | spot melody      | 0x.8
0x10  |0x00    | clear melody     | 0x.4
0x20  |0x00    | race trap melody | 0x.3
0x00  |0x80    | extend sound     | 0x.9
0x00  |0x40    | oli paralyze     | 0x.a
0x00  |0x04    | chu out          |
0x00  |0x08    | mystery sound    |
0x00  |0x01    | oli out          | 0x5.?
0x00  |0x10    | chu has food     |
0x00  |0x00    | <stop playing>   | 0x.0
-----------------------------------------------------------
       0x02?     unused?
       0x20?     unused?

$7004 writes, related to $7000 reads
0x00-0x0f <don't care (retains previous value>
0x10-0x1f 0x28 -> sample ROM 0x0500
0x20-0x2f 0x00 -> sample ROM 0x0000
0x30-0x3f 0x48 -> sample ROM 0x0900
0x40-0x4f 0x00 -> sample ROM 0x0000
0x50-0x5f 0x80 -> sample ROM 0x1000
0x60-0x6f 0x3a -> 0x60 and above is invalid (opcodes instead of offsets)
0x70-0x7f 0x70
0x80-0x8f 0x0f
0x90-0x9f 0x0a
0xa0-0xaf 0x3a
0xb0-0xbf 0x61
0xc0-0xcf 0xca
0xd0-0xdf 0x01
0xe0-0xef 0x14
0xf0-0xff 0x32

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class olibochu_state : public driver_device
{
public:
	olibochu_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ay(*this, "aysnd"),
		m_soundlatch(*this, "soundlatch%u", 0)
	{ }

	void olibochu(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_spriteram2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_ay;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;

	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void flipscreen_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	/* misc */
	u16 m_soundcmd = 0;
	u8 m_sample_address = 0;

	void sound_command_w(offs_t offset, u8 data);
	void sample_address_w(u8 data);
	void sample_start_w(u8 data);
	u8 soundlatch_r();

	void main_map(address_map &map);
	void sound_map(address_map &map);
};



void olibochu_state::palette(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	for (int i = 0; i < 0x100; i++)
	{
		palette.set_pen_indirect(i, (color_prom[i] & 0xf) | 0x10);
		palette.set_pen_indirect(i + 0x100, (color_prom[i + 0x100] & 0xf) | 0x10);
	}
}

void olibochu_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void olibochu_state::colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void olibochu_state::flipscreen_w(u8 data)
{
	flip_screen_set(data & 0x80);

	// other bits are used, but unknown
}

TILE_GET_INFO_MEMBER(olibochu_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] | ((attr & 0x20) << 3);
	int color = (attr & 0x1f) | 0x20;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo.category = BIT(attr, 5);
	tileinfo.set(0, code, color, flags);
}

void olibochu_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(olibochu_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
}

void olibochu_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	// 8x8 sprites
	for (int offs = m_spriteram2.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = m_spriteram2[offs + 1];
		int code = m_spriteram2[offs];
		int color = attr & 0x3f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = m_spriteram2[offs + 3];
		int sy = m_spriteram2[offs + 2];

		if (flip_screen())
		{
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(
				bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);
	}

	// 16x16 sprites
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs];
		int color = attr & 0x3f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = m_spriteram[offs + 3];
		int sy = ((m_spriteram[offs + 2] + 8) & 0xff) - 8;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(
				bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);
	}
}

u32 olibochu_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1));
	return 0;
}


void olibochu_state::sound_command_w(offs_t offset, u8 data)
{
	u16 prev_lo = m_soundcmd & 0x003f;
	if (offset == 0)
		m_soundcmd = (m_soundcmd & 0x00ff) | data << 8;
	else
		m_soundcmd = (m_soundcmd & 0xff00) | data;

	u8 c;
	u16 hi = m_soundcmd & 0xffc0;
	u16 lo = m_soundcmd & 0x003f;

	// soundcmd low bits (edge-triggered) = soundlatch d4-d7
	if (lo && lo != prev_lo)
	{
		c = count_leading_zeros_32(lo) - 26;
		m_soundlatch[1]->write(c & 0xf);
	}

	// soundcmd high bits = soundlatch d0-d3
	for (c = 0; c < 16 && !BIT(hi, c); c++) { ; }
	m_soundlatch[0]->write((16 - c) & 0xf);
}

void olibochu_state::sample_address_w(u8 data)
{
	m_sample_address = data;
}

void olibochu_state::sample_start_w(u8 data)
{
	if (data & 0x80)
	{
		// start sample at m_sample_address * 0x20
		m_soundlatch[1]->clear_w();
	}
	else
	{
		// stop playing sample
	}
}

u8 olibochu_state::soundlatch_r()
{
	return (m_soundlatch[0]->read() & 0xf) | (m_soundlatch[1]->read() << 4 & 0xf0);
}


void olibochu_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(olibochu_state::videoram_w)).share("videoram");
	map(0x8400, 0x87ff).ram().w(FUNC(olibochu_state::colorram_w)).share("colorram");
	map(0x9000, 0x901f).writeonly().share("spriteram");
	map(0x9020, 0x903f).nopw(); // discard?
	map(0x9800, 0x983f).writeonly().share("spriteram2");
	map(0xa000, 0xa000).portr("IN0");
	map(0xa001, 0xa001).portr("IN1");
	map(0xa002, 0xa002).portr("IN2");
	map(0xa003, 0xa003).portr("DSW0");
	map(0xa004, 0xa004).portr("DSW1");
	map(0xa005, 0xa005).portr("DSW2");
	map(0xa800, 0xa801).w(FUNC(olibochu_state::sound_command_w));
	map(0xa802, 0xa802).w(FUNC(olibochu_state::flipscreen_w));
	map(0xf000, 0xffff).ram();
}

void olibochu_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x6000, 0x63ff).ram();
	map(0x7000, 0x7000).r(FUNC(olibochu_state::soundlatch_r));
	map(0x7000, 0x7001).w(m_ay, FUNC(ay8910_device::address_data_w));
	map(0x7004, 0x7004).w(FUNC(olibochu_state::sample_address_w));
	map(0x7006, 0x7006).w(FUNC(olibochu_state::sample_start_w));
}


static INPUT_PORTS_START( olibochu )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )  /* works in service mode but not in game */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0") /* Listed as sw1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) /* Nothing listed for this DIP */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) /* Nothing listed for this DIP */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Cross Hatch Pattern" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") /* Most likely not a bank of Dip Switches */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") /* Listed as sw2 */
	PORT_DIPNAME( 0x01, 0x01, "Stop Mode (Cheat)") /* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, "Invulnerability (Cheat)" ) /* Listed as "No Hit" */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) /* Listed as "Start Pattern"... Level Select or Preview?? */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) /* Listed as "Screen 180" currently has no effect */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( gfx_olibochu )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 64 )
GFXDECODE_END



void olibochu_state::machine_start()
{
	save_item(NAME(m_soundcmd));
	save_item(NAME(m_sample_address));
}

void olibochu_state::machine_reset()
{
	m_soundcmd = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(olibochu_state::scanline)
{
	int scanline = param;

	if(scanline == 248) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h

	if(scanline == 0) // sprite buffer irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80 - RST 08h
}

void olibochu_state::olibochu(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &olibochu_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(olibochu_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &olibochu_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(olibochu_state::irq0_line_hold), attotime::from_hz(120));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(olibochu_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_olibochu);
	PALETTE(config, m_palette, FUNC(olibochu_state::palette), 256*2, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	AY8910(config, m_ay, 3072000/2).add_route(ALL_OUTPUTS, "mono", 0.50);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( olibochu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1b.3n",        0x0000, 0x1000, CRC(bf17f4f4) SHA1(1075456f4b70a68548e0e1b6271fd4b845a77ce4) )
	ROM_LOAD( "2b.3lm",       0x1000, 0x1000, CRC(63833b0d) SHA1(0135c449c92470241d03a87709c739209139d660) )
	ROM_LOAD( "3b.3k",        0x2000, 0x1000, CRC(a4038e8b) SHA1(d7dce830239c8975ac135b213a99eec0c20ec3e2) )
	ROM_LOAD( "4b.3j",        0x3000, 0x1000, CRC(aad4bec4) SHA1(9203564ac841a8de2f9b8183d4086acce95e3d47) )
	ROM_LOAD( "5b.3h",        0x4000, 0x1000, CRC(66efa79f) SHA1(535369d958461834435d3202cd7310ecd0aa528c) )
	ROM_LOAD( "6b.3f",        0x5000, 0x1000, CRC(1123d1ef) SHA1(6094e732e61915c45b14acd90c1343f05385daf4) )
	ROM_LOAD( "7c.3e",        0x6000, 0x1000, CRC(89c26fb4) SHA1(ebc51e40612af894b20bd7fc3a5179cd35aaac9b) )
	ROM_LOAD( "8b.3d",        0x7000, 0x1000, CRC(af19e5a5) SHA1(5a55bbee5b2f20e2988171a310c8293dabbd9a72) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "17.4j",        0x0000, 0x1000, CRC(57f07402) SHA1(a763a835ac512c69b4351c1ec72b0a64e46203aa) )
	ROM_LOAD( "18.4l",        0x1000, 0x1000, CRC(0a903e9c) SHA1(d893c2f5373f748d8bebf3673b15014f4a8d4b5c) )

	ROM_REGION( 0x2000, "samples", 0 )
	ROM_LOAD( "15.1k",        0x0000, 0x1000, CRC(fb5dd281) SHA1(fba947ae7b619c2559b5af69ef02acfb15733f0d) )
	ROM_LOAD( "16.1m",        0x1000, 0x1000, CRC(c07614a5) SHA1(d13d271a324f99d008429c16193c4504e5894493) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "13.6n",        0x0000, 0x1000, CRC(b4fcf9af) SHA1(b360daa0670160dca61512823c98bc37ad99b9cf) )
	ROM_LOAD( "14.4n",        0x1000, 0x1000, CRC(af54407e) SHA1(1883928b721e03e452fd0c626c403dc374b02ed7) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "9.6a",         0x0000, 0x1000, CRC(fa69e16e) SHA1(5a493a0a108b3e496884d1f499f3445d4e241ecd) )
	ROM_LOAD( "10.2a",        0x1000, 0x1000, CRC(10359f84) SHA1(df55f06fd98233d0efbc30e3e24bf9b8cab1a5cc) )
	ROM_LOAD( "11.4a",        0x2000, 0x1000, CRC(1d968f5f) SHA1(4acf78d865ca36355bb15dc1d476f5e97a5d91b7) )
	ROM_LOAD( "12.2a",        0x3000, 0x1000, CRC(d8f0c157) SHA1(a7b0c873e016c3b3252c2c9b6400b0fd3d650b2f) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c-1",          0x0000, 0x0020, CRC(e488e831) SHA1(6264741f7091c614093ae1ea4f6ead3d0cef83d3) ) // palette
	ROM_LOAD( "c-2",          0x0020, 0x0100, CRC(698a3ba0) SHA1(3c1a6cb881ef74647c651462a27d812234408e45) ) // char lookup table
	ROM_LOAD( "c-3",          0x0120, 0x0100, CRC(efc4e408) SHA1(f0796426cf324791853aa2ae6d0c3d1f8108d5c2) ) // sprite lookup table
ROM_END

} // anonymous namespace


GAME( 1981, olibochu, 0, olibochu, olibochu, olibochu_state, empty_init, ROT270, "Irem (GDI license)", "Oli-Boo-Chu", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
