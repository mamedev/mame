// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/espial.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Espial has two 256x4 palette PROMs.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
  bit 0 -- 470 ohm resistor  -- GREEN
  bit 3 -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(espial_state, espial)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i + palette.entries()] >> 0) & 0x01;
		bit2 = (color_prom[i + palette.entries()] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + palette.entries()] >> 2) & 0x01;
		bit2 = (color_prom[i + palette.entries()] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(espial_state::get_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	UINT8 col = m_colorram[tile_index];
	UINT8 attr = m_attributeram[tile_index];
	SET_TILE_INFO_MEMBER(0,
					code | ((attr & 0x03) << 8),
					col & 0x3f,
					TILE_FLIPYX(attr >> 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void espial_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(espial_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_flipscreen));
}

VIDEO_START_MEMBER(espial_state,netwars)
{
	/* Net Wars has a tile map that's twice as big as Espial's */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(espial_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 64);

	m_bg_tilemap->set_scroll_cols(32);
	m_bg_tilemap->set_scrolldy(0, 0x100);

	save_item(NAME(m_flipscreen));
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(espial_state::espial_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(espial_state::espial_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(espial_state::espial_attributeram_w)
{
	m_attributeram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(espial_state::espial_scrollram_w)
{
	m_scrollram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}


WRITE8_MEMBER(espial_state::espial_flipscreen_w)
{
	m_flipscreen = data;
	m_bg_tilemap->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

void espial_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	/* Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = 0; offs < 16; offs++)
	{
		int sx, sy, code, color, flipx, flipy;


		sx = m_spriteram_1[offs + 16];
		sy = m_spriteram_2[offs];
		code = m_spriteram_1[offs] >> 1;
		color = m_spriteram_2[offs + 16];
		flipx = m_spriteram_3[offs] & 0x04;
		flipy = m_spriteram_3[offs] & 0x08;

		if (m_flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}

		if (m_spriteram_1[offs] & 1) /* double height */
		{
			if (m_flipscreen)
			{
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code,color,
						flipx,flipy,
						sx,sy + 16,0);
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code + 1,
						color,
						flipx,flipy,
						sx,sy,0);
			}
			else
			{
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code,color,
						flipx,flipy,
						sx,sy - 16,0);
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code + 1,color,
						flipx,flipy,
						sx,sy,0);
			}
		}
		else
		{
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,color,
					flipx,flipy,
					sx,sy,0);
		}
	}
}


UINT32 espial_state::screen_update_espial(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
