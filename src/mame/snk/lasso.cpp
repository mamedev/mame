// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria, Luca Elia
/*******************************************************************************

  Lasso and similar hardware

  driver by Phil Stroffolino, Nicola Salmoria, Luca Elia

--------------------------------------------------------------------------------
Year + Game                 By              CPUs        Sound Chips    Misc Info
--------------------------------------------------------------------------------
82  Lasso                   SNK             3 x 6502    2 x SN76489
83  Chameleon               Jaleco          2 x 6502    2 x SN76489
84  Wai Wai Jockey Gate-In! Jaleco/Casio    2 x 6502    2 x SN76489
84  Pinbo                   Jaleco          6502 + Z80  2 x AY-8910    6502 @ 18MHz/24, Z80 @ 18MHz/6, AY @ 18MHz/12
--------------------------------------------------------------------------------------

Notes:

- unknown CPU speeds unless noted above (affect game timing), currently using
  same as the Rock-Ola games of the same area.  Lot of similarities between
  these hardware.
- Lasso: fire button auto-repeats on high score entry screen (real behavior?)

********************************************************************************

Video hardware notes:

Every game has 1 256 x 256 tilemap (non scrollable) made of 8 x 8 tiles, and
16 x 16 sprites (some games use 32, some more). The graphics for tiles and
sprites are held inside the same ROMs, but aren't shared between the two:

the first $100 tiles are for the tilemap, the following $100 are for sprites.
This constitutes the first graphics bank. There can be several.

Lasso has an additional pixel layer (256 x 256 x 1) and a third CPU devoted to
drawing into it (the lasso!)

Wwjgtin has an additional $800 x $400 scrolling tilemap in ROM and $100 more
16 x 16 x 4 tiles for it.

The colors are static ($40 colors, 2 PROMs) but the background color can be
changed at runtime. Wwjgtin can change the last 4 colors (= last palette) too.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lasso_state : public driver_device
{
public:
	lasso_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_back_color(*this, "back_color"),
		m_chip_data(*this, "chip_data"),
		m_bitmap_ram(*this, "bitmap_ram"),
		m_last_colors(*this, "last_colors"),
		m_track_scroll(*this, "track_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn(*this, "sn76489_%u", 0),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void base(machine_config &config);
	void wwjgtin(machine_config &config);
	void lasso(machine_config &config);
	void chameleo(machine_config &config);
	void pinbo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_back_color;
	optional_shared_ptr<uint8_t> m_chip_data;
	optional_shared_ptr<uint8_t> m_bitmap_ram; // 0x2000 bytes for a 256 x 256 x 1 bitmap
	optional_shared_ptr<uint8_t> m_last_colors;
	optional_shared_ptr<uint8_t> m_track_scroll;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_track_tilemap = nullptr;
	uint8_t m_vidctrl = 0;
	uint8_t m_gfxbank = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device_array<sn76489_device, 2> m_sn;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t sound_status_r();
	void sound_select_w(uint8_t data);
	void lasso_videoram_w(offs_t offset, uint8_t data);
	void lasso_colorram_w(offs_t offset, uint8_t data);
	void common_vidctrl_w(uint8_t data);
	void lasso_vidctrl_w(uint8_t data);
	void wwjgtin_vidctrl_w(uint8_t data);
	void pinbo_vidctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(lasso_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(wwjgtin_get_track_tile_info);
	TILE_GET_INFO_MEMBER(pinbo_get_bg_tile_info);
	void lasso_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(wwjgtin);
	void wwjgtin_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(pinbo);
	uint32_t screen_update_lasso(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_chameleo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wwjgtin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pinbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	static rgb_t get_color(int data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int reverse);
	void draw_lasso(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void chameleo_audio_map(address_map &map) ATTR_COLD;
	void chameleo_main_map(address_map &map) ATTR_COLD;
	void lasso_audio_map(address_map &map) ATTR_COLD;
	void lasso_coprocessor_map(address_map &map) ATTR_COLD;
	void lasso_main_map(address_map &map) ATTR_COLD;
	void pinbo_audio_io_map(address_map &map) ATTR_COLD;
	void pinbo_audio_map(address_map &map) ATTR_COLD;
	void pinbo_main_map(address_map &map) ATTR_COLD;
	void wwjgtin_audio_map(address_map &map) ATTR_COLD;
	void wwjgtin_main_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Colors (BBGGGRRR)
 *
 *************************************/

rgb_t lasso_state::get_color(int data)
{
	int bit0, bit1, bit2;

	// red component
	bit0 = BIT(data, 0);
	bit1 = BIT(data, 1);
	bit2 = BIT(data, 2);
	int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	// green component
	bit0 = BIT(data, 3);
	bit1 = BIT(data, 4);
	bit2 = BIT(data, 5);
	int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	// blue component
	bit0 = BIT(data, 6);
	bit1 = BIT(data, 7);
	int const b = 0x52 * bit0 + 0xad * bit1;

	return rgb_t(r, g, b);
}


void lasso_state::lasso_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x40; i++)
		palette.set_pen_color(i, get_color(color_prom[i]));
}


void lasso_state::wwjgtin_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x40; i++)
		palette.set_indirect_color(i, get_color(color_prom[i]));

	// characters/sprites
	for (int i = 0; i < 0x40; i++)
		palette.set_pen_indirect(i, i);

	// track
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (i & 0x03) ? ((((i & 0xf0) >> 2) + (i & 0x03)) & 0x3f) : 0;
		palette.set_pen_indirect(i + 0x40, ctabentry);
	}
}


/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

