// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Lee Taylor
/***************************************************************************

 cosmic.c

 emulation of video hardware of cosmic machines of 1979-1980(ish)

***************************************************************************/

#include "emu.h"
#include "includes/cosmic.h"


WRITE8_MEMBER(cosmic_state::cosmic_color_register_w)
{
	m_color_registers[offset] = data ? 1 : 0;
}


pen_t cosmic_state::panic_map_color( uint8_t x, uint8_t y )
{
	offs_t offs = (m_color_registers[0] << 9) | (m_color_registers[2] << 10) | ((x >> 4) << 5) | (y >> 3);
	pen_t pen = memregion("user1")->base()[offs];

	if (m_color_registers[1])
		pen >>= 4;

	return pen & 0x0f;
}

pen_t cosmic_state::cosmica_map_color( uint8_t x, uint8_t y )
{
	offs_t offs = (m_color_registers[0] << 9) | ((x >> 4) << 5) | (y >> 3);
	pen_t pen = memregion("user1")->base()[offs];

	if (m_color_registers[1]) // 0 according to the schematics, but that breaks alien formation colors
		pen >>= 4;

	return pen & 0x07;
}

pen_t cosmic_state::cosmicg_map_color( uint8_t x, uint8_t y )
{
	offs_t offs = (m_color_registers[0] << 8) | (m_color_registers[1] << 9) | ((y >> 4) << 4) | (x >> 4);
	pen_t pen = memregion("user1")->base()[offs];

	/* the upper 4 bits are for cocktail mode support */
	return pen & 0x0f;
}

pen_t cosmic_state::magspot_map_color( uint8_t x, uint8_t y )
{
	offs_t offs = (m_color_registers[0] << 9) | ((x >> 3) << 4) | (y >> 4);
	pen_t pen = memregion("user1")->base()[offs];

	if (m_color_registers[1])
		pen >>= 4;

	return pen & m_magspot_pen_mask;
}



/*
 * Panic Color table setup
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 * First 8 colors are normal intensities
 *
 * But, bit 3 can be used to pull Blue via a 2k resistor to 5v
 * (1k to ground) so second version of table has blue set to 2/3
 */

void cosmic_state::panic_palette(palette_device &palette)
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x10; i++)
	{
		int const r = pal1bit(i >> 0);
		int const g = pal1bit(i >> 1);
		int const b = ((i & 0x0c) == 0x08) ? 0xaa : pal1bit(i >> 2);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// background uses colors 0x00-0x0f
	for (int i = 0; i < 0x0f; i++)
		palette.set_pen_indirect(i, i);

	// sprites use colors 0x00-0x07
	for (int i = 0; i < 0x20; i++)
		palette.set_pen_indirect(i + 0x10, color_prom[i] & 0x07);

	m_map_color = &cosmic_state::panic_map_color;
}


/*
 * Cosmic Alien Color table setup
 *
 * 8 colors, 16 sprite color codes
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 */

void cosmic_state::cosmica_palette(palette_device &palette)
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x08; i++)
		palette.set_indirect_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));

	// background and sprites use colors 0x00-0x07
	for (int i = 0; i < 0x08; i++)
		palette.set_pen_indirect(i, i);

	for (int i = 0; i < 0x20; i++)
	{
		palette.set_pen_indirect(i + 0x08, (color_prom[i] >> 0) & 0x07);
		palette.set_pen_indirect(i + 0x28, (color_prom[i] >> 4) & 0x07);
	}

	m_map_color = &cosmic_state::cosmica_map_color;
}


/*
 * Cosmic guerilla table setup
 *
 * Use AA for normal, FF for Full Red
 * Bit 0 = R, bit 1 = G, bit 2 = B, bit 4 = High Red
 *
 * It's possible that the background is dark gray and not black, as the
 * resistor chain would never drop to zero, Anybody know ?
 */
void cosmic_state::cosmicg_palette(palette_device &palette)
{
	for (int i = 0; i < palette.entries(); i++)
	{
		int const r = (i > 8) ? 0xff : 0xaa * BIT(i, 0);
		int const g = 0xaa * BIT(i, 1);
		int const b = 0xaa * BIT(i, 2);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	m_map_color = &cosmic_state::cosmicg_map_color;
}


void cosmic_state::magspot_palette(palette_device &palette)
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x10; i++)
	{
		int const r = ((i & 0x09) == 0x08) ? 0xaa : pal1bit(i >> 0);
		int const g = pal1bit(i >> 1);
		int const b = pal1bit(i >> 2);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// background uses colors 0x00-0x0f
	for (int i = 0; i < 0x0f; i++)
		palette.set_pen_indirect(i, i);

	// sprites use colors 0x00-0x0f
	for (int i = 0; i < 0x20; i++)
		palette.set_pen_indirect(i + 0x10, color_prom[i] & 0x0f);

	m_map_color = &cosmic_state::magspot_map_color;
	m_magspot_pen_mask = 0x0f;
}


