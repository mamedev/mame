// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria
/*******************************************************************************

Splendor Blast    (c) 1985 Alpha Denshi Co.
Splendor Blast II (c) 1985 Alpha Denshi Co.
High Voltage      (c) 1985 Alpha Denshi Co.

Driver by Acho A. Tang, Nicola Salmoria


Stephh's notes (based on the games M68000 code and some tests) :

splndrbt:
- When starting a 2 players game, when player 1 game is over, the game enters in
  an infinite loop on displaying the "GAME OVER" message.
- You can test player 2 by putting 0xff instead of 0x00 at 0x040009 ($9,A6).
- FYI, what should change the contents of $9,A6 is the routine at 0x000932,
  but I haven't found where this routine could be called 8( 8303 issue ?

hvoltage:
- There is sort of "debug mode" that you can access if 0x000038.w returns 0x0000
  instead of 0xffff under irq 1. To enable it, use the MAME debugger or cheats.
  This also applies to all non-parent splndrbt sets, at least for the freeze part.
- When you are in "debug mode", the Inputs and Dip Switches have special features:

  * pressing IPT_JOYSTICK_DOWN of player 2 freezes the game
  * pressing IPT_JOYSTICK_UP of player 2 unfreezes the game
  * pressing IPT_COIN1 gives invulnerability (the collision routine isn't called)
  * pressing IPT_COIN2 speeds up the game and you don't need to kill the bosses
  * when bit 2 is On, you are given invulnerability (same effect as IPT_COIN1)
  * when bit 3 is On, you don't need to kill the bosses (only the last one)
  * when bit 4 is On ("Lives" Dip Switch set to "5"), some coordonates are displayed
  * when bit 7 is On ("Coinage" Dip Switch set to "A 1/3C B 1/6C" or "A 2/1C B 3/1C"),
    a "band" is displayed at the left of the screen


Notes:
-----
- splndrbt hardware only appears to be capable of displaying 24 sprites.
  This time, they are consecutive in RAM.

- splndrbt2 is different in many areas, most notibly in the title screen and the
  operation of the missiles which is a power-up pickup rather than a cumulative
  collecting of missiles that can run out.

- see equites.cpp driver for more notes (this used to be in the same driver file)


TODO:
----
- splndrbt, hvoltage: the interpretation of the scaling PROMs might be wrong.
  The sprite x scaling is not used at all because I couldn't figure it out.
  Sprite y scaling is slightly wrong and leaves gaps in tall objects.
  Note that sprites are 30x30 instead of 32x32.

- The "road" background in splndrbt is slightly wrong. Apparently, the black lines
  visible in some parts of the background should never disappear in the distance.
  Currently, they may or may not disappear depending on the X position.

- dump the MCU ROM for hvoltage, though it currently works fine with the ROM from
  another chip.

*******************************************************************************/

#include "emu.h"

#include "ad_sound.h"
#include "alpha8201.h"

#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

