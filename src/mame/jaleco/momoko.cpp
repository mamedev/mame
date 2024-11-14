// license:BSD-3-Clause
// copyright-holders: Uki

/*****************************************************************************

    Momoko 120% (c) 1986 Jaleco

    Driver by Uki

    02/Mar/2001 -

******************************************************************************

Notes

The real machine has some bugs(escalator bug, sprite garbage).
They are not emulation bugs.
Flipped screen looks wrong, but it is correct.

Note that the game-breaking escalator bug only happens on an 8-way joystick,
it's safe to assume that this game dedicated control panel was 4-way.


Stephh's notes (based on the game Z80 code and some tests) :

  - Accoding to the "initialisation" routine (code at 0x2a23),
    the "Bonus Life" Dip Switches shall be coded this way :

      PORT_DIPNAME( 0x03, 0x03, "1st Bonus Life" )
      PORT_DIPSETTING(    0x01, "20k" )
      PORT_DIPSETTING(    0x03, "30k" )
      PORT_DIPSETTING(    0x02, "50k" )
      PORT_DIPSETTING(    0x00, "100k" )
      PORT_DIPNAME( 0x0c, 0x0c, "2nd Bonus Life" )
      PORT_DIPSETTING(    0x04, "100k" )
      PORT_DIPSETTING(    0x08, "200k" )
      PORT_DIPSETTING(    0x00, "500k" )
      PORT_DIPSETTING(    0x0c, DEF_STR( None ) )

    But in the "check score for life" routine (code at 29c3),
    there is a 'ret' instruction (0xc9) instead of 'retc' (0xd8)
    that doesn't allow the player to get a 2nd bonus life !
  - DSW1 is read each frame (code at 0x2197), but result is
    completely discarded due to 'xor' instruction at 0x219a.
    I can't tell what this read is supposed to do.

*****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class momoko_state : public driver_device
{
public:
	momoko_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_bg_scrolly(*this, "bg_scrolly"),
		m_bg_scrollx(*this, "bg_scrollx"),
		m_bg_gfx(*this, "bg_gfx"),
		m_bg_map(*this, "bg_map"),
		m_bg_col_map(*this, "bg_col_map"),
		m_fg_map(*this, "fg_map"),
		m_proms(*this, "proms"),
		m_bgbank(*this, "bgbank"),
		m_io_fake(*this, "FAKE"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void momoko(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_bg_scrolly;
	required_shared_ptr<u8> m_bg_scrollx;
	required_region_ptr<u8> m_bg_gfx;
	required_region_ptr<u8> m_bg_map;
	required_region_ptr<u8> m_bg_col_map;
	required_region_ptr<u8> m_fg_map;
	required_region_ptr<u8> m_proms;
	required_memory_bank m_bgbank;
	required_ioport m_io_fake;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	u8 m_fg_scrollx = 0;
	u8 m_fg_scrolly = 0;
	u8 m_fg_select = 0;
	u8 m_text_scrolly = 0;
	u8 m_text_mode = 0;
	u8 m_bg_select = 0;
	u8 m_bg_priority = 0;
	u8 m_bg_mask = 0;
	u8 m_fg_mask = 0;
	u8 m_flipscreen = 0;

	void bg_read_bank_w(u8 data);
	void fg_scrollx_w(u8 data);
	void fg_scrolly_w(u8 data);
	void fg_select_w(u8 data);
	void text_scrolly_w(u8 data);
	void text_mode_w(u8 data);
	void bg_scrollx_w(offs_t offset, u8 data);
	void bg_scrolly_w(offs_t offset, u8 data);
	void bg_select_w(u8 data);
	void bg_priority_w(u8 data);
	void flipscreen_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bg_pri(bitmap_ind16 &bitmap, int chr, int col, int flipx, int flipy, int x, int y, int pri);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int flip);
	void draw_text_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_fg_romtilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_bg_romtilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, bool high);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************

    Video consists of
    - Sprites
    - 2x ROM based tilemaps (bg and fg) with priority over some sprites
    - 1x RAM based tilemap (text layer) (no wrapping?)

    TODO:
    - update to use tilemap system?
    - check if any of this is common Jaleco/NMK etc. hardware and use shared
      devices if possible

*******************************************************************************/

