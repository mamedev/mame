// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  Mr. Do's Castle hardware

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/docastle.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mr. Do's Castle / Wild Ride / Run Run have a 256 bytes palette PROM which
  is connected to the RGB output this way:

  bit 7 -- 200 ohm resistor  -- RED
        -- 390 ohm resistor  -- RED
        -- 820 ohm resistor  -- RED
        -- 200 ohm resistor  -- GREEN
        -- 390 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- GREEN
        -- 200 ohm resistor  -- BLUE
  bit 0 -- 390 ohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(docastle_state, docastle)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 5) & 0x01;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = 0x23 * bit0 + 0x4b * bit1 + 0x91 * bit2;
		/* green component */
		bit0 = (*color_prom >> 2) & 0x01;
		bit1 = (*color_prom >> 3) & 0x01;
		bit2 = (*color_prom >> 4) & 0x01;
		g = 0x23 * bit0 + 0x4b * bit1 + 0x91 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 0) & 0x01;
		bit2 = (*color_prom >> 1) & 0x01;
		b = 0x23 * bit0 + 0x4b * bit1 + 0x91 * bit2;

		/* because the graphics are decoded as 4bpp with the top bit used for transparency
		   or priority, we create matching 3bpp sets of palette entries, which effectively
		   ignores the value of the top bit */
		palette.set_pen_color(((i & 0xf8) << 1) | 0x00 | (i & 0x07), rgb_t(r,g,b));
		palette.set_pen_color(((i & 0xf8) << 1) | 0x08 | (i & 0x07), rgb_t(r,g,b));
		color_prom++;
	}
}

WRITE8_MEMBER(docastle_state::docastle_videoram_w)
{
	m_videoram[offset] = data;
	m_do_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(docastle_state::docastle_colorram_w)
{
	m_colorram[offset] = data;
	m_do_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(docastle_state::flipscreen_r)
{
	flip_screen_set(offset);
	return (offset ? 1 : 0); // is this really needed?
}

WRITE8_MEMBER(docastle_state::flipscreen_w)
{
	flip_screen_set(offset);
}

TILE_GET_INFO_MEMBER(docastle_state::get_tile_info)
{
	int code = m_videoram[tile_index] + 8 * (m_colorram[tile_index] & 0x20);
	int color = m_colorram[tile_index] & 0x1f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void docastle_state::video_start_common( UINT32 tile_transmask )
{
	m_do_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(docastle_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_do_tilemap->set_scrolldy(-32, -32);
	m_do_tilemap->set_transmask(0, tile_transmask, 0x0000);
}

void docastle_state::video_start()
{
	video_start_common(0x00ff);
}

VIDEO_START_MEMBER(docastle_state,dorunrun)
{
	video_start_common(0xff00);
}

void docastle_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	screen.priority().fill(1);

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, flipx, flipy, code, color;

		if (m_gfxdecode->gfx(1)->elements() > 256)
		{
			/* spriteram

			 indoor soccer appears to have a slightly different spriteram
			 format to the other games, allowing a larger number of sprite
			 tiles

			 yyyy yyyy  xxxx xxxx  TX-T pppp  tttt tttt

			 y = ypos
			 x = xpos
			 X = x-flip
			 T = extra tile number bits
			 p = palette
			 t = tile number

			*/

			code = m_spriteram[offs + 3];
			color = m_spriteram[offs + 2] & 0x0f;
			sx = ((m_spriteram[offs + 1] + 8) & 0xff) - 8;
			sy = m_spriteram[offs] - 32;
			flipx = m_spriteram[offs + 2] & 0x40;
			flipy = 0;
			if (m_spriteram[offs + 2] & 0x10) code += 0x100;
			if (m_spriteram[offs + 2] & 0x80) code += 0x200;
		}
		else
		{
			/* spriteram

			this is the standard spriteram layout, used by most games

			 yyyy yyyy  xxxx xxxx  YX-p pppp  tttt tttt

			 y = ypos
			 x = xpos
			 X = x-flip
			 Y = y-flip
			 p = palette
			 t = tile number

			*/

			code = m_spriteram[offs + 3];
			color = m_spriteram[offs + 2] & 0x1f;
			sx = ((m_spriteram[offs + 1] + 8) & 0xff) - 8;
			sy = m_spriteram[offs] - 32;
			flipx = m_spriteram[offs + 2] & 0x40;
			flipy = m_spriteram[offs + 2] & 0x80;
		}

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 176 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		/* first draw the sprite, visible */
		m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,
				screen.priority(),
				0x00,0x80ff);

		/* then draw the mask, behind the background but obscuring following sprites */
		m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,
				screen.priority(),
				0x02,0x7fff);
	}
}

UINT32 docastle_state::screen_update_docastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_do_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(screen, bitmap, cliprect);
	m_do_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