namespace {

class splendor_state : public driver_device
{
public:
	splendor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_alpha_8201(*this, "alpha_8201"),
		m_mainlatch(*this, "mainlatch"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram", 0x800, ENDIANNESS_BIG),
		m_spriteram(*this, "spriteram%u", 1U),
		m_scale_rom(*this, "scale%u", 1U)
	{ }

	void init_splndrbt();
	void splndrbt(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<alpha_8201_device> m_alpha_8201;
	required_device<ls259_device> m_mainlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_bg_videoram;
	memory_share_creator<uint8_t> m_fg_videoram;
	required_shared_ptr_array<uint16_t, 2> m_spriteram;
	required_region_ptr_array<uint8_t, 2> m_scale_rom;

	uint8_t fg_videoram_r(offs_t offset);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgcolor_w(offs_t offset, uint8_t data);
	void selchar_w(int state);
	void bg_scrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(fg_info);
	TILE_GET_INFO_MEMBER(bg_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void copy_bg(bitmap_ind16 &dst_bitmap, const rectangle &cliprect);

	void splndrbt_map(address_map &map);

	void unpack_block(const char *region, int offset, int size);
	void unpack_region(const char *region);

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_bgcolor = 0U;
	int m_fg_char_bank = 0;
	uint16_t m_bg_scrollx = 0U;
	uint16_t m_bg_scrolly = 0U;
};


/******************************************************************************/
// Palette handling

void splendor_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
		palette.set_indirect_color(i, rgb_t(pal4bit(color_prom[i]), pal4bit(color_prom[i + 0x100]), pal4bit(color_prom[i + 0x200])));

	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	// point to the bg CLUT
	color_prom += 0x300;

	for (int i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i + 0x100, color_prom[i] + 0x10);

	// point to the sprite CLUT
	color_prom += 0x100;

	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i + 0x180, color_prom[i]);
}



/******************************************************************************/
// Callbacks for the tilemap code

TILE_GET_INFO_MEMBER(splendor_state::fg_info)
{
	int tile = m_fg_videoram[2 * tile_index] + (m_fg_char_bank << 8);
	int color = m_fg_videoram[2 * tile_index + 1] & 0x3f;

	tileinfo.set(0, tile, color, 0);
	if (color & 0x10)
		tileinfo.flags |= TILE_FORCE_LAYER0;
}

TILE_GET_INFO_MEMBER(splendor_state::bg_info)
{
	int data = m_bg_videoram[tile_index];
	int tile = data & 0x1ff;
	int color = (data & 0xf800) >> 11;
	int fxy = (data & 0x0600) >> 9;

	tileinfo.set(1, tile, color, TILE_FLIPXY(fxy));
	tileinfo.group = color;
}



/******************************************************************************/
// Video system start

void splendor_state::video_start()
{
	assert(m_screen->format() == BITMAP_FORMAT_IND16);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(splendor_state::fg_info)), TILEMAP_SCAN_COLS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldx(8, -8);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(splendor_state::bg_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(1), 0x10);
}



/******************************************************************************/
// Video update

void splendor_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*
	This is (probably) the sprite x scaling PROM.
	The layout is strange. Clearly every line is for one xscale setting. However,
	it seems that bytes 0-3 are handled separately from bytes 4-F.
	Also, note that sprites are 30x30, not 32x32.
	00020200 00000000 00000000 00000000
	00020200 01000000 00000000 00000002
	00020200 01000000 01000002 00000002
	00020200 01000100 01000002 00020002
	00020200 01000100 01010202 00020002
	02020201 01000100 01010202 00020002
	02020201 01010100 01010202 00020202
	02020201 01010101 01010202 02020202
	02020201 03010101 01010202 02020203
	02020201 03010103 01010202 03020203
	02020201 03010103 01030302 03020203
	02020201 03010303 01030302 03030203
	03020203 03010303 01030302 03030203
	03020203 03030303 01030302 03030303
	03020203 03030303 03030303 03030303
	03020303 03030303 03030303 03030303
	*/
	const uint8_t * const xrom = m_scale_rom[1];
	const uint8_t * const yrom = xrom + 0x100;

	gfx_element* gfx = m_gfxdecode->gfx(2);

	// note that sprites are actually 30x30, contained in 32x32 squares. The outer edge is not used.

	for (int offs = 0x3f; offs < 0x6f; offs += 2)   // 24 sprites
	{
		int data = m_spriteram[0][offs];
		int tile = data & 0x007f;
		int fx = (data & 0x2000) >> 13;
		int fy = (data & 0x1000) >> 12;
		int scaley = (data & 0x0f00) >> 8;

		int data2 = m_spriteram[0][offs + 1];
		int sx = data2 & 0x00ff;
		int color = (data2 & 0x1f00) >> 8;
		int transmask = m_palette->transpen_mask(*gfx, color, 0);

		int sy = m_spriteram[1][offs + 0] & 0x00ff;
		int scalex = m_spriteram[1][offs + 1] & 0x000f;

		//const uint8_t * const xromline = xrom + (scalex << 4);
		const uint8_t * const yromline = yrom + (scaley << 4) + (15 - scaley);
		const uint8_t* const srcgfx = gfx->get_data(tile);
		const pen_t *paldata = &m_palette->pen(gfx->colorbase() + gfx->granularity() * color);
		int x,yy;

		sy += 16;

		if (flip_screen())
		{
			// sx NOT inverted
			fx = fx ^ 1;
			fy = fy ^ 1;
		}
		else
		{
			sy = 256 - sy;
		}

		for (yy = 0; yy <= scaley; ++yy)
		{
			int const line = yromline[yy];
			int yhalf;

			for (yhalf = 0; yhalf < 2; ++yhalf) // top or bottom half
			{
				int const y = yhalf ? sy + 1 + yy : sy - yy;

				if (y >= cliprect.top() && y <= cliprect.bottom())
				{
					for (x = 0; x <= (scalex << 1); ++x)
					{
						int bx = (sx + x) & 0xff;

						if (bx >= cliprect.left() && bx <= cliprect.right())
						{
							int xx = scalex ? (x * 29 + scalex) / (scalex << 1) + 1 : 16; // FIXME This is wrong. Should use the PROM.
							int const offset = (fx ? (31 - xx) : xx) + ((fy ^ yhalf) ? (16 + line) : (15 - line)) * gfx->rowbytes();

							int pen = srcgfx[offset];

							if ((transmask & (1 << pen)) == 0)
								bitmap.pix(y, bx) = paldata[pen];
						}
					}
				}
			}
		}
	}
}