void cosmic_state::nomnlnd_palette(palette_device &palette)
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x10; i++)
	{
		rgb_t const color = rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
		palette.set_indirect_color(i, color);
	}

	// background uses colors 0x00-0x07
	for (int i = 0; i < 0x07; i++)
		palette.set_pen_indirect(i, i);

	// sprites use colors 0x00-0x07 */
	for (int i = 0; i < 0x20; i++)
		palette.set_pen_indirect(i + 0x10, color_prom[i] & 0x07);

	m_map_color = &cosmic_state::magspot_map_color;
	m_magspot_pen_mask = 0x07;
}


WRITE8_MEMBER(cosmic_state::cosmic_background_enable_w)
{
	m_background_enable = data;
}


void cosmic_state::draw_bitmap( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t data = m_videoram[offs];

		uint8_t x = offs << 3;
		uint8_t const y = offs >> 5;

		pen_t pen = (this->*m_map_color)(x, y);

		for (int i = 0; i < 8; i++)
		{
			if (data & 0x80)
			{
				if (flip_screen())
					bitmap.pix16(255-y, 255-x) = pen;
				else
					bitmap.pix16(y, x) = pen;
			}

			x++;
			data <<= 1;
		}
	}
}


void cosmic_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int extra_sprites )
{
	int offs;

	for (offs = m_spriteram.bytes() - 4;offs >= 0;offs -= 4)
	{
		if (m_spriteram[offs] != 0)
		{
			int code, color;

			code  = ~m_spriteram[offs] & 0x3f;
			color = ~m_spriteram[offs + 3] & color_mask;

			if (extra_sprites)
				code |= (m_spriteram[offs + 3] & 0x08) << 3;

			if (m_spriteram[offs] & 0x80)
				/* 16x16 sprite */
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code, color,
						0, ~m_spriteram[offs] & 0x40,
						256-m_spriteram[offs + 2],m_spriteram[offs + 1],0);
			else
				/* 32x32 sprite */
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code >> 2, color,
						0, ~m_spriteram[offs] & 0x40,
						256-m_spriteram[offs + 2],m_spriteram[offs + 1],0);
		}
	}
}


void cosmic_state::cosmica_draw_starfield( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t y = 0;
	uint8_t map = 0;
	uint8_t *PROM = memregion("user2")->base();

	while (1)
	{
		int va  =  y       & 0x01;
		int vb  = (y >> 1) & 0x01;

		uint8_t x = 0;

		while (1)
		{
			uint8_t x1;
			int hc, hb_;

			if (flip_screen())
				x1 = x - screen.frame_number();
			else
				x1 = x + screen.frame_number();

			hc  = (x1 >> 2) & 0x01;
			hb_ = (x  >> 5) & 0x01;  /* not a bug, this one is the real x */

			if ((x1 & 0x1f) == 0)
				// flip-flop at IC11 is clocked
				map = PROM[(x1 >> 5) | (y >> 1 << 3)];

			if (((!(hc & va)) & (vb ^ hb_)) &&          /* right network */
				(((x1 ^ map) & (hc | 0x1e)) == 0x1e))   /* left network */
			{
				/* RGB order is reversed -- bit 7=R, 6=G, 5=B */
				int col = (map >> 7) | ((map >> 5) & 0x02) | ((map >> 3) & 0x04);

				bitmap.pix16(y, x) = col;
			}

			x++;
			if (x == 0)  break;
		}

		y++;
		if (y == 0)  break;
	}
}


void cosmic_state::devzone_draw_grid( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t y;
	uint8_t *horz_PROM = memregion("user2")->base();
	uint8_t *vert_PROM = memregion("user3")->base();
	offs_t horz_addr = 0;

	uint8_t count = 0;
	uint8_t horz_data = 0;
	uint8_t vert_data;

	for (y = 32; y < 224; y++)
	{
		uint8_t x = 0;

		while (1)
		{
			int x1;

			/* for the vertical lines, each bit indicates
			   if there should be a line at the x position */
			vert_data = vert_PROM[x >> 3];

			/* the horizontal (perspective) lines are RLE encoded.
			   When the screen is flipped, the address should be
			   decrementing.  But since it's just a mirrored image,
			   this is easier. */
			if (count == 0)
				count = horz_PROM[horz_addr++];

			count++;

			if (count == 0)
				horz_data = horz_PROM[horz_addr++];

			for (x1 = 0; x1 < 8; x1++)
			{
				if (!(vert_data & horz_data & 0x80))    /* NAND gate */
				{
					/* blue */
					if (flip_screen())
						bitmap.pix16(255-y, 255-x) = 4;
					else
						bitmap.pix16(y, x) = 4;
				}

				horz_data = (horz_data << 1) | 0x01;
				vert_data = (vert_data << 1) | 0x01;

				x++;
			}

			if (x == 0)  break;
		}
	}
}