void momoko_state::fg_scrollx_w(u8 data)
{
	m_fg_scrollx = data;
}

void momoko_state::fg_scrolly_w(u8 data)
{
	m_fg_scrolly = data;
}

void momoko_state::fg_select_w(u8 data)
{
	m_fg_select = data & 0x0f;
	m_fg_mask = data & 0x10;
}

void momoko_state::text_scrolly_w(u8 data)
{
	m_text_scrolly = data;
}

void momoko_state::text_mode_w(u8 data)
{
	m_text_mode = data;
}

void momoko_state::bg_scrollx_w(offs_t offset, u8 data)
{
	m_bg_scrollx[offset] = data;
}

void momoko_state::bg_scrolly_w(offs_t offset, u8 data)
{
	m_bg_scrolly[offset] = data;
}

void momoko_state::bg_select_w(u8 data)
{
	m_bg_select = data & 0x0f;
	m_bg_mask = data & 0x10;
}

void momoko_state::bg_priority_w(u8 data)
{
	m_bg_priority = data & 0x01;
}

void momoko_state::flipscreen_w(u8 data)
{
	m_flipscreen = data & 0x01;
}

/****************************************************************************/

void momoko_state::draw_bg_pri(bitmap_ind16 &bitmap, int chr, int col, int flipx, int flipy, int x, int y, int pri)
{
	for (int sy = 0; sy < 8; sy++)
	{
		const u32 gfxadr = chr * 16 + sy * 2;
		for (int xx = 0; xx < 2; xx++)
		{
			u8 d0 = m_bg_gfx[gfxadr + xx * 4096];
			u8 d1 = m_bg_gfx[gfxadr + xx * 4096 + 1];

			for (int sx = 0; sx < 4; sx++)
			{
				const u8 dot = (d0 & 0x08) | ((d0 & 0x80) >> 5) | ((d1 & 0x08) >> 2) | ((d1 & 0x80) >> 7);
				const int px = (flipx == 0) ? (sx + xx * 4 + x) : (7 - sx - xx * 4 + x);
				const int py = (flipy == 0) ? (sy + y) : (7 - sy + y);

				if (dot >= pri)
					bitmap.pix(py, px) = col * 16 + dot + 256;

				d0 <<= 1;
				d1 <<= 1;
			}
		}
	}
}


void momoko_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int flip)
{
	for (int offs = start; offs < end; offs += 4)
	{
		int px, py;
		u32 chr = m_spriteram[offs + 1] | ((m_spriteram[offs + 2] & 0x60) << 3);
		chr = ((chr & 0x380) << 1) | (chr & 0x7f);
		const int col = m_spriteram[offs + 2] & 0x07;
		const int fx = ((m_spriteram[offs + 2] & 0x10) >> 4) ^ flip;
		const int fy = ((m_spriteram[offs + 2] & 0x08) >> 3) ^ flip; // ???
		const int x = m_spriteram[offs + 3];
		const int y = m_spriteram[offs + 0];

		if (flip == 0)
		{
			px = x;
			py = 239 - y;
		}
		else
		{
			px = 248 - x;
			py = y + 1;
		}
		m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
			chr,
			col,
			!fx, fy,
			px, py, 0);
	}
}

void momoko_state::draw_text_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	for (int y = 16; y < 240; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py, col;
			int sy = y;
			if (m_text_mode == 0)
				col = m_proms[(sy >> 3) + 0x100] & 0x0f;
			else
			{
				if (m_proms[y] < 0x08)
					sy += m_text_scrolly;
				col = (m_proms[y] & 0x07) + 0x10;
			}
			const int dy = sy & 7;
			if (flip == 0)
			{
				px = x * 8;
				py = y;
			}
			else
			{
				px = 248 - x * 8;
				py = 255 - y;
			}
			const int ramoffset = (sy >> 3) * 32 + x;

			if (ramoffset < 0x400) // high score table, no wrapping?
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					m_videoram[ramoffset] * 8 + dy,
					col,
					flip, 0,
					px, py, 0);
		}
	}
}

