// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/*** Video *******************************************************************/
/* see drivers/pgm.c for notes on where improvements can be made */

#include "emu.h"
#include "includes/pgm.h"

/******************************************************************************
 Sprites

 these are fairly complex to render due to the data format, unless you
 pre-decode the data you have to draw pixels in the order they're decoded from
 the ROM which becomes quite complex with flipped and zoomed cases
******************************************************************************/

// nothing pri is 0
// bg pri is 2
// sprite already here is 1 / 3

inline void pgm_state::pgm_draw_pix(s16 xdrawpos, u8 pri, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.left()) && (xdrawpos <= cliprect.right()))
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

inline void pgm_state::pgm_draw_pix_nopri(s16 xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.left()) && (xdrawpos <= cliprect.right()))
	{
		if (!(destpri[xdrawpos] & 1))
		{
			dest[xdrawpos] = srcdat;
		}
		destpri[xdrawpos] |= 1;
	}
}

inline void pgm_state::pgm_draw_pix_pri(s16 xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.left()) && (xdrawpos <= cliprect.right()))
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

void pgm_state::draw_sprite_line(u16 wide, u16* dest, u8* destpri, const rectangle &cliprect,
		u32 xzoom, bool xgrow, u8 flip, s16 xpos, u8 pri, s16 realxsize, u8 palt, bool draw)
{
	u8 xzoombit;
	int xoffset = 0;
	s16 xdrawpos = 0;

	const u8 *adata = m_sprite_a_region.get();
	size_t  adatasize = m_sprite_a_region_size - 1;

	s16 xcntdraw = 0;

	for (s16 xcnt = 0; xcnt < wide; xcnt++)
	{
		u16 const msk = ((m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) |( m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0));

		for (int x = 0; x < 16; x++)
		{
			if (!BIT(msk, x))
			{
				u16 const srcdat = adata[m_aoffset & adatasize] + palt * 32;
				m_aoffset++;

				if (draw)
				{
					xzoombit = BIT(xzoom, xoffset & 0x1f);
					xoffset++;

					if (xzoombit == 1 && xgrow)
					{ // double this column

						if (BIT(~flip, 0))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

						xcntdraw++;

						if (BIT(~flip, 0))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

						xcntdraw++;
					}
					else if (xzoombit == 1 && (!xgrow))
					{
						/* skip this column */
					}
					else //normal column
					{
						if (BIT(~flip, 0))
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
				xzoombit = BIT(xzoom, xoffset & 0x1f);
				xoffset++;
				if (xzoombit == 1 && xgrow) { xcntdraw+=2; }
				else if (xzoombit == 1 && (!xgrow)) { }
				else { xcntdraw++; }
			}
		}

		m_boffset += 2;
	}
}

void pgm_state::draw_sprite_new_zoomed(u16 wide, u16 high, s16 xpos, s16 ypos, u8 palt, u8 flip,
		bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, u8 pri)
{
	s16 ydrawpos;
	u8 yzoombit;
	u8 xzoombit;
	s16 xcnt = 0;


	m_aoffset = (m_bdata[(m_boffset + 3) & m_bdata.mask()] << 24) | (m_bdata[(m_boffset + 2) & m_bdata.mask()] << 16) |
				(m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) | (m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0);
	m_aoffset = m_aoffset >> 2; m_aoffset *= 3;

	m_boffset+=4;

	/* precalculate where drawing will end, for flipped zoomed cases. */
	/* if we're to avoid pre-decoding the data for each sprite each time we draw then we have to draw the sprite data
	   in the order it is in ROM due to the nature of the compresson scheme.  This means drawing upwards from the end point
	   in the case of flipped sprites */
	s16 ycnt = 0;
	s16 ycntdraw = 0;
	s16 realysize = 0;

	while (ycnt < high)
	{
		yzoombit = BIT(yzoom, ycnt & 0x1f);
		if (yzoombit == 1 && ygrow) { realysize+=2; }
		else if (yzoombit == 1 && (!ygrow)) { }
		else { realysize++; };

		ycnt++;
	}
	realysize--;

	s16 realxsize = 0;

	while (xcnt < wide * 16)
	{
		xzoombit = BIT(xzoom, xcnt & 0x1f);
		if (xzoombit == 1 && xgrow) { realxsize+=2; }
		else if (xzoombit == 1 && (!xgrow)) { }
		else { realxsize++; };

		xcnt++;
	}
	realxsize--;


	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;

	while (ycnt < high)
	{
		yzoombit = BIT(yzoom, ycnt & 0x1f);

		if (yzoombit == 1 && ygrow) // double this line
		{
			u32 temp_aoffset = m_aoffset;
			u32 temp_boffset = m_boffset;

			if (BIT(~flip, 1))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.top()) && (ydrawpos <= cliprect.bottom()))
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

			if (BIT(~flip, 1))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.top()) && (ydrawpos <= cliprect.bottom()))
			{
				u16 *dest = &bitmap.pix16(ydrawpos);
				u8 *destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (BIT(~flip, 1))
				{
					if (ydrawpos > cliprect.bottom())
						return;
				}
				else
				{
					if (ydrawpos < cliprect.top())
						return;
				}
			}

			ycntdraw++;

		}
		else if (yzoombit == 1 && (!ygrow))
		{
			/* skip this line */
			draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);
		}
		else /* normal line */
		{
			if (BIT(~flip, 1))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.top()) && (ydrawpos <= cliprect.bottom()))
			{
				u16 *dest = &bitmap.pix16(ydrawpos);
				u8 *destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (BIT(~flip, 1))
				{
					if (ydrawpos > cliprect.bottom())
						return;
				}
				else
				{
					if (ydrawpos < cliprect.top())
						return;
				}

			}

			ycntdraw++;
		}

		ycnt++;
	}
}


