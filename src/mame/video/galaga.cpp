// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/galaga.h"


#define STARS_COLOR_BASE (64*4+64*4)

/***************************************************************************

  Convert the color PROMs.

  Galaga has one 32x8 palette PROM and two 256x4 color lookup table PROMs
  (one for characters, one for sprites). Only the first 128 bytes of the
  lookup tables seem to be used.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(galaga_state,galaga)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i;

	/* core palette */
	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = ((*color_prom) >> 0) & 0x01;
		bit1 = ((*color_prom) >> 1) & 0x01;
		bit2 = ((*color_prom) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = ((*color_prom) >> 3) & 0x01;
		bit1 = ((*color_prom) >> 4) & 0x01;
		bit2 = ((*color_prom) >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = ((*color_prom) >> 6) & 0x01;
		bit2 = ((*color_prom) >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i,rgb_t(r,g,b));
		color_prom++;
	}

	/* palette for the stars */
	for (i = 0;i < 64;i++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];

		palette.set_indirect_color(32 + i,rgb_t(r,g,b));
	}

	/* characters */
	for (i = 0;i < 64*4;i++)
		palette.set_pen_indirect(i, (*(color_prom++) & 0x0f) + 0x10);   /* chars */

	/* sprites */
	for (i = 0;i < 64*4;i++)
		palette.set_pen_indirect(64*4+i, (*(color_prom++) & 0x0f));

	/* now the stars */
	for (i = 0;i < 64;i++)
		palette.set_pen_indirect(64*4+64*4+i, 32 + i);

	/*
	  Galaga star line and pixel locations pulled directly from
	  a clocked stepping of the 05 starfield. The chip was clocked
	  on a test rig with hblank and vblank simulated, each X & Y
	  location of a star being recorded along with it's color value.

	  The lookup table is generated using a reverse engineered
	  linear feedback shift register + XOR boolean expression.

	  Because the starfield begins generating stars at the point
	  in time it's enabled the exact horiz location of the stars
	  on Galaga depends on the length of time of the POST for the
	  original board.

	  Two control bits determine which of two sets are displayed
	  set 0 or 1 and simultaneously 2 or 3.

	  There are 63 stars in each set, 126 displayed at any one time
	*/

	const uint16_t feed = 0x9420;

	int idx = 0;
	for (uint16_t sf = 0; sf < 4; ++sf)
	{
		// starfield select flags
		uint16_t sf1 = (sf >> 1) & 1;
		uint16_t sf2 = sf & 1;

		uint16_t i = 0x70cc;
		for (int cnt = 0; cnt < 65535; ++cnt)
		{
			// output enable lookup
			uint16_t xor1 = i ^ (i >> 3);
			uint16_t xor2 = xor1 ^ (i >> 2);
			uint16_t oe = (sf1 ? 0 : 0x4000) | ((sf1 ^ sf2) ? 0 : 0x1000);
			if ((i & 0x8007) == 0x8007
			    && (~i & 0x2008) == 0x2008
			    && (xor1 & 0x0100) == (sf1 ? 0 : 0x0100)
			    && (xor2 & 0x0040) == (sf2 ? 0 : 0x0040)
			    && (i & 0x5000) == oe
			    && cnt >= 256 * 4)
			{
				// color lookup
				uint16_t xor3 = (i >> 1) ^ (i >> 6);
				uint16_t clr =
					(((i >> 9) & 0x07)
					 | ((xor3 ^ (i >> 4) ^ (i >> 7)) & 0x08)
					 | (~xor3 & 0x10)
					 | (((i >> 2) ^ (i >> 5)) & 0x20))
					^ ((i & 0x4000) ? 0 : 0x24)
					^ ((((i >> 2) ^ i) & 0x1000) ? 0x21 : 0);

				m_star_seed_tab[idx].x = cnt % 256;
				m_star_seed_tab[idx].y = cnt / 256;
				m_star_seed_tab[idx].col = clr;
				m_star_seed_tab[idx].set = sf;
				++idx;
			}

			// update the LFSR
			if (i & 1)
				i = (i >> 1) ^ feed;
			else
				i = (i >> 1);
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(galaga_state::tilemap_scan)
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


TILE_GET_INFO_MEMBER(galaga_state::get_tile_info)
{
	/* the hardware has two character sets, one normal and one x-flipped. When
	   screen is flipped, character y flip is done by the hardware inverting the
	   timing signals, while x flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	int color = m_videoram[tile_index + 0x400] & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			(m_videoram[tile_index] & 0x7f) | (flip_screen() ? 0x80 : 0) | (m_galaga_gfxbank << 8),
			color,
			flip_screen() ? TILE_FLIPX : 0);
	tileinfo.group = color;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(galaga_state,galaga)
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(galaga_state::get_tile_info),this),tilemap_mapper_delegate(FUNC(galaga_state::tilemap_scan),this),8,8,36,28);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);

	m_galaga_gfxbank = 0;

	save_item(NAME(m_stars_scrollx));
	save_item(NAME(m_stars_scrolly));
	save_item(NAME(m_galaga_gfxbank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/


WRITE8_MEMBER(galaga_state::galaga_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE_LINE_MEMBER(galaga_state::gatsbee_bank_w)
{
	m_galaga_gfxbank = state;
	m_fg_tilemap->mark_all_dirty();
}



/***************************************************************************

  Display refresh

***************************************************************************/

void galaga_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = m_galaga_ram1 + 0x380;
	uint8_t *spriteram_2 = m_galaga_ram2 + 0x380;
	uint8_t *spriteram_3 = m_galaga_ram3 + 0x380;
	int offs;


	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs] & 0x7f;
		int color = spriteram[offs+1] & 0x3f;
		int sx = spriteram_2[offs+1] - 40 + 0x100*(spriteram_3[offs+1] & 3);
		int sy = 256 - spriteram_2[offs] + 1;   // sprites are buffered and delayed by one scanline
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizex = (spriteram_3[offs] & 0x04) >> 2;
		int sizey = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;  // fix wraparound

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x, sy + 16*y,
					m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x0f));
			}
		}
	}
}


