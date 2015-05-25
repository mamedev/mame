// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/****************************************************************************

    Irem M57 hardware

****************************************************************************/

#include "emu.h"
#include "includes/m57.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Tropical Angel has two 256x4 character palette PROMs, one 32x8 sprite
  palette PROM, and one 256x4 sprite color lookup table PROM.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably something like this; note that RED and BLUE
  are swapped wrt the usual configuration.

  bit 7 -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(m57_state, m57)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* character palette */
	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r,g,b));
		palette.set_pen_indirect(i, i);
		color_prom++;
	}

	color_prom += 256;
	/* color_prom now points to the beginning of the sprite palette */

	/* sprite palette */
	for (i = 0; i < 16; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i + 256, rgb_t(r,g,b));
		color_prom++;
	}

	color_prom += 16;
	/* color_prom now points to the beginning of the sprite lookup table */


	/* sprite lookup table */
	for (i = 0; i < 32 * 8; i++)
	{
		palette.set_pen_indirect(i + 32 * 8, 256 + (~*color_prom & 0x0f));
		color_prom++;
	}
}


/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m57_state::get_tile_info)
{
	UINT8 attr = m_videoram[tile_index * 2 + 0];
	UINT16 code = m_videoram[tile_index * 2 + 1] | ((attr & 0xc0) << 2);

	SET_TILE_INFO_MEMBER(0, code, attr & 0x0f, TILE_FLIPXY(attr >> 4));
}


/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE8_MEMBER(m57_state::m57_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


/*************************************
 *
 *  Video startup
 *
 *************************************/

void m57_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m57_state::get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap->set_scroll_rows(256);

	save_item(NAME(m_flipscreen));
}


/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_MEMBER(m57_state::m57_flipscreen_w)
{
	/* screen flip is handled both by software and hardware */
	m_flipscreen = (data & 0x01) ^ (~ioport("DSW2")->read() & 0x01);
	m_bg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	coin_counter_w(machine(), 0,data & 0x02);
	coin_counter_w(machine(), 1,data & 0x20);
}


/*************************************
 *
 *  Background rendering
 *
 *************************************/

void m57_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x;
	INT16 scrolly;

	// from 64 to 127: not wrapped
	for (y = 64; y < 128; y++)
		m_bg_tilemap->set_scrollx(y, m_scrollram[0x40]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// from 128 to 255: wrapped
	for (y = 128; y <= cliprect.max_y; y++)
	{
		scrolly = m_scrollram[y] + (m_scrollram[y + 0x100] << 8);

		if (scrolly >= 0)
		{
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				if ((x + scrolly) <= cliprect.max_x)
					bitmap.pix16(y, x) = bitmap.pix16(y, x + scrolly);
				else
					bitmap.pix16(y, x) = bitmap.pix16(y, cliprect.max_x);
			}
		} else {
			for (x = cliprect.max_x; x >= cliprect.min_x; x--)
			{
				if ((x + scrolly) >= cliprect.min_x)
					bitmap.pix16(y, x) = bitmap.pix16(y, x + scrolly);
				else
					bitmap.pix16(y, x) = bitmap.pix16(y, cliprect.min_x);
			}
		}
	}
}

/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void m57_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		UINT8 attributes = m_spriteram[offs + 1];
		int sx = m_spriteram[offs + 3];
		int sy = ((224 - m_spriteram[offs + 0] - 32) & 0xff) + 32;
		int code = m_spriteram[offs + 2];
		int color = attributes & 0x1f;
		int flipy = attributes & 0x80;
		int flipx = attributes & 0x40;

		int tile_number = code & 0x3f;

		int bank = 0;
		if (code & 0x80) bank += 1;
		if (attributes & 0x20) bank += 2;

		if (m_flipscreen)
		{
			sx = 240 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1 + bank)->transmask(bitmap,cliprect,
			tile_number,
			color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 256 + 15));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 m57_state::screen_update_m57(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}
