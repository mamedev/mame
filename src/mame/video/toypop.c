// license:???
// copyright-holders:Edgardo E. Contini Salvan
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/toypop.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  toypop has three 256x4 palette PROM and two 256x8 color lookup table PROMs
  (one for characters, one for sprites).

***************************************************************************/

PALETTE_INIT_MEMBER(toypop_state, toypop)
{
	const UINT8 *color_prom = memregion("proms")->base();

	for (int i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		// red component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = (color_prom[i+0x100] >> 0) & 0x01;
		bit1 = (color_prom[i+0x100] >> 1) & 0x01;
		bit2 = (color_prom[i+0x100] >> 2) & 0x01;
		bit3 = (color_prom[i+0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = (color_prom[i+0x200] >> 0) & 0x01;
		bit1 = (color_prom[i+0x200] >> 1) & 0x01;
		bit2 = (color_prom[i+0x200] >> 2) & 0x01;
		bit3 = (color_prom[i+0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r,g,b));
	}

	for (int i = 0;i < 256;i++)
	{
		UINT8 entry;

		// characters
		palette.set_pen_indirect(i + 0*256, (color_prom[i + 0x300] & 0x0f) | 0x70);
		palette.set_pen_indirect(i + 1*256, (color_prom[i + 0x300] & 0x0f) | 0xf0);
		// sprites
		entry = color_prom[i + 0x500];
		palette.set_pen_indirect(i + 2*256, entry);
	}
	for (int i = 0;i < 16;i++)
	{
		// background
		palette.set_pen_indirect(i + 3*256 + 0*16, 0x60 + i);
		palette.set_pen_indirect(i + 3*256 + 1*16, 0xe0 + i);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(toypop_state::tilemap_scan)
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

TILE_GET_INFO_MEMBER(toypop_state::get_tile_info)
{
	UINT8 attr = m_videoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index],
			(attr & 0x3f) + 0x40 * m_palettebank,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void toypop_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toypop_state::get_tile_info),this), tilemap_mapper_delegate(FUNC(toypop_state::tilemap_scan),this),8,8,36,28);

	m_bg_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(toypop_state::toypop_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(toypop_state::toypop_palettebank_w)
{
	if (m_palettebank != (offset & 1))
	{
		m_palettebank = offset & 1;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE16_MEMBER(toypop_state::toypop_flipscreen_w)
{
	m_bitmapflip = offset & 1;
}

READ16_MEMBER(toypop_state::toypop_merged_background_r)
{
	int data1, data2;

	// 0x0a0b0c0d is read as 0xabcd
	data1 = m_bg_image[2*offset];
	data2 = m_bg_image[2*offset + 1];
	return ((data1 & 0xf00) << 4) | ((data1 & 0xf) << 8) | ((data2 & 0xf00) >> 4) | (data2 & 0xf);
}

WRITE16_MEMBER(toypop_state::toypop_merged_background_w)
{
	// 0xabcd is written as 0x0a0b0c0d in the background image
	if (ACCESSING_BITS_8_15)
		m_bg_image[2*offset] = ((data & 0xf00) >> 8) | ((data & 0xf000) >> 4);

	if (ACCESSING_BITS_0_7)
		m_bg_image[2*offset+1] = (data & 0xf) | ((data & 0xf0) << 4);
}

void toypop_state::draw_background(bitmap_ind16 &bitmap)
{
	pen_t pen_base = 0x300 + 0x10*m_palettebank;

	// copy the background image from RAM (0x190200-0x19FDFF) to bitmap
	if (m_bitmapflip)
	{
		int offs = 0xFDFE/2;
		for (int y = 0; y < 224; y++)
		{
			UINT16 *scanline = &bitmap.pix16(y);
			for (int x = 0; x < 288; x+=2)
			{
				UINT16 data = m_bg_image[offs];
				scanline[x]   = pen_base | (data & 0x0f);
				scanline[x+1] = pen_base | (data >> 8);
				offs--;
			}
		}
	}
	else
	{
		int offs = 0x200/2;
		for (int y = 0; y < 224; y++)
		{
			UINT16 *scanline = &bitmap.pix16(y);
			for (int x = 0; x < 288; x+=2)
			{
				UINT16 data = m_bg_image[offs];
				scanline[x]   = pen_base | (data >> 8);
				scanline[x+1] = pen_base | (data & 0x0f);
				offs++;
			}
		}
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/


void toypop_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *spriteram_base)
{
	UINT8 *spriteram = spriteram_base + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x800;
	UINT8 *spriteram_3 = spriteram_2 + 0x800;
	enum { xoffs = -31, yoffs = -8 };
	int flip = flip_screen();

	for (int offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const UINT8 gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int sprite = spriteram[offs];
			int color = spriteram[offs+1];
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 40 + xoffs;
			int sy = 256 - spriteram_2[offs] + yoffs + 1;   // sprites are buffered and delayed by one scanline
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = (spriteram_3[offs] & 0x04) >> 2;
			int sizey = (spriteram_3[offs] & 0x08) >> 3;

			sprite &= ~sizex;
			sprite &= ~(sizey << 1);

			sy -= 16 * sizey;
			sy = (sy & 0xff) - 32;  // fix wraparound

			if (flip)
			{
				flipx ^= 1;
				flipy ^= 1;
			}

			for (int y = 0;y <= sizey;y++)
			{
				for (int x = 0;x <= sizex;x++)
				{
					m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
						sprite + gfx_offs[y ^ (sizey & flipy)][x ^ (sizex & flipx)],
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0xff));
				}
			}
		}
	}
}


UINT32 toypop_state::screen_update_toypop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap, cliprect, m_spriteram);
	return 0;
}
