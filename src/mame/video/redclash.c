// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ladybug.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm using the same palette conversion as Lady Bug, but the Zero Hour
  schematics show a different resistor network.

***************************************************************************/

PALETTE_INIT_MEMBER(ladybug_state,redclash)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		r = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* star colors */
	for (i = 0x20; i < 0x40; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 3) & 0x01;
		bit1 = ((i - 0x20) >> 4) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = ((i - 0x20) >> 1) & 0x01;
		bit1 = ((i - 0x20) >> 2) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		r = 0x47 * bit0;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters */
	for (i = 0; i < 0x20; i++)
	{
		UINT8 ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[(i - 0x20) >> 1];

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 0) & 0x0f, 7,6,5,4,0,1,2,3);
		palette.set_pen_indirect(i + 0x00, ctabentry);

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 4) & 0x0f, 7,6,5,4,0,1,2,3);
		palette.set_pen_indirect(i + 0x20, ctabentry);
	}

	/* stars */
	for (i = 0x60; i < 0x80; i++)
		palette.set_pen_indirect(i, (i - 0x60) + 0x20);
}

WRITE8_MEMBER( ladybug_state::redclash_videoram_w )
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER( ladybug_state::redclash_gfxbank_w )
{
	if (m_gfxbank != (data & 0x01))
	{
		m_gfxbank = data & 0x01;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER( ladybug_state::redclash_flipscreen_w )
{
	flip_screen_set(data & 0x01);
}

/*
star_speed:
0 = unused
1 = unused
2 = forward fast
3 = forward medium
4 = forward slow
5 = backwards slow
6 = backwards medium
7 = backwards fast
*/
WRITE8_MEMBER( ladybug_state::redclash_star0_w )
{
	m_star_speed = (m_star_speed & ~1) | ((data & 1) << 0);
	redclash_set_stars_speed(m_star_speed);
}

WRITE8_MEMBER( ladybug_state::redclash_star1_w )
{
	m_star_speed = (m_star_speed & ~2) | ((data & 1) << 1);
	redclash_set_stars_speed(m_star_speed);
}

WRITE8_MEMBER( ladybug_state::redclash_star2_w )
{
	m_star_speed = (m_star_speed & ~4) | ((data & 1) << 2);
	redclash_set_stars_speed( m_star_speed);
}

WRITE8_MEMBER( ladybug_state::redclash_star_reset_w )
{
	redclash_set_stars_enable(1);
}

TILE_GET_INFO_MEMBER(ladybug_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = (m_videoram[tile_index] & 0x70) >> 4; // ??

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

VIDEO_START_MEMBER(ladybug_state,redclash)
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ladybug_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

void ladybug_state::redclash_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int i, offs;

	for (offs = m_spriteram.bytes() - 0x20; offs >= 0; offs -= 0x20)
	{
		i = 0;
		while (i < 0x20 && spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
			i -= 4;

			if (spriteram[offs + i] & 0x80)
			{
				int color = spriteram[offs + i + 2] & 0x0f;
				int sx = spriteram[offs + i + 3];
				int sy = offs / 4 + (spriteram[offs + i] & 0x07);


				switch ((spriteram[offs + i] & 0x18) >> 3)
				{
					case 3: /* 24x24 */
					{
						int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((m_gfxbank & 1) << 4);

						m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
								code,
								color,
								0,0,
								sx,sy - 16,0);
						/* wraparound */
						m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
								code,
								color,
								0,0,
								sx - 256,sy - 16,0);
						break;
					}

					case 2: /* 16x16 */
						if (spriteram[offs + i] & 0x20) /* zero hour spaceships */
						{
							int code = ((spriteram[offs + i + 1] & 0xf8) >> 3) + ((m_gfxbank & 1) << 5);
							int bank = (spriteram[offs + i + 1] & 0x02) >> 1;

							m_gfxdecode->gfx(4+bank)->transpen(bitmap,cliprect,
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						else
						{
							int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((m_gfxbank & 1) << 4);

							m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						break;

					case 1: /* 8x8 */
						m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
								spriteram[offs + i + 1],// + 4 * (spriteram[offs + i + 2] & 0x10),
								color,
								0,0,
								sx,sy - 16,0);
						break;

					case 0:
						popmessage("unknown sprite size 0");
						break;
				}
			}
		}
	}
}

void ladybug_state::redclash_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < 0x20; offs++)
	{
//      sx = m_videoramoffs];
		int sx = 8 * offs + (m_videoram[offs] & 0x07);   /* ?? */
		int sy = 0xff - m_videoram[offs + 0x20];

		if (flip_screen())
		{
			sx = 240 - sx;
		}

		if (cliprect.contains(sx, sy))
			bitmap.pix16(sy, sx) = 0x19;
	}
}