TILE_GET_INFO_MEMBER(lasso_state::lasso_get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	tileinfo.set(0, code | (m_gfxbank << 8), color & 0x0f, 0);
}

TILE_GET_INFO_MEMBER(lasso_state::wwjgtin_get_track_tile_info)
{
	uint8_t *ROM = memregion("user1")->base();
	int code = ROM[tile_index];
	int color = ROM[tile_index + 0x2000];

	tileinfo.set(2, code, color & 0x0f, 0);
}

TILE_GET_INFO_MEMBER(lasso_state::pinbo_get_bg_tile_info)
{
	int code  = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	tileinfo.set(0, code + ((color & 0x30) << 4), color & 0x0f, 0);
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

void lasso_state::video_start()
{
	// create tilemap
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lasso_state::lasso_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(lasso_state, wwjgtin)
{
	// create tilemaps
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lasso_state::lasso_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);

	m_track_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lasso_state::wwjgtin_get_track_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 64);
}

VIDEO_START_MEMBER(lasso_state, pinbo)
{
	// create tilemap
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lasso_state::pinbo_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

void lasso_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int reverse)
{
	const uint8_t *finish, *source;
	int inc;

	if (reverse)
	{
		source = m_spriteram;
		finish = m_spriteram + m_spriteram.bytes();
		inc = 4;
	}
	else
	{
		source = m_spriteram + m_spriteram.bytes() - 4;
		finish = m_spriteram - 4;
		inc = -4;
	}

	while (source != finish)
	{
		int sx = source[3];
		int sy = source[0];
		int flipx = source[1] & 0x40;
		int flipy = source[1] & 0x80;

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y())
			flipy = !flipy;
		else
			sy = 240 - sy;

		int code = (source[1] & 0x3f) | (m_gfxbank << 6);
		int color = source[2] & 0x0f;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);

		// wraparound
		const int dx = flip_screen_x() ? +256 : -256;
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx + dx, sy, 0);

		source += inc;
	}
}


void lasso_state::draw_lasso(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs >> 5;

		if (flip_screen_y())
			y = ~y;

		if ((y < cliprect.min_y) || (y > cliprect.max_y))
			continue;

		uint8_t x = (offs & 0x1f) << 3;
		uint8_t data = m_bitmap_ram[offs];

		if (flip_screen_x())
			x = ~x;

		for (int bit = 0; bit < 8; bit++)
		{
			if ((data & 0x80) && (x >= cliprect.min_x) && (x <= cliprect.max_x))
				bitmap.pix(y, x) = 0x3f;

			if (flip_screen_x())
				x--;
			else
				x++;

			data <<= 1;
		}
	}
}


uint32_t lasso_state::screen_update_lasso(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_color(0, get_color(*m_back_color));
	bitmap.fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_lasso(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0);

	return 0;
}

uint32_t lasso_state::screen_update_chameleo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_pen_color(0, get_color(*m_back_color));
	bitmap.fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0);

	return 0;
}

uint32_t lasso_state::screen_update_wwjgtin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_palette->set_indirect_color(0, get_color(*m_back_color));

	// the last palette entries can be changed
	for (int i = 0; i < 3; i++)
		m_palette->set_indirect_color(0x3d + i, get_color(m_last_colors[i]));

	m_track_tilemap->set_scrollx(0, m_track_scroll[0] + m_track_scroll[1] * 256);
	m_track_tilemap->set_scrolly(0, m_track_scroll[2] + m_track_scroll[3] * 256);

	if (m_vidctrl & 8)
		m_track_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	draw_sprites(bitmap, cliprect, 1); // reverse order
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

uint32_t lasso_state::screen_update_pinbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// background color (always 0xa0?)
	int r = pal4bit(*m_back_color & 7);
	int g = pal4bit(*m_back_color >> 3 & 7);
	int b = pal4bit(*m_back_color >> 4 & 0xc);

	m_palette->set_pen_color(0, rgb_t(r, g, b));
	bitmap.fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0);

	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void lasso_state::lasso_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void lasso_state::lasso_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void lasso_state::common_vidctrl_w(uint8_t data)
{
	// don't know which is which, but they are always set together
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	machine().tilemap().set_flip_all((flip_screen_x() ? TILEMAP_FLIPX : 0) | (flip_screen_y() ? TILEMAP_FLIPY : 0));

	m_vidctrl = data;
}

void lasso_state::lasso_vidctrl_w(uint8_t data)
{
	if (m_gfxbank != BIT(data, 2))
		machine().tilemap().mark_all_dirty();
	m_gfxbank = BIT(data, 2);

	common_vidctrl_w(data);
}

void lasso_state::wwjgtin_vidctrl_w(uint8_t data)
{
	m_gfxbank = BIT(~data, 2) | BIT(data, 4) << 1;
	common_vidctrl_w(data);
}

void lasso_state::pinbo_vidctrl_w(uint8_t data)
{
	m_gfxbank = (data & 0x0c) >> 2;
	common_vidctrl_w(data);
}


