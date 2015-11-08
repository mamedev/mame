// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria
#include "emu.h"
#include "includes/polepos.h"


/* modified vertical position built from three nibbles (12 bit)
 * of ROMs 136014-142, 136014-143, 136014-144
 * The value RVP (road vertical position, lower 12 bits) is added
 * to this value and the upper 10 bits of the result are used to
 * address the playfield video memory (AB0 - AB9).
 */


/***************************************************************************

  Convert the color PROMs.

  Pole Position has three 256x4 palette PROMs (one per gun)
  and a lot ;-) of 256x4 lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(polepos_state,polepos)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i, j;

	/*******************************************************
	 * Color PROMs
	 * Sheet 15B: middle, 136014-137,138,139
	 * Inputs: MUX0 ... MUX3, ALPHA/BACK, SPRITE/BACK, 128V, COMPBLANK
	 *
	 * Note that we only decode the lower 128 colors because
	 * the upper 128 are all black and used during the
	 * horizontal and vertical blanking periods.
	 * The purpose of the 128V input is to use a different palette for the
	 * background and for the road; it is irrelevant for alpha and
	 * sprites because their palette is the same in both halves.
	 * Anyway, we emulate that to a certain extent, using different
	 * colortables for the two halves of the screen. We don't support the
	 * palette change in the middle of a sprite, however.
	 * Also, note that priority encoding is done is such a way that alpha
	 * will use palette bank 2 or 3 depending on whether there is a sprite
	 * below the pixel or not. That would be tricky to emulate, and it's
	 * not needed because of course the two banks are the same.
	 *******************************************************/
	for (i = 0; i < 128; i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* Sheet 15B: 136014-0137 red component */
		bit0 = (color_prom[0x000 + i] >> 0) & 1;
		bit1 = (color_prom[0x000 + i] >> 1) & 1;
		bit2 = (color_prom[0x000 + i] >> 2) & 1;
		bit3 = (color_prom[0x000 + i] >> 3) & 1;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* Sheet 15B: 136014-0138 green component */
		bit0 = (color_prom[0x100 + i] >> 0) & 1;
		bit1 = (color_prom[0x100 + i] >> 1) & 1;
		bit2 = (color_prom[0x100 + i] >> 2) & 1;
		bit3 = (color_prom[0x100 + i] >> 3) & 1;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* Sheet 15B: 136014-0139 blue component */
		bit0 = (color_prom[0x200 + i] >> 0) & 1;
		bit1 = (color_prom[0x200 + i] >> 1) & 1;
		bit2 = (color_prom[0x200 + i] >> 2) & 1;
		bit3 = (color_prom[0x200 + i] >> 3) & 1;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i,rgb_t(r,g,b));
	}

	/*******************************************************
	 * Alpha colors (colors 0x000-0x1ff)
	 * Sheet 15B: top left, 136014-140
	 * Inputs: SHFT0, SHFT1 and CHA8* ... CHA13*
	 *******************************************************/
	for (i = 0; i < 64*4; i++)
	{
		int color = color_prom[0x300 + i];
		palette.set_pen_indirect(0x0000 + i, (color != 15) ? (0x020 + color) : 0x2f);
		palette.set_pen_indirect(0x0100 + i, (color != 15) ? (0x060 + color) : 0x2f);
	}

	/*******************************************************
	 * Background colors (colors 0x200-0x2ff)
	 * Sheet 13A: left, 136014-141
	 * Inputs: SHFT2, SHFT3 and CHA8 ... CHA13
	 * The background is only in the top half of the screen
	 *******************************************************/
	for (i = 0; i < 64*4; i++)
	{
		int color = color_prom[0x400 + i];
		palette.set_pen_indirect(0x0200 + i, 0x000 + color);
	}

	/*******************************************************
	 * Sprite colors (colors 0x300-0xaff)
	 * Sheet 14B: right, 136014-146
	 * Inputs: CUSTOM0 ... CUSTOM3 and DATA0 ... DATA5
	 *******************************************************/
	for (i = 0; i < 64*16; i++)
	{
		int color = color_prom[0xc00 + i];
		palette.set_pen_indirect(0x0300 + i, (color != 15) ? (0x010 + color) : 0x1f);
		palette.set_pen_indirect(0x0700 + i, (color != 15) ? (0x050 + color) : 0x1f);
	}

	/*******************************************************
	 * Road colors (colors 0xb00-0x0eff)
	 * Sheet 13A: bottom left, 136014-145
	 * Inputs: R1 ... R6 and CHA0 ... CHA3
	 * The road is only in the bottom half of the screen
	 *******************************************************/
	for (i = 0; i < 64*16; i++)
	{
		int color = color_prom[0x800 + i];
		palette.set_pen_indirect(0x0b00 + i, 0x040 + color);
	}

	/* 136014-142, 136014-143, 136014-144 Vertical position modifiers */
	for (i = 0; i < 256; i++)
	{
		j = color_prom[0x500 + i] + (color_prom[0x600 + i] << 4) + (color_prom[0x700 + i] << 8);
		m_vertical_position_modifier[i] = j;
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(polepos_state::bg_get_tile_info)
{
	UINT16 word = m_view16_memory[tile_index];
	int code = (word & 0xff) | ((word & 0x4000) >> 6);
	int color = (word & 0x3f00) >> 8;
	SET_TILE_INFO_MEMBER(1,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(polepos_state::tx_get_tile_info)
{
	UINT16 word = m_alpha16_memory[tile_index];
	int code = (word & 0xff) | ((word & 0x4000) >> 6);
	int color = (word & 0x3f00) >> 8;

	/* I assume the purpose of CHACL is to allow the Z80 to control
	   the display (therefore using only the bottom 8 bits of tilemap RAM)
	   in case the Z8002 is not working. */
	if (m_chacl == 0)
	{
		code &= 0xff;
		color = 0;
	}

	/* 128V input to the palette PROM */
	if (tile_index >= 32*16) color |= 0x40;

	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			0);
	tileinfo.group = color;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(polepos_state,polepos)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(polepos_state::bg_get_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,16);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(polepos_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x2f);
}


/***************************************************************************

  Sprite memory

***************************************************************************/

READ16_MEMBER(polepos_state::polepos_sprite16_r)
{
	return m_sprite16_memory[offset];
}

WRITE16_MEMBER(polepos_state::polepos_sprite16_w)
{
	COMBINE_DATA(&m_sprite16_memory[offset]);
}

READ8_MEMBER(polepos_state::polepos_sprite_r)
{
	return m_sprite16_memory[offset] & 0xff;
}

WRITE8_MEMBER(polepos_state::polepos_sprite_w)
{
	m_sprite16_memory[offset] = (m_sprite16_memory[offset] & 0xff00) | data;
}


/***************************************************************************

  Road memory

***************************************************************************/

READ16_MEMBER(polepos_state::polepos_road16_r)
{
	return m_road16_memory[offset];
}

WRITE16_MEMBER(polepos_state::polepos_road16_w)
{
	COMBINE_DATA(&m_road16_memory[offset]);
}

READ8_MEMBER(polepos_state::polepos_road_r)
{
	return m_road16_memory[offset] & 0xff;
}

WRITE8_MEMBER(polepos_state::polepos_road_w)
{
	m_road16_memory[offset] = (m_road16_memory[offset] & 0xff00) | data;
}

WRITE16_MEMBER(polepos_state::polepos_road16_vscroll_w)
{
	COMBINE_DATA(&m_road16_vscroll);
}


/***************************************************************************

  View memory

***************************************************************************/

READ16_MEMBER(polepos_state::polepos_view16_r)
{
	return m_view16_memory[offset];
}

WRITE16_MEMBER(polepos_state::polepos_view16_w)
{
	COMBINE_DATA(&m_view16_memory[offset]);
	if (offset < 0x400)
		m_bg_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(polepos_state::polepos_view_r)
{
	return m_view16_memory[offset] & 0xff;
}

WRITE8_MEMBER(polepos_state::polepos_view_w)
{
	m_view16_memory[offset] = (m_view16_memory[offset] & 0xff00) | data;
	if (offset < 0x400)
		m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(polepos_state::polepos_view16_hscroll_w)
{
	COMBINE_DATA(&m_scroll);
	m_bg_tilemap->set_scrollx(0,m_scroll);
}

WRITE8_MEMBER(polepos_state::polepos_chacl_w)
{
	if (m_chacl != (data & 1))
	{
		m_chacl = data & 1;
		m_tx_tilemap->mark_all_dirty();
	}
}


/***************************************************************************

  Alpha memory

***************************************************************************/

READ16_MEMBER(polepos_state::polepos_alpha16_r)
{
	return m_alpha16_memory[offset];
}

WRITE16_MEMBER(polepos_state::polepos_alpha16_w)
{
	COMBINE_DATA(&m_alpha16_memory[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(polepos_state::polepos_alpha_r)
{
	return m_alpha16_memory[offset] & 0xff;
}

WRITE8_MEMBER(polepos_state::polepos_alpha_w)
{
	m_alpha16_memory[offset] = (m_alpha16_memory[offset] & 0xff00) | data;
	m_tx_tilemap->mark_tile_dirty(offset);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void polepos_state::draw_road(bitmap_ind16 &bitmap)
{
	const UINT8 *road_control = memregion("gfx5")->base();
	const UINT8 *road_bits1 = road_control + 0x2000;
	const UINT8 *road_bits2 = road_control + 0x4000;
	int x, y, i;

	/* loop over the lower half of the screen */
	for (y = 128; y < 256; y++)
	{
		int xoffs, yoffs, xscroll, roadpal;
		UINT16 scanline[256 + 8];
		UINT16 *dest = scanline;
		pen_t pen_base;

		/* first add the vertical position modifier and the vertical scroll */
		yoffs = ((m_vertical_position_modifier[y] + m_road16_vscroll) >> 3) & 0x1ff;

		/* then use that as a lookup into the road memory */
		roadpal = m_road16_memory[yoffs] & 15;

		/* this becomes the palette base for the scanline */
		pen_base = 0x0b00 + (roadpal << 6);

		/* now fetch the horizontal scroll offset for this scanline */
		xoffs = m_road16_memory[0x380 + (y & 0x7f)] & 0x3ff;

		/* the road is drawn in 8-pixel chunks, so round downward and adjust the base */
		/* note that we assume there is at least 8 pixels of slop on the left/right */
		xscroll = xoffs & 7;
		xoffs &= ~7;

		/* loop over 8-pixel chunks */
		for (x = 0; x < 256 / 8 + 1; x++, xoffs += 8)
		{
			/* if the 0x200 bit of the xoffset is set, a special pin on the custom */
			/* chip is set and the /CE and /OE for the road chips is disabled */
			if (xoffs & 0x200)
			{
				/* in this case, it looks like we just fill with 0 */
				for (i = 0; i < 8; i++)
					*dest++ = pen_base | 0;
			}

			/* otherwise, we clock in the bits and compute the road value */
			else
			{
				/* the road ROM offset comes from the current scanline and the X offset */
				int romoffs = ((y & 0x07f) << 6) + ((xoffs & 0x1f8) >> 3);

				/* fetch the current data from the road ROMs */
				int control = road_control[romoffs];
				int bits1 = road_bits1[romoffs];
				int bits2 = road_bits2[(romoffs & 0xfff) | ((romoffs & 0x1000) >> 1)];

				/* extract the road value and the carry-in bit */
				int roadval = control & 0x3f;
				int carin = control >> 7;

				/* draw this 8-pixel chunk */
				for (i = 8; i > 0; i--)
				{
					int bits = BIT(bits1,i) + (BIT(bits2,i) << 1);
					if (!carin && bits) bits++;
					*dest++ = pen_base | (roadval & 0x3f);
					roadval += bits;
				}
			}
		}

		/* draw the scanline */
		draw_scanline16(bitmap, 0, y, 256, &scanline[xscroll], NULL);
	}
}

void polepos_state::zoom_sprite(bitmap_ind16 &bitmap,int big,
		UINT32 code,UINT32 color,int flipx,int sx,int sy,
		int sizex,int sizey)
{
	gfx_element *gfx = m_gfxdecode->gfx(big ? 3 : 2);
	const UINT8 *gfxdata = gfx->get_data(code % gfx->elements());
	UINT8 *scaling_rom = memregion("gfx6")->base();
	UINT32 transmask = m_palette->transpen_mask(*gfx, color, 0x1f);
	int coloroffs = gfx->colorbase() + color * gfx->granularity();
	int x,y;

	if (flipx) flipx = big ? 0x1f : 0x0f;

	for (y = 0;y <= sizey;y++)
	{
		int yy = (sy + y) & 0x1ff;

		/* the following should be a reasonable reproduction of how the real hardware works */
		if (yy >= 0x10 && yy < 0xf0)
		{
			int dy = scaling_rom[(y << 6) + sizey] & 0x1f;
			int xx = sx & 0x3ff;
			int siz = 0;
			int offs = 0;
			const UINT8 *src;

			if (!big) dy >>= 1;
			src = gfxdata + dy * gfx->rowbytes();

			for (x = (big ? 0x40 : 0x20);x > 0;x--)
			{
				if (xx < 0x100)
				{
					int pen = src[offs/2 ^ flipx];

					if (!((transmask >> pen) & 1))
						bitmap.pix16(yy, xx) = pen + coloroffs;
				}
				offs++;

				siz = siz+1+sizex;
				if (siz & 0x40)
				{
					siz &= 0x3f;
					xx = (xx+1) & 0x3ff;
				}
			}
		}
	}
}

void polepos_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 *posmem = &m_sprite16_memory[0x380];
	UINT16 *sizmem = &m_sprite16_memory[0x780];
	int i;

	for (i = 0; i < 64; i++, posmem += 2, sizmem += 2)
	{
		int sx = (posmem[1] & 0x3ff) - 0x40 + 4;
		int sy = 512 - (posmem[0] & 0x1ff) + 1; // sprites are buffered and delayed by one scanline
		int sizex = (sizmem[1] & 0x3f00) >> 8;
		int sizey = (sizmem[0] & 0x3f00) >> 8;
		int code = sizmem[0] & 0x7f;
		int flipx = sizmem[0] & 0x80;
		int color = sizmem[1] & 0x3f;

		/* 128V input to the palette PROM */
		if (sy >= 128) color |= 0x40;

		zoom_sprite(bitmap, (sizmem[0] & 0x8000) ? 1 : 0,
					code,
					color,
					flipx,
					sx, sy,
					sizex,sizey);
	}
}


UINT32 polepos_state::screen_update_polepos(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip = cliprect;
	clip.max_y = 127;
	m_bg_tilemap->draw(screen, bitmap, clip, 0,0);
	draw_road(bitmap);
	draw_sprites(bitmap,cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