void splendor_state::copy_bg(bitmap_ind16 &dst_bitmap, const rectangle &cliprect)
{
	bitmap_ind16 &src_bitmap = m_bg_tilemap->pixmap();
	bitmap_ind8 &flags_bitmap = m_bg_tilemap->flagsmap();
	const uint8_t * const xrom = m_scale_rom[0];
	const uint8_t * const yrom = xrom + 0x2000;
	int scroll_x = m_bg_scrollx;
	int scroll_y = m_bg_scrolly;
	int const dinvert = flip_screen() ? 0xff : 0x00;
	int src_y = 0;
	int dst_y;

	if (flip_screen())
	{
		scroll_x = -scroll_x - 8;
		scroll_y = -scroll_y;
	}

	for (dst_y = 32; dst_y < 256-32; ++dst_y)
	{
		if (dst_y >= cliprect.top() && dst_y <= cliprect.bottom())
		{
			const uint8_t * const romline = &xrom[(dst_y ^ dinvert) << 5];
			const uint16_t * const src_line = &src_bitmap.pix((src_y + scroll_y) & 0x1ff);
			const uint8_t * const flags_line = &flags_bitmap.pix((src_y + scroll_y) & 0x1ff);
			uint16_t * const dst_line = &dst_bitmap.pix(dst_y);
			int dst_x = 0;
			int src_x;

			for (src_x = 0; src_x < 256 && dst_x < 128; ++src_x)
			{
				if ((romline[31 - (src_x >> 3)] >> (src_x & 7)) & 1)
				{
					int sx;

					sx = (256+128 + scroll_x + src_x) & 0x1ff;
					if (flags_line[sx] & TILEMAP_PIXEL_LAYER0)
						dst_line[128 + dst_x] = src_line[sx];

					sx = (255+128 + scroll_x - src_x) & 0x1ff;
					if (flags_line[sx] & TILEMAP_PIXEL_LAYER0)
						dst_line[127 - dst_x] = src_line[sx];

					++dst_x;
				}
			}
		}

		src_y += 1 + yrom[dst_y ^ dinvert];
	}
}

uint32_t splendor_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);

	copy_bg(bitmap, cliprect);

	if (m_fg_char_bank)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	if (!m_fg_char_bank)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/******************************************************************************/
// Interrupt Handlers

// Same as alpha/equites.cpp, including the almost empty irq 1 service observed with bullfgtr
TIMER_DEVICE_CALLBACK_MEMBER(splendor_state::scanline_cb)
{
	int scanline = param;

	// vblank-in irq
	if(scanline == 224)
		m_maincpu->set_input_line(2, HOLD_LINE);

	// vblank-out irq or sprite DMA end, applies bg scroll writes
	// and checks debug flag (if available) for screen freeze.
	if(scanline == 32)
		m_maincpu->set_input_line(1, HOLD_LINE);
}



/******************************************************************************/
// CPU Handlers

uint8_t splendor_state::fg_videoram_r(offs_t offset)
{
	// 8-bit
	return m_fg_videoram[offset];
}

void splendor_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void splendor_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_bg_videoram + offset);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void splendor_state::bgcolor_w(offs_t offset, uint8_t data)
{
	m_bgcolor = data;
}

void splendor_state::selchar_w(int state)
{
	// select active char map
	m_fg_char_bank = (state == 0) ? 0 : 1;
	m_fg_tilemap->mark_all_dirty();
}

void splendor_state::bg_scrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_scrollx);
}

void splendor_state::bg_scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_scrolly);
}



/******************************************************************************/
// CPU Memory Maps