INPUT_CHANGED_MEMBER(lasso_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


uint8_t lasso_state::sound_status_r()
{
	// 0x01: chip#0 ready; 0x02: chip#1 ready
	return m_sn[0]->ready_r() | m_sn[1]->ready_r() << 1;
}

void lasso_state::sound_select_w(uint8_t data)
{
	for (int i = 0; i < 2; i++)
	{
		// 0x01: chip#0; 0x02: chip#1
		if (BIT(~data, i))
			m_sn[i]->write(bitswap<8>(*m_chip_data, 0, 1, 2, 3, 4, 5, 6, 7));
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void lasso_state::lasso_main_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07ff).ram().w(FUNC(lasso_state::lasso_videoram_w)).share("videoram");
	map(0x0800, 0x0bff).ram().w(FUNC(lasso_state::lasso_colorram_w)).share("colorram");
	map(0x0c00, 0x0c7f).ram().share("spriteram");
	map(0x1000, 0x17ff).ram().share("share1");
	map(0x1800, 0x1800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1801, 0x1801).writeonly().share("back_color");
	map(0x1802, 0x1802).w(FUNC(lasso_state::lasso_vidctrl_w));
	map(0x1804, 0x1804).portr("1804");
	map(0x1805, 0x1805).portr("1805");
	map(0x1806, 0x1806).portr("1806").nopw(); // game uses 'lsr' to read port
	map(0x1807, 0x1807).portr("1807");
	map(0x8000, 0xbfff).mirror(0x4000).rom();
}


void lasso_state::lasso_audio_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x5000, 0x7fff).rom();
	map(0xb000, 0xb000).writeonly().share("chip_data");
	map(0xb001, 0xb001).w(FUNC(lasso_state::sound_select_w));
	map(0xb004, 0xb004).r(FUNC(lasso_state::sound_status_r));
	map(0xb005, 0xb005).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf000, 0xffff).rom().region("audiocpu", 0x7000);
}


void lasso_state::lasso_coprocessor_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("share1");
	map(0x2000, 0x3fff).ram().share("bitmap_ram");
	map(0x8000, 0x8fff).mirror(0x7000).rom();
}


void lasso_state::chameleo_main_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07ff).ram().w(FUNC(lasso_state::lasso_videoram_w)).share("videoram");
	map(0x0800, 0x0bff).ram().w(FUNC(lasso_state::lasso_colorram_w)).share("colorram");
	map(0x0c00, 0x0fff).ram();
	map(0x1000, 0x107f).ram().share("spriteram");
	map(0x1080, 0x10ff).ram();
	map(0x1800, 0x1800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1801, 0x1801).writeonly().share("back_color");
	map(0x1802, 0x1802).w(FUNC(lasso_state::lasso_vidctrl_w));
	map(0x1804, 0x1804).portr("1804");
	map(0x1805, 0x1805).portr("1805");
	map(0x1806, 0x1806).portr("1806");
	map(0x1807, 0x1807).portr("1807");
	map(0x4000, 0xbfff).rom();
	map(0xe000, 0xffff).rom().region("maincpu", 0xa000);
}


void lasso_state::chameleo_audio_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x1000, 0x1fff).rom();
	map(0x6000, 0x7fff).rom();
	map(0xb000, 0xb000).writeonly().share("chip_data");
	map(0xb001, 0xb001).w(FUNC(lasso_state::sound_select_w));
	map(0xb004, 0xb004).r(FUNC(lasso_state::sound_status_r));
	map(0xb005, 0xb005).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf000, 0xffff).rom().region("audiocpu", 0x7000);
}


void lasso_state::wwjgtin_main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0bff).ram().w(FUNC(lasso_state::lasso_videoram_w)).share("videoram");
	map(0x0c00, 0x0fff).ram().w(FUNC(lasso_state::lasso_colorram_w)).share("colorram");
	map(0x1000, 0x10ff).ram().share("spriteram");
	map(0x1800, 0x1800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1801, 0x1801).writeonly().share("back_color");
	map(0x1802, 0x1802).w(FUNC(lasso_state::wwjgtin_vidctrl_w));
	map(0x1804, 0x1804).portr("1804");
	map(0x1805, 0x1805).portr("1805");
	map(0x1806, 0x1806).portr("1806");
	map(0x1807, 0x1807).portr("1807");
	map(0x1c00, 0x1c02).nopr().writeonly().share("last_colors");
	map(0x1c04, 0x1c07).nopr().writeonly().share("track_scroll");
	map(0x4000, 0xbfff).rom();
	map(0xc000, 0xffff).rom().region("maincpu", 0x8000);
}


void lasso_state::wwjgtin_audio_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x4000, 0x7fff).mirror(0x8000).rom();
	map(0xb000, 0xb000).writeonly().share("chip_data");
	map(0xb001, 0xb001).w(FUNC(lasso_state::sound_select_w));
	map(0xb003, 0xb003).nopw(); // ?
	map(0xb004, 0xb004).r(FUNC(lasso_state::sound_status_r));
	map(0xb005, 0xb005).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


void lasso_state::pinbo_main_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07ff).ram().w(FUNC(lasso_state::lasso_videoram_w)).share("videoram");
	map(0x0800, 0x0bff).ram().w(FUNC(lasso_state::lasso_colorram_w)).share("colorram");
	map(0x1000, 0x10ff).ram().share("spriteram");
	map(0x1800, 0x1800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1801, 0x1801).writeonly().share("back_color");
	map(0x1802, 0x1802).w(FUNC(lasso_state::pinbo_vidctrl_w));
	map(0x1804, 0x1804).portr("1804");
	map(0x1805, 0x1805).portr("1805");
	map(0x1806, 0x1806).portr("1806");
	map(0x1807, 0x1807).portr("1807");
	map(0x2000, 0x3fff).rom();
	map(0x6000, 0xbfff).rom();
	map(0xe000, 0xffff).rom().region("maincpu", 0xa000);
}


void lasso_state::pinbo_audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0xf000, 0xffff).ram();
}


