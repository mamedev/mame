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

INLINE void pgm_draw_pix( int xdrawpos, int pri, UINT16* dest, UINT8* destpri, UINT16 srcdat)
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

INLINE void pgm_draw_pix_nopri( int xdrawpos, UINT16* dest, UINT8* destpri, UINT16 srcdat)
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

INLINE void pgm_draw_pix_pri( int xdrawpos, UINT16* dest, UINT8* destpri, UINT16 srcdat)
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

static void draw_sprite_line( running_machine &machine, int wide, UINT16* dest, UINT8* destpri, int xzoom, int xgrow, int flip, int xpos, int pri, int realxsize, int palt, int draw )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	int xcnt,xcntdraw;
	int xzoombit;
	int xoffset = 0;
	int xdrawpos = 0;

	UINT8 *adata = state->m_sprite_a_region;
	size_t  adatasize = state->m_sprite_a_region_size - 1;

	UINT16 msk;
	UINT16 srcdat;

	xcnt = 0;
	xcntdraw = 0;

	for (xcnt = 0 ; xcnt < wide ; xcnt++)
	{
		int x;

		msk = ((state->m_bdata[(state->m_boffset + 1) & state->m_bdatasize] << 8) |( state->m_bdata[(state->m_boffset + 0) & state->m_bdatasize] << 0));

		for (x = 0; x < 16; x++)
		{
			if (!(msk & 0x0001))
			{
				srcdat = adata[state->m_aoffset & adatasize] + palt * 32;
				state->m_aoffset++;

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

		state->m_boffset += 2;
	}
}

static void draw_sprite_new_zoomed( pgm_state *state, running_machine &machine, int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, UINT32 xzoom, int xgrow, UINT32 yzoom, int ygrow, int pri )
{
	int ycnt;
	int ydrawpos;
	UINT16 *dest;
	UINT8* destpri;
	int ycntdraw;
	int yzoombit;
	int xzoombit;
	int xcnt = 0;


	state->m_aoffset = (state->m_bdata[(state->m_boffset + 3) & state->m_bdatasize] << 24) | (state->m_bdata[(state->m_boffset + 2) & state->m_bdatasize] << 16) |
				(state->m_bdata[(state->m_boffset + 1) & state->m_bdatasize] << 8) | (state->m_bdata[(state->m_boffset + 0) & state->m_bdatasize] << 0);
	state->m_aoffset = state->m_aoffset >> 2; state->m_aoffset *= 3;

	state->m_boffset+=4;

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
			int temp_aoffset = state->m_aoffset;
			int temp_boffset = state->m_boffset;

			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = &bitmap.pix16(ydrawpos);
				destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(machine, wide, dest, destpri, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 1);
			}
			else
			{
				draw_sprite_line(machine, wide, NULL, NULL, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);
			}

			ycntdraw++;

			// we need to draw this line again, so restore our pointers to previous values
			state->m_aoffset = temp_aoffset;
			state->m_boffset = temp_boffset;

			if (!(flip & 0x02))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = &bitmap.pix16(ydrawpos);
				destpri = &priority_bitmap.pix8(ydrawpos);
				draw_sprite_line(machine, wide, dest, destpri, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 1);
			}
			else
			{
				draw_sprite_line(machine, wide, NULL, NULL, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);

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
			draw_sprite_line(machine, wide, NULL, NULL, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);
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
				draw_sprite_line(machine, wide, dest, destpri, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 1);
			}
			else
			{
				draw_sprite_line(machine, wide, NULL, NULL, xzoom, xgrow, flip, xpos, pri, realxsize, palt, 0);

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


static void draw_sprite_line_basic( running_machine &machine, int wide, UINT16* dest, UINT8* destpri, int flip, int xpos, int pri, int realxsize, int palt, int draw )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	int xcnt,xcntdraw;
	int xoffset = 0;
	int xdrawpos = 0;
	UINT8 *adata = state->m_sprite_a_region;
	size_t  adatasize = state->m_sprite_a_region_size - 1;

	UINT16 msk;
	UINT16 srcdat;

	xcnt = 0;
	xcntdraw = 0;

	if (!pri)
	{
		for (xcnt = 0 ; xcnt < wide ; xcnt++)
		{
			int x;

			msk = ((state->m_bdata[(state->m_boffset + 1) & state->m_bdatasize] << 8) |( state->m_bdata[(state->m_boffset + 0) & state->m_bdatasize] << 0));

			for (x = 0; x < 16; x++)
			{
				if (!(msk & 0x0001))
				{
					srcdat = adata[state->m_aoffset & adatasize] + palt * 32;
					state->m_aoffset++;

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

			state->m_boffset += 2;
		}
	}
	else
	{
		for (xcnt = 0 ; xcnt < wide ; xcnt++)
		{
			int x;

			msk = ((state->m_bdata[(state->m_boffset + 1) & state->m_bdatasize] << 8) |( state->m_bdata[(state->m_boffset + 0) & state->m_bdatasize] << 0));

			for (x = 0; x < 16; x++)
			{
				if (!(msk & 0x0001))
				{
					srcdat = adata[state->m_aoffset & adatasize] + palt * 32;
					state->m_aoffset++;

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

			state->m_boffset += 2;
		}
	}
}

/*************************************************************************
 Basic Sprite Renderer
  simplified version for non-zoomed cases, a bit faster
*************************************************************************/

static void draw_sprite_new_basic( pgm_state *state, running_machine &machine, int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, int pri )
{
	int ycnt;
	int ydrawpos;
	UINT16 *dest;
	UINT8* destpri;
	int ycntdraw;

	state->m_aoffset = (state->m_bdata[(state->m_boffset + 3) & state->m_bdatasize] << 24) | (state->m_bdata[(state->m_boffset + 2) & state->m_bdatasize] << 16) |
				(state->m_bdata[(state->m_boffset + 1) & state->m_bdatasize] << 8) | (state->m_bdata[(state->m_boffset + 0) & state->m_bdatasize] << 0);
	state->m_aoffset = state->m_aoffset >> 2; state->m_aoffset *= 3;

	state->m_boffset+=4;

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
			draw_sprite_line_basic(machine, wide, dest, destpri, flip, xpos, pri, realxsize, palt, 1);
		}
		else
		{
			draw_sprite_line_basic(machine, wide, NULL, NULL, flip, xpos, pri, realxsize, palt, 0);

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


static void draw_sprites( pgm_state *state, running_machine &machine, bitmap_ind16& spritebitmap, UINT16 *sprite_source, bitmap_ind8& priority_bitmap )
{
	/* ZZZZ Zxxx xxxx xxxx
       zzzz z-yy yyyy yyyy
       -ffp pppp Pvvv vvvv
       vvvv vvvv vvvv vvvv
       wwww wwwh hhhh hhhh
    */

	const UINT16 *finish = state->m_spritebufferram + (0xa00 / 2);

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

		UINT16* sprite_zoomtable = &state->m_videoregs[0x1000 / 2];

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

		state->m_boffset = boff;
		if ((!xzoom) && (!yzoom)) draw_sprite_new_basic(state, machine, wide, high, xpos, ypos, palt, flip, spritebitmap, priority_bitmap, pri );
		else draw_sprite_new_zoomed(state, machine, wide, high, xpos, ypos, palt, flip, spritebitmap, priority_bitmap, xzoom, xgrow, yzoom, ygrow, pri );

		sprite_source -= 5;
	}
}

/* TX Layer */
WRITE16_MEMBER(pgm_state::pgm_tx_videoram_w)
{
	m_tx_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_pgm_tx_tilemap_tile_info )
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
	pgm_state *state = machine.driver_data<pgm_state>();
	int tileno, colour, flipyx; //,game;

	tileno = state->m_tx_videoram[tile_index * 2] & 0xffff;
	colour = (state->m_tx_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	flipyx = (state->m_tx_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO(0,tileno,colour,TILE_FLIPYX(flipyx));
}

/* BG Layer */

WRITE16_MEMBER(pgm_state::pgm_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_pgm_bg_tilemap_tile_info )
{
	/* pretty much the same as tx layer */

	pgm_state *state = machine.driver_data<pgm_state>();
	int tileno, colour, flipyx;

	tileno = state->m_bg_videoram[tile_index *2] & 0xffff;

	colour = (state->m_bg_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	flipyx = (state->m_bg_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	SET_TILE_INFO(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

VIDEO_START( pgm )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	int i;

	state->m_bdata = machine.region("sprmask")->base();
	state->m_bdatasize = machine.region("sprmask")->bytes() - 1;
	state->m_aoffset = 0;
	state->m_boffset = 0;

	state->m_tx_tilemap = tilemap_create(machine, get_pgm_tx_tilemap_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_tx_tilemap->set_transparent_pen(15);

	state->m_bg_tilemap = tilemap_create(machine, get_pgm_bg_tilemap_tile_info, tilemap_scan_rows, 32, 32, 64, 16);
	state->m_bg_tilemap->set_transparent_pen(31);
	state->m_bg_tilemap->set_scroll_rows(16 * 32);

	for (i = 0; i < 0x1200 / 2; i++)
		palette_set_color(machine, i, MAKE_RGB(0, 0, 0));

	state->m_spritebufferram = auto_alloc_array_clear(machine, UINT16, 0xa00/2);

	state->save_pointer(NAME(state->m_spritebufferram), 0xa00/2);
}

SCREEN_UPDATE_IND16( pgm )
{
	pgm_state *state = screen.machine().driver_data<pgm_state>();
	int y;

	bitmap.fill(0x3ff, cliprect); // ddp2 igs logo needs 0x3ff

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg_tilemap->set_scrolly(0, state->m_videoregs[0x2000/2]);

	for (y = 0; y < 224; y++)
		state->m_bg_tilemap->set_scrollx((y + state->m_videoregs[0x2000 / 2]) & 0x1ff, state->m_videoregs[0x3000 / 2] + state->m_rowscrollram[y]);


	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 2);

	draw_sprites(state, screen.machine(), bitmap, state->m_spritebufferram, screen.machine().priority_bitmap);

	state->m_tx_tilemap->set_scrolly(0, state->m_videoregs[0x5000/2]);
	state->m_tx_tilemap->set_scrollx(0, state->m_videoregs[0x6000/2]); // Check


	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);


	return 0;

}

SCREEN_VBLANK( pgm )
{
	// rising edge
	if (vblank_on)
	{
		pgm_state *state = screen.machine().driver_data<pgm_state>();

		/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
		memcpy(state->m_spritebufferram, pgm_mainram, 0xa00);
	}
}
