// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    video/brkthru.c

***************************************************************************/

#include "emu.h"
#include "includes/brkthru.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Break Thru has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(brkthru_state, brkthru)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r,g,b));

		color_prom++;
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

TILE_GET_INFO_MEMBER(brkthru_state::get_bg_tile_info)
{
	/* BG RAM format
	    0         1
	    ---- -c-- ---- ---- = Color
	    ---- --xx xxxx xxxx = Code
	*/

	int code = (m_videoram[tile_index * 2] | ((m_videoram[tile_index * 2 + 1]) << 8)) & 0x3ff;
	int region = 1 + (code >> 7);
	int colour = m_bgbasecolor + ((m_videoram[tile_index * 2 + 1] & 0x04) >> 2);

	SET_TILE_INFO_MEMBER(region, code & 0x7f, colour,0);
}

WRITE8_MEMBER(brkthru_state::brkthru_bgram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


TILE_GET_INFO_MEMBER(brkthru_state::get_fg_tile_info)
{
	UINT8 code = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

WRITE8_MEMBER(brkthru_state::brkthru_fgram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void brkthru_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(brkthru_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(brkthru_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
}


WRITE8_MEMBER(brkthru_state::brkthru_1800_w)
{
	if (offset == 0)    /* low 8 bits of scroll */
		m_bgscroll = (m_bgscroll & 0x100) | data;
	else if (offset == 1)
	{
		/* bit 0-2 = ROM bank select */
		membank("bank1")->set_entry(data & 0x07);

		/* bit 3-5 = background tiles color code */
		if (((data & 0x38) >> 2) != m_bgbasecolor)
		{
			m_bgbasecolor = (data & 0x38) >> 2;
			m_bg_tilemap->mark_all_dirty();
		}

		/* bit 6 = screen flip */
		if (m_flipscreen != (data & 0x40))
		{
			m_flipscreen = data & 0x40;
			m_bg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_fg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		}

		/* bit 7 = high bit of scroll */
		m_bgscroll = (m_bgscroll & 0xff) | ((data & 0x80) << 1);
	}
}


#if 0
void brkthru_state::show_register( bitmap_ind16 &bitmap, int x, int y, UINT32 data )
{
	char buf[5];

	sprintf(buf, "%04X", data);
	ui_draw_text(y, x, buf);
}
#endif


void brkthru_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int prio )
{
	int offs;
	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */

	/* Sprite RAM format
	    0         1         2         3
	    ccc- ---- ---- ---- ---- ---- ---- ---- = Color
	    ---d ---- ---- ---- ---- ---- ---- ---- = Double Size
	    ---- p--- ---- ---- ---- ---- ---- ---- = Priority
	    ---- -bb- ---- ---- ---- ---- ---- ---- = Bank
	    ---- ---e ---- ---- ---- ---- ---- ---- = Enable/Disable
	    ---- ---- ssss ssss ---- ---- ---- ---- = Sprite code
	    ---- ---- ---- ---- yyyy yyyy ---- ---- = Y position
	    ---- ---- ---- ---- ---- ---- xxxx xxxx = X position
	*/

	for (offs = 0;offs < m_spriteram.bytes(); offs += 4)
	{
		if ((m_spriteram[offs] & 0x09) == prio)  /* Enable && Low Priority */
		{
			int sx, sy, code, color;

			sx = 240 - m_spriteram[offs + 3];
			if (sx < -7)
				sx += 256;

			sy = 240 - m_spriteram[offs + 2];
			code = m_spriteram[offs + 1] + 128 * (m_spriteram[offs] & 0x06);
			color = (m_spriteram[offs] & 0xe0) >> 5;
			if (m_flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (m_spriteram[offs] & 0x10)    /* double height */
			{
				m_gfxdecode->gfx(9)->transpen(bitmap,cliprect,
						code & ~1,
						color,
						m_flipscreen, m_flipscreen,
						sx, m_flipscreen ? sy + 16 : sy - 16,0);
				m_gfxdecode->gfx(9)->transpen(bitmap,cliprect,
						code | 1,
						color,
						m_flipscreen, m_flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				m_gfxdecode->gfx(9)->transpen(bitmap,cliprect,
						code & ~1,
						color,
						m_flipscreen, m_flipscreen,
						sx,(m_flipscreen ? sy + 16 : sy - 16) + 256,0);
				m_gfxdecode->gfx(9)->transpen(bitmap,cliprect,
						code | 1,
						color,
						m_flipscreen, m_flipscreen,
						sx,sy + 256,0);

			}
			else
			{
				m_gfxdecode->gfx(9)->transpen(bitmap,cliprect,
						code,
						color,
						m_flipscreen, m_flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				m_gfxdecode->gfx(9)->transpen(bitmap,cliprect,
						code,
						color,
						m_flipscreen, m_flipscreen,
						sx,sy + 256,0);

			}
		}
	}
}

UINT32 brkthru_state::screen_update_brkthru(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bgscroll);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	/* low priority sprites */
	draw_sprites(bitmap, cliprect, 0x01);

	/* draw background over low priority sprites */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* high priority sprites */
	draw_sprites(bitmap, cliprect, 0x09);

	/* fg layer */
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

/*  show_register(bitmap, 8, 8, (UINT32)m_flipscreen); */

	return 0;
}