void lasso_state::pinbo_audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x02).r("ay1", FUNC(ay8910_device::data_r));
	map(0x04, 0x05).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x06, 0x06).r("ay2", FUNC(ay8910_device::data_r));
	map(0x08, 0x08).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw(); // ?
	map(0x14, 0x14).nopw(); // ?
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( lasso )
	PORT_START("1804")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // lasso
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // shoot
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("1805")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("1806")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:!2,!3,!4")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )        // Not documented
//  PORT_DIPSETTING(    0x0a, DEF_STR( 1C_1C ) )        // Not documented
//  PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )        // Not documented
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
//  PORT_DIPSETTING(    0x30, "3" )                     // Not documented
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, "Warm-Up Instructions" )  PORT_DIPLOCATION("SW1:!4") // Listed as "Unused"
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("1807")
	PORT_DIPNAME( 0x01, 0x00, "Warm-Up" )               PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Warm-Up Language" )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW1:!5" )       // Listed as "Unused"
	PORT_DIPNAME( 0x08, 0x00, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW1:!6") // Listed as "Test"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(lasso_state::coin_inserted), 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(lasso_state::coin_inserted), 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1  )
INPUT_PORTS_END

static INPUT_PORTS_START( chameleo )
	PORT_INCLUDE( lasso )

	PORT_MODIFY("1804")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("1805")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("1806")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x30, "5" )
//  PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "Infinite (Cheat)")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!4" )

	PORT_MODIFY("1807")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW1:!3" )      // Probably unused
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:!2" )      // Probably unused
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!5" )      // Probably unused
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wwjgtin )
	PORT_INCLUDE( lasso )

	PORT_MODIFY("1805")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("1806")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:!1" )      // used - has to do with the controls
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )      // probably unused
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )      // probably unused
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!4" )      // probably unused

	PORT_MODIFY("1807")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life) )    PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!5" )      // probably unused
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!6" )      // probably unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(lasso_state::coin_inserted), 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(lasso_state::coin_inserted), 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( pinbo )
	PORT_INCLUDE( lasso )

	PORT_MODIFY("1804")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("1805")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("1806")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "70 (Cheat)")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!4" )      // probably unused

	PORT_MODIFY("1807")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life) )    PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, "500000, 1000000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Controls ) )     PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Reversed" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW1:!5" )       // probably unused
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pinboa )
	PORT_INCLUDE( pinbo )

	PORT_MODIFY("1806")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x30, "70 (Cheat)")
INPUT_PORTS_END


/*************************************
 *
 *  GFX layouts
 *
 *************************************/

static const gfx_layout lasso_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout lasso_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4) },
	{ STEP8(0,1), STEP8(8*8*1,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};

static const gfx_layout wwjgtin_tracklayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0,1), STEP8(8*8*1,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};

// Pinbo is 3bpp, otherwise the same
static const gfx_layout pinbo_charlayout =
{
	8,8,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(0,6), RGN_FRAC(2,6), RGN_FRAC(4,6) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout pinbo_spritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(1,6), RGN_FRAC(3,6), RGN_FRAC(5,6) },
	{ STEP8(0,1), STEP8(8*8*1,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};


static GFXDECODE_START( gfx_lasso )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_spritelayout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_wwjgtin )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, lasso_spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, wwjgtin_tracklayout, 4*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_pinbo )
	GFXDECODE_ENTRY( "gfx1", 0, pinbo_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, pinbo_spritelayout, 0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void lasso_state::machine_start()
{
	save_item(NAME(m_vidctrl));
	save_item(NAME(m_gfxbank));
}

void lasso_state::base(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 11'289'000/16); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &lasso_state::lasso_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(lasso_state::irq0_line_hold));

	M6502(config, m_audiocpu, 11'289'000/16);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lasso_state::lasso_audio_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(61); // guess based on intro song finish time compared to PCB
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(lasso_state::screen_update_lasso));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lasso);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	SN76489(config, m_sn[0], 11'289'000/4).add_route(ALL_OUTPUTS, "speaker", 0.5); // correct
	SN76489(config, m_sn[1], 11'289'000/4).add_route(ALL_OUTPUTS, "speaker", 0.5); // "
}

void lasso_state::lasso(machine_config &config)
{
	base(config);

	// basic machine hardware
	m6502_device &blitter(M6502(config, "blitter", 11'289'000/16)); // guess
	blitter.set_addrmap(AS_PROGRAM, &lasso_state::lasso_coprocessor_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	PALETTE(config, m_palette, FUNC(lasso_state::lasso_palette), 0x40);
}

void lasso_state::chameleo(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_clock(18'000'000/24);
	m_maincpu->set_addrmap(AS_PROGRAM, &lasso_state::chameleo_main_map);

	m_audiocpu->set_clock(18'000'000/24);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lasso_state::chameleo_audio_map);

	// video hardware
	PALETTE(config, m_palette, FUNC(lasso_state::lasso_palette), 0x40);

	m_screen->set_refresh_hz(59); // guess based on intro song finish time compared to PCB
	m_screen->set_screen_update(FUNC(lasso_state::screen_update_chameleo));

	// sound hardware
	m_sn[0]->set_clock(18'000'000/6); // correct
	m_sn[1]->set_clock(18'000'000/6); // "
}

void lasso_state::wwjgtin(machine_config &config)
{
	chameleo(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &lasso_state::wwjgtin_main_map);

	m_audiocpu->set_addrmap(AS_PROGRAM, &lasso_state::wwjgtin_audio_map);

	// video hardware
	m_screen->set_screen_update(FUNC(lasso_state::screen_update_wwjgtin));
	m_gfxdecode->set_info(gfx_wwjgtin); // has 1 additional layer

	PALETTE(config.replace(), m_palette, FUNC(lasso_state::wwjgtin_palette), 0x40 + 16*16, 64);
	MCFG_VIDEO_START_OVERRIDE(lasso_state, wwjgtin)
}

