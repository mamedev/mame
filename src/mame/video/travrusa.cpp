// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg,Tomasz Slanina
/***************************************************************************

  video.c

  Traverse USA

L Taylor
J Clegg

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/travrusa.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Traverse USA has one 256x8 character palette PROM (some versions have two
  256x4), one 32x8 sprite palette PROM, and one 256x4 sprite color lookup
  table PROM.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably something like this; note that RED and BLUE
  are swapped wrt the usual configuration.

  bit 7 -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(travrusa_state, travrusa)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x80; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (i = 0x80; i < 0x90; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[(i - 0x80) + 0x200] >> 6) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x200] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[(i - 0x80) + 0x200] >> 3) & 0x01;
		bit1 = (color_prom[(i - 0x80) + 0x200] >> 4) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x200] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[(i - 0x80) + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[(i - 0x80) + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x200] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x220;

	/* characters */
	for (i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i, i);

	/* sprites */
	for (i = 0x80; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i - 0x80] & 0x0f) | 0x80;
		palette.set_pen_indirect(i, ctabentry);
	}
}

PALETTE_INIT_MEMBER(travrusa_state,shtrider)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x80; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 3) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (i = 0x80; i < 0x90; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[(i - 0x80) + 0x200] >> 6) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x200] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[(i - 0x80) + 0x200] >> 3) & 0x01;
		bit1 = (color_prom[(i - 0x80) + 0x200] >> 4) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x200] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[(i - 0x80) + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[(i - 0x80) + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x200] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x220;

	/* characters */
	for (i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i, i);

	/* sprites */
	for (i = 0x80; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i - 0x80] & 0x0f) | 0x80;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(travrusa_state::get_tile_info)
{
	UINT8 attr = m_videoram[2 * tile_index + 1];
	int flags = TILE_FLIPXY((attr & 0x30) >> 4);

	tileinfo.group = ((attr & 0x0f) == 0x0f) ? 1 : 0;   /* tunnels */

	SET_TILE_INFO_MEMBER(0,
			m_videoram[2 * tile_index] + ((attr & 0xc0) << 2),
			attr & 0x0f,
			flags);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void travrusa_state::video_start()
{
	save_item(NAME(m_scrollx));

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(travrusa_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap->set_transmask(0, 0xff, 0x00); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x3f, 0xc0); /* split type 1 has pens 6 and 7 opaque - tunnels */

	m_bg_tilemap->set_scroll_rows(4);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(travrusa_state::travrusa_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


void travrusa_state::set_scroll(  )
{
	int i;

	for (i = 0; i <= 2; i++)
		m_bg_tilemap->set_scrollx(i, m_scrollx[0] + 256 * m_scrollx[1]);

	m_bg_tilemap->set_scrollx(3, 0);
}

WRITE8_MEMBER(travrusa_state::travrusa_scroll_x_low_w)
{
	m_scrollx[0] = data;
	set_scroll();
}

WRITE8_MEMBER(travrusa_state::travrusa_scroll_x_high_w)
{
	m_scrollx[1] = data;
	set_scroll();
}


WRITE8_MEMBER(travrusa_state::travrusa_flipscreen_w)
{
	/* screen flip is handled both by software and hardware */
	data ^= ~ioport("DSW2")->read() & 1;

	flip_screen_set(data & 1);

	coin_counter_w(machine(), 0, data & 0x02);
	coin_counter_w(machine(), 1, data & 0x20);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void travrusa_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int offs;
	const rectangle spritevisiblearea(1*8, 31*8-1, 0*8, 24*8-1);
	const rectangle spritevisibleareaflip(1*8, 31*8-1, 8*8, 32*8-1);
	rectangle clip = cliprect;
	if (flip_screen())
		clip &= spritevisibleareaflip;
	else
		clip &= spritevisiblearea;


	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sx = ((m_spriteram[offs + 3] + 8) & 0xff) - 8;
		int sy = 240 - m_spriteram[offs];
		int code = m_spriteram[offs + 2];
		int attr = m_spriteram[offs + 1];
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,clip,
				code,
				attr & 0x0f,
				flipx, flipy,
				sx, sy, 0);
	}
}


UINT32 travrusa_state::screen_update_travrusa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
