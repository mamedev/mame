// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/*** Video *******************************************************************/
/* see drivers/pgm.c for notes on where improvements can be made */

#include "emu.h"
#include "includes/pgm.h"
#include "screen.h"

/******************************************************************************
 Sprites

 these are fairly complex to render due to the data format, unless you
 pre-decode the data you have to draw pixels in the order they're decoded from
 the ROM which becomes quite complex with flipped and zoomed cases
******************************************************************************/

// nothing pri is 0
// bg pri is 2
// sprite already here is 1 / 3

inline void pgm_state::pgm_draw_pix(int xdrawpos, int pri, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.min_x) && (xdrawpos <= cliprect.max_x))
	{
		if (!(destpri[xdrawpos] & 1))
		{
			if (!pri)
			{
				dest[xdrawpos] = srcdat;
			}
			else
			{
				if (!(destpri[xdrawpos] & 2))
				{
					dest[xdrawpos] = srcdat;
				}
			}
		}

		destpri[xdrawpos] |= 1;
	}
}

inline void pgm_state::pgm_draw_pix_nopri(int xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.min_x) && (xdrawpos <= cliprect.max_x))
	{
		if (!(destpri[xdrawpos] & 1))
		{
			dest[xdrawpos] = srcdat;
		}
		destpri[xdrawpos] |= 1;
	}
}

inline void pgm_state::pgm_draw_pix_pri(int xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.min_x) && (xdrawpos <= cliprect.max_x))
	{
		if (!(destpri[xdrawpos] & 1))
		{
			if (!(destpri[xdrawpos] & 2))
			{
				dest[xdrawpos] = srcdat;
			}
		}
		destpri[xdrawpos] |= 1;
	}
}

/*************************************************************************
 Full Sprite Renderer
  for complex zoomed cases
*************************************************************************/

void pgm_state::draw_sprite_line(int wide, u16* dest, u8* destpri, const rectangle &cliprect, int xzoom, bool xgrow, int flip, int xpos, int pri, int realxsize, int palt, bool draw)
{
	int xoffset = 0;
	int xdrawpos = 0;

	u8 *adata = m_sprite_a_region.get();
	size_t adatasize = m_sprite_a_region_size - 1;

	int xcntdraw = 0;

	for (int xcnt = 0; xcnt < wide; xcnt++)
	{
		u16 msk = ((m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) |(m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0));

		for (int x = 0; x < 16; x++)
		{
			if (!(BIT(msk, 0)))
			{
				const u16 srcdat = adata[m_aoffset & adatasize] + palt * 32;
				m_aoffset++;

				if (draw)
				{
					const bool xzoombit = BIT(xzoom, xoffset & 0x1f);
					xoffset++;

					if (xzoombit && xgrow)
					{ // double this column

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

						xcntdraw++;

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

						xcntdraw++;
					}
					else if (xzoombit && (!xgrow))
					{
						/* skip this column */
					}
					else //normal column
					{
						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

						xcntdraw++;
					}
				}

			}
			else
			{
				const bool xzoombit = BIT(xzoom, xoffset & 0x1f);
				xoffset++;
				if (xzoombit && xgrow) { xcntdraw += 2; }
				else if (xzoombit && (!xgrow)) { }
				else { xcntdraw++; }
			}

			msk >>= 1;
		}

		m_boffset += 2;
	}
}

void pgm_state::draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, int pri)
{
	int ydrawpos;
	int xcnt = 0;

	m_aoffset = (m_bdata[(m_boffset + 3) & m_bdata.mask()] << 24) | (m_bdata[(m_boffset + 2) & m_bdata.mask()] << 16) |
				(m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) | (m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0);
	m_aoffset = m_aoffset >> 2; m_aoffset *= 3;

	m_boffset += 4;

	/* precalculate where drawing will end, for flipped zoomed cases. */
	/* if we're to avoid pre-decoding the data for each sprite each time we draw then we have to draw the sprite data
	   in the order it is in ROM due to the nature of the compresson scheme.  This means drawing upwards from the end point
	   in the case of flipped sprites */
	int ycnt = 0;
	int ycntdraw = 0;
	int realysize = 0;

	while (ycnt < high)
	{
		const bool yzoombit = BIT(yzoom, ycnt & 0x1f);
		if (yzoombit && ygrow) { realysize += 2; }
		else if (yzoombit && (!ygrow)) { }
		else { realysize++; };

		ycnt++;
	}
	realysize--;

	int realxsize = 0;

	while (xcnt < wide * 16)
	{
		const bool xzoombit = BIT(xzoom, xcnt & 0x1f);
		if (xzoombit && xgrow) { realxsize += 2; }
		else if (xzoombit && (!xgrow)) { }
		else { realxsize++; };

		xcnt++;
	}
	realxsize--;

	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;

	while (ycnt < high)
	{
		const bool yzoombit = BIT(yzoom, ycnt & 0x1f);

		if (yzoombit && ygrow) // double this line
		{
			const int temp_aoffset = m_aoffset;
			const int temp_boffset = m_boffset;

			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix16(ydrawpos);
				u8 *destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);
			}

			ycntdraw++;

			// we need to draw this line again, so restore our pointers to previous values
			m_aoffset = temp_aoffset;
			m_boffset = temp_boffset;

			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix16(ydrawpos);
				u8 *destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (!(flip & 0x02))
				{
					if (ydrawpos >= cliprect.max_y)
						return;
				}
				else
				{
					if (ydrawpos < cliprect.min_y)
						return;
				}
			}

			ycntdraw++;

		}
		else if (yzoombit && (!ygrow))
		{
			/* skip this line */
			draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);
		}
		else /* normal line */
		{
			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix16(ydrawpos);
				u8 *destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (!(flip & 0x02))
				{
					if (ydrawpos >= cliprect.max_y)
						return;
				}
				else
				{
					if (ydrawpos < cliprect.min_y)
						return;
				}

			}

			ycntdraw++;
		}

		ycnt++;
	}
}


