// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Couriersud
/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

    This file is also used by scregg.c

***************************************************************************/

#include "emu.h"
#include "includes/btime.h"

/***************************************************************************

    Burger Time doesn't have a color PROM. It uses RAM to dynamically
    create the palette.
    The palette RAM is connected to the RGB output this way:

    bit 7 -- 15 kohm resistor  -- BLUE (inverted)
          -- 33 kohm resistor  -- BLUE (inverted)
          -- 15 kohm resistor  -- GREEN (inverted)
          -- 33 kohm resistor  -- GREEN (inverted)
          -- 47 kohm resistor  -- GREEN (inverted)
          -- 15 kohm resistor  -- RED (inverted)
          -- 33 kohm resistor  -- RED (inverted)
    bit 0 -- 47 kohm resistor  -- RED (inverted)

***************************************************************************/

PALETTE_INIT_MEMBER(btime_state,btime)
{
	/* Burger Time doesn't have a color PROM, but Hamburge has. */
	/* This function is also used by Eggs. */
	if (m_prom_region == nullptr)
	{
		return;
	}

	const UINT8 *color_prom = m_prom_region->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		/* red component */
		int bit0 = (color_prom[i] >> 0) & 0x01;
		int bit1 = (color_prom[i] >> 1) & 0x01;
		int bit2 = (color_prom[i] >> 2) & 0x01;
		int r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		int g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		int b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}

/***************************************************************************

    Convert the color PROMs into a more useable format.

    The PROM is connected to the RGB output this way:

    bit 7 -- 47 kohm resistor  -- RED
          -- 33 kohm resistor  -- RED
          -- 15 kohm resistor  -- RED
          -- 47 kohm resistor  -- GREEN
          -- 33 kohm resistor  -- GREEN
          -- 15 kohm resistor  -- GREEN
          -- 33 kohm resistor  -- BLUE
    bit 0 -- 15 kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(btime_state,lnc)
{
	const UINT8 *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		/* red component */
		int bit0 = (color_prom[i] >> 7) & 0x01;
		int bit1 = (color_prom[i] >> 6) & 0x01;
		int bit2 = (color_prom[i] >> 5) & 0x01;
		int r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		int g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 0) & 0x01;
		int b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}

/***************************************************************************

Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(btime_state,disco)
{
	// graphics are in RAM
	m_gfxdecode->gfx(0)->set_source(m_deco_charram);
	m_gfxdecode->gfx(1)->set_source(m_deco_charram);
}

VIDEO_START_MEMBER(btime_state,bnj)
{
	/* the background area is twice as wide as the screen */
	int width = 256;
	int height = 256;
	m_background_bitmap = std::make_unique<bitmap_ind16>(2 * width, height);

	save_item(NAME(*m_background_bitmap));
}

WRITE8_MEMBER(btime_state::lnc_videoram_w)
{
	m_videoram[offset] = data;
	m_colorram[offset] = *m_lnc_charbank;
}

READ8_MEMBER(btime_state::btime_mirrorvideoram_r)
{
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return m_videoram[offset];
}

READ8_MEMBER(btime_state::btime_mirrorcolorram_r)
{
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return m_colorram[offset];
}

WRITE8_MEMBER(btime_state::btime_mirrorvideoram_w)
{
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	m_videoram[offset] = data;
}

WRITE8_MEMBER(btime_state::lnc_mirrorvideoram_w)
{
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	lnc_videoram_w(space, offset, data);
}

WRITE8_MEMBER(btime_state::btime_mirrorcolorram_w)
{
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	m_colorram[offset] = data;
}

WRITE8_MEMBER(btime_state::deco_charram_w)
{
	if (m_deco_charram[offset] == data)
		return;

	m_deco_charram[offset] = data;

	offset &= 0x1fff;

	/* dirty sprite */
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 5);

	/* diry char */
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 3);
}

WRITE8_MEMBER(btime_state::bnj_background_w)
{
	m_bnj_backgroundram[offset] = data;
}

WRITE8_MEMBER(btime_state::bnj_scroll1_w)
{
	m_bnj_scroll1 = data;
}

WRITE8_MEMBER(btime_state::bnj_scroll2_w)
{
	m_bnj_scroll2 = data;
}

WRITE8_MEMBER(btime_state::btime_video_control_w)
{
	// Btime video control
	//
	// Bit 0   = Flip screen
	// Bit 1-7 = Unknown

	flip_screen_set(data & 0x01);
}

