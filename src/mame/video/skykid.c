// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/skykid.h"


/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(skykid_state, skykid)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* text palette */
	for (i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	/* tiles/sprites */
	for (i = 0x100; i < 0x500; i++)
	{
		UINT8 ctabentry = color_prom[i - 0x100];
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(skykid_state::tx_tilemap_scan)
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

TILE_GET_INFO_MEMBER(skykid_state::tx_get_tile_info)
{
	int code = m_textram[tile_index];
	int attr = m_textram[tile_index + 0x400];
	tileinfo.category = code >> 4 & 0xf;

	/* the hardware has two character sets, one normal and one flipped. When
	   screen is flipped, character flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	SET_TILE_INFO_MEMBER(0,
			code | (flip_screen() ? 0x100 : 0),
			attr & 0x3f,
			flip_screen() ? (TILE_FLIPY | TILE_FLIPX) : 0);
}


TILE_GET_INFO_MEMBER(skykid_state::bg_get_tile_info)
{
	int code = m_videoram[tile_index];
	int attr = m_videoram[tile_index+0x800];

	SET_TILE_INFO_MEMBER(1,
			code + ((attr & 0x01) << 8),
			((attr & 0x7e) >> 1) | ((attr & 0x01) << 6),
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void skykid_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skykid_state::tx_get_tile_info),this),tilemap_mapper_delegate(FUNC(skykid_state::tx_tilemap_scan),this),  8,8,36,28);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skykid_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,     8,8,64,32);

	m_tx_tilemap->set_transparent_pen(0);

	save_item(NAME(m_priority));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_MEMBER(skykid_state::skykid_videoram_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(skykid_state::skykid_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

READ8_MEMBER(skykid_state::skykid_textram_r)
{
	return m_textram[offset];
}

WRITE8_MEMBER(skykid_state::skykid_textram_w)
{
	m_textram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(skykid_state::skykid_scroll_x_w)
{
	m_scroll_x = offset;
}

WRITE8_MEMBER(skykid_state::skykid_scroll_y_w)
{
	m_scroll_y = offset;
}

WRITE8_MEMBER(skykid_state::skykid_flipscreen_priority_w)
{
	m_priority = data;
	flip_screen_set(offset & 1);
}



/***************************************************************************

  Display Refresh

***************************************************************************/

/* the sprite generator IC is the same as Mappy */
void skykid_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x0800;
	UINT8 *spriteram_3 = spriteram_2 + 0x0800;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs] + ((spriteram_3[offs] & 0x80) << 1);
		int color = (spriteram[offs+1] & 0x3f);
		int sx = (spriteram_2[offs+1]) + 0x100*(spriteram_3[offs+1] & 1) - 71;
		int sy = 256 - spriteram_2[offs] - 7;
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizex = (spriteram_3[offs] & 0x04) >> 2;
		int sizey = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;  // fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				m_gfxdecode->gfx(2)->transmask(bitmap,cliprect,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x,sy + 16*y,
					m_palette->transpen_mask(*m_gfxdecode->gfx(2), color, 0xff));
			}
		}
	}
}


UINT32 skykid_state::screen_update_skykid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
	{
		m_bg_tilemap->set_scrollx(0, 189 - (m_scroll_x ^ 1));
		m_bg_tilemap->set_scrolly(0, 7 - m_scroll_y);
	}
	else
	{
		m_bg_tilemap->set_scrollx(0, m_scroll_x + 35);
		m_bg_tilemap->set_scrolly(0, m_scroll_y + 25);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);

	if (m_priority & 0x04)
	{
		// textlayer priority enabled?
		int cat, pri = m_priority >> 4;

		// draw low priority tiles
		m_tx_tilemap->draw(screen, bitmap, cliprect, pri, 0);

		draw_sprites(bitmap, cliprect);

		// draw the other tiles
		for (cat = 0; cat < 0xf; cat++)
			if (cat != pri) m_tx_tilemap->draw(screen, bitmap, cliprect, cat, 0);
	}
	else
	{
		draw_sprites(bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES, 0);
	}

	return 0;
}