/*
 * These functions emulate the star generator board
 * All this comes from the schematics for Zero Hour
 *
 * It has a 17-bit LFSR which has a period of 2^17-1 clocks
 * (This is one pixel shy of "two screens" worth.)
 * So, there are two starfields drawn on alternate frames
 * These will scroll at a rate controlled by the speed register
 *
 * I'm basically doing the same thing by drawing each
 *  starfield on alternate frames, and then offseting them
 */

/* This line can reset the LFSR to zero and disables the star generator */
void ladybug_state::redclash_set_stars_enable(UINT8 on)
{
	if ((m_stars_enable == 0) && (on == 1))
	{
		m_stars_offset = 0;
	}

	m_stars_enable = on;
}

/* This sets up which starfield to draw and the offset, */
/* To be called from screen_eof_*() */

void ladybug_state::redclash_update_stars_state()
{
	if (m_stars_enable == 0)
		return;

	m_stars_count++;
	m_stars_count %= 2;

	if (m_stars_count == 0)
	{
		m_stars_offset += ((m_stars_speed * 2) - 0x09);
		m_stars_offset %= 256 * 256;
		m_stars_state = 0;
	}
	else
		m_stars_state = 0x1fc71;
}

/* Set the speed register (3 bits) */

/*
 * 0 left/down fastest (-9/2 pix per frame)
 * 1 left/down faster  (-7/2 pix per frame)
 * 2 left/down fast    (-5/2 pix per frame)
 * 3 left/down medium  (-3/2 pix per frame)
 * 4 left/down slow    (-1/2 pix per frame)
 * 5 right/up slow     (+1/2 pix per frame)
 * 6 right/up medium   (+3/2 pix per frame)
 * 7 right/up fast     (+5/2 pix per frame)
 */

void ladybug_state::redclash_set_stars_speed(UINT8 speed )
{
	m_stars_speed = speed;
}

/* Draw the stars */

/* Space Raider doesn't use the Va bit, and it is also set up to */
/* window the stars to a certain x range */

void ladybug_state::redclash_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx )
{
	int i;
	UINT8 tempbit, feedback, star_color, xloc, yloc;
	UINT32 state;
	UINT8 hcond, vcond;

	if (m_stars_enable == 0)
		return;

	state = m_stars_state;

	for(i = 0; i < 256 * 256; i++)
	{
		xloc = (m_stars_offset + i) % 256;
		yloc = ((m_stars_offset + i) /256 ) % 256;

		if ((state & 0x10000) == 0)
			tempbit = 1;
		else
			tempbit = 0;

		if ((state & 0x00020) != 0)
			feedback = tempbit ^ 1;
		else
			feedback = tempbit ^ 0;

		hcond = ((xloc + 8) & 0x10) >> 4;

		// sraider doesn't have Va hooked up
		if (sraider)
			vcond = 1;
		else
			vcond = yloc & 0x01;

		if (cliprect.contains(xloc, yloc))
		{
			if ((hcond ^ vcond) == 0)
			{
				/* enable condition */
				if (((state & 0x000ff) == 0x000ff) && (feedback == 0))
				{
					/* used by space raider */
					if ((xloc >= firstx) && (xloc <= lastx))
					{
						star_color = (state >> 9) & 0x1f;
						bitmap.pix16(yloc, xloc) = palette_offset + star_color;
					}
				}
			}
		}

		/* update LFSR state */
		state = ((state << 1) & 0x1fffe) | feedback;
	}
}

void ladybug_state::screen_eof_redclash(screen_device &screen, bool state)
{
	// falling edge
	if (!state)
		redclash_update_stars_state();
}

UINT32 ladybug_state::screen_update_redclash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	redclash_draw_stars(bitmap, cliprect, 0x60, 0, 0x00, 0xff);
	redclash_draw_sprites(bitmap, cliprect);
	redclash_draw_bullets(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