void lasso_state::pinbo(machine_config &config)
{
	chameleo(config);

	// basic machine hardware
	m_maincpu->set_clock(18_MHz_XTAL/24);
	m_maincpu->set_addrmap(AS_PROGRAM, &lasso_state::pinbo_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(lasso_state::irq0_line_hold));

	Z80(config.replace(), m_audiocpu, 18_MHz_XTAL/6);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lasso_state::pinbo_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &lasso_state::pinbo_audio_io_map);

	// video hardware
	m_gfxdecode->set_info(gfx_pinbo);

	PALETTE(config.replace(), m_palette, palette_device::RGB_444_PROMS, "proms", 256);
	MCFG_VIDEO_START_OVERRIDE(lasso_state, pinbo)

	m_screen->set_screen_update(FUNC(lasso_state::screen_update_pinbo));

	// sound hardware
	config.device_remove("sn76489_0");
	config.device_remove("sn76489_1");

	AY8910(config, "ay1", 18_MHz_XTAL/12).add_route(ALL_OUTPUTS, "speaker", 0.5);
	AY8910(config, "ay2", 18_MHz_XTAL/12).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( lasso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wm3",       0x8000, 0x2000, CRC(f93addd6) SHA1(b0a1b263874da8608c3bab4e8785358e2aa19c2e) )
	ROM_LOAD( "wm4",       0xa000, 0x2000, CRC(77719859) SHA1(d206b6af9a567f70d69624866ae9973652527065) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "wmc",       0x5000, 0x1000, CRC(8b4eb242) SHA1(55ada50036abbaa10799f37e35254e6ff70ee947) )
	ROM_LOAD( "wmb",       0x6000, 0x1000, CRC(4658bcb9) SHA1(ecc83ef99edbe5f69a884a142478ff0f56edba12) )
	ROM_LOAD( "wma",       0x7000, 0x1000, CRC(2e7de3e9) SHA1(665a89b9914ca16b9c08b751e142cf7320aaf793) )

	ROM_REGION( 0x10000, "blitter", 0 )
	ROM_LOAD( "wm5",       0x8000, 0x1000, CRC(7dc3ff07) SHA1(46aaa9186940d06fd679a573330e9ad3796aa647) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "wm1",       0x0000, 0x0800, CRC(7db77256) SHA1(d12305bdfb6923c32982982a5544ae9bd8dbc2cb) )
	ROM_CONTINUE(          0x1000, 0x0800 )
	ROM_CONTINUE(          0x0800, 0x0800 )
	ROM_CONTINUE(          0x1800, 0x0800 )
	ROM_LOAD( "wm2",       0x2000, 0x0800, CRC(9e7d0b6f) SHA1(c82be332209bf7331718e51926004fe9aa6f5ebd) )
	ROM_CONTINUE(          0x3000, 0x0800 )
	ROM_CONTINUE(          0x2800, 0x0800 )
	ROM_CONTINUE(          0x3800, 0x0800 )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "82s123.69", 0x0000, 0x0020, CRC(1eabb04d) SHA1(3dc5b407bc1b1dea77337b4e913f1e945386d5c9) )
	ROM_LOAD( "82s123.70", 0x0020, 0x0020, CRC(09060f8c) SHA1(8f14b00bcfb7ab89d2e443cc82f7a65dc96ee819) )
ROM_END

ROM_START( chameleo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "chamel4.bin", 0x4000, 0x2000, CRC(97379c47) SHA1(b29fa2318d4260c29fc95d22a461173dc960ad1a) )
	ROM_LOAD( "chamel5.bin", 0x6000, 0x2000, CRC(0a2cadfd) SHA1(1ccc43accd60ca15b8f03ed1c3fda76a840a2bb1) )
	ROM_LOAD( "chamel6.bin", 0x8000, 0x2000, CRC(b023c354) SHA1(0424ecf81ac9f0e055f9ff01cf0bd6d5c9ff866c) )
	ROM_LOAD( "chamel7.bin", 0xa000, 0x2000, CRC(a5a03375) SHA1(c1eac4596c2bda419f3c513ecd3df9fae49ae159) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "chamel3.bin", 0x1000, 0x1000, CRC(52eab9ec) SHA1(554c34134e3af970262da89fe82feeaf47fd30bc) )
	ROM_LOAD( "chamel2.bin", 0x6000, 0x1000, CRC(81dcc49c) SHA1(7e1b4351775f9c140a43f531da8b055271b7b28c) )
	ROM_LOAD( "chamel1.bin", 0x7000, 0x1000, CRC(96031d3b) SHA1(a143b54b98891423d355e0ba08c3b88d70fa0e23) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "chamel8.bin", 0x0800, 0x0800, CRC(dc67916b) SHA1(8b3fad0d5d42925b44e51df7f88ea4b6a8dbb4f6) )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x0000, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_LOAD( "chamel9.bin", 0x2800, 0x0800, CRC(6b559bf1) SHA1(b7b8b8bccbd88ea868e2d3ccb42513615120d8e6) )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "chambprm.bin", 0x0000, 0x0020, CRC(e3ad76df) SHA1(cd115cece4931bfcfc0f60147b942998a5c21bf7) )
	ROM_LOAD( "chamaprm.bin", 0x0020, 0x0020, CRC(c7063b54) SHA1(53baed3806848207ab3a8fafd182cabec3be4b04) )
ROM_END

