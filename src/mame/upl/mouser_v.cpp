// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/*******************************************************************************

     Mouser - Video Hardware:

     Character map with scrollable rows, 1024 possible characters.
        - index = byte from videoram + 2 bits from colorram)
        - (if row is scrolled, videoram is offset, colorram is not)
        - 16 4-color combinations for each char, from colorram

     15 Sprites controlled by 4-byte records
        - 16 4-color combinations
        - 2 banks of 64 sprite characters each

*******************************************************************************/

#include "emu.h"
#include "includes/mouser.h"

void mouser_state::mouser_palette(palette_device &palette) const
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
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

WRITE_LINE_MEMBER(mouser_state::flip_screen_x_w)
{
	flip_screen_x_set(!state);
}

WRITE_LINE_MEMBER(mouser_state::flip_screen_y_w)
{
	flip_screen_y_set(!state);
}

uint32_t mouser_state::screen_update_mouser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const spriteram = m_spriteram;

	// for every character in the Video RAM
	for (int offs = 0x3ff; offs >= 0; offs--)
	{
		int sx = offs % 32;
		if (flip_screen_x())
			sx = 31 - sx;

		int sy = offs / 32;
		if (flip_screen_y())
			sy = 31 - sy;

		/* This bit of spriteram appears to be for row scrolling */
		/* Note: this is dependant on flipping in y */
		int const scrolled_y_position = (256 + 8*sy - spriteram[offs%32])%256;
		/* I think we still need to fetch the colorram bits to from the ram underneath, which is not scrolled */
		/* Ideally we would merge these on a pixel-by-pixel basis, but it's ok to do this char-by-char, */
		/* Since it's only for the MOUSER logo and it looks fine */
		/* Note: this is _not_ dependant on flipping */
		int const color_offs = offs % 32 + ((256 + 8 * (offs / 32) - spriteram[offs % 32] )% 256) / 8 * 32;

		m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
				m_videoram[offs] | (m_colorram[color_offs] >> 5) * 256 | ((m_colorram[color_offs] >> 4) & 1) * 512,
				m_colorram[color_offs]%16,
				flip_screen_x(),flip_screen_y(),
				8*sx,scrolled_y_position);
	}

	// There seem to be two sets of sprites, each decoded identically

	// This is the first set of 7 sprites
	for (int offs = 0x0084; offs < 0x00a0; offs += 4)
	{
		if (BIT(spriteram[offs + 1], 4))
		{
			int flipx = BIT(spriteram[offs], 6);
			int flipy = BIT(spriteram[offs], 7);

			int sx = spriteram[offs + 3];
			if (flip_screen_x())
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			int sy = 0xef - spriteram[offs + 2];
			if (flip_screen_y())
			{
				flipy = !flipy;
				sy = 238 - sy;
			}

			m_gfxdecode->gfx(1+((spriteram[offs+1]&0x20)>>5))->transpen(bitmap,cliprect,
					spriteram[offs]&0x3f,
					spriteram[offs+1]%16,
					flipx,flipy,
					sx,sy,0);
		}
	}

	// This is the second set of 8 sprites
	for (int offs = 0x00c4; offs < 0x00e4; offs += 4)
	{
		if (BIT(spriteram[offs + 1], 4))
		{
			int flipx = BIT(spriteram[offs], 6);
			int flipy = BIT(spriteram[offs], 7);

			int sx = spriteram[offs + 3];
			if (flip_screen_x())
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			int sy = 0xef - spriteram[offs + 2];
			if (flip_screen_y())
			{
				flipy = !flipy;
				sy = 238 - sy;
			}

			m_gfxdecode->gfx(1+((spriteram[offs+1]&0x20)>>5))->transpen(bitmap,cliprect,
					spriteram[offs]&0x3f,
					spriteram[offs+1]%16,
					flipx,flipy,
					sx,sy,0);
		}
	}

	return 0;
}
