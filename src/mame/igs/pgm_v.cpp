// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/*** Video *******************************************************************/
/* see drivers/pgm.c for notes on where improvements can be made */

#include "emu.h"
#include "pgm.h"

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

static constexpr bool get_flipy(u8 flip) { return BIT(flip, 1); }
static constexpr bool get_flipx(u8 flip) { return BIT(flip, 0); }

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

inline u8 pgm_state::get_sprite_pix()
{
	const u8 srcdat = ((m_adata[m_aoffset & (m_adata.length() - 1)] >> m_abit) & 0x1f);
	m_abit += 5; // 5 bit per pixel, 3 pixels in each word; 15 bit used
	if (m_abit >= 15)
	{
		m_aoffset++;
		m_abit = 0;
	}
	return srcdat;
}

/*************************************************************************
 Full Sprite Renderer
  for complex zoomed cases
*************************************************************************/

void pgm_state::draw_sprite_line(int wide, u16* dest, u8* destpri, const rectangle &cliprect, int xzoom, bool xgrow, int flip, int xpos, int pri, int realxsize, int palt, bool draw)
{
	int xoffset = 0;
	int xdrawpos = 0;
	int xcntdraw = 0;

	for (int xcnt = 0; xcnt < wide; xcnt++)
	{
		u16 msk = m_bdata[m_boffset & (m_bdata.length() - 1)];

		for (int x = 0; x < 16; x++)
		{
			if (!(BIT(msk, 0)))
			{
				const u16 srcdat = get_sprite_pix() + palt * 32;

				if (draw)
				{
					const bool xzoombit = BIT(xzoom, xoffset & 0x1f);
					xoffset++;

					if (xzoombit && xgrow)
					{ // double this column

						if (!get_flipx(flip))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

						xcntdraw++;

						if (!get_flipx(flip))
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
						if (!get_flipx(flip))
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

		m_boffset++;
	}
}

void pgm_state::draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, int pri)
{
	int ydrawpos;
	int xcnt = 0;

	m_aoffset = (m_bdata[(m_boffset + 1) & (m_bdata.length() - 1)] << 16) | (m_bdata[(m_boffset + 0) & (m_bdata.length() - 1)] << 0);
	m_aoffset = m_aoffset >> 2;
	m_abit = 0;

	m_boffset += 2;

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
			const int temp_abit = m_abit;
			const int temp_boffset = m_boffset;

			if (!get_flipy(flip))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix(ydrawpos);
				u8 *destpri = &priority_bitmap.pix(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);
			}

			ycntdraw++;

			// we need to draw this line again, so restore our pointers to previous values
			m_aoffset = temp_aoffset;
			m_abit = temp_abit;
			m_boffset = temp_boffset;

			if (!get_flipy(flip))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix(ydrawpos);
				u8 *destpri = &priority_bitmap.pix(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (!get_flipy(flip))
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
			if (!get_flipy(flip))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix(ydrawpos);
				u8 *destpri = &priority_bitmap.pix(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (!get_flipy(flip))
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
	int xcntdraw = 0;

	if (!pri)
	{
		for (int xcnt = 0; xcnt < wide; xcnt++)
		{
			u16 msk = m_bdata[m_boffset & (m_bdata.length() - 1)];

			for (int x = 0; x < 16; x++)
			{
				if (!(BIT(msk, 0)))
				{
					const u16 srcdat = get_sprite_pix() + palt * 32;

					if (draw)
					{
						xoffset++;

						if (!get_flipx(flip))
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

			m_boffset++;
		}
	}
	else
	{
		for (int xcnt = 0; xcnt < wide; xcnt++)
		{
			u16 msk = m_bdata[m_boffset & (m_bdata.length() - 1)];

			for (int x = 0; x < 16; x++)
			{
				if (!(BIT(msk, 0)))
				{
					const u16 srcdat = get_sprite_pix() + palt * 32;

					if (draw)
					{
						xoffset++;

						if (!get_flipx(flip))
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

			m_boffset++;
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

	m_aoffset = (m_bdata[(m_boffset + 1) & (m_bdata.length() - 1)] << 16) | (m_bdata[(m_boffset + 0) & (m_bdata.length() - 1)] << 0);
	m_aoffset = m_aoffset >> 2;
	m_abit = 0;

	m_boffset += 2;

	const int realysize = high - 1;
	const int realxsize = (wide * 16) - 1;

	/* now draw it */
	int ycnt = 0;
	int ycntdraw = 0;

	while (ycnt < high)
	{
		if (!get_flipy(flip))
			ydrawpos = ypos + ycntdraw;
		else
			ydrawpos = ypos + realysize - ycntdraw;

		if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
		{
			u16 *dest = &bitmap.pix(ydrawpos);
			u8 *destpri = &priority_bitmap.pix(ydrawpos);
			draw_sprite_line_basic(wide, dest, destpri, cliprect, flip, xpos, pri, realxsize, palt, true);
		}
		else
		{
			draw_sprite_line_basic(wide, nullptr, nullptr, cliprect, flip, xpos, pri, realxsize, palt, false);

			if (!get_flipy(flip))
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


void pgm_state::draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, bitmap_ind8& priority_bitmap)
{
	struct sprite_t *sprite_ptr = m_sprite_ptr_pre;
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		m_boffset = sprite_ptr->offs;
		if ((!sprite_ptr->xzoom) && (!sprite_ptr->yzoom))
		{
			draw_sprite_new_basic(sprite_ptr->width, sprite_ptr->height,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->color, sprite_ptr->flip,
				spritebitmap, cliprect, priority_bitmap,
				sprite_ptr->pri);
		}
		else
		{
			draw_sprite_new_zoomed(sprite_ptr->width, sprite_ptr->height,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->color, sprite_ptr->flip,
				spritebitmap, cliprect, priority_bitmap,
				sprite_ptr->xzoom, sprite_ptr->xgrow, sprite_ptr->yzoom, sprite_ptr->ygrow, sprite_ptr->pri);
		}
	}
}

/*
        Sprite list format (10 bytes per sprites, 256 entries)

    Offset Bits
           fedcba98 76543210
    00     x------- -------- Horizontal Zoom/Shrink mode select
           -xxxx--- -------- Horizontal Zoom/Shrink table select
           -----xxx xxxxxxxx X position (11 bit signed)

    02     x------- -------- Vertical Zoom/Shrink mode select
           -xxxx--- -------- Vertical Zoom/Shrink table select
           -----xxx xxxxxxxx Y position (10 bit signed)

    04     -x------ -------- Flip Y
           --x----- -------- Flip X
           ---xxxxx -------- Palette select (32 color each)
           -------- x------- Priority (Over(0) or Under(1) background)
           -------- -xxxxxxx Sprite mask ROM address MSB
    06     xxxxxxxx xxxxxxxx Sprite mask ROM address LSB

    08     x------- -------- Another sprite width bit?
           -xxxxxx- -------- Sprite width (16 pixel each)
           -------x xxxxxxxx Sprite height (1 pixel each)

*/
void pgm_state::get_sprites()
{
	m_sprite_ptr_pre = m_spritelist.get();

	u16 *sprite_source = &m_mainram[0];
	const u16 *finish = &m_mainram[0xa00 / 2];
	const u16* sprite_zoomtable = &m_videoregs[0x1000 / 2];

	while (sprite_source < finish)
	{
		if (!sprite_source[4]) break; /* is this right? */

		int xzom =                 (sprite_source[0] & 0x7800) >> 11;
		const bool xgrow =         (sprite_source[0] & 0x8000) >> 15;
		m_sprite_ptr_pre->x =      (sprite_source[0] & 0x03ff) - (sprite_source[0] & 0x0400);

		int yzom =                 (sprite_source[1] & 0x7800) >> 11;
		const bool ygrow =         (sprite_source[1] & 0x8000) >> 15;
		m_sprite_ptr_pre->y =      (sprite_source[1] & 0x01ff) - (sprite_source[1] & 0x0200);

		m_sprite_ptr_pre->flip =   (sprite_source[2] & 0x6000) >> 13;
		m_sprite_ptr_pre->color =  (sprite_source[2] & 0x1f00) >> 8;
		m_sprite_ptr_pre->pri =    (sprite_source[2] & 0x0080) >>  7;
		m_sprite_ptr_pre->offs =  ((sprite_source[2] & 0x007f) << 16) | (sprite_source[3] & 0xffff);

		m_sprite_ptr_pre->width =  (sprite_source[4] & 0x7e00) >> 9;
		m_sprite_ptr_pre->height =  sprite_source[4] & 0x01ff;

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

		m_sprite_ptr_pre->xzoom = (sprite_zoomtable[xzom * 2] << 16) | sprite_zoomtable[xzom * 2 + 1];
		m_sprite_ptr_pre->yzoom = (sprite_zoomtable[yzom * 2] << 16) | sprite_zoomtable[yzom * 2 + 1];
		m_sprite_ptr_pre->xgrow = xgrow;
		m_sprite_ptr_pre->ygrow = ygrow;
		m_sprite_ptr_pre++;
		sprite_source += 5;
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

	tileinfo.set(0,tileno,colour,TILE_FLIPYX(flipyx));
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

	tileinfo.set(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

void pgm_state::video_start()
{
	// assumes it can make an address mask with .length() - 1 on these
	assert(!(m_adata.length() & (m_adata.length() - 1)));
	assert(!(m_bdata.length() & (m_bdata.length() - 1)));

	m_spritelist = std::make_unique<sprite_t[]>(0xa00/2/5);
	m_sprite_ptr_pre = m_spritelist.get();

	m_aoffset = 0;
	m_abit = 0;
	m_boffset = 0;

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pgm_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tx_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pgm_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 32, 32, 64, 16);
	m_bg_tilemap->set_transparent_pen(31);
	m_bg_tilemap->set_scroll_rows(16 * 32);
}

u32 pgm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x3ff, cliprect); // ddp2 igs logo needs 0x3ff

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->set_scrolly(0, m_videoregs[0x2000/2]);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
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
		get_sprites();

		// vblank start interrupt
		m_maincpu->set_input_line(M68K_IRQ_6, HOLD_LINE);
	}
}