void momoko_state::draw_fg_romtilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	const int dx = (7 - m_fg_scrollx) & 7;
	const int dy = (7 - m_fg_scrolly) & 7;
	const int rx = m_fg_scrollx >> 3;
	const int ry = m_fg_scrolly >> 3;

	for (int y = 0; y < 29; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py;
			const int radr = ((ry + y + 34) & 0x3f) * 0x20 + ((rx + x) & 0x1f) + (m_fg_select & 3) * 0x800;
			const u32 chr = m_fg_map[radr];
			if (flip == 0)
			{
				px = 8 * x + dx - 6;
				py = 8 * y + dy + 9;
			}
			else
			{
				px = 248 - (8 * x + dx - 8);
				py = 248 - (8 * y + dy + 9);
			}
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
				chr,
				0, // color
				flip, flip, // flip
				px, py, 0);
		}
	}
}

void momoko_state::draw_bg_romtilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int flip, bool high)
{
	const int dx = (7 - m_bg_scrollx[0]) & 7;
	const int dy = (7 - m_bg_scrolly[0]) & 7;
	const int rx = (m_bg_scrollx[0] + m_bg_scrollx[1] * 256) >> 3;
	const int ry = (m_bg_scrolly[0] + m_bg_scrolly[1] * 256) >> 3;

	for (int y = 0; y < 29; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py;
			const int radr = ((ry + y + 2) & 0x3ff) * 128 + ((rx + x) & 0x7f);
			u32 chr = m_bg_map[radr];
			int col = m_bg_col_map[chr + m_bg_select * 512 + m_bg_priority * 256];
			chr = chr + m_bg_select * 512;

			if (flip == 0)
			{
				px = 8 * x + dx - 6;
				py = 8 * y + dy + 9;
			}
			else
			{
				px = 248 - (8 * x + dx - 8);
				py = 248 - (8 * y + dy + 9);
			}

			if (!high)
			{
				m_gfxdecode->gfx(1)->opaque(bitmap, cliprect,
					chr,
					col,
					flip, flip,
					px, py);
			}
			else
			{
				const u8 pri = (col & 0x10) >> 1;
				if (pri != 0)
				{
					col = col & 0x0f;
					draw_bg_pri(bitmap, chr, col, flip, flip, px, py, pri);
				}
			}
		}
	}
}

/****************************************************************************/

u32 momoko_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int flip = m_flipscreen ^ (m_io_fake->read() & 0x01);

	// draw BG layer - all tiles

	if (m_bg_mask == 0)
		draw_bg_romtilemap(screen, bitmap, cliprect, flip, false);
	else
		bitmap.fill(256, cliprect);

	// draw sprites (momoko)
	draw_sprites(screen, bitmap, cliprect, 0, 9 * 4, flip);

	// draw BG layer - high priority tiles
	if (m_bg_mask == 0)
		draw_bg_romtilemap(screen, bitmap, cliprect, flip, true);

	// draw sprites (others)
	draw_sprites(screen, bitmap, cliprect, 9 * 4, m_spriteram.bytes(), flip);

	// draw text layer
	draw_text_tilemap(screen, bitmap, cliprect, flip);

	// draw FG layer
	if (m_fg_mask == 0)
		draw_fg_romtilemap(screen, bitmap, cliprect, flip);

	return 0;
}


void momoko_state::bg_read_bank_w(u8 data)
{
	m_bgbank->set_entry(data & 0x1f);
}

/****************************************************************************/

