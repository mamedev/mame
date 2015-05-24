// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, David Haywood
/******************************************************************************

    Gomoku Narabe Renju
    (c)1981 Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/06 -
    Updated to compile again by David Haywood 19th Oct 2002

******************************************************************************/

#include "emu.h"
#include "includes/gomoku.h"


/******************************************************************************

    palette RAM

******************************************************************************/

PALETTE_INIT_MEMBER(gomoku_state, gomoku)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int bit0, bit1, bit2, r, g, b;

	for (i = 0; i < palette.entries(); i++)
	{
		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

TILE_GET_INFO_MEMBER(gomoku_state::get_fg_tile_info)
{
	int code = (m_videoram[tile_index]);
	int attr = (m_colorram[tile_index]);
	int color = (attr& 0x0f);
	int flipyx = (attr & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			TILE_FLIPYX(flipyx));
}

WRITE8_MEMBER(gomoku_state::gomoku_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gomoku_state::gomoku_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gomoku_state::gomoku_bgram_w)
{
	m_bgram[offset] = data;
}

WRITE8_MEMBER(gomoku_state::gomoku_flipscreen_w)
{
	m_flipscreen = (data & 0x02) ? 0 : 1;
}

WRITE8_MEMBER(gomoku_state::gomoku_bg_dispsw_w)
{
	m_bg_dispsw = (data & 0x02) ? 0 : 1;
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

void gomoku_state::video_start()
{
	UINT8 *GOMOKU_BG_X = memregion( "user1" )->base();
	UINT8 *GOMOKU_BG_Y = memregion( "user2" )->base();
	UINT8 *GOMOKU_BG_D = memregion( "user3" )->base();
	int x, y;
	int bgdata;
	int color;

	m_screen->register_screen_bitmap(m_bg_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gomoku_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	/* make background bitmap */
	m_bg_bitmap.fill(0x20);

	// board
	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 256; x++)
		{
			bgdata = GOMOKU_BG_D[ GOMOKU_BG_X[x] + (GOMOKU_BG_Y[y] << 4) ];

			color = 0x20;                       // outside frame (black)

			if (bgdata & 0x01) color = 0x21;    // board (brown)
			if (bgdata & 0x02) color = 0x20;    // frame line (while)

			m_bg_bitmap.pix16((255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
		}
	}
}


/******************************************************************************

    Display refresh

******************************************************************************/

UINT32 gomoku_state::screen_update_gomoku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *GOMOKU_BG_X = memregion( "user1" )->base();
	UINT8 *GOMOKU_BG_Y = memregion( "user2" )->base();
	UINT8 *GOMOKU_BG_D = memregion( "user3" )->base();
	int x, y;
	int bgram;
	int bgoffs;
	int bgdata;
	int color;

	/* draw background layer */
	if (m_bg_dispsw)
	{
		/* copy bg bitmap */
		copybitmap(bitmap, m_bg_bitmap, 0, 0, 0, 0, cliprect);

		// stone
		for (y = 0; y < 256; y++)
		{
			for (x = 0; x < 256; x++)
			{
				bgoffs = ((((255 - x - 2) / 14) | (((255 - y - 10) / 14) << 4)) & 0xff);

				bgdata = GOMOKU_BG_D[ GOMOKU_BG_X[x] + (GOMOKU_BG_Y[y] << 4) ];
				bgram = m_bgram[bgoffs];

				if (bgdata & 0x04)
				{
					if (bgram & 0x01)
					{
						color = 0x2f;   // stone (black)
					}
					else if (bgram & 0x02)
					{
						color = 0x22;   // stone (white)
					}
					else continue;
				}
				else continue;

				bitmap.pix16((255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
			}
		}

		// cursor
		for (y = 0; y < 256; y++)
		{
			for (x = 0; x < 256; x++)
			{
				bgoffs = ((((255 - x - 2) / 14) | (((255 - y - 10) / 14) << 4)) & 0xff);

				bgdata = GOMOKU_BG_D[ GOMOKU_BG_X[x] + (GOMOKU_BG_Y[y] << 4) ];
				bgram = m_bgram[bgoffs];

				if (bgdata & 0x08)
				{
					if (bgram & 0x04)
					{
							color = 0x2f;   // cursor (black)
					}
					else if (bgram & 0x08)
					{
						color = 0x22;       // cursor (white)
					}
					else continue;
				}
				else continue;

				bitmap.pix16((255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
			}
		}
	}
	else
	{
		bitmap.fill(0x20);
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