void cosmic_state::nomnlnd_draw_background( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t y = 0;
	uint8_t water = screen.frame_number();
	uint8_t *PROM = memregion("user2")->base();

	/* all positioning is via logic gates:

	   tree is displayed where
	   __          __
	   HD' ^ HC' ^ HB'

	   and
	    __          __              __
	   (VB' ^ VC' ^ VD')  X  (VB' ^ VC' ^ VD')

	   water is displayed where
	         __         __
	   HD' ^ HC' ^ HB ^ HA'

	   and vertically the same equation as the trees,
	   but the final result is inverted.


	   The colors are coded in logic gates:

	   trees:
	                            P1 P2  BGR
	     R = Plane1 ^ Plane2     0  0  000
	     G = Plane2              0  1  010
	     B = Plane1 ^ ~Plane2    1  0  100
	                             1  1  011
	   water:
	                            P1 P2  BGR or
	     R = Plane1 ^ Plane2     0  0  100 000
	     G = Plane2 v Plane2     0  1  110 010
	     B = ~Plane1 or          1  0  010 010
	         0 based oh HD       1  1  011 011

	     Not sure about B, the logic seems convulated for such
	     a simple result.
	*/

	while (1)
	{
		int vb_ = (y >> 5) & 0x01;
		int vc_ = (y >> 6) & 0x01;
		int vd_ =  y >> 7;

		uint8_t x = 0;

		while (1)
		{
			int color = 0;

			int hd  = (x >> 3) & 0x01;
			int ha_ = (x >> 4) & 0x01;
			int hb_ = (x >> 5) & 0x01;
			int hc_ = (x >> 6) & 0x01;
			int hd_ =  x >> 7;

			if (((!vb_) & vc_ & (!vd_)) ^ (vb_ & (!vc_) & vd_))
			{
				/* tree */
				if (!hd_ && hc_ && !hb_)
				{
					offs_t offs = ((x >> 3) & 0x03) | ((y & 0x1f) << 2) |
									(flip_screen() ? 0x80 : 0);

					uint8_t plane1 = PROM[offs         ] << (x & 0x07);
					uint8_t plane2 = PROM[offs | 0x0400] << (x & 0x07);

					plane1 >>= 7;
					plane2 >>= 7;

					color = (plane1 & plane2)       |   // R
							(plane2         )  << 1 |   // G
							(plane1 & ~plane2) << 2;    // B
				}
			}
			else
			{
				/* water */
				if (hd_ && !hc_ && hb_ && !ha_)
				{
					offs_t offs = hd | (water << 1) | 0x0200;

					uint8_t plane1 = PROM[offs         ] << (x & 0x07);
					uint8_t plane2 = PROM[offs | 0x0400] << (x & 0x07);

					plane1 >>= 7;
					plane2 >>= 7;

					color = ( plane1 & plane2)      |   // R
							( plane1 | plane2) << 1 |   // G
							((!plane1) & hd)     << 2;  // B - see above
				}
			}

			if (color != 0)
			{
				if (flip_screen())
					bitmap.pix16(255-y, 255-x) = color;
				else
					bitmap.pix16(y, x) = color;
			}

			x++;
			if (x == 0)  break;
		}


		// this is obviously wrong
//      if (vb_)
			water++;

		y++;
		if (y == 0)  break;
	}
}


uint32_t cosmic_state::screen_update_cosmicg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_bitmap(bitmap, cliprect);
	return 0;
}


uint32_t cosmic_state::screen_update_panic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0x07, 1);
	return 0;
}


uint32_t cosmic_state::screen_update_cosmica(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	cosmica_draw_starfield(screen, bitmap, cliprect);
	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0x0f, 0);
	return 0;
}


uint32_t cosmic_state::screen_update_magspot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0x07, 0);
	return 0;
}


uint32_t cosmic_state::screen_update_devzone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (m_background_enable)
		devzone_draw_grid(bitmap, cliprect);

	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0x07, 0);
	return 0;
}


uint32_t cosmic_state::screen_update_nomnlnd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* according to the video summation logic on pg4, the trees and river
	   have the highest priority */

	bitmap.fill(0, cliprect);
	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0x07, 0);

	if (m_background_enable)
		nomnlnd_draw_background(screen, bitmap, cliprect);

	return 0;
}
