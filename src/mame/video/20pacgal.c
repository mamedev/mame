// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "includes/20pacgal.h"


#define SCREEN_HEIGHT   (224)
#define SCREEN_WIDTH    (288)
#define NUM_PENS        (0x1000)
#define NUM_STAR_PENS   (64)


/*************************************
 *
 *  Palette handling
 *
 *************************************/

void _20pacgal_state::get_pens(pen_t *pens)
{
	offs_t offs;
	UINT8 *color_prom = memregion("proms")->base() + (NUM_PENS * m_game_selected);

	for (offs = 0; offs < NUM_PENS ;offs++)
	{
		int bit0,bit1,bit2,r,g,b;

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

		pens[offs] = rgb_t(r, g, b);

		color_prom++;
	}
	/* Star field */

	/* palette for the stars */
	for (offs = 0;offs < 64;offs++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		bits = (offs >> 0) & 0x03;
		r = map[bits];
		bits = (offs >> 2) & 0x03;
		g = map[bits];
		bits = (offs >> 4) & 0x03;
		b = map[bits];

		pens[NUM_PENS + offs] = rgb_t(r, g, b);
	}
}


void _20pacgal_state::do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y, x;
	pen_t pens[NUM_PENS + NUM_STAR_PENS];

	get_pens(pens);

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		for(x = cliprect.min_x; x <= cliprect.max_x; x++)
			bitmap.pix32(y, x) = pens[bitmap.pix32(y, x)];
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void _20pacgal_state::draw_sprite(bitmap_rgb32 &bitmap, int y, int x,
						UINT8 code, UINT8 color, int flip_y, int flip_x)
{
	int sy;

	offs_t pen_base = (color & 0x1f) << 2;
	pen_base += m_sprite_pal_base;

	if (flip_y)
		y = y + 0x0f;

	if (flip_x)
		x = x + 0x0f;

	/* for each row in the sprite */
	for (sy = 0; sy < 0x10; sy++)
	{
		int x_sav = x;

		if ((y >= 0) && (y < SCREEN_HEIGHT))
		{
			int sx;
			UINT32 data;

			offs_t gfx_offs = ((code & 0x7f) << 6) | (sy << 2);

			/* address mangling */
			gfx_offs = (gfx_offs & 0x1f83) | ((gfx_offs & 0x003c) << 1) | ((gfx_offs & 0x0040) >> 4);

			data = (m_sprite_gfx_ram[gfx_offs + 0] << 24) |
					(m_sprite_gfx_ram[gfx_offs + 1] << 16) |
					(m_sprite_gfx_ram[gfx_offs + 2] << 8) |
					(m_sprite_gfx_ram[gfx_offs + 3] << 0);

			/* for each pixel in the row */
			for (sx = 0; sx < 0x10; sx++)
			{
				if ((x >= 0) && (x < SCREEN_WIDTH))
				{
					offs_t pen = (data & 0xc0000000) >> 30;
					UINT8 col;

					col = m_sprite_color_lookup[pen_base | pen] & 0x0f;

					/* pen bits A0-A3 */
					if (col)
						bitmap.pix32(y, x) = (bitmap.pix32(y, x) & 0xff0) | col;
				}

				/* next pixel */
				if (flip_x)
					x = x - 1;
				else
					x = x + 1;

				data = data << 2;
			}
		}

		/* next row */
		if (flip_y)
			y = y - 1;
		else
			y = y + 1;

		x = x_sav;
	}
}


void _20pacgal_state::draw_sprites(bitmap_rgb32 &bitmap)
{
	int offs;

	for (offs = 0x80 - 2; offs >= 0; offs -= 2)
	{
		static const int code_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int x, y;

		UINT8 code = m_sprite_ram[offs + 0x000];
		UINT8 color = m_sprite_ram[offs + 0x001];

		int sx = m_sprite_ram[offs + 0x081] - 41 + 0x100*(m_sprite_ram[offs + 0x101] & 3);
		int sy = 256 - m_sprite_ram[offs + 0x080] + 1;

		int flip_x = (m_sprite_ram[offs + 0x100] & 0x01) >> 0;
		int flip_y = (m_sprite_ram[offs + 0x100] & 0x02) >> 1;
		int size_x = (m_sprite_ram[offs + 0x100] & 0x04) >> 2;
		int size_y = (m_sprite_ram[offs + 0x100] & 0x08) >> 3;

		sy = sy - (16 * size_y);
		sy = (sy & 0xff) - 32;  /* fix wraparound */

		/* only Galaga appears to be effected by the global flip state */
		if (m_game_selected && (m_flip[0] & 0x01))
		{
			flip_x = !flip_x;
			flip_y = !flip_y;
		}

		for (y = 0; y <= size_y; y++)
			for (x = 0; x <= size_x; x++)
				draw_sprite(bitmap,
							sy + (16 * y), sx + (16 * x),
							code + code_offs[y ^ (size_y * flip_y)][x ^ (size_x * flip_x)],
							color,
							flip_y, flip_x);
	}
}



/*************************************
 *
 *  Character map drawing
 *
 *************************************/