void pgm_state::draw_sprite_line_basic(int wide, u16* dest, u8* destpri, const rectangle &cliprect, int flip, int xpos, int pri, int realxsize, int palt, bool draw)
{
	int xoffset = 0;
	int xdrawpos = 0;
	u8 *adata = m_sprite_a_region.get();
	size_t adatasize = m_sprite_a_region_size - 1;

	int xcntdraw = 0;

	if (!pri)
	{
		for (int xcnt = 0; xcnt < wide; xcnt++)
		{
			u16 msk = ((m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) |(m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0));

			for (int x = 0; x < 16; x++)
			{
				if (!(BIT(msk, 0)))
				{
					const u16 srcdat = adata[m_aoffset & adatasize] + palt * 32;
					m_aoffset++;

					if (draw)
					{
						xoffset++;

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix_nopri(xdrawpos, dest, destpri, cliprect, srcdat);

						xcntdraw++;
					}

				}
				else
				{
					xoffset++;
					xcntdraw++;
				}

				msk >>= 1;
			}

			m_boffset += 2;
		}
	}
	else
	{
		for (int xcnt = 0; xcnt < wide; xcnt++)
		{
			u16 msk = ((m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) |(m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0));

			for (int x = 0; x < 16; x++)
			{
				if (!(BIT(msk, 0)))
				{
					const u16 srcdat = adata[m_aoffset & adatasize] + palt * 32;
					m_aoffset++;

					if (draw)
					{
						xoffset++;

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix_pri(xdrawpos, dest, destpri, cliprect, srcdat);

						xcntdraw++;
					}

				}
				else
				{
					xoffset++;
					xcntdraw++;
				}

				msk >>= 1;
			}

			m_boffset += 2;
		}
	}
}

/*************************************************************************
 Basic Sprite Renderer
  simplified version for non-zoomed cases, a bit faster
*************************************************************************/

void pgm_state::draw_sprite_new_basic(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri)
{
	int ydrawpos;

	m_aoffset = (m_bdata[(m_boffset + 3) & m_bdata.mask()] << 24) | (m_bdata[(m_boffset + 2) & m_bdata.mask()] << 16) |
				(m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) | (m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0);
	m_aoffset = m_aoffset >> 2; m_aoffset *= 3;

	m_boffset += 4;

	const int realysize = high - 1;
	const int realxsize = (wide * 16) - 1;

	/* now draw it */
	int ycnt = 0;
	int ycntdraw = 0;

	while (ycnt < high)
	{
		if (!(flip & 0x02))
			ydrawpos = ypos + ycntdraw;
		else
			ydrawpos = ypos + realysize - ycntdraw;

		if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
		{
			u16 *dest = &bitmap.pix16(ydrawpos);
			u8 *destpri = &priority_bitmap.pix8(ydrawpos);
			draw_sprite_line_basic(wide, dest, destpri, cliprect, flip, xpos, pri, realxsize, palt, true);
		}
		else
		{
			draw_sprite_line_basic(wide, nullptr, nullptr, cliprect, flip, xpos, pri, realxsize, palt, false);

			if (!(flip & 0x02))
			{
				if (ydrawpos >= cliprect.max_y)
					return;
			}
			else
			{
				if (ydrawpos < cliprect.min_y)
					return;
			}
		}

		ycntdraw++;
		ycnt++;
	}
}