void momoko_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd064, 0xd0ff).ram().share(m_spriteram);
	map(0xd400, 0xd400).portr("IN0").nopw(); // interrupt ack?
	map(0xd402, 0xd402).portr("IN1").w(FUNC(momoko_state::flipscreen_w));
	map(0xd404, 0xd404).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xd406, 0xd406).portr("DSW0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xd407, 0xd407).portr("DSW1");
	map(0xd800, 0xdbff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xdc00, 0xdc00).w(FUNC(momoko_state::fg_scrolly_w));
	map(0xdc01, 0xdc01).w(FUNC(momoko_state::fg_scrollx_w));
	map(0xdc02, 0xdc02).w(FUNC(momoko_state::fg_select_w));
	map(0xe000, 0xe3ff).ram().share(m_videoram);
	map(0xe800, 0xe800).w(FUNC(momoko_state::text_scrolly_w));
	map(0xe801, 0xe801).w(FUNC(momoko_state::text_mode_w));
	map(0xf000, 0xffff).bankr(m_bgbank);
	map(0xf000, 0xf001).w(FUNC(momoko_state::bg_scrolly_w)).share(m_bg_scrolly);
	map(0xf002, 0xf003).w(FUNC(momoko_state::bg_scrollx_w)).share(m_bg_scrollx);
	map(0xf004, 0xf004).w(FUNC(momoko_state::bg_read_bank_w));
	map(0xf006, 0xf006).w(FUNC(momoko_state::bg_select_w));
	map(0xf007, 0xf007).w(FUNC(momoko_state::bg_priority_w));
}

void momoko_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).nopw(); // unknown
	map(0xa000, 0xa001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xb000, 0xb000).nopw(); // unknown
	map(0xc000, 0xc001).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


/****************************************************************************/

static INPUT_PORTS_START( momoko )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       // see notes
	PORT_DIPSETTING(    0x01, "20k" )
	PORT_DIPSETTING(    0x03, "30k" )
	PORT_DIPSETTING(    0x02, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                   // see notes
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                   // see notes
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("FAKE")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	256,    // 256 characters
	2,      // 2 bits per pixel
	{4, 0},
	{0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};

static const gfx_layout spritelayout =
{
	8,16,     // 8*16 characters
	2048-128, // 1024 sprites ( ccc 0ccccccc )
	4,        // 4 bits per pixel
	{12,8,4,0},
	{0, 1, 2, 3, 4096*8+0, 4096*8+1, 4096*8+2, 4096*8+3},
	{0, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16},
	8*32
};

static const gfx_layout tilelayout =
{
	8,8,      // 8*8 characters
	8192-256, // 4096 tiles ( cccc0 cccccccc )
	4,        // 4 bits per pixel
	{4,0,12,8},
	{0, 1, 2, 3, 4096*8+0, 4096*8+1, 4096*8+2, 4096*8+3},
	{0, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16},
	8*16
};

static const gfx_layout charlayout1 =
{
	8,1,    // 8*1 characters
	256*8,  // 2048 characters
	2,      // 2 bits per pixel
	{4, 0},
	{0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3},
	{8*0},
	8*1
};

static GFXDECODE_START( gfx_momoko )
	GFXDECODE_ENTRY( "text",    0x0000, charlayout1,      0,  24 )
	GFXDECODE_ENTRY( "bg_gfx",  0x0000, tilelayout,     256,  16 )
	GFXDECODE_ENTRY( "fg_gfx",  0x0000, charlayout,       0,   1 )
	GFXDECODE_ENTRY( "spr_gfx", 0x0000, spritelayout,   128,   8 )
GFXDECODE_END

/****************************************************************************/

void momoko_state::machine_start()
{
	m_bgbank->configure_entries(0, 32, &m_bg_map[0x0000], 0x1000);

	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_fg_select));
	save_item(NAME(m_text_scrolly));
	save_item(NAME(m_text_mode));
	save_item(NAME(m_bg_select));
	save_item(NAME(m_bg_priority));
	save_item(NAME(m_bg_mask));
	save_item(NAME(m_fg_mask));
	save_item(NAME(m_flipscreen));
}

void momoko_state::machine_reset()
{
	m_fg_scrollx = 0;
	m_fg_scrolly = 0;
	m_fg_select = 0;
	m_text_scrolly = 0;
	m_text_mode = 0;
	m_bg_select = 0;
	m_bg_priority = 0;
	m_bg_mask = 0;
	m_fg_mask = 0;
	m_flipscreen = 0;
}