void _20pacgal_state::draw_chars(bitmap_rgb32 &bitmap)
{
	offs_t offs;

	int flip = m_flip[0] & 0x01;

	/* for each byte in the video RAM */
	for (offs = 0; offs < 0x400; offs++)
	{
		int sy;
		int y, x;

		UINT8 *gfx = &m_char_gfx_ram.target()[m_video_ram[0x0000 | offs] << 4];
		UINT32 color_base = (m_video_ram[0x0400 | offs] & 0x3f) << 2;

		/* map the offset to (x, y) character coordinates */
		if ((offs & 0x03c0) == 0)
		{
			y = (offs & 0x1f) - 2;
			x = (offs >> 5) + 34;
		}
		else if ((offs & 0x03c0) == 0x3c0)
		{
			y = (offs & 0x1f) - 2;
			x = (offs >> 5) - 30;
		}
		else
		{
			y = (offs >> 5) - 2;
			x = (offs & 0x1f) + 2;
		}

		if ((y < 0) || (y > 27)) continue;

		/* conver to pixel coordinates */
		y = y << 3;
		x = x << 3;

		if (flip)
		{
			y = SCREEN_HEIGHT - 1 - y;
			x = SCREEN_WIDTH - 1 - x;
		}

		/* for each row in the character */
		for (sy = 0; sy < 8; sy++)
		{
			int sx;
			int x_sav = x;

			UINT16 data = (gfx[8] << 8) | gfx[0];

			/* for each pixel in the row */
			for (sx = 0; sx < 8; sx++)
			{
				UINT32 col = ((data & 0x8000) >> 14) | ((data & 0x0800) >> 11);

				/* pen bits A4-A11 */
				if ( col != 0 )
					bitmap.pix32(y, x) = (color_base | col) << 4;

				/* next pixel */
				if (flip)
					x = x - 1;
				else
					x = x + 1;

				if (sx == 0x03)
					data = data << 5;
				else
					data = data << 1;
			}

			/* next row */
			if (flip)
				y = y - 1;
			else
				y = y + 1;

			x = x_sav;

			gfx = gfx + 1;
		}
	}
}

/*************************************
 *
 *  Draw stars
 *
 *************************************/

/* starfield lfsr code is at d616 and at d648
 *
 * Code at d616:
 *
 * bit 6 (0x40) from 4be4 (port 8A) is rotated into top bit of 586D/586C
 * Based on bit 4 (0x10) and the carry out , bit 6 of 4be4 is set to:
 *
 * Carry out   bit 4    Result
 *    1          1         1
 *    1          0         0
 *    0          1         0
 *    0          0         1
 *
 * Code at d648 is opposite direction.
 *
 * The two videos show, that the starfield is covering the whole screen.
 * Galaga is different, the 256H signal is delivered with 12 pixels
 * delay ( NAND of 1H,2H,4H) to the starfield generator 05xx.
 *
 *    http://www.youtube.com/watch?v=c1zIitLkpGs
 *
 *    http://www.youtube.com/watch?v=bS2Zcin6OwM&feature=PlayList&p=649FD471A1803ABB&playnext_from=PL&playnext=1&index=8
 *
 * Galaga (for comparison)
 *
 *    http://www.youtube.com/watch?v=-R4CCB2g5bE
 *
 * The typical lfsr star count pattern is
 *
 * 132 stars
 * 125 stars
 * 129 stars
 * 68 stars
 * 132 stars
 * 125 stars
 * 129 stars
 * 68 stars
 *
 * This is close to Galaga's 63/126
 *
 */

void _20pacgal_state::draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	if ( (m_stars_ctrl[0] >> 5) & 1 )
	{
		int clock;
		UINT16 lfsr =   m_stars_seed[0] + m_stars_seed[1]*256;
		UINT8 feedback = (m_stars_ctrl[0] >> 6) & 1;
		UINT16 star_seta = (m_stars_ctrl[0] >> 3) & 0x01;
		UINT16 star_setb = (m_stars_ctrl[0] >> 3) & 0x02;
		int cnt = 0;

		/* This is a guess based on galaga star sets */
		star_seta = 0x3fc0 | (star_seta << 14);
		star_setb = 0x3fc0 | (star_setb << 14);


		for (clock=0; clock < 288*224; clock++)
		{
			int x, y;
			int carryout;

			x = clock % 288;
			y = clock / 288;

			/* code at d616 translates into:
			 * carryout = lfsr & 1;
			 * lfsr = lfsr>>1;
			 * lfsr = (feedback << 15) | lfsr;
			 * feedback = (((lfsr>>4) & 1) ^ (carryout & 1)) ^ 1;
			 *
			 * and needs a Hack:
			 *  x = 288 - x;
			 *
			 */

			/* code at d648 */
			carryout = ((lfsr >> 4) ^ feedback ^ 1) & 1;
			feedback = (lfsr >> 15) & 1;
			lfsr = (lfsr << 1) | carryout;

			if (((lfsr & 0xffc0) == star_seta) || ((lfsr & 0xffc0) == star_setb))
			{
				if (y >= cliprect.min_y && y <= cliprect.max_y)
					bitmap.pix32(y, x) = NUM_PENS + (lfsr & 0x3f);
				cnt++;
			}
		}
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 _20pacgal_state::screen_update_20pacgal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_stars(bitmap,cliprect);
	draw_chars(bitmap);
	draw_sprites(bitmap);
	do_pen_lookup(bitmap, cliprect);

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( 20pacgal_video )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(SCREEN_WIDTH, SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT - 1)
	MCFG_SCREEN_UPDATE_DRIVER(_20pacgal_state, screen_update_20pacgal)
MACHINE_CONFIG_END