WRITE8_MEMBER(btime_state::bnj_video_control_w)
{
	/* Bnj/Lnc works a little differently than the btime/eggs (apparently). */
	/* According to the information at: */
	/* http://www.davesclassics.com/arcade/Switch_Settings/BumpNJump.sw */
	/* SW8 is used for cocktail video selection (as opposed to controls), */
	/* but bit 7 of the input port is used for vblank input. */
	/* My guess is that this switch open circuits some connection to */
	/* the monitor hardware. */
	/* For now we just check 0x40 in DSW1, and ignore the write if we */
	/* are in upright controls mode. */

	if (ioport("DSW1")->read() & 0x40) /* cocktail mode */
		btime_video_control_w(space, offset, data);
}

WRITE8_MEMBER(btime_state::zoar_video_control_w)
{
	// Zoar video control
	//
	// Bit 0-2 = Unknown (always 0). Marked as MCOL on schematics
	// Bit 3-4 = Palette
	// Bit 7   = Flip Screen

	m_btime_palette = (data & 0x30) >> 3;

	if (ioport("DSW1")->read() & 0x40) /* cocktail mode */
		flip_screen_set(data & 0x80);
}

WRITE8_MEMBER(btime_state::disco_video_control_w)
{
	m_btime_palette = (data >> 2) & 0x03;

	if (!(ioport("DSW1")->read() & 0x40)) /* cocktail mode */
		flip_screen_set(data & 0x01);
}


void btime_state::draw_chars( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 transparency, UINT8 color, int priority )
{
	offs_t offs;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		UINT8 x = 31 - (offs / 32);
		UINT8 y = offs % 32;

		UINT16 code = m_videoram[offs] + 256 * (m_colorram[offs] & 3);

		/* check priority */
		if ((priority != -1) && (priority != ((code >> 7) & 0x01)))
			continue;

		if (flip_screen())
		{
			x = 31 - x;
			y = 31 - y;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flip_screen(),flip_screen(),
				8*x,8*y,
				transparency ? 0 : -1);
	}
}

void btime_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 color,
							UINT8 sprite_y_adjust, UINT8 sprite_y_adjust_flip_screen,
							UINT8 *sprite_ram, offs_t interleave )
{
	int i;
	offs_t offs;

	/* draw the sprites */
	for (i = 0, offs = 0; i < 8; i++, offs += 4 * interleave)
	{
		int x, y;
		UINT8 flipx, flipy;

		if (!(sprite_ram[offs + 0] & 0x01)) continue;

		x = 240 - sprite_ram[offs + 3 * interleave];
		y = 240 - sprite_ram[offs + 2 * interleave];

		flipx = sprite_ram[offs + 0] & 0x04;
		flipy = sprite_ram[offs + 0] & 0x02;

		if (flip_screen())
		{
			x = 240 - x;
			y = 240 - y + sprite_y_adjust_flip_screen;

			flipx = !flipx;
			flipy = !flipy;
		}

		y = y - sprite_y_adjust;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				sprite_ram[offs + interleave],
				color,
				flipx,flipy,
				x, y,0);

		y = y + (flip_screen() ? -256 : 256);

		// Wrap around
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				sprite_ram[offs + interleave],
				color,
				flipx,flipy,
				x,y,0);
	}
}


void btime_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8* tmap, UINT8 color )
{
	int i;
	const UINT8 *gfx = memregion("bg_map")->base();
	int scroll = -(m_bnj_scroll2 | ((m_bnj_scroll1 & 0x03) << 8));

	// One extra iteration for wrap around
	for (i = 0; i < 5; i++, scroll += 256)
	{
		offs_t offs;
		offs_t tileoffset = tmap[i & 3] * 0x100;

		// Skip if this tile is completely off the screen
		if (scroll > 256)
			break;
		if (scroll < -256)
			continue;

		for (offs = 0; offs < 0x100; offs++)
		{
			int x = 240 - (16 * (offs / 16) + scroll) - 1;
			int y = 16 * (offs % 16);

			if (flip_screen())
			{
				x = 240 - x;
				y = 240 - y;
			}

			m_gfxdecode->gfx(2)->opaque(bitmap,cliprect,
					gfx[tileoffset + offs],
					color,
					flip_screen(),flip_screen(),
					x,y);
		}
	}
}