void momoko_state::momoko(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(10'000'000) / 2);   // 5.0MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &momoko_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(momoko_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(10'000'000) / 4));  // 2.5MHz
	audiocpu.set_addrmap(AS_PROGRAM, &momoko_state::sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 2*8, 29*8-1);
	screen.set_screen_update(FUNC(momoko_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_momoko);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 512);
	m_palette->set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(10'000'000) / 8));
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.40);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(10'000'000) / 8));
	ym2.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.40);
}

/****************************************************************************/

ROM_START( momoko )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "momoko03.m6", 0x0000,  0x8000, CRC(386e26ed) SHA1(ad746ed1b87bafc5b4df9a28aade58cf894f4e7b) ) // age progression text in Japanese
	ROM_LOAD( "momoko02.m5", 0x8000,  0x4000, CRC(4255e351) SHA1(27a0e8d8aea223d2128139582e3b66106f3608ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "momoko01.u4", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "text", 0 )
	ROM_LOAD( "momoko13.u4", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) ) // On the FP-8631 PCB

	ROM_REGION( 0x2000, "fg_gfx", 0 )
	ROM_LOAD( "momoko14.p2", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "spr_gfx", 0 )
	ROM_LOAD16_BYTE( "momoko16.e5", 0x0000,  0x8000, CRC(fc6876fc) SHA1(b2d06bc01ef9f4db9bf8902d67f31ccbb0fea61a) ) // On the FP-8631 PCB
	ROM_LOAD16_BYTE( "momoko17.e6", 0x0001,  0x8000, CRC(45dc0247) SHA1(1b2bd4197ab7d237966e037c249b5bd623646c0b) ) // On the FP-8631 PCB

	ROM_REGION( 0x20000, "bg_gfx", 0 )
	ROM_LOAD16_BYTE( "momoko09.e8", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.c8", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "momoko10.d8", 0x10000, 0x8000, CRC(ae17e74b) SHA1(f52657ea6b6ac518b70fd7b811d9699da27f67d9) )
	ROM_LOAD16_BYTE( "momoko12.a8", 0x10001, 0x8000, CRC(1e29c9c4) SHA1(d78f102cefc9852b529dd317a76c7003ec2ad3d5) )

	ROM_REGION( 0x20000, "bg_map", 0 )
	ROM_LOAD( "momoko04.r8", 0x0000,  0x8000, CRC(3ab3c2c3) SHA1(d4a0d7f83bf64769e90a2c264c6114ac308cb8b5) )
	ROM_LOAD( "momoko05.p8", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.n8", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.l8", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "bg_col_map", 0 )
	ROM_LOAD( "momoko08.h8", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "fg_map", 0 )
	ROM_LOAD( "momoko15.k2", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) ) // On the FP-8631 PCB

	ROM_REGION( 0x0120, "proms", 0 ) // text color
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