void splendor_state::splndrbt_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x040fff).ram();
	map(0x080000, 0x080001).portr("IN0");
	map(0x0c0000, 0x0c0001).portr("IN1");
	map(0x0c0000, 0x0c0000).select(0x020000).w(FUNC(splendor_state::bgcolor_w));
	map(0x0c0001, 0x0c0001).select(0x03c000).lw8(NAME([this] (offs_t offset, u8 data) { m_mainlatch->write_a3(offset >> 14); }));
	map(0x100000, 0x100001).w(FUNC(splendor_state::bg_scrollx_w));
	map(0x140001, 0x140001).w("sound_board", FUNC(ad_59mc07_device::sound_command_w));
	map(0x1c0000, 0x1c0001).w(FUNC(splendor_state::bg_scrolly_w));
	map(0x180000, 0x1807ff).rw(m_alpha_8201, FUNC(alpha_8201_device::ext_ram_r), FUNC(alpha_8201_device::ext_ram_w)).umask16(0x00ff);
	map(0x200000, 0x200fff).mirror(0x001000).rw(FUNC(splendor_state::fg_videoram_r), FUNC(splendor_state::fg_videoram_w)).umask16(0x00ff);
	map(0x400000, 0x4007ff).ram().w(FUNC(splendor_state::bg_videoram_w)).share("bg_videoram");
	map(0x400800, 0x400fff).ram();
	map(0x600000, 0x6000ff).ram().share("spriteram1"); // sprite RAM 0,1 (2*8 bit)
	map(0x600100, 0x6001ff).ram().share("spriteram2"); // sprite RAM 2,none (8 bit)
}


/******************************************************************************/
// Port Maps

#define FRQ_ADJUSTER_TAG    "FRQ"

static INPUT_PORTS_START( splndrbt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1")
	PORT_DIPSETTING(      0xc000, "A 2C/1C B 3C/1C" )
	PORT_DIPSETTING(      0x0000, "A 1C/1C B 2C/1C" )
	PORT_DIPSETTING(      0x4000, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(      0x8000, "A 1C/3C B 1C/6C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(28, "MSM5232 Clock") // approximate factory setting
INPUT_PORTS_END


static INPUT_PORTS_START( hvoltage )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR ( Lives ) ) PORT_DIPLOCATION("SW:!4") // See notes
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR ( Bonus_Life ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, "50k, 100k then every 100k" )
	PORT_DIPSETTING(      0x2000, "50k, 200k then every 100k" )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1") // See notes
	PORT_DIPSETTING(      0xc000, "A 2C/1C B 3C/1C" )
	PORT_DIPSETTING(      0x0000, "A 1C/1C B 2C/1C" )
	PORT_DIPSETTING(      0x4000, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(      0x8000, "A 1C/3C B 1C/6C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(27, "MSM5232 Clock") // approximate factory setting
INPUT_PORTS_END



/******************************************************************************/
// Graphics Layouts

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP8(0*8,8) },
	16*8
};

static const gfx_layout tilelayout_2bpp =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP16(0*8,8) },
	64*8
};

static const gfx_layout spritelayout_32x32 =
{
	32,32,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(0*8+3,-1), STEP4(1*8+3,-1), STEP4(2*8+3,-1), STEP4(3*8+3,-1), STEP4(4*8+3,-1), STEP4(5*8+3,-1), STEP4(6*8+3,-1), STEP4(7*8+3,-1) },
	{ STEP16(0*8*8,8*8), STEP16(31*8*8,-8*8) },
	256*8
};


static GFXDECODE_START( gfx_splndrbt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0x000, 0x100/4 ) // chars
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_2bpp,    0x100, 0x080/4 ) // tiles
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_32x32, 0x180, 0x100/8 ) // sprites
GFXDECODE_END



/******************************************************************************/

void splendor_state::machine_start()
{
	save_item(NAME(m_bgcolor));
	save_item(NAME(m_fg_char_bank));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
}

void splendor_state::splndrbt(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/4); // 68000P8 running at 6mhz, verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &splendor_state::splndrbt_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(splendor_state::scanline_cb), "screen", 0, 1);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(splendor_state::flip_screen_set));
	m_mainlatch->q_out_cb<1>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w));
	m_mainlatch->q_out_cb<2>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w)).invert();
	m_mainlatch->q_out_cb<3>().set(FUNC(splendor_state::selchar_w));

	AD_59MC07(config, "sound_board");

	ALPHA_8201(config, m_alpha_8201, 4000000/8); // 8303 or 8304 (same device!)
	config.set_perfect_quantum("alpha_8201:mcu");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 4*8, 28*8-1);
	m_screen->set_screen_update(FUNC(splendor_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_splndrbt);
	PALETTE(config, m_palette, FUNC(splendor_state::palette), 0x280, 0x100);
}



/******************************************************************************/
// Splendor Blast ROM Map