ROM_START( wwjgtin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic2.6", 0x4000, 0x4000, CRC(744ba45b) SHA1(cccf3e2dd3c27bf54d2abd366cd9a044311aa031) )
	ROM_LOAD( "ic5.5", 0x8000, 0x4000, CRC(af751614) SHA1(fc0f0a3967524b1743a182c1da4f9b0c3097a157) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic59.9", 0x4000, 0x4000, CRC(2ecb4d98) SHA1(d5b0d447b24f64fca452dc13e6ff95b090fce2d7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "ic81.7", 0x0000, 0x0800, CRC(a27f1a63) SHA1(3c770424bd4996f648687afce4aecea252da83a7) )
	ROM_CONTINUE(       0x2000, 0x0800 )
	ROM_CONTINUE(       0x0800, 0x0800 )
	ROM_CONTINUE(       0x2800, 0x0800 )
	ROM_CONTINUE(       0x1000, 0x0800 )
	ROM_CONTINUE(       0x3000, 0x0800 )
	ROM_CONTINUE(       0x1800, 0x0800 )
	ROM_CONTINUE(       0x3800, 0x0800 )
	ROM_LOAD( "ic82.8", 0x4000, 0x0800, CRC(ea2862b3) SHA1(f7604fd324560c54311c35f806a17e30e018032a) )
	ROM_CONTINUE(       0x6000, 0x0800 )
	ROM_CONTINUE(       0x4800, 0x0800 )
	ROM_CONTINUE(       0x6800, 0x0800 )
	ROM_CONTINUE(       0x5000, 0x0800 )
	ROM_CONTINUE(       0x7000, 0x0800 )
	ROM_CONTINUE(       0x5800, 0x0800 )
	ROM_CONTINUE(       0x7800, 0x0800 )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "ic47.3", 0x0000, 0x2000, CRC(40594c59) SHA1(94533be8e267d9aa5bcdd52b45f6974436d3fed5) ) // 1xxxxxxxxxxxx = 0xFF
	ROM_LOAD( "ic46.4", 0x2000, 0x2000, CRC(d1921348) SHA1(8b5506ff80a31ce721aed515cad1b4a7e52e47a2) )

	ROM_REGION( 0x4000, "user1", 0 ) // tilemap
	ROM_LOAD( "ic48.2", 0x0000, 0x2000, CRC(a4a7df77) SHA1(476aab702346a402169ab404a8b06589e4932d37) )
	ROM_LOAD( "ic49.1", 0x2000, 0x2000, CRC(e480fbba) SHA1(197c86747ef8477040169f90eb6e04d928aedbe5) ) // FIXED BITS (1111xxxx)

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "2.bpr",  0x0000, 0x0020, CRC(79adda5d) SHA1(e54de3eb02f744d49f524cd81e1cf993338916e3) )
	ROM_LOAD( "1.bpr",  0x0020, 0x0020, CRC(c1a93cc8) SHA1(805641ea2ce86589b968f1ff44e5d3ab9377769d) )
ROM_END