ROM_START( momokoe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.m6", 0x0000,  0x8000, CRC(84053a7d) SHA1(6e8fb22bb48954f4fed2530991ebe5b872c9c089) ) // age progression text in English
	ROM_LOAD( "2.m5", 0x8000,  0x4000, CRC(98ad397b) SHA1(b7ae218d0d397b1e258ec6d1f836cb998f984092) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "momoko01.u4", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "text", 0 )
	ROM_LOAD( "momoko13.u4", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) ) // On the FP-8631 PCB

	ROM_REGION( 0x2000, "fg_gfx", 0 )
	ROM_LOAD( "momoko14.p2", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "spr_gfx", 0 )
	ROM_LOAD16_BYTE( "momoko16.e5", 0x0000,  0x8000, CRC(fc6876fc) SHA1(b2d06bc01ef9f4db9bf8902d67f31ccbb0fea61a) ) // On the FP-8631 PCB
	ROM_LOAD16_BYTE( "momoko17.e6", 0x0001,  0x8000, CRC(45dc0247) SHA1(1b2bd4197ab7d237966e037c249b5bd623646c0b) ) // On the FP-8631 PCB

	ROM_REGION( 0x20000, "bg_gfx", 0 )
	ROM_LOAD16_BYTE( "momoko09.e8", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.c8", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "momoko10.d8", 0x10000, 0x8000, CRC(ae17e74b) SHA1(f52657ea6b6ac518b70fd7b811d9699da27f67d9) )
	ROM_LOAD16_BYTE( "momoko12.a8", 0x10001, 0x8000, CRC(1e29c9c4) SHA1(d78f102cefc9852b529dd317a76c7003ec2ad3d5) )

	ROM_REGION( 0x20000, "bg_map", 0 )
	ROM_LOAD( "momoko04.r8", 0x0000,  0x8000, CRC(3ab3c2c3) SHA1(d4a0d7f83bf64769e90a2c264c6114ac308cb8b5) )
	ROM_LOAD( "momoko05.p8", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.n8", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.l8", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "bg_col_map", 0 )
	ROM_LOAD( "momoko08.h8", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "fg_map", 0 )
	ROM_LOAD( "momoko15.k2", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) ) // On the FP-8631 PCB

	ROM_REGION( 0x0120, "proms", 0 ) // text color
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

ROM_START( momokob ) // bootleg board, almost exact copy of an original one
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin", 0x0000,  0x8000, CRC(a18d7e78) SHA1(5d2dd498be3e22b5e8fc5ffe17e1ef463c1e9a02) ) // age progression text in Engrish, title screen in English
	ROM_LOAD( "2.bin", 0x8000,  0x4000, CRC(2dcf50ed) SHA1(6d02cb86fce031859bc0a5a26ecf7a8c8b89dea3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "momoko01.u4", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "text", 0 )
	ROM_LOAD( "momoko13.u4", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) )

	ROM_REGION( 0x2000, "fg_gfx", 0 )
	ROM_LOAD( "momoko14.p2", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "spr_gfx", 0 )
	ROM_LOAD16_BYTE( "16.bin", 0x0000,  0x8000, CRC(49de49a1) SHA1(b4954286cba50332d4366a8160e9fbfd574c60ed) )
	ROM_LOAD16_BYTE( "17.bin", 0x0001,  0x8000, CRC(f06a3d1a) SHA1(f377ffad958fdc9cff2baee70ce4ba9080b5fe0d) )

	ROM_REGION( 0x20000, "bg_gfx", 0 )
	ROM_LOAD16_BYTE( "momoko09.e8", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.c8", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "10.bin",      0x10000, 0x8000, CRC(68b9156d) SHA1(e157434d7ee33837ba35e720d221bf1eb21b7020) )
	ROM_LOAD16_BYTE( "12.bin",      0x10001, 0x8000, CRC(c32f5e19) SHA1(488da565e20bf002ff3dffca1efedbdf29e6e559) )

	ROM_REGION( 0x20000, "bg_map", 0 )
	ROM_LOAD( "4.bin",       0x0000,  0x8000, CRC(1f0226d5) SHA1(6411e85c51e23dfe6c643692987dc7eeef37538f) )
	ROM_LOAD( "momoko05.p8", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.n8", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.l8", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "bg_col_map", 0 )
	ROM_LOAD( "momoko08.h8", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "fg_map", 0 )
	ROM_LOAD( "momoko15.k2", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) )

	ROM_REGION( 0x0120, "proms", 0 ) // text color
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

} // anonymous namespace


GAME( 1986, momoko,       0, momoko, momoko, momoko_state, empty_init, ROT0, "Jaleco",  "Momoko 120% (Japanese text)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, momokoe, momoko, momoko, momoko, momoko_state, empty_init, ROT0, "Jaleco",  "Momoko 120% (English text)",  MACHINE_SUPPORTS_SAVE )
GAME( 1986, momokob, momoko, momoko, momoko, momoko_state, empty_init, ROT0, "bootleg", "Momoko 120% (bootleg)",       MACHINE_SUPPORTS_SAVE )