UINT32 btime_state::screen_update_btime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_bnj_scroll1 & 0x10)
	{
		int i, start;

		// Generate tile map
		if (flip_screen())
			start = 0;
		else
			start = 1;

		for (i = 0; i < 4; i++)
		{
			m_btime_tilemap[i] = start | (m_bnj_scroll1 & 0x04);
			start = (start + 1) & 0x03;
		}

		draw_background(bitmap, cliprect, m_btime_tilemap, 0);
		draw_chars(bitmap, cliprect, TRUE, 0, -1);
	}
	else
		draw_chars(bitmap, cliprect, FALSE, 0, -1);

	draw_sprites(bitmap, cliprect, 0, 1, 0, m_videoram, 0x20);

	return 0;
}


UINT32 btime_state::screen_update_eggs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_chars(bitmap, cliprect, FALSE, 0, -1);
	draw_sprites(bitmap, cliprect, 0, 0, 0, m_videoram, 0x20);

	return 0;
}


UINT32 btime_state::screen_update_lnc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_chars(bitmap, cliprect, FALSE, 0, -1);
	draw_sprites(bitmap, cliprect, 0, 1, 2, m_videoram, 0x20);

	return 0;
}


UINT32 btime_state::screen_update_zoar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_bnj_scroll1 & 0x04)
	{
		draw_background(bitmap, cliprect, m_zoar_scrollram, m_btime_palette);
		draw_chars(bitmap, cliprect, TRUE, m_btime_palette + 1, -1);
	}
	else
		draw_chars(bitmap, cliprect, FALSE, m_btime_palette + 1, -1);

	/* The order is important for correct priorities */
	draw_sprites(bitmap, cliprect, m_btime_palette + 1, 1, 2, m_videoram + 0x1f, 0x20);
	draw_sprites(bitmap, cliprect, m_btime_palette + 1, 1, 2, m_videoram, 0x20);

	return 0;
}


UINT32 btime_state::screen_update_bnj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_bnj_scroll1)
	{
		int scroll, offs;

		for (offs = m_bnj_backgroundram.bytes() - 1; offs >=0; offs--)
		{
			int sx, sy;

			sx = 16 * ((offs < 0x100) ? ((offs % 0x80) / 8) : ((offs % 0x80) / 8) + 16);
			sy = 16 * (((offs % 0x100) < 0x80) ? offs % 8 : (offs % 8) + 8);
			sx = 496 - sx;

			if (flip_screen())
			{
				sx = 496 - sx;
				sy = 240 - sy;
			}

			m_gfxdecode->gfx(2)->opaque(*m_background_bitmap,m_background_bitmap->cliprect(),
					(m_bnj_backgroundram[offs] >> 4) + ((offs & 0x80) >> 3) + 32,
					0,
					flip_screen(), flip_screen(),
					sx, sy);
		}

		/* copy the background bitmap to the screen */
		scroll = (m_bnj_scroll1 & 0x02) * 128 + 511 - m_bnj_scroll2;
		if (!flip_screen())
			scroll = 767 - scroll;
		copyscrollbitmap(bitmap, *m_background_bitmap, 1, &scroll, 0, nullptr, cliprect);

		/* copy the low priority characters followed by the sprites
		   then the high priority characters */
		draw_chars(bitmap, cliprect, TRUE, 0, 1);
		draw_sprites(bitmap, cliprect, 0, 0, 0, m_videoram, 0x20);
		draw_chars(bitmap, cliprect, TRUE, 0, 0);
	}
	else
	{
		draw_chars(bitmap, cliprect, FALSE, 0, -1);
		draw_sprites(bitmap, cliprect, 0, 0, 0, m_videoram, 0x20);
	}

	return 0;
}


UINT32 btime_state::screen_update_cookrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = m_bnj_backgroundram.bytes() - 1; offs >=0; offs--)
	{
		int sx, sy;

		sx = 31 - (offs / 32);
		sy = offs % 32;

		if (flip_screen())
		{
			sx = 31 - sx;
			sy = 31 - sy;
		}

		m_gfxdecode->gfx(2)->opaque(bitmap,cliprect,
				m_bnj_backgroundram[offs],
				0,
				flip_screen(), flip_screen(),
				8*sx,8*sy);
	}

	draw_chars(bitmap, cliprect, TRUE, 0, -1);
	draw_sprites(bitmap, cliprect, 0, 1, 0, m_videoram, 0x20);

	return 0;
}


UINT32 btime_state::screen_update_disco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_chars(bitmap, cliprect, FALSE, m_btime_palette, -1);
	draw_sprites(bitmap, cliprect, m_btime_palette, 0, 0, m_spriteram, 1);

	return 0;
}