ROM_START( photof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic2.bin", 0x4000, 0x4000, CRC(4d960b54) SHA1(fe6c4943cbf9a9c79a2fd1dd86bb6e1f414b3c8d) )
	ROM_LOAD( "ic6.bin", 0x8000, 0x4000, CRC(a4ad21dc) SHA1(55b3ecdf80b4a384a0d9932756330fb3021502f8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic59.bin", 0x4000, 0x4000, CRC(2ecb4d98) SHA1(d5b0d447b24f64fca452dc13e6ff95b090fce2d7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "ic81.bin", 0x0000, 0x0800, CRC(0f170253) SHA1(e8b09cf4e9bae6c762ff325a559fb860a80133aa) )
	ROM_CONTINUE(       0x2000, 0x0800 )
	ROM_CONTINUE(       0x0800, 0x0800 )
	ROM_CONTINUE(       0x2800, 0x0800 )
	ROM_CONTINUE(       0x1000, 0x0800 )
	ROM_CONTINUE(       0x3000, 0x0800 )
	ROM_CONTINUE(       0x1800, 0x0800 )
	ROM_CONTINUE(       0x3800, 0x0800 )
	ROM_LOAD( "ic82.bin", 0x4000, 0x0800, CRC(c4cadee9) SHA1(46cc0ecc3642c432625c0d131aa31fec2e060d2f) )
	ROM_CONTINUE(       0x6000, 0x0800 )
	ROM_CONTINUE(       0x4800, 0x0800 )
	ROM_CONTINUE(       0x6800, 0x0800 )
	ROM_CONTINUE(       0x5000, 0x0800 )
	ROM_CONTINUE(       0x7000, 0x0800 )
	ROM_CONTINUE(       0x5800, 0x0800 )
	ROM_CONTINUE(       0x7800, 0x0800 )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "3-ic47.bin", 0x0000, 0x2000, CRC(40594c59) SHA1(94533be8e267d9aa5bcdd52b45f6974436d3fed5) ) // 1xxxxxxxxxxxx = 0xFF
	ROM_LOAD( "4-ic46.bin", 0x2000, 0x2000, CRC(d1921348) SHA1(8b5506ff80a31ce721aed515cad1b4a7e52e47a2) )

	ROM_REGION( 0x4000, "user1", 0 ) // tilemap
	ROM_LOAD( "2-ic48.bin", 0x0000, 0x2000, CRC(a4a7df77) SHA1(476aab702346a402169ab404a8b06589e4932d37) )
	ROM_LOAD( "1-ic49.bin", 0x2000, 0x2000, CRC(e480fbba) SHA1(197c86747ef8477040169f90eb6e04d928aedbe5) ) // FIXED BITS (1111xxxx)

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "2.bpr",  0x0000, 0x0020, CRC(79adda5d) SHA1(e54de3eb02f744d49f524cd81e1cf993338916e3) )
	ROM_LOAD( "1.bpr",  0x0020, 0x0020, CRC(c1a93cc8) SHA1(805641ea2ce86589b968f1ff44e5d3ab9377769d) )
ROM_END

ROM_START( pinbo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom2.b7",     0x2000, 0x2000, CRC(9a185338) SHA1(4029cf927686b5e14ef7600b17ea3056cc58b15b) )
	ROM_LOAD( "rom3.e7",     0x6000, 0x2000, CRC(1cd1b3bd) SHA1(388ea72568f5bfd39856d872415327a2afaf7fad) )
	ROM_LOAD( "rom4.h7",     0x8000, 0x2000, CRC(ba043fa7) SHA1(ef3d67b6dab5c82035c58290879a3ca969a0256d) )
	ROM_LOAD( "rom5.j7",     0xa000, 0x2000, CRC(e71046c4) SHA1(f49133544c98df5f3e1a1d2ae92e17261b1504fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom1.s8",     0x0000, 0x2000, CRC(ca45a1be) SHA1(d0b2d8f1e6d01b60cba83d2bd458a57548549b4b) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom6.a1",     0x0000, 0x0800, CRC(74fe8e98) SHA1(3c9ac38d7054b2831a515786b6f204b1804aaea3) ) // tiles
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x0800, 0x0800 )
	ROM_CONTINUE(            0x2800, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_LOAD( "rom8.c1",     0x4000, 0x0800, CRC(5a800fe7) SHA1(375269ec73fab7f0cf017a79e002e31b006f5ad7) )
	ROM_CONTINUE(            0x6000, 0x0800 )
	ROM_CONTINUE(            0x4800, 0x0800 )
	ROM_CONTINUE(            0x6800, 0x0800 )
	ROM_CONTINUE(            0x5000, 0x0800 )
	ROM_CONTINUE(            0x7000, 0x0800 )
	ROM_CONTINUE(            0x5800, 0x0800 )
	ROM_CONTINUE(            0x7800, 0x0800 )
	ROM_LOAD( "rom7.d1",     0x8000, 0x0800, CRC(327a3c21) SHA1(e938915d28ac4ec033b20d33728788493e3f30f6) ) // 3rd bitplane
	ROM_CONTINUE(            0xa000, 0x0800 )
	ROM_CONTINUE(            0x8800, 0x0800 )
	ROM_CONTINUE(            0xa800, 0x0800 )
	ROM_CONTINUE(            0x9000, 0x0800 )
	ROM_CONTINUE(            0xb000, 0x0800 )
	ROM_CONTINUE(            0x9800, 0x0800 )
	ROM_CONTINUE(            0xb800, 0x0800 )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "red.l10",     0x0000, 0x0100, CRC(e6c9ba52) SHA1(6ea96f9bd71de6181d675b0f2d59a8c5e1be5aa3) ) // 2nd half is garbage?
	ROM_LOAD( "green.k10",   0x0100, 0x0100, CRC(1bf2d335) SHA1(dcb074d3de939dfc652743e25bc66bd6fbdc3289) ) // "
	ROM_LOAD( "blue.n10",    0x0200, 0x0100, CRC(e41250ad) SHA1(2e9a2babbacb1753057d46cf1dd6dc183611747e) ) // "
ROM_END

ROM_START( pinboa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom2.b7",     0x2000, 0x2000, CRC(9a185338) SHA1(4029cf927686b5e14ef7600b17ea3056cc58b15b) )
	ROM_LOAD( "6.bin",       0x6000, 0x2000, CRC(f80b204c) SHA1(ee9b4ae1d8ea2fc062022fcfae67df87ed7aff41) )
	ROM_LOAD( "5.bin",       0x8000, 0x2000, CRC(c57fe503) SHA1(11b7371c07c9b2c73ab61420a2cc609653c48d37) )
	ROM_LOAD( "4.bin",       0xa000, 0x2000, CRC(d632b598) SHA1(270a5a790a66eaf3d90bf8081ab144fd1af9db3d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "8.bin",       0x0000, 0x2000, CRC(32d1df14) SHA1(c0d4181378bbd6f2c594e923e2f8b21647c7fb0e) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom6.a1",     0x0000, 0x0800, CRC(74fe8e98) SHA1(3c9ac38d7054b2831a515786b6f204b1804aaea3) ) // tiles
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x0800, 0x0800 )
	ROM_CONTINUE(            0x2800, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_LOAD( "rom8.c1",     0x4000, 0x0800, CRC(5a800fe7) SHA1(375269ec73fab7f0cf017a79e002e31b006f5ad7) )
	ROM_CONTINUE(            0x6000, 0x0800 )
	ROM_CONTINUE(            0x4800, 0x0800 )
	ROM_CONTINUE(            0x6800, 0x0800 )
	ROM_CONTINUE(            0x5000, 0x0800 )
	ROM_CONTINUE(            0x7000, 0x0800 )
	ROM_CONTINUE(            0x5800, 0x0800 )
	ROM_CONTINUE(            0x7800, 0x0800 )
	ROM_LOAD( "2.bin",       0x8000, 0x0800, CRC(33cac92e) SHA1(55d4ff3ae9c9519a59bd6021a53584c873b4d327) ) // 3rd bitplane
	ROM_CONTINUE(            0xa000, 0x0800 )
	ROM_CONTINUE(            0x8800, 0x0800 )
	ROM_CONTINUE(            0xa800, 0x0800 )
	ROM_CONTINUE(            0x9000, 0x0800 )
	ROM_CONTINUE(            0xb000, 0x0800 )
	ROM_CONTINUE(            0x9800, 0x0800 )
	ROM_CONTINUE(            0xb800, 0x0800 )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "red.l10",     0x0000, 0x0100, CRC(e6c9ba52) SHA1(6ea96f9bd71de6181d675b0f2d59a8c5e1be5aa3) )
	ROM_LOAD( "green.k10",   0x0100, 0x0100, CRC(1bf2d335) SHA1(dcb074d3de939dfc652743e25bc66bd6fbdc3289) )
	ROM_LOAD( "blue.n10",    0x0200, 0x0100, CRC(e41250ad) SHA1(2e9a2babbacb1753057d46cf1dd6dc183611747e) )
ROM_END

ROM_START( pinbos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4.bin",      0x2000, 0x2000, CRC(d9452d4f) SHA1(c744ee037275b880c0ddc2fd83b3c05eb0a53621) )
	ROM_LOAD( "b5.bin",      0x6000, 0x2000, CRC(f80b204c) SHA1(ee9b4ae1d8ea2fc062022fcfae67df87ed7aff41) )
	ROM_LOAD( "b6.bin",      0x8000, 0x2000, CRC(ae967d83) SHA1(e79db85917a31821d10f919c4c429da33e97894d) )
	ROM_LOAD( "b7.bin",      0xa000, 0x2000, CRC(7a584b4e) SHA1(2eb55b706815228b3b12ee5c0f6c415cd1d612e6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b8.bin",      0x0000, 0x2000, CRC(32d1df14) SHA1(c0d4181378bbd6f2c594e923e2f8b21647c7fb0e) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom6.a1",     0x0000, 0x0800, CRC(74fe8e98) SHA1(3c9ac38d7054b2831a515786b6f204b1804aaea3) ) // tiles
	ROM_CONTINUE(            0x2000, 0x0800 )
	ROM_CONTINUE(            0x0800, 0x0800 )
	ROM_CONTINUE(            0x2800, 0x0800 )
	ROM_CONTINUE(            0x1000, 0x0800 )
	ROM_CONTINUE(            0x3000, 0x0800 )
	ROM_CONTINUE(            0x1800, 0x0800 )
	ROM_CONTINUE(            0x3800, 0x0800 )
	ROM_LOAD( "rom8.c1",     0x4000, 0x0800, CRC(5a800fe7) SHA1(375269ec73fab7f0cf017a79e002e31b006f5ad7) )
	ROM_CONTINUE(            0x6000, 0x0800 )
	ROM_CONTINUE(            0x4800, 0x0800 )
	ROM_CONTINUE(            0x6800, 0x0800 )
	ROM_CONTINUE(            0x5000, 0x0800 )
	ROM_CONTINUE(            0x7000, 0x0800 )
	ROM_CONTINUE(            0x5800, 0x0800 )
	ROM_CONTINUE(            0x7800, 0x0800 )
	ROM_LOAD( "rom7.d1",     0x8000, 0x0800, CRC(327a3c21) SHA1(e938915d28ac4ec033b20d33728788493e3f30f6) ) // 3rd bitplane
	ROM_CONTINUE(            0xa000, 0x0800 )
	ROM_CONTINUE(            0x8800, 0x0800 )
	ROM_CONTINUE(            0xa800, 0x0800 )
	ROM_CONTINUE(            0x9000, 0x0800 )
	ROM_CONTINUE(            0xb000, 0x0800 )
	ROM_CONTINUE(            0x9800, 0x0800 )
	ROM_CONTINUE(            0xb800, 0x0800 )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "red.l10",     0x0000, 0x0100, CRC(e6c9ba52) SHA1(6ea96f9bd71de6181d675b0f2d59a8c5e1be5aa3) )
	ROM_LOAD( "green.k10",   0x0100, 0x0100, CRC(1bf2d335) SHA1(dcb074d3de939dfc652743e25bc66bd6fbdc3289) )
	ROM_LOAD( "blue.n10",    0x0200, 0x0100, CRC(e41250ad) SHA1(2e9a2babbacb1753057d46cf1dd6dc183611747e) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game Drivers
 *
 *************************************/

GAME( 1982, lasso,    0,       lasso,    lasso,    lasso_state, empty_init, ROT90, "SNK",              "Lasso",                   MACHINE_SUPPORTS_SAVE )
GAME( 1983, chameleo, 0,       chameleo, chameleo, lasso_state, empty_init, ROT0,  "Jaleco",           "Chameleon",               MACHINE_SUPPORTS_SAVE )
GAME( 1984, wwjgtin,  0,       wwjgtin,  wwjgtin,  lasso_state, empty_init, ROT0,  "Jaleco / Casio",   "Wai Wai Jockey Gate-In!", MACHINE_SUPPORTS_SAVE )
GAME( 1991, photof,   wwjgtin, wwjgtin,  wwjgtin,  lasso_state, empty_init, ROT0,  "Jaleco / Casio",   "Photo Finish (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, pinbo,    0,       pinbo,    pinbo,    lasso_state, empty_init, ROT90, "Jaleco",           "Pinbo (set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1984, pinboa,   pinbo,   pinbo,    pinboa,   lasso_state, empty_init, ROT90, "Jaleco",           "Pinbo (set 2)",           MACHINE_SUPPORTS_SAVE )
GAME( 1985, pinbos,   pinbo,   pinbo,    pinboa,   lasso_state, empty_init, ROT90, "bootleg (Strike)", "Pinbo (bootleg)",         MACHINE_SUPPORTS_SAVE )
