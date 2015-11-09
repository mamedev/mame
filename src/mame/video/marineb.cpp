// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/marineb.h"


PALETTE_INIT_MEMBER(marineb_state, marineb)
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

TILE_GET_INFO_MEMBER(marineb_state::get_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	UINT8 col = m_colorram[tile_index];

	SET_TILE_INFO_MEMBER(0,
					code | ((col & 0xc0) << 2),
					(col & 0x0f) | (m_palette_bank << 4),
					TILE_FLIPXY((col >> 4) & 0x03));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void marineb_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(marineb_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_palette_bank));
	save_item(NAME(m_column_scroll));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(marineb_state::marineb_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(marineb_state::marineb_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(marineb_state::marineb_column_scroll_w)
{
	m_column_scroll = data;
}


WRITE8_MEMBER(marineb_state::marineb_palette_bank_0_w)
{
	UINT8 old = m_palette_bank;

	m_palette_bank = (m_palette_bank & 0x02) | (data & 0x01);

	if (old != m_palette_bank)
	{
		m_bg_tilemap->mark_all_dirty();
	}
}


WRITE8_MEMBER(marineb_state::marineb_palette_bank_1_w)
{
	UINT8 old = m_palette_bank;

	m_palette_bank = (m_palette_bank & 0x01) | ((data & 0x01) << 1);

	if (old != m_palette_bank)
	{
		m_bg_tilemap->mark_all_dirty();
	}
}


WRITE8_MEMBER(marineb_state::marineb_flipscreen_x_w)
{
	m_flipscreen_x = data ^ m_marineb_active_low_flipscreen;
	m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
}


WRITE8_MEMBER(marineb_state::marineb_flipscreen_y_w)
{
	m_flipscreen_y = data ^ m_marineb_active_low_flipscreen;
	m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void marineb_state::set_tilemap_scrolly( int cols )
{
	int col;

	for (col = 0; col < cols; col++)
		m_bg_tilemap->set_scrolly(col, m_column_scroll);

	for (; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, 0);
}


UINT32 marineb_state::screen_update_marineb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	set_tilemap_scrolly(24);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx, sx, sy, code, col, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  /* no sprites here */

		if (offs < 8)
			offs2 = 0x0018 + offs;
		else
			offs2 = 0x03d8 - 8 + offs;

		code = m_videoram[offs2];
		sx = m_videoram[offs2 + 0x20];
		sy = m_colorram[offs2];
		col = (m_colorram[offs2 + 0x20] & 0x0f) + 16 * m_palette_bank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!m_flipscreen_y)
		{
			sy = 256 - m_gfxdecode->gfx(gfx)->width() - sy;
			flipy = !flipy;
		}

		if (m_flipscreen_x)
		{
			sx++;
		}

		m_gfxdecode->gfx(gfx)->transpen(bitmap,cliprect,
				code,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}


UINT32 marineb_state::screen_update_changes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, sx, sy, code, col, flipx, flipy;

	set_tilemap_scrolly(26);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the small sprites */
	for (offs = 0x05; offs >= 0; offs--)
	{
		int offs2;

		offs2 = 0x001a + offs;

		code = m_videoram[offs2];
		sx = m_videoram[offs2 + 0x20];
		sy = m_colorram[offs2];
		col = (m_colorram[offs2 + 0x20] & 0x0f) + 16 * m_palette_bank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (!m_flipscreen_y)
		{
			sy = 256 - m_gfxdecode->gfx(1)->width() - sy;
			flipy = !flipy;
		}

		if (m_flipscreen_x)
		{
			sx++;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code >> 2,
				col,
				flipx,flipy,
				sx,sy,0);
	}

	/* draw the big sprite */

	code = m_videoram[0x3df];
	sx = m_videoram[0x3ff];
	sy = m_colorram[0x3df];
	col = m_colorram[0x3ff];
	flipx = code & 0x02;
	flipy = !(code & 0x01);

	if (!m_flipscreen_y)
	{
		sy = 256 - m_gfxdecode->gfx(2)->width() - sy;
		flipy = !flipy;
	}

	if (m_flipscreen_x)
	{
		sx++;
	}

	code >>= 4;

	m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code,
			col,
			flipx,flipy,
			sx,sy,0);

	/* draw again for wrap around */

	m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code,
			col,
			flipx,flipy,
			sx-256,sy,0);
	return 0;
}


UINT32 marineb_state::screen_update_springer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	set_tilemap_scrolly(0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx, sx, sy, code, col, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  /* no sprites here */

		offs2 = 0x0010 + offs;

		code = m_videoram[offs2];
		sx = 240 - m_videoram[offs2 + 0x20];
		sy = m_colorram[offs2];
		col = (m_colorram[offs2 + 0x20] & 0x0f) + 16 * m_palette_bank;
		flipx = !(code & 0x02);
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			sx -= 0x10;
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!m_flipscreen_y)
		{
			sy = 256 - m_gfxdecode->gfx(gfx)->width() - sy;
			flipy = !flipy;
		}

		if (!m_flipscreen_x)
		{
			sx--;
		}

		m_gfxdecode->gfx(gfx)->transpen(bitmap,cliprect,
				code,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}


UINT32 marineb_state::screen_update_hoccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	set_tilemap_scrolly(0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0x07; offs >= 0; offs--)
	{
		int sx, sy, code, col, flipx, flipy, offs2;

		offs2 = 0x0018 + offs;

		code = m_spriteram[offs2];
		sx = m_spriteram[offs2 + 0x20];
		sy = m_colorram[offs2];
		col = m_colorram[offs2 + 0x20];
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (!m_flipscreen_y)
		{
			sy = 256 - m_gfxdecode->gfx(1)->width() - sy;
			flipy = !flipy;
		}

		if (m_flipscreen_x)
		{
			sx = 256 - m_gfxdecode->gfx(1)->width() - sx;
			flipx = !flipx;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code >> 2,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}


UINT32 marineb_state::screen_update_hopprobo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	set_tilemap_scrolly(0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx, sx, sy, code, col, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  /* no sprites here */

		offs2 = 0x0010 + offs;

		code = m_videoram[offs2];
		sx = m_videoram[offs2 + 0x20];
		sy = m_colorram[offs2];
		col = (m_colorram[offs2 + 0x20] & 0x0f) + 16 * m_palette_bank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!m_flipscreen_y)
		{
			sy = 256 - m_gfxdecode->gfx(gfx)->width() - sy;
			flipy = !flipy;
		}

		if (!m_flipscreen_x)
		{
			sx--;
		}

		m_gfxdecode->gfx(gfx)->transpen(bitmap,cliprect,
				code,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}
