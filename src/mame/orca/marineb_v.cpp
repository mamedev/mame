// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "marineb.h"


void marineb_state::marineb_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3) & 0x01;
		bit1 = BIT(color_prom[i + palette.entries()], 0);
		bit2 = BIT(color_prom[i + palette.entries()], 1);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i + palette.entries()], 2);
		bit2 = BIT(color_prom[i + palette.entries()], 3);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(marineb_state::get_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	uint8_t col = m_colorram[tile_index];

	tileinfo.set(0,
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
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(marineb_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
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

void marineb_state::marineb_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void marineb_state::marineb_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void marineb_state::marineb_column_scroll_w(uint8_t data)
{
	m_column_scroll = data;
}


void marineb_state::marineb_palette_bank_0_w(uint8_t data)
{
	uint8_t old = m_palette_bank;

	m_palette_bank = (m_palette_bank & 0x02) | (data & 0x01);

	if (old != m_palette_bank)
	{
		m_bg_tilemap->mark_all_dirty();
	}
}


void marineb_state::marineb_palette_bank_1_w(uint8_t data)
{
	uint8_t old = m_palette_bank;

	m_palette_bank = (m_palette_bank & 0x01) | ((data & 0x01) << 1);

	if (old != m_palette_bank)
	{
		m_bg_tilemap->mark_all_dirty();
	}
}


WRITE_LINE_MEMBER(marineb_state::flipscreen_x_w)
{
	m_flipscreen_x = state;
	m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));
}


WRITE_LINE_MEMBER(marineb_state::flipscreen_y_w)
{
	m_flipscreen_y = state;
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


uint32_t marineb_state::screen_update_marineb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


uint32_t marineb_state::screen_update_changes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


uint32_t marineb_state::screen_update_springer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


uint32_t marineb_state::screen_update_hoccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


uint32_t marineb_state::screen_update_hopprobo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
