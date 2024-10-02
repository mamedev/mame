// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "galaga.h"

#define LOG_DEBUG           (1U << 1)
#define LOG_ALL             (LOG_DEBUG)

#define VERBOSE             (LOG_ALL)

#include "logmacro.h"



/***************************************************************************

  Convert the color PROMs.

  Galaga has one 32x8 palette PROM and two 256x4 color lookup table PROMs
  (one for characters, one for sprites). Only the first 128 bytes of the
  lookup tables seem to be used.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void galaga_state::galaga_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// core palette
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
		color_prom++;
	}

	// palette for the stars
	for (int i = 0; i < 64; i++)
	{
		static constexpr int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		int const r = map[(i >> 0) & 0x03];
		int const g = map[(i >> 2) & 0x03];
		int const b = map[(i >> 4) & 0x03];

		palette.set_indirect_color(32 + i, rgb_t(r, g, b));
	}

	// characters
	for (int i = 0; i < 64*4; i++)
		palette.set_pen_indirect(i, (*color_prom++ & 0x0f) | 0x10);

	// sprites
	for (int i = 0; i < 64*4; i++)
		palette.set_pen_indirect(64*4 + i, *color_prom++ & 0x0f);

	// now the stars
	for (int i = 0; i < 64; i++)
		palette.set_pen_indirect(64*4 + 64*4 + i, 32 + i);
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(galaga_state::tilemap_scan)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}


TILE_GET_INFO_MEMBER(galaga_state::get_tile_info)
{
	/* the hardware has two character sets, one normal and one x-flipped. When
	   screen is flipped, character y flip is done by the hardware inverting the
	   timing signals, while x flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	int color = m_videoram[tile_index + 0x400] & 0x3f;
	tileinfo.set(0,
			(m_videoram[tile_index] & 0x7f) | (flip_screen() ? 0x80 : 0) | (m_galaga_gfxbank << 8),
			color,
			flip_screen() ? TILE_FLIPX : 0);
	tileinfo.group = color;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void galaga_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaga_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(galaga_state::tilemap_scan)), 8,8,36,28);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);

	m_galaga_gfxbank = 0;

	save_item(NAME(m_galaga_gfxbank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/


void galaga_state::galaga_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void galaga_state::gatsbee_bank_w(int state)
{
	m_galaga_gfxbank = state;
	m_fg_tilemap->mark_all_dirty();
}



/***************************************************************************

  Display refresh

***************************************************************************/

void galaga_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = &m_galaga_ram1[0x380];
	uint8_t *spriteram_2 = &m_galaga_ram2[0x380];
	uint8_t *spriteram_3 = &m_galaga_ram3[0x380];

	for (int offs = 0; offs < 0x80; offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		const int sprite = spriteram[offs] & 0x7f;
		const int color = spriteram[offs + 1] & 0x3f;
		int sx = spriteram_2[offs + 1] - 40 + 0x100*(spriteram_3[offs + 1] & 3);
		int sy = 256 - spriteram_2[offs] + 1;   // sprites are buffered and delayed by one scanline
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		const int sizex = (spriteram_3[offs] & 0x04) >> 2;
		const int sizey = (spriteram_3[offs] & 0x08) >> 3;

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;  // fix wraparound

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		for (int y = 0; y <= sizey; y++)
		{
			for (int x = 0; x <= sizex; x++)
			{
				m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x, sy + 16*y,
					m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x0f));
			}
		}
	}
}



uint32_t galaga_state::screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_starfield->draw_starfield(bitmap,cliprect, 0);
	draw_sprites(bitmap,cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect);
	return 0;
}



void galaga_state::screen_vblank_galaga(int state)
{
	// falling edge
	if (!state)
	{
		// Galaga only scrolls in X direction - the SCROLL_Y pins
		// of the 05XX chip are tied to ground.
		const uint8_t speed_index_X = (m_videolatch->q2_r()<<2) | (m_videolatch->q1_r()<<1) | (m_videolatch->q0_r()<<0);
		const uint8_t speed_index_Y = 0;
		m_starfield->set_scroll_speed(speed_index_X,speed_index_Y);

		m_starfield->set_active_starfield_sets(m_videolatch->q3_r(), m_videolatch->q4_r() | 2);

		// _STARCLR signal enables/disables starfield
		m_starfield->enable_starfield(m_videolatch->q5_r());
	}
}
