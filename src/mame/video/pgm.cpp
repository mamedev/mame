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

inline void pgm_state::pgm_draw_pix( int xdrawpos, int pri, UINT16* dest, UINT8* destpri, UINT16 srcdat)
{
	if ((xdrawpos >= 0) && (xdrawpos < 448))
	{
		if (!(destpri[xdrawpos]&1))
		{
			if (!pri)
			{
				dest[xdrawpos] = srcdat;
			}
			else
			{
				if (!(destpri[xdrawpos]&2))
				{
					dest[xdrawpos] = srcdat;
				}
			}
		}

		destpri[xdrawpos]|=1;
	}
}

inline void pgm_state::pgm_draw_pix_nopri( int xdrawpos, UINT16* dest, UINT8* destpri, UINT16 srcdat)
{
	if ((xdrawpos >= 0) && (xdrawpos < 448))
	{
		if (!(destpri[xdrawpos]&1))
		{
			dest[xdrawpos] = srcdat;
		}
		destpri[xdrawpos]|=1;
	}
}

inline void pgm_state::pgm_draw_pix_pri( int xdrawpos, UINT16* dest, UINT8* destpri, UINT16 srcdat)
{
	if ((xdrawpos >= 0) && (xdrawpos < 448))
	{
		if (!(destpri[xdrawpos]&1))
		{
			if (!(destpri[xdrawpos]&2))
			{
				dest[xdrawpos] = srcdat;
			}
		}
		destpri[xdrawpos]|=1;
	}
}

/*************************************************************************
 Full Sprite Renderer
  for complex zoomed cases
*************************************************************************/

void pgm_state::draw_sprite_line( int wide, UINT16* dest, UINT8* destpri, int xzoom, int xgrow, int flip, int xpos, int pri, int realxsize, int palt, int draw )
{
	int xcnt,xcntdraw;
	int xzoombit;
	int xoffset = 0;
	int xdrawpos = 0;

	UINT8 *adata = m_sprite_a_region.get();
	size_t  adatasize = m_sprite_a_region_size - 1;

	UINT16 msk;
	UINT16 srcdat;

	xcnt = 0;
	xcntdraw = 0;

	for (xcnt = 0 ; xcnt < wide ; xcnt++)
	{
		int x;

		msk = ((m_bdata[(m_boffset + 1) & m_bdatasize] << 8) |( m_bdata[(m_boffset + 0) & m_bdatasize] << 0));

		for (x = 0; x < 16; x++)
		{
			if (!(msk & 0x0001))
			{
				srcdat = adata[m_aoffset & adatasize] + palt * 32;
				m_aoffset++;

				if (draw)
				{
					xzoombit = (xzoom >> (xoffset & 0x1f)) & 1;
					xoffset++;

					if (xzoombit == 1 && xgrow == 1)
					{ // double this column

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, srcdat);

						xcntdraw++;

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, srcdat);

						xcntdraw++;
					}
					else if (xzoombit == 1 && xgrow == 0)
					{
						/* skip this column */
					}
					else //normal column
					{
						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix(xdrawpos, pri, dest, destpri, srcdat);

						xcntdraw++;
					}
				}

			}
			else
			{
				xzoombit = (xzoom >> (xoffset & 0x1f)) & 1;
				xoffset++;
				if (xzoombit == 1 && xgrow == 1) { xcntdraw+=2; }
				else if (xzoombit == 1 && xgrow == 0) { }
				else { xcntdraw++; }
			}

			msk >>= 1;


		}

		m_boffset += 2;
	}
}

void pgm_state::draw_sprite_new_zoomed( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, UINT32 xzoom, int xgrow, UINT32 yzoom, int ygrow, int pri )
{
	int ycnt;
	int ydrawpos;
	UINT16 *dest;
	UINT8* destpri;
	int ycntdraw;
	int yzoombit;
	int xzoombit;
	int xcnt = 0;


	m_aoffset = (m_bdata[(m_boffset + 3) & m_bdatasize] << 24) | (m_bdata[(m_boffset + 2) & m_bdatasize] << 16) |
				(m_bdata[(m_boffset + 1) & m_bdatasize] << 8) | (m_bdata[(m_boffset + 0) & m_bdatasize] << 0);
	m_aoffset = m_aoffset >> 2; m_aoffset *= 3;

	m_boffset+=4;

	/* precalculate where drawing will end, for flipped zoomed cases. */
	/* if we're to avoid pre-decoding the data for each sprite each time we draw then we have to draw the sprite data
	   in the order it is in ROM due to the nature of the compresson scheme.  This means drawing upwards from the end point
	   in the case of flipped sprites */
	ycnt = 0;
	ycntdraw = 0;
	int realysize = 0;

	while (ycnt < high)
	{
		yzoombit = (yzoom >> (ycnt & 0x1f)) & 1;
		if (yzoombit == 1 && ygrow == 1) { realysize+=2; }
		else if (yzoombit == 1 && ygrow == 0) { }
		else { realysize++; };

		ycnt++;
	}
	realysize--;

	int realxsize = 0;

	while (xcnt < wide * 16)
	{
		xzoombit = (xzoom >> (xcnt & 0x1f)) & 1;
		if (xzoombit == 1 && xgrow == 1) { realxsize+=2; }
		else if (xzoombit == 1 && xgrow == 0) { }
		else { realxsize++; };

		xcnt++;
	}
	realxsize--;


	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;

	while (ycnt < high)
	{
		yzoombit = (yzoom >> (ycnt & 0x1f)) & 1;

		if (yzoombit == 1 && ygrow == 1) // double this line
		{
			int temp_aoffset = m_aoffset;
			int temp_boffset = m_boffset;

			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = &bitmap.pix16(ydrawpos);
				destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 1);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);
			}

			ycntdraw++;

			// we need to draw this line again, so restore our pointers to previous values
			m_aoffset = temp_aoffset;
			m_boffset = temp_boffset;

			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = &bitmap.pix16(ydrawpos);
				destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 1);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);

				if (!(flip & 0x02))
				{
					if (ydrawpos>224)
						return;
				}
				else
				{
					if (ydrawpos<0)
						return;
				}
			}

			ycntdraw++;

		}
		else if (yzoombit == 1 && ygrow == 0)
		{
			/* skip this line */
			draw_sprite_line(wide, nullptr, nullptr, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);
		}
		else /* normal line */
		{
			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = &bitmap.pix16(ydrawpos);
				destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(wide, dest, destpri, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 1);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);

				if (!(flip & 0x02))
				{
					if (ydrawpos>224)
						return;
				}
				else
				{
					if (ydrawpos<0)
						return;
				}

			}

			ycntdraw++;
		}

		ycnt++;
	}
}