/*
Splendor Blast (JPN Ver.)
(c)1985 Alpha denshi

ALPHA 68K24
CPU  :HD68000-8
OSC  :24.000MHz
Other:ALPHA-8303

SOUND BOARD NO.59 MC 07
CPU  :TMP8085AP
Sound:AY-3-8910A,OKI M5232,M5L8155P
OSC  :6.144MHz
*/
ROM_START( splndrbt )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4)
	ROM_LOAD16_BYTE( "1.16a", 0x00001, 0x4000, CRC(4bf4b047) SHA1(ef0efffa2f49905e17e4ed3a03cac419793b26d1) )
	ROM_LOAD16_BYTE( "2.16c", 0x00000, 0x4000, CRC(27acb656) SHA1(5f2f8d05f2f1c6c92c8364e9e6831ca525cbacd0) )
	ROM_LOAD16_BYTE( "3.15a", 0x08001, 0x4000, CRC(5b182189) SHA1(50ebb1fddcb6838442e8a20261f200f3386ce8a8) )
	ROM_LOAD16_BYTE( "4.15c", 0x08000, 0x4000, CRC(cde99613) SHA1(250b59f75eee84442da3cc7c599d1e16f0294df9) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "1_v.1m", 0x00000, 0x2000, CRC(1b3a6e42) SHA1(41a4f0503c939ec0a739c8bc6bf3c8fc354912ee) )
	ROM_LOAD( "2_v.1l", 0x02000, 0x2000, CRC(2a618c72) SHA1(6ad459d94352c317150ae6344d4db9bb613938dd) )
	ROM_LOAD( "3_v.1k", 0x04000, 0x2000, CRC(bbee5346) SHA1(753cb784b04f081fa1f8590dc28056d9918f313b) )
	ROM_LOAD( "4_v.1h", 0x06000, 0x2000, CRC(10f45af4) SHA1(00fa599bad8bf3ba6deee54165f381403096e8f9) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "10.8c",  0x00000, 0x2000, CRC(501887d4) SHA1(3cf4401d6fddff1500066219a71ac3b30ecbdd28) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "8.14m",  0x00000, 0x4000, CRC(c2c86621) SHA1(a715c70ace98502f2c0d4a81539cd79d19e9b6c4) )
	ROM_LOAD( "9.12m",  0x04000, 0x4000, CRC(4f7da6ff) SHA1(0516271df4a36d6ea38d1b8a5e471e1d2a79e8c1) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // sprites
	ROM_LOAD( "6.18n", 0x00000, 0x2000, CRC(aa72237f) SHA1(0a26746a6c448a7fb853ef708e2bdeb76edd99cf) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(      0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "5.18m", 0x08000, 0x4000, CRC(5f618b39) SHA1(2891067e71b8e1183ee5741487faa1561316cade) )
	ROM_LOAD( "7.17m", 0x0c000, 0x4000, CRC(abdd8483) SHA1(df8c8338c24fa487c49b01ce26db7eb28c8c6b85) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(ca1f08ce) SHA1(e46e2850d3ee3c8cbb23c10645f07d406c7ff50b) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(66f89177) SHA1(caa51c1bf071764d5089487342794cbf023136c0) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(d14318bc) SHA1(e219963b3e40eb246e608fbe10daa85dbb4c1226) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(e1770ad3) SHA1(e408b175b8fff934e07b0ded1ee21d7f91a9523d) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(7f6cf709) SHA1(5938faf937b682dcc83e53444cbf5e0bd7741363) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "scale1", 0 ) // bg scaling
	ROM_LOAD( "0.8h",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "scale2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "s3.8l",  0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END

ROM_START( splndrbta )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4) - Red band on program rom labels
	ROM_LOAD16_BYTE( "1red.16b", 0x00001, 0x4000, CRC(3e342030) SHA1(82529e12378c0036097a654fe059f82d69fac8e6) )
	ROM_LOAD16_BYTE( "2red.16c", 0x00000, 0x4000, CRC(757e270b) SHA1(be615829fd21609ded21888e7a75456cbeecb603) )
	ROM_LOAD16_BYTE( "3red.15b", 0x08001, 0x4000, CRC(788deb02) SHA1(a4e79621bf4cda50dfb8dfab7f70dc4021065794) )
	ROM_LOAD16_BYTE( "4red.15c", 0x08000, 0x4000, CRC(d02a5606) SHA1(6bb2e5d95ea711452dd40218bd90488d70f82006) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "8v.1l",  0x00000, 0x4000, CRC(71b2ec29) SHA1(89c630c5bf9c4752b01006183d1419fe6a458f5c) )
	ROM_LOAD( "9v.1h",  0x04000, 0x4000, CRC(e95abcb5) SHA1(1680875fc16d1a4e1054ccdabdf6fd06d434a163) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "10.8c",  0x00000, 0x2000, CRC(501887d4) SHA1(3cf4401d6fddff1500066219a71ac3b30ecbdd28) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "8.14m",  0x00000, 0x4000, CRC(c2c86621) SHA1(a715c70ace98502f2c0d4a81539cd79d19e9b6c4) )
	ROM_LOAD( "9.12m",  0x04000, 0x4000, CRC(4f7da6ff) SHA1(0516271df4a36d6ea38d1b8a5e471e1d2a79e8c1) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // sprites
	ROM_LOAD( "6.18n", 0x00000, 0x2000, CRC(aa72237f) SHA1(0a26746a6c448a7fb853ef708e2bdeb76edd99cf) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(      0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "5.18m", 0x08000, 0x4000, CRC(5f618b39) SHA1(2891067e71b8e1183ee5741487faa1561316cade) )
	ROM_LOAD( "7.17m", 0x0c000, 0x4000, CRC(abdd8483) SHA1(df8c8338c24fa487c49b01ce26db7eb28c8c6b85) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(ca1f08ce) SHA1(e46e2850d3ee3c8cbb23c10645f07d406c7ff50b) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(66f89177) SHA1(caa51c1bf071764d5089487342794cbf023136c0) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(d14318bc) SHA1(e219963b3e40eb246e608fbe10daa85dbb4c1226) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(e1770ad3) SHA1(e408b175b8fff934e07b0ded1ee21d7f91a9523d) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(7f6cf709) SHA1(5938faf937b682dcc83e53444cbf5e0bd7741363) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "scale1", 0 ) // bg scaling
	ROM_LOAD( "0.8h",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "scale2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "s3.8l",  0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END

ROM_START( splndrbtb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4) - Blue band on program rom labels
	ROM_LOAD16_BYTE( "1blue.16a", 0x00001, 0x4000, CRC(f8507502) SHA1(35a915db9ef90e45aac8ce9e349c319e99a36810) )
	ROM_LOAD16_BYTE( "2blue.16c", 0x00000, 0x4000, CRC(8969bd04) SHA1(6cd8a0ab58ce0e4a43cf5ca4fcd10b30962a13b3) )
	ROM_LOAD16_BYTE( "3blue.15a", 0x08001, 0x4000, CRC(bce26d4f) SHA1(81a295e665af9e46ff28618f2f77f31f41f14a4f) )
	ROM_LOAD16_BYTE( "4blue.15c", 0x08000, 0x4000, CRC(5715ec1b) SHA1(fddf45a4e1b2fd319b0a47376c11ce2a41c40eb2) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "1_v.1m", 0x00000, 0x2000, CRC(1b3a6e42) SHA1(41a4f0503c939ec0a739c8bc6bf3c8fc354912ee) )
	ROM_LOAD( "2_v.1l", 0x02000, 0x2000, CRC(2a618c72) SHA1(6ad459d94352c317150ae6344d4db9bb613938dd) )
	ROM_LOAD( "3_v.1k", 0x04000, 0x2000, CRC(bbee5346) SHA1(753cb784b04f081fa1f8590dc28056d9918f313b) )
	ROM_LOAD( "4_v.1h", 0x06000, 0x2000, CRC(10f45af4) SHA1(00fa599bad8bf3ba6deee54165f381403096e8f9) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "10.8c",  0x00000, 0x2000, CRC(501887d4) SHA1(3cf4401d6fddff1500066219a71ac3b30ecbdd28) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "8.14m",  0x00000, 0x4000, CRC(c2c86621) SHA1(a715c70ace98502f2c0d4a81539cd79d19e9b6c4) )
	ROM_LOAD( "9.12m",  0x04000, 0x4000, CRC(4f7da6ff) SHA1(0516271df4a36d6ea38d1b8a5e471e1d2a79e8c1) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // sprites
	ROM_LOAD( "6.18n", 0x00000, 0x2000, CRC(aa72237f) SHA1(0a26746a6c448a7fb853ef708e2bdeb76edd99cf) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(      0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "5.18m", 0x08000, 0x4000, CRC(5f618b39) SHA1(2891067e71b8e1183ee5741487faa1561316cade) )
	ROM_LOAD( "7.17m", 0x0c000, 0x4000, CRC(abdd8483) SHA1(df8c8338c24fa487c49b01ce26db7eb28c8c6b85) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(ca1f08ce) SHA1(e46e2850d3ee3c8cbb23c10645f07d406c7ff50b) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(66f89177) SHA1(caa51c1bf071764d5089487342794cbf023136c0) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(d14318bc) SHA1(e219963b3e40eb246e608fbe10daa85dbb4c1226) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(e1770ad3) SHA1(e408b175b8fff934e07b0ded1ee21d7f91a9523d) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(7f6cf709) SHA1(5938faf937b682dcc83e53444cbf5e0bd7741363) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "scale1", 0 ) // bg scaling
	ROM_LOAD( "0.8h",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "scale2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "s3.8l",  0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END

ROM_START( splndrbt2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4)
	ROM_LOAD16_BYTE( "1.a16", 0x00001, 0x4000, CRC(0fd3121d) SHA1(f9767af477442a09a70c04e4d427914557fddcd9) )
	ROM_LOAD16_BYTE( "2.c16", 0x00000, 0x4000, CRC(227d8a1b) SHA1(8ce976e6d3dce1236a784e48f4829f42c801249c) )
	ROM_LOAD16_BYTE( "3.a15", 0x08001, 0x4000, CRC(936f7cc9) SHA1(ef1601097659700f4a4b53fb57cd6d73efa03e0d) )
	ROM_LOAD16_BYTE( "4.c15", 0x08000, 0x4000, CRC(3ff7c7b5) SHA1(4997efd4427f09a5427f752d0147b648fbdce252) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "s1.m1",  0x00000, 0x02000, CRC(045eac1b) SHA1(49ecc73b999719e470b2ef0afee6a84df620e0d9) )
	ROM_LOAD( "s2.l1",  0x02000, 0x02000, CRC(65a3d094) SHA1(f6415eb323478a2d38acd4507404d9530fac77c4) )
	ROM_LOAD( "s3.k1",  0x04000, 0x02000, CRC(980d38be) SHA1(c07f9851cfb6352781568f333d931b4ca08fd888) )
	ROM_LOAD( "s4.h1",  0x06000, 0x02000, CRC(10f45af4) SHA1(00fa599bad8bf3ba6deee54165f381403096e8f9) )
	ROM_LOAD( "s5.f1",  0x08000, 0x02000, CRC(0d76cac0) SHA1(15d0d5860035f06020589115b40d347c06d7ecbe) )
	ROM_LOAD( "s6.e1",  0x0a000, 0x02000, CRC(bc65d469) SHA1(45145974d3ae7040fd00c776418702166c06b0dc) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "5.b8",   0x00000, 0x02000, CRC(77a5dc55) SHA1(49f19e8816629b661c135b0db6f6e087eb2690ff) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "8.m13",  0x00000, 0x4000, CRC(c2c86621) SHA1(a715c70ace98502f2c0d4a81539cd79d19e9b6c4) )
	ROM_LOAD( "9.m12",  0x04000, 0x4000, CRC(4f7da6ff) SHA1(0516271df4a36d6ea38d1b8a5e471e1d2a79e8c1) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // sprites
	ROM_LOAD( "8.n18",  0x00000, 0x4000, CRC(15b8277b) SHA1(36d80e9c1200f587cafdf43fafafe844d56296aa) )
	// empty space to unpack previous ROM
	// ROM_CONTINUE(       0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "5.m18",  0x08000, 0x4000, CRC(5f618b39) SHA1(2891067e71b8e1183ee5741487faa1561316cade) )
	ROM_LOAD( "7.m17",  0x0c000, 0x4000, CRC(abdd8483) SHA1(df8c8338c24fa487c49b01ce26db7eb28c8c6b85) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(ca1f08ce) SHA1(e46e2850d3ee3c8cbb23c10645f07d406c7ff50b) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(66f89177) SHA1(caa51c1bf071764d5089487342794cbf023136c0) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(d14318bc) SHA1(e219963b3e40eb246e608fbe10daa85dbb4c1226) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(e1770ad3) SHA1(e408b175b8fff934e07b0ded1ee21d7f91a9523d) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(7f6cf709) SHA1(5938faf937b682dcc83e53444cbf5e0bd7741363) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "scale1", 0 ) // bg scaling
	ROM_LOAD( "0.h7",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "scale2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "s3.8l",  0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END

/******************************************************************************/
// High Voltage ROM Map

/*
High Voltage (JPN Ver.)
(c)1985 Alpha denshi

ALPHA 68K24
CPU  :HD68000-8
OSC  :24.000MHz
Other:ALPHA-8304

SOUND BOARD NO.59 MC 07
CPU  :TMP8085AP
Sound:AY-3-8910A,OKI M5232,D8155HC
OSC  :6.144MHz
*/
ROM_START( hvoltage )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4)
	ROM_LOAD16_BYTE( "1.16a", 0x00001, 0x4000, CRC(82606e3b) SHA1(25c3172928d8f1eda2c4c757d505fdfd91f21ea1) )
	ROM_LOAD16_BYTE( "2.16c", 0x00000, 0x4000, CRC(1d74fef2) SHA1(3df3dc98a78a137da8c5cddf6a8519b477824fb9) )
	ROM_LOAD16_BYTE( "3.15a", 0x08001, 0x4000, CRC(677abe14) SHA1(78b343122f9ad187c823bf49e8f001288c762586) )
	ROM_LOAD16_BYTE( "4.15c", 0x08000, 0x4000, CRC(8aab5a20) SHA1(fb90817173ad69c0e00d03814b4e10b18955c07e) )

	ROM_REGION( 0x10000, "sound_board:audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "5_v.1l", 0x00000, 0x4000, CRC(ed9bb6ea) SHA1(73b0251b86835368ec2a4e98a5f61e28e58fd234) )
	ROM_LOAD( "6_v.1h", 0x04000, 0x4000, CRC(e9542211) SHA1(482f2c90e842fe5cc31cc6a39025adf65ba47ce9) )
	ROM_LOAD( "7_v.1e", 0x08000, 0x4000, CRC(44d38554) SHA1(6765971376eafa218fda1accb1e173a7c1850cc8) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8505_44801c57.bin", 0x0000, 0x2000, BAD_DUMP CRC(1f5a1405) SHA1(23f2e23db402f88037a5cbdab2935ec1b9a05298) ) // 8304 is not dumped yet, using 8505 instead, works ok

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "5.8c",   0x00000, 0x2000, CRC(656d53cd) SHA1(9971ed7e7da0e8bf46e97e8f75a2c2201b33fc2f) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "9.14m",  0x00000, 0x4000, CRC(506a0989) SHA1(0e7f2c9bab5e83f06a8148f69d8d0cbfe7d55c5e) )
	ROM_LOAD( "10.12m", 0x04000, 0x4000, CRC(98f87d4f) SHA1(94a7a14b0905597993595b347102436d97fc1dc9) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "8.18n", 0x00000, 0x2000, CRC(725acae5) SHA1(ba54598a087f8bb5fa7182b0e85d0e038003e622) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(      0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "6.18m", 0x08000, 0x4000, CRC(9baf2c68) SHA1(208e5ac8eb157d4bf949ab4330827da032a04235) )
	ROM_LOAD( "7.17m", 0x0c000, 0x4000, CRC(12d25fb1) SHA1(99f5d68bd6d6ee5f2acb7685aceacfb0894c4961) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(98eccbf6) SHA1(a55755e8388d3edf3020b1129a638fe1e99362b6) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(fab2ed23) SHA1(6f63b6a3196dda76eb9a885b17d886a14365f922) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(7274961b) SHA1(d13070060e216d633675a528cf0dc3de94c95ffb) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(685f4e44) SHA1(110cb8f5a37f22ce9d391bd0cd46dcbb8fcf66b8) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(b09bcc73) SHA1(f8139feaa9563324b69aeac5c17beccfdbfa0864) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "scale1", 0 ) // bg scaling
	ROM_LOAD( "0.8h",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "scale2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "3.8l",   0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END



/******************************************************************************/
// Initializations

void splendor_state::unpack_block(const char *region, int offset, int size)
{
	uint8_t *rom = memregion(region)->base();

	for (int i = 0; i < size; i++)
	{
		rom[(offset + i + size)] = (rom[(offset + i)] >> 4);
		rom[(offset + i)] &= 0x0f;
	}
}

void splendor_state::unpack_region(const char *region)
{
	unpack_block(region, 0x0000, 0x2000);
	unpack_block(region, 0x4000, 0x2000);
}

void splendor_state::init_splndrbt()
{
	unpack_region("gfx3");
}

} // anonymous namespace



/******************************************************************************/
// Game Entries

GAME( 1985, splndrbt,  0,        splndrbt, splndrbt, splendor_state, init_splndrbt, ROT0,  "Alpha Denshi Co.", "Splendor Blast (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, splndrbta, splndrbt, splndrbt, splndrbt, splendor_state, init_splndrbt, ROT0,  "Alpha Denshi Co.", "Splendor Blast (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, splndrbtb, splndrbt, splndrbt, splndrbt, splendor_state, init_splndrbt, ROT0,  "Alpha Denshi Co.", "Splendor Blast (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, splndrbt2, 0,        splndrbt, splndrbt, splendor_state, init_splndrbt, ROT0,  "Alpha Denshi Co.", "Splendor Blast II", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, hvoltage,  0,        splndrbt, hvoltage, splendor_state, init_splndrbt, ROT0,  "Alpha Denshi Co.", "High Voltage", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