void pgm_state::draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, u16 *sprite_source, bitmap_ind8& priority_bitmap)
{
	/* ZZZZ Zxxx xxxx xxxx
	   zzzz z-yy yyyy yyyy
	   -ffp pppp Pvvv vvvv
	   vvvv vvvv vvvv vvvv
	   wwww wwwh hhhh hhhh
	*/

	const u16 *finish = m_spritebufferram.get() + (0xa00 / 2);

	u16* start = sprite_source;

	while (sprite_source < finish)
	{
		if (!sprite_source[4]) break; /* is this right? */
		sprite_source += 5;
	}
	sprite_source -= 5;

	while (sprite_source >= start)
	{
		int xpos =          sprite_source[0] & 0x07ff;
		int ypos =          sprite_source[1] & 0x03ff;
		int xzom =         (sprite_source[0] & 0x7800) >> 11;
		const bool xgrow = (sprite_source[0] & 0x8000) >> 15;
		int yzom =         (sprite_source[1] & 0x7800) >> 11;
		const bool ygrow = (sprite_source[1] & 0x8000) >> 15;
		const int palt =   (sprite_source[2] & 0x1f00) >> 8;
		const int flip =   (sprite_source[2] & 0x6000) >> 13;
		int boff =        ((sprite_source[2] & 0x007f) << 16) | (sprite_source[3] & 0xffff);
		const int wide =   (sprite_source[4] & 0x7e00) >> 9;
		const int high =    sprite_source[4] & 0x01ff;
		const int pri =    (sprite_source[2] & 0x0080) >>  7;

		const u16* sprite_zoomtable = &m_videoregs[0x1000 / 2];

		if (xgrow)
		{
		//  xzom = 0xf - xzom; // would make more sense but everything gets zoomed slightly in dragon world 2 ?!
			xzom = 0x10 - xzom; // this way it doesn't but there is a bad line when zooming after the level select?
		}

		if (ygrow)
		{
		//  yzom = 0xf - yzom; // see comment above
			yzom = 0x10 - yzom;
		}

		u32 xzoom = (sprite_zoomtable[xzom * 2] << 16) | sprite_zoomtable[xzom * 2 + 1];
		u32 yzoom = (sprite_zoomtable[yzom * 2] << 16) | sprite_zoomtable[yzom * 2 + 1];

		boff *= 2;
		if (xpos > 0x3ff) xpos -= 0x800;
		if (ypos > 0x1ff) ypos -= 0x400;

		//if ((priority == 1) && (pri == 0)) break;

		m_boffset = boff;
		if ((!xzoom) && (!yzoom)) draw_sprite_new_basic(wide, high, xpos, ypos, palt, flip, spritebitmap, cliprect, priority_bitmap, pri);
		else draw_sprite_new_zoomed(wide, high, xpos, ypos, palt, flip, spritebitmap, cliprect, priority_bitmap, xzoom, xgrow, yzoom, ygrow, pri);

		sprite_source -= 5;
	}
}

/* TX Layer */
void pgm_state::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_tx_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(pgm_state::get_tx_tile_info)
{
/* 0x904000 - 0x90ffff is the Text Overlay Ram (pgm_tx_videoram)
    each tile uses 4 bytes, the tilemap is 64x128?

   the layer uses 4bpp 8x8 tiles from the 'T' roms
   colours from 0xA01000 - 0xA017FF

   scroll registers are at 0xB05000 (Y) and 0xB06000 (X)

    ---- ---- ffpp ppp- nnnn nnnn nnnn nnnn

    n = tile number
    p = palette
    f = flip
*/
	const u32 tileno = m_tx_videoram[tile_index * 2] & 0xffff;
	const u32 colour = (m_tx_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	const u8  flipyx = (m_tx_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(0,tileno,colour,TILE_FLIPYX(flipyx));
}

/* BG Layer */

void pgm_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(pgm_state::get_bg_tile_info)
{
	/* pretty much the same as tx layer */

	const u32 tileno = m_bg_videoram[tile_index *2] & 0xffff;
	const u32 colour = (m_bg_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	const u8  flipyx = (m_bg_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

void pgm_state::video_start()
{
	m_aoffset = 0;
	m_boffset = 0;

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pgm_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tx_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pgm_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 32, 32, 64, 16);
	m_bg_tilemap->set_transparent_pen(31);
	m_bg_tilemap->set_scroll_rows(16 * 32);

	m_spritebufferram = make_unique_clear<u16[]>(0xa00/2);

	save_pointer(NAME(m_spritebufferram), 0xa00/2);
}

u32 pgm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x3ff, cliprect); // ddp2 igs logo needs 0x3ff

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->set_scrolly(0, m_videoregs[0x2000/2]);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		m_bg_tilemap->set_scrollx((y + m_videoregs[0x2000 / 2]) & 0x1ff, m_videoregs[0x3000 / 2] + m_rowscrollram[y]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(bitmap, cliprect, m_spritebufferram.get(), screen.priority());

	m_tx_tilemap->set_scrolly(0, m_videoregs[0x5000/2]);
	m_tx_tilemap->set_scrollx(0, m_videoregs[0x6000/2]); // Check

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

WRITE_LINE_MEMBER(pgm_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
		memcpy(m_spritebufferram.get(), m_mainram, 0xa00);
	}
}