void pgm_state::draw_sprite_line_basic(u16 wide, u16* dest, u8* destpri, const rectangle &cliprect, u8 flip, s16 xpos, u8 pri, s16 realxsize, u8 palt, bool draw)
{
	int xoffset = 0;
	s16 xdrawpos = 0;
	const u8 *adata = m_sprite_a_region.get();
	size_t  adatasize = m_sprite_a_region_size - 1;

	s16 xcntdraw = 0;

	if (!pri)
	{
		for (s16 xcnt = 0; xcnt < wide; xcnt++)
		{
			u16 const msk = ((m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) |( m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0));

			for (int x = 0; x < 16; x++)
			{
				if (!BIT(msk, x))
				{
					u16 const srcdat = adata[m_aoffset & adatasize] + palt * 32;
					m_aoffset++;

					if (draw)
					{
						xoffset++;

						if (BIT(~flip, 0))
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
			}

			m_boffset += 2;
		}
	}
	else
	{
		for (s16 xcnt = 0; xcnt < wide; xcnt++)
		{
			u16 const msk = ((m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) |( m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0));

			for (int x = 0; x < 16; x++)
			{
				if (!BIT(msk, x))
				{
					u16 const srcdat = adata[m_aoffset & adatasize] + palt * 32;
					m_aoffset++;

					if (draw)
					{
						xoffset++;

						if (BIT(~flip, 0))
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
			}

			m_boffset += 2;
		}
	}
}

/*************************************************************************
 Basic Sprite Renderer
  simplified version for non-zoomed cases, a bit faster
*************************************************************************/

void pgm_state::draw_sprite_new_basic(u16 wide, u16 high, s16 xpos, s16 ypos, u8 palt, u8 flip,
		bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, u8 pri)
{
	s16 ydrawpos;

	m_aoffset = (m_bdata[(m_boffset + 3) & m_bdata.mask()] << 24) | (m_bdata[(m_boffset + 2) & m_bdata.mask()] << 16) |
				(m_bdata[(m_boffset + 1) & m_bdata.mask()] << 8) | (m_bdata[(m_boffset + 0) & m_bdata.mask()] << 0);
	m_aoffset = m_aoffset >> 2; m_aoffset *= 3;

	m_boffset+=4;

	s16 realysize = high-1;
	s16 realxsize = (wide * 16)-1;

	/* now draw it */
	s16 ycnt = 0;
	s16 ycntdraw = 0;

	while (ycnt < high)
	{
		if (BIT(~flip, 1))
			ydrawpos = ypos + ycntdraw;
		else
			ydrawpos = ypos + realysize - ycntdraw;

		if ((ydrawpos >= cliprect.top()) && (ydrawpos <= cliprect.bottom()))
		{
			u16 *dest = &bitmap.pix16(ydrawpos);
			u8 *destpri = &priority_bitmap.pix8(ydrawpos);
			draw_sprite_line_basic(wide, dest, destpri, cliprect, flip, xpos, pri, realxsize, palt, true);
		}
		else
		{
			draw_sprite_line_basic(wide, nullptr, nullptr, cliprect, flip, xpos, pri, realxsize, palt, false);

			if (BIT(~flip, 1))
			{
				if (ydrawpos > cliprect.bottom())
					return;
			}
			else
			{
				if (ydrawpos < cliprect.top())
					return;
			}
		}

		ycntdraw++;
		ycnt++;
	}
}


void pgm_state::draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, bitmap_ind8& priority_bitmap)
{
	/* ZZZZ Zxxx xxxx xxxx
	   zzzz z-yy yyyy yyyy
	   -ffp pppp Pvvv vvvv
	   vvvv vvvv vvvv vvvv
	   wwww wwwh hhhh hhhh
	*/

	u16 *sprite_source = m_spritebufferram.get();
	const u16 *finish = sprite_source + (0xa00 / 2);

	u16* start = sprite_source;

	while (sprite_source < finish)
	{
		if (!sprite_source[4]) break; /* is this right? */
		sprite_source += 5;
	}
	sprite_source-=5;

	while (sprite_source >= start)
	{
		s16 xpos        = sprite_source[0] & 0x07ff;
		s16 ypos        = sprite_source[1] & 0x03ff;
		u8 xzom        = (sprite_source[0] & 0x7800) >> 11;
		bool const xgrow    = (sprite_source[0] & 0x8000) >> 15;
		u8 yzom        = (sprite_source[1] & 0x7800) >> 11;
		bool const ygrow    = (sprite_source[1] & 0x8000) >> 15;
		u8 const palt  = (sprite_source[2] & 0x1f00) >> 8;
		u8 const flip  = (sprite_source[2] & 0x6000) >> 13;
		u32 const boff = ((sprite_source[2] & 0x007f) << 16) | (sprite_source[3] & 0xffff);
		u16 const wide = (sprite_source[4] & 0x7e00) >> 9;
		u16 const high = sprite_source[4] & 0x01ff;
		u8 const pri   = (sprite_source[2] & 0x0080) >>  7;

		u16* sprite_zoomtable = &m_videoregs[0x1000 / 2];

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

		u32 const xzoom = (sprite_zoomtable[xzom * 2] << 16) | sprite_zoomtable[xzom * 2 + 1];
		u32 const yzoom = (sprite_zoomtable[yzom * 2] << 16) | sprite_zoomtable[yzom * 2 + 1];

		if (xpos & 0x400) xpos -= 0x800;
		if (ypos & 0x200) ypos -= 0x400;

		//if ((priority == 1) && (pri == 0)) break;

		m_boffset = boff << 1;
		if ((!xzoom) && (!yzoom))
			draw_sprite_new_basic(wide, high, xpos, ypos, palt, flip, spritebitmap, priority_bitmap, cliprect, pri);
		else
			draw_sprite_new_zoomed(wide, high, xpos, ypos, palt, flip, spritebitmap, priority_bitmap, cliprect, xzoom, xgrow, yzoom, ygrow, pri);

		sprite_source -= 5;
	}
}

/* TX Layer */
void pgm_state::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_tx_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(pgm_state::get_tx_tilemap_tile_info)
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

	u16 const tileno = m_tx_videoram[tile_index * 2] & 0xffff;
	u8 const colour  = (m_tx_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	u8 const flipyx  = (m_tx_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(0,tileno,colour,TILE_FLIPYX(flipyx));
}

/* BG Layer */

void pgm_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(pgm_state::get_bg_tilemap_tile_info)
{
	/* pretty much the same as tx layer */

	u16 const tileno = m_bg_videoram[tile_index * 2] & 0xffff;
	u8 const colour  = (m_bg_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	u8 const flipyx  = (m_bg_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

void pgm_state::video_start()
{
	m_aoffset = 0;
	m_boffset = 0;

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(pgm_state::get_tx_tilemap_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tx_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(pgm_state::get_bg_tilemap_tile_info),this), TILEMAP_SCAN_ROWS, 32, 32, 64, 16);
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

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		m_bg_tilemap->set_scrollx((y + m_videoregs[0x2000 / 2]) & 0x1ff, m_videoregs[0x3000 / 2] + m_rowscrollram[y]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(bitmap, cliprect, screen.priority());

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
		m_maincpu->set_input_line(6, HOLD_LINE);
	}
}
