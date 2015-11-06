// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria
/*************************************************************************

    Equites, Splendor Blast driver
    
    functions to emulate the video hardware

*************************************************************************/

#include "emu.h"
#include "includes/equites.h"


/*************************************
 *
 *  Palette handling
 *
 *************************************/

PALETTE_INIT_MEMBER(equites_state,equites)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 256; i++)
		palette.set_indirect_color(i, rgb_t(pal4bit(color_prom[i]), pal4bit(color_prom[i + 0x100]), pal4bit(color_prom[i + 0x200])));

	// point to the CLUT
	color_prom += 0x380;

	for (i = 0; i < 256; i++)
		palette.set_pen_indirect(i, i);

	for (i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i + 0x100, color_prom[i]);
}

PALETTE_INIT_MEMBER(equites_state,splndrbt)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 0x100; i++)
		palette.set_indirect_color(i, rgb_t(pal4bit(color_prom[i]), pal4bit(color_prom[i + 0x100]), pal4bit(color_prom[i + 0x200])));

	for (i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	// point to the bg CLUT
	color_prom += 0x300;

	for (i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i + 0x100, color_prom[i] + 0x10);

	// point to the sprite CLUT
	color_prom += 0x100;

	for (i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i + 0x180, color_prom[i]);
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

TILE_GET_INFO_MEMBER(equites_state::equites_fg_info)
{
	int tile = m_fg_videoram[2 * tile_index];
	int color = m_fg_videoram[2 * tile_index + 1] & 0x1f;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
	if (color & 0x10)
		tileinfo.flags |= TILE_FORCE_LAYER0;
}

TILE_GET_INFO_MEMBER(equites_state::splndrbt_fg_info)
{
	int tile = m_fg_videoram[2 * tile_index] + (m_fg_char_bank << 8);
	int color = m_fg_videoram[2 * tile_index + 1] & 0x3f;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
	if (color & 0x10)
		tileinfo.flags |= TILE_FORCE_LAYER0;
}

TILE_GET_INFO_MEMBER(equites_state::equites_bg_info)
{
	int data = m_bg_videoram[tile_index];
	int tile = data & 0x1ff;
	int color = (data & 0xf000) >> 12;
	int fxy = (data & 0x0600) >> 9;

	SET_TILE_INFO_MEMBER(1, tile, color, TILE_FLIPXY(fxy));
}

TILE_GET_INFO_MEMBER(equites_state::splndrbt_bg_info)
{
	int data = m_bg_videoram[tile_index];
	int tile = data & 0x1ff;
	int color = (data & 0xf800) >> 11;
	int fxy = (data & 0x0600) >> 9;

	SET_TILE_INFO_MEMBER(1, tile, color, TILE_FLIPXY(fxy));
	tileinfo.group = color;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(equites_state,equites)
{
	m_fg_videoram = auto_alloc_array(machine(), UINT8, 0x800);
	save_pointer(NAME(m_fg_videoram), 0x800);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(equites_state::equites_fg_info),this), TILEMAP_SCAN_COLS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(equites_state::equites_bg_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(0, -10);
}

VIDEO_START_MEMBER(equites_state,splndrbt)
{
	assert(m_screen->format() == BITMAP_FORMAT_IND16);

	m_fg_videoram = auto_alloc_array(machine(), UINT8, 0x800);
	save_pointer(NAME(m_fg_videoram), 0x800);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(equites_state::splndrbt_fg_info),this), TILEMAP_SCAN_COLS,  8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldx(8, -8);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(equites_state::splndrbt_bg_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(1), 0x10);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(equites_state::equites_fg_videoram_r)
{
	// 8-bit
	return m_fg_videoram[offset];
}

WRITE8_MEMBER(equites_state::equites_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(equites_state::equites_bg_videoram_w)
{
	COMBINE_DATA(m_bg_videoram + offset);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(equites_state::equites_bgcolor_w)
{
	// bg color register
	if (offset == 0)
		m_bgcolor = data;
}

WRITE16_MEMBER(equites_state::equites_scrollreg_w)
{
	if (ACCESSING_BITS_0_7)
		m_bg_tilemap->set_scrolly(0, data & 0xff);

	if (ACCESSING_BITS_8_15)
		m_bg_tilemap->set_scrollx(0, data >> 8);
}

WRITE16_MEMBER(equites_state::splndrbt_selchar_w)
{
	// data bit is A16 (offset)
	data = (offset == 0) ? 0 : 1;
	
	// select active char map
	if (m_fg_char_bank != data)
	{
		m_fg_char_bank = data;
		m_fg_tilemap->mark_all_dirty();
	}
}

WRITE16_MEMBER(equites_state::equites_flipw_w)
{
	// data bit is A16 (offset)
	flip_screen_set(offset != 0);
}

WRITE8_MEMBER(equites_state::equites_flipb_w)
{
	// data bit is A16 (offset)
	flip_screen_set(offset != 0);
}

WRITE16_MEMBER(equites_state::splndrbt_bg_scrollx_w)
{
	COMBINE_DATA(&m_splndrbt_bg_scrollx);
}

WRITE16_MEMBER(equites_state::splndrbt_bg_scrolly_w)
{
	COMBINE_DATA(&m_splndrbt_bg_scrolly);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void equites_state::equites_draw_sprites_block( bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end )
{
	for (int offs = end - 2; offs >= start; offs -= 2)
	{
		int attr = m_spriteram[offs + 1];
		if (!(attr & 0x800))    // disable or x MSB?
		{
			int tile = attr & 0x1ff;
			int fx = ~attr & 0x400;
			int fy = ~attr & 0x200;
			int color = (~attr & 0xf000) >> 12;
			int sx = (m_spriteram[offs] & 0xff00) >> 8;
			int sy = (m_spriteram[offs] & 0x00ff);
			int transmask = m_palette->transpen_mask(*m_gfxdecode->gfx(2), color, 0);

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				fx = !fx;
				fy = !fy;
			}

			// align
			sx -= 4;

			// sprites are 16x14 centered in a 16x16 square, so skip the first line
			sy += 1;

			m_gfxdecode->gfx(2)->transmask(bitmap,cliprect,
					tile,
					color,
					fx, fy,
					sx, sy, transmask);
		}
	}
}

void equites_state::equites_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// note that we draw the sprites in three blocks; in each blocks, sprites at
	// a lower address have priority. This gives good priorities in gekisou.
	equites_draw_sprites_block(bitmap, cliprect, 0x000/2, 0x060/2);
	equites_draw_sprites_block(bitmap, cliprect, 0x0e0/2, 0x100/2);
	equites_draw_sprites_block(bitmap, cliprect, 0x1a4/2, 0x200/2);
}


/*
This is (probabbly) the sprite x scaling PROM.
The layout is strange. Clearly every line os for one xscale setting. However,
it seems that bytes 0-3 are handled separately from bytes 4-F.
Also, note that sprites are 30x30, not 32x32.
00020200 00000000 00000000 00000000
00020200 01000000 00000000 00000002
00020200 01000000 01000002 00000002
00020200 01000100 01000002 00020002
00020200 01000100 01010202 00020002
02020201 01000100 01010202 00020002
02020201 01010100 01010202 00020202
02020201 01010101 01010202 02020202
02020201 03010101 01010202 02020203
02020201 03010103 01010202 03020203
02020201 03010103 01030302 03020203
02020201 03010303 01030302 03030203
03020203 03010303 01030302 03030203
03020203 03030303 01030302 03030303
03020203 03030303 03030303 03030303
03020303 03030303 03030303 03030303
*/

void equites_state::splndrbt_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8 * const xrom = memregion("user2")->base();
	const UINT8 * const yrom = xrom + 0x100;
	gfx_element* gfx = m_gfxdecode->gfx(2);

	// note that sprites are actually 30x30, contained in 32x32 squares. The outer edge is not used.

	for (int offs = 0x3f; offs < 0x6f; offs += 2)   // 24 sprites
	{
		int data = m_spriteram[offs];
		int fx = (data & 0x2000) >> 13;
		int fy = (data & 0x1000) >> 12;
		int tile = data & 0x007f;
		int scaley = (data & 0x0f00) >> 8;
		int data2 = m_spriteram[offs + 1];
		int color = (data2 & 0x1f00) >> 8;
		int sx = data2 & 0x00ff;
		int sy = m_spriteram_2[offs + 0] & 0x00ff;
		int scalex = m_spriteram_2[offs + 1] & 0x000f;
		int transmask = m_palette->transpen_mask(*gfx, color, 0);

//      const UINT8 * const xromline = xrom + (scalex << 4);
		const UINT8 * const yromline = yrom + (scaley << 4) + (15 - scaley);
		const UINT8* const srcgfx = gfx->get_data(tile);
		const pen_t *paldata = &m_palette->pen(gfx->colorbase() + gfx->granularity() * color);
		int x,yy;

		sy += 16;

		if (flip_screen())
		{
			// sx NOT inverted
			fx = fx ^ 1;
			fy = fy ^ 1;
		}
		else
		{
			sy = 256 - sy;
		}

		for (yy = 0; yy <= scaley; ++yy)
		{
			int const line = yromline[yy];
			int yhalf;

			for (yhalf = 0; yhalf < 2; ++yhalf) // top or bottom half
			{
				int const y = yhalf ? sy + 1 + yy : sy - yy;

				if (y >= cliprect.min_y && y <= cliprect.max_y)
				{
					for (x = 0; x <= (scalex << 1); ++x)
					{
						int bx = (sx + x) & 0xff;

						if (bx >= cliprect.min_x && bx <= cliprect.max_x)
						{
							int xx = scalex ? (x * 29 + scalex) / (scalex << 1) + 1 : 16;   // FIXME This is wrong. Should use the PROM.
							int const offset = (fx ? (31 - xx) : xx) + ((fy ^ yhalf) ? (16 + line) : (15 - line) ) * gfx->rowbytes();

							int pen = srcgfx[offset];

							if ((transmask & (1 << pen)) == 0)
								bitmap.pix16(y, bx) = paldata[pen];
						}
					}
				}
			}
		}
	}
}


void equites_state::splndrbt_copy_bg( bitmap_ind16 &dst_bitmap, const rectangle &cliprect )
{
	bitmap_ind16 &src_bitmap = m_bg_tilemap->pixmap();
	bitmap_ind8 &flags_bitmap = m_bg_tilemap->flagsmap();
	const UINT8 * const xrom = memregion("user1")->base();
	const UINT8 * const yrom = xrom + 0x2000;
	int scroll_x = m_splndrbt_bg_scrollx;
	int scroll_y = m_splndrbt_bg_scrolly;
	int const dinvert = flip_screen() ? 0xff : 0x00;
	int src_y = 0;
	int dst_y;

	if (flip_screen())
	{
		scroll_x = -scroll_x - 8;
		scroll_y = -scroll_y;
	}

	for (dst_y = 32; dst_y < 256-32; ++dst_y)
	{
		if (dst_y >= cliprect.min_y && dst_y <= cliprect.max_y)
		{
			const UINT8 * const romline = &xrom[(dst_y ^ dinvert) << 5];
			const UINT16 * const src_line = &src_bitmap.pix16((src_y + scroll_y) & 0x1ff);
			const UINT8 * const flags_line = &flags_bitmap.pix8((src_y + scroll_y) & 0x1ff);
			UINT16 * const dst_line = &dst_bitmap.pix16(dst_y);
			int dst_x = 0;
			int src_x;

			for (src_x = 0; src_x < 256 && dst_x < 128; ++src_x)
			{
				if ((romline[31 - (src_x >> 3)] >> (src_x & 7)) & 1)
				{
					int sx;

					sx = (256+128 + scroll_x + src_x) & 0x1ff;
					if (flags_line[sx] & TILEMAP_PIXEL_LAYER0)
						dst_line[128 + dst_x] = src_line[sx];

					sx = (255+128 + scroll_x - src_x) & 0x1ff;
					if (flags_line[sx] & TILEMAP_PIXEL_LAYER0)
						dst_line[127 - dst_x] = src_line[sx];

					++dst_x;
				}
			}
		}

		src_y += 1 + yrom[dst_y ^ dinvert];
	}
}



UINT32 equites_state::screen_update_equites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	equites_draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

UINT32 equites_state::screen_update_splndrbt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);

	splndrbt_copy_bg(bitmap, cliprect);

	if (m_fg_char_bank)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	splndrbt_draw_sprites(bitmap, cliprect);

	if (!m_fg_char_bank)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
