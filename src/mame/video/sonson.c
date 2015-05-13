// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/sonson.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Son Son has two 32x8 palette PROMs and two 256x4 lookup table PROMs (one
  for characters, one for sprites).
  The palette PROMs are connected to the RGB output this way:

  I don't know the exact values of the resistors between the PROMs and the
  RGB output. I assumed these values (the same as Commando)
  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

  bit 7 -- unused
        -- unused
        -- unused
        -- unused
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(sonson_state, sonson)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x20] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x20] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x20] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x20] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + 0x00] >> 4) & 0x01;
		bit1 = (color_prom[i + 0x00] >> 5) & 0x01;
		bit2 = (color_prom[i + 0x00] >> 6) & 0x01;
		bit3 = (color_prom[i + 0x00] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 0x00] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x00] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x00] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x00] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	/* characters use colors 0-0x0f */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites use colors 0x10-0x1f */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

WRITE8_MEMBER(sonson_state::sonson_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sonson_state::sonson_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sonson_state::sonson_scrollx_w)
{
	int row;

	for (row = 5; row < 32; row++)
		m_bg_tilemap->set_scrollx(row, data);
}

WRITE8_MEMBER(sonson_state::sonson_flipscreen_w)
{
	flip_screen_set(~data & 0x01);
}

TILE_GET_INFO_MEMBER(sonson_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + 256 * (attr & 0x03);
	int color = attr >> 2;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void sonson_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sonson_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_rows(32);
}

void sonson_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs + 2] + ((spriteram[offs + 1] & 0x20) << 3);
		int color = spriteram[offs + 1] & 0x1f;
		int flipx = ~spriteram[offs + 1] & 0x40;
		int flipy = ~spriteram[offs + 1] & 0x80;
		int sx = spriteram[offs + 3];
		int sy = spriteram[offs + 0];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* wrap-around */
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, color, flipx, flipy, sx - 256, sy, 0);
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, color, flipx, flipy, sx, sy - 256, 0);
	}
}

UINT32 sonson_state::screen_update_sonson(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