void galaga_state::draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* draw the stars */

	/* $a005 controls the stars ON/OFF */
	if ( m_videolatch->q5_r() == 1 )
	{
		int star_cntr;
		int set_a, set_b;

		/* two sets of stars controlled by these bits */
		set_a = m_videolatch->q3_r();
		set_b = m_videolatch->q4_r() | 2;

		for (star_cntr = 0;star_cntr < MAX_STARS ;star_cntr++)
		{
			int x,y;

			if ( (set_a == m_star_seed_tab[star_cntr].set) || ( set_b == m_star_seed_tab[star_cntr].set) )
			{
				x = (m_star_seed_tab[star_cntr].x + m_stars_scrollx) % 256 + 16;
				y = (112 + m_star_seed_tab[star_cntr].y + m_stars_scrolly) % 256;
				/* 112 is a tweak to get alignment about perfect */

				if (cliprect.contains(x, y))
					bitmap.pix16(y, x) = STARS_COLOR_BASE + m_star_seed_tab[ star_cntr ].col;
			}

		}
	}
}

uint32_t galaga_state::screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	draw_stars(bitmap,cliprect);
	draw_sprites(bitmap,cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}



WRITE_LINE_MEMBER(galaga_state::screen_vblank_galaga)
{
	// falling edge
	if (!state)
	{
		/* this function is called by galaga_interrupt_1() */
		int s0,s1,s2;
		static const int speeds[8] = { -1, -2, -3, 0, 3, 2, 1, 0 };

		s0 = m_videolatch->q0_r();
		s1 = m_videolatch->q1_r();
		s2 = m_videolatch->q2_r();

		m_stars_scrollx += speeds[s0 + s1*2 + s2*4];
	}
}