void pgm_state::draw_sprite_line_basic( int wide, UINT16* dest, UINT8* destpri, int flip, int xpos, int pri, int realxsize, int palt, int draw )
{
	int xcnt,xcntdraw;
	int xoffset = 0;
	int xdrawpos = 0;
	UINT8 *adata = m_sprite_a_region.get();
	size_t  adatasize = m_sprite_a_region_size - 1;

	UINT16 msk;
	UINT16 srcdat;

	xcnt = 0;
	xcntdraw = 0;

	if (!pri)
	{
		for (xcnt = 0 ; xcnt < wide ; xcnt++)
		{
			int x;

			msk = ((m_bdata[(m_boffset + 1) & m_bdatasize] << 8) |( m_bdata[(m_boffset + 0) & m_bdatasize] << 0));

			for (x = 0; x < 16; x++)
			{
				if (!(msk & 0x0001))
				{
					srcdat = adata[m_aoffset & adatasize] + palt * 32;
					m_aoffset++;

					if (draw)
					{
						xoffset++;

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix_nopri(xdrawpos, dest, destpri, srcdat);

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
		for (xcnt = 0 ; xcnt < wide ; xcnt++)
		{
			int x;

			msk = ((m_bdata[(m_boffset + 1) & m_bdatasize] << 8) |( m_bdata[(m_boffset + 0) & m_bdatasize] << 0));

			for (x = 0; x < 16; x++)
			{
				if (!(msk & 0x0001))
				{
					srcdat = adata[m_aoffset & adatasize] + palt * 32;
					m_aoffset++;

					if (draw)
					{
						xoffset++;

						if (!(flip & 0x01))
							xdrawpos = xpos + xcntdraw;
						else
							xdrawpos = xpos + realxsize - xcntdraw;

						pgm_draw_pix_pri(xdrawpos, dest, destpri, srcdat);

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

void pgm_state::draw_sprite_new_basic( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, int pri )
{
	int ycnt;
	int ydrawpos;
	UINT16 *dest;
	UINT8* destpri;
	int ycntdraw;

	m_aoffset = (m_bdata[(m_boffset + 3) & m_bdatasize] << 24) | (m_bdata[(m_boffset + 2) & m_bdatasize] << 16) |
				(m_bdata[(m_boffset + 1) & m_bdatasize] << 8) | (m_bdata[(m_boffset + 0) & m_bdatasize] << 0);
	m_aoffset = m_aoffset >> 2; m_aoffset *= 3;

	m_boffset+=4;

	int realysize = high-1;
	int realxsize = (wide * 16)-1;

	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;

	while (ycnt < high)
	{
		if (!(flip & 0x02))
			ydrawpos = ypos + ycntdraw;
		else
			ydrawpos = ypos + realysize - ycntdraw;

		if ((ydrawpos >= 0) && (ydrawpos < 224))
		{
			dest = &bitmap.pix16(ydrawpos);
			destpri = &priority_bitmap.pix8(ydrawpos);
			draw_sprite_line_basic(wide, dest, destpri, flip, xpos, pri, realxsize, palt, 1);
		}
		else
		{
			draw_sprite_line_basic(wide, nullptr, nullptr, flip, xpos, pri, realxsize, palt, 0);

			if (!(flip & 0x02))
			{
				if (ydrawpos>224)
					return;
			}
			else
			{
				if (ydrawpos<0)
					return;
			}
		}

		ycntdraw++;
		ycnt++;
	}
}


void pgm_state::draw_sprites( bitmap_ind16& spritebitmap, UINT16 *sprite_source, bitmap_ind8& priority_bitmap )
{
	/* ZZZZ Zxxx xxxx xxxx
	   zzzz z-yy yyyy yyyy
	   -ffp pppp Pvvv vvvv
	   vvvv vvvv vvvv vvvv
	   wwww wwwh hhhh hhhh
	*/

	const UINT16 *finish = m_spritebufferram.get() + (0xa00 / 2);

	UINT16* start = sprite_source;

	while (sprite_source < finish)
	{
		if (!sprite_source[4]) break; /* is this right? */
		sprite_source += 5;
	}
	sprite_source-=5;

	while (sprite_source >= start)
	{
		int xpos = sprite_source[0] & 0x07ff;
		int ypos = sprite_source[1] & 0x03ff;
		int xzom = (sprite_source[0] & 0x7800) >> 11;
		int xgrow = (sprite_source[0] & 0x8000) >> 15;
		int yzom = (sprite_source[1] & 0x7800) >> 11;
		int ygrow = (sprite_source[1] & 0x8000) >> 15;
		int palt = (sprite_source[2] & 0x1f00) >> 8;
		int flip = (sprite_source[2] & 0x6000) >> 13;
		int boff = ((sprite_source[2] & 0x007f) << 16) | (sprite_source[3] & 0xffff);
		int wide = (sprite_source[4] & 0x7e00) >> 9;
		int high = sprite_source[4] & 0x01ff;
		int pri = (sprite_source[2] & 0x0080) >>  7;

		UINT32 xzoom, yzoom;

		UINT16* sprite_zoomtable = &m_videoregs[0x1000 / 2];

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

		xzoom = (sprite_zoomtable[xzom * 2] << 16) | sprite_zoomtable[xzom * 2 + 1];
		yzoom = (sprite_zoomtable[yzom * 2] << 16) | sprite_zoomtable[yzom * 2 + 1];

		boff *= 2;
		if (xpos > 0x3ff) xpos -=0x800;
		if (ypos > 0x1ff) ypos -=0x400;

		//if ((priority == 1) && (pri == 0)) break;

		m_boffset = boff;
		if ((!xzoom) && (!yzoom)) draw_sprite_new_basic(wide, high, xpos, ypos, palt, flip, spritebitmap, priority_bitmap, pri );
		else draw_sprite_new_zoomed(wide, high, xpos, ypos, palt, flip, spritebitmap, priority_bitmap, xzoom, xgrow, yzoom, ygrow, pri );

		sprite_source -= 5;
	}
}

/* TX Layer */
WRITE16_MEMBER(pgm_state::pgm_tx_videoram_w)
{
	m_tx_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(pgm_state::get_pgm_tx_tilemap_tile_info)
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
	int tileno, colour, flipyx; //,game;

	tileno = m_tx_videoram[tile_index * 2] & 0xffff;
	colour = (m_tx_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	flipyx = (m_tx_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(0,tileno,colour,TILE_FLIPYX(flipyx));
}

/* BG Layer */

WRITE16_MEMBER(pgm_state::pgm_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(pgm_state::get_pgm_bg_tilemap_tile_info)
{
	/* pretty much the same as tx layer */

	int tileno, colour, flipyx;

	tileno = m_bg_videoram[tile_index *2] & 0xffff;

	colour = (m_bg_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	flipyx = (m_bg_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO_MEMBER(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

VIDEO_START_MEMBER(pgm_state,pgm)
{
	int i;

	m_bdata = memregion("sprmask")->base();
	m_bdatasize = memregion("sprmask")->bytes() - 1;
	m_aoffset = 0;
	m_boffset = 0;

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pgm_state::get_pgm_tx_tilemap_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tx_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pgm_state::get_pgm_bg_tilemap_tile_info),this), TILEMAP_SCAN_ROWS, 32, 32, 64, 16);
	m_bg_tilemap->set_transparent_pen(31);
	m_bg_tilemap->set_scroll_rows(16 * 32);

	for (i = 0; i < 0x1200 / 2; i++)
		m_palette->set_pen_color(i, rgb_t(0, 0, 0));

	m_spritebufferram = make_unique_clear<UINT16[]>(0xa00/2);

	save_pointer(NAME(m_spritebufferram.get()), 0xa00/2);
}

UINT32 pgm_state::screen_update_pgm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	bitmap.fill(0x3ff, cliprect); // ddp2 igs logo needs 0x3ff

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->set_scrolly(0, m_videoregs[0x2000/2]);

	for (y = 0; y < 224; y++)
		m_bg_tilemap->set_scrollx((y + m_videoregs[0x2000 / 2]) & 0x1ff, m_videoregs[0x3000 / 2] + m_rowscrollram[y]);


	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(bitmap, m_spritebufferram.get(), screen.priority());

	m_tx_tilemap->set_scrolly(0, m_videoregs[0x5000/2]);
	m_tx_tilemap->set_scrollx(0, m_videoregs[0x6000/2]); // Check


	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);


	return 0;

}

void pgm_state::screen_eof_pgm(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
		memcpy(m_spritebufferram.get(), m_mainram, 0xa00);
	}
}
