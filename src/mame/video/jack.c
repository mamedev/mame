// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  video/jack.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/jack.h"



WRITE8_MEMBER(jack_state::jack_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(jack_state::jack_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(jack_state::jack_flipscreen_r)
{
	flip_screen_set(offset);
	return 0;
}

WRITE8_MEMBER(jack_state::jack_flipscreen_w)
{
	flip_screen_set(offset);
}


/**************************************************************************/

TILE_GET_INFO_MEMBER(jack_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x18) << 5);
	int color = m_colorram[tile_index] & 0x07;

	// striv: m_colorram[tile_index] & 0x80 ???

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(jack_state::tilemap_scan_cols_flipy)
{
	/* logical (col,row) -> memory offset */
	return (col * num_rows) + (num_rows - 1 - row);
}

void jack_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jack_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(jack_state::tilemap_scan_cols_flipy),this), 8, 8, 32, 32);
}


/**************************************************************************/

void jack_state::jack_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sy = spriteram[offs];
		int sx = spriteram[offs + 1];
		int code = spriteram[offs + 2] | ((spriteram[offs + 3] & 0x08) << 5);
		int color = spriteram[offs + 3] & 0x07;
		int flipx = (spriteram[offs + 3] & 0x80) >> 7;
		int flipy = (spriteram[offs + 3] & 0x40) >> 6;

		if (flip_screen())
		{
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

UINT32 jack_state::screen_update_jack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	jack_draw_sprites(bitmap, cliprect);
	return 0;
}


UINT32 jack_state::screen_update_striv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// no sprites
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}






/***************************************************************************

   Joinem has a bit different video hardware with proms based palette,
   3bpp gfx and different banking / colors bits

***************************************************************************/

WRITE8_MEMBER(jack_state::joinem_scroll_w)
{
	switch (offset & 3)
	{
		// byte 0: column scroll
		case 0:
			m_bg_tilemap->set_scrolly(offset >> 2, -data);
			break;

		// byte 1/2/3: no effect?
		default:
			break;
	}

	m_scrollram[offset] = data;
}


/**************************************************************************/

PALETTE_INIT_MEMBER(jack_state,joinem)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}


TILE_GET_INFO_MEMBER(jack_state::joinem_get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x03) << 8);
	int color = (m_colorram[tile_index] & 0x38) >> 3 | m_joinem_palette_bank;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

VIDEO_START_MEMBER(jack_state,joinem)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jack_state::joinem_get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(jack_state::tilemap_scan_cols_flipy),this), 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);
}


/**************************************************************************/

void jack_state::joinem_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sy = spriteram[offs];
		int sx = spriteram[offs + 1];
		int code = spriteram[offs + 2] | ((spriteram[offs + 3] & 0x03) << 8);
		int color = (spriteram[offs + 3] & 0x38) >> 3 | m_joinem_palette_bank;
		int flipx = (spriteram[offs + 3] & 0x80) >> 7;
		int flipy = (spriteram[offs + 3] & 0x40) >> 6;

		if (flip_screen())
		{
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

UINT32 jack_state::screen_update_joinem(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	joinem_draw_sprites(bitmap, cliprect);
	return 0;
}
