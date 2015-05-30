// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  thepit.c

  Functions to emulate the video hardware of the machine.

  I have a feeling sprite area masking should be done based on tile
  attributes, not a custom cliprect.

***************************************************************************/

#include "emu.h"
#include "includes/thepit.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED


***************************************************************************/

PALETTE_INIT_MEMBER(thepit_state, thepit)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 32; i++)
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

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	/* allocate primary colors for the background and foreground
	   this is wrong, but I don't know where to pick the colors from */
	for (i = 0; i < 8; i++)
		palette.set_pen_color(i + 32, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
}


/***************************************************************************

 Super Mouse has 5 bits per gun (maybe 6 for green), exact weights are
 unknown.

***************************************************************************/

PALETTE_INIT_MEMBER(thepit_state,suprmous)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 32; i++)
	{
		UINT8 b = BITSWAP8(color_prom[i + 0x00], 0, 1, 2, 3, 4, 5, 6, 7);
		UINT8 g = BITSWAP8(color_prom[i + 0x20], 0, 1, 2, 3, 4, 5, 6, 7);
		UINT8 r = (b>>5&7)<<2 | (g>>6&3);

		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal4bit(b));
	}

	/* allocate primary colors for the background and foreground
	   this is wrong, but I don't know where to pick the colors from */
	for (i = 0; i < 8; i++)
		palette.set_pen_color(i + 32, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(thepit_state::solid_get_tile_info)
{
	UINT8 back_color = (m_colorram[tile_index] & 0x70) >> 4;
	int priority = (back_color != 0) && ((m_colorram[tile_index] & 0x80) == 0);
	tileinfo.pen_data = m_dummy_tile;
	tileinfo.palette_base = back_color + 32;
	tileinfo.category = priority;
}


TILE_GET_INFO_MEMBER(thepit_state::get_tile_info)
{
	UINT8 fore_color = m_colorram[tile_index] % m_gfxdecode->gfx(0)->colors();
	UINT8 code = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(2 * m_graphics_bank, code, fore_color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void thepit_state::video_start()
{
	m_solid_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(thepit_state::solid_get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(thepit_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);
	m_tilemap->set_transparent_pen(0);

	m_solid_tilemap->set_scroll_cols(32);
	m_tilemap->set_scroll_cols(32);

	m_dummy_tile = auto_alloc_array_clear(machine(), UINT8, 8*8);

	m_graphics_bank = 0;    /* only used in intrepid */

	save_item(NAME(m_graphics_bank));
	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(thepit_state::videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(thepit_state::colorram_w)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
	m_solid_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(thepit_state::flip_screen_x_w)
{
	int flip;

	m_flip_x = data & 0x01;

	flip = m_flip_x ? TILEMAP_FLIPX : 0;
	if (m_flip_y)
		flip |= TILEMAP_FLIPY ;

	m_tilemap->set_flip(flip);
	m_solid_tilemap->set_flip(flip);

}


WRITE8_MEMBER(thepit_state::flip_screen_y_w)
{
	int flip;

	m_flip_y = data & 0x01;

	flip = m_flip_x ? TILEMAP_FLIPX : 0;
	if (m_flip_y)
		flip |= TILEMAP_FLIPY ;

	m_tilemap->set_flip(flip);
	m_solid_tilemap->set_flip(flip);

}


WRITE8_MEMBER(thepit_state::intrepid_graphics_bank_w)
{
	if (m_graphics_bank != (data & 0x01))
	{
		m_graphics_bank = data & 0x01;

		m_tilemap->mark_all_dirty();
	}
}


READ8_MEMBER(thepit_state::input_port_0_r)
{
	/* Read either the real or the fake input ports depending on the
	   horizontal flip switch. (This is how the real PCB does it) */
	if (m_flip_x)
	{
		return ioport("IN2")->read();
	}
	else
	{
		return ioport("IN0")->read();
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void thepit_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority_to_draw)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		if (((m_spriteram[offs + 2] & 0x08) >> 3) == priority_to_draw)
		{
			UINT8 y, x, flipx, flipy;

			if ((m_spriteram[offs + 0] == 0) || (m_spriteram[offs + 3] == 0))
			{
				continue;
			}

			y = 240 - m_spriteram[offs];
			x = m_spriteram[offs + 3] + 1;

			flipx = m_spriteram[offs + 1] & 0x40;
			flipy = m_spriteram[offs + 1] & 0x80;

			if (m_flip_y)
			{
				y = 240 - y;
				flipy = !flipy;
			}

			if (m_flip_x)
			{
				x = 242 - x;
				flipx = !flipx;
			}

			/* sprites 0-3 are drawn one pixel down */
			if (offs < 16) y++;


					m_gfxdecode->gfx(2 * m_graphics_bank + 1)->transpen(bitmap,cliprect,
					m_spriteram[offs + 1] & 0x3f,
					m_spriteram[offs + 2],
					flipx, flipy, x, y, 0);


					m_gfxdecode->gfx(2 * m_graphics_bank + 1)->transpen(bitmap,cliprect,
					m_spriteram[offs + 1] & 0x3f,
					m_spriteram[offs + 2],
					flipx, flipy, x-256, y, 0);

		}
	}
}


UINT32 thepit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle spritevisiblearea(2*8+1, 32*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(0*8, 30*8-2, 2*8, 30*8-1);

	offs_t offs;

	for (offs = 0; offs < 32; offs++)
	{
		int xshift = m_flip_x ? 128 : 0;
		int yshift = m_flip_y ? -8 : 0;

		m_tilemap->set_scrollx(offs, xshift);
		m_solid_tilemap->set_scrollx(offs, xshift);

		m_tilemap->set_scrolly(offs, yshift + m_attributesram[offs << 1]);
		m_solid_tilemap->set_scrolly(offs, yshift + m_attributesram[offs << 1]);
	}

	/* low priority tiles */
	m_solid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* low priority sprites */
	draw_sprites(bitmap, m_flip_x ? spritevisibleareaflipx : spritevisiblearea, 0);

	/* high priority tiles */
	m_solid_tilemap->draw(screen, bitmap, cliprect, 1, 1);

	/* high priority sprites */
	draw_sprites(bitmap, m_flip_x ? spritevisibleareaflipx : spritevisiblearea, 1);

	return 0;
}

UINT32 thepit_state::screen_update_desertdan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	offs_t offs;
	const rectangle spritevisiblearea(0*8+1, 24*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(8*8, 32*8-2, 2*8, 30*8-1);

	for (offs = 0; offs < 32; offs++)
	{
		int xshift = m_flip_x ? 128 : 0;
		int yshift = m_flip_y ? -8 : 0;

		m_tilemap->set_scrollx(offs, xshift);
		m_solid_tilemap->set_scrollx(offs, xshift);

		m_tilemap->set_scrolly(offs, yshift + m_attributesram[offs << 1]);
		m_solid_tilemap->set_scrolly(offs, yshift + m_attributesram[offs << 1]);
	}

	/* low priority tiles */
	m_graphics_bank = 0;
	m_solid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* low priority sprites */
	m_graphics_bank = 1;
	draw_sprites(bitmap, m_flip_y ? spritevisibleareaflipx : spritevisiblearea, 0);

	/* high priority tiles */ // not sure about this, draws a white block over the title logo sprite, looks like it should be behind?
	m_graphics_bank = 0;
	m_solid_tilemap->draw(screen, bitmap, cliprect, 1, 1);

	/* high priority sprites */
	m_graphics_bank = 1;
	draw_sprites(bitmap, m_flip_y ? spritevisibleareaflipx : spritevisiblearea, 1);

	return 0;
}
