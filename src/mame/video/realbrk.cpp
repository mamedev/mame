// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                      -= Billiard Academy Real Break =-

                    driver by   Luca Elia (l.elia@tin.it)

    This hardware provides for:

        -   2 scrolling background layers, 1024 x 512 in size
            made of 16 x 16 tiles with 256 colors

        -   1 text layer (fixed?), 512 x 256 in size
            made of 8 x 8 tiles with 16 colors

        -   0x300 sprites made of 16x16 tiles, both 256 or 16 colors
            per tile and from 1 to 32x32 (more?) tiles per sprite.
            Sprites can zoom / shrink / rotate


***************************************************************************/

#include "emu.h"
#include "includes/realbrk.h"


WRITE16_MEMBER(realbrk_state::realbrk_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0,    data & 0x0001);
		coin_counter_w(machine(), 1,    data & 0x0004);

		flip_screen_set(    data & 0x0080);
	}

	if (ACCESSING_BITS_8_15)
	{
		m_disable_video =   data & 0x8000;
	}
}

WRITE16_MEMBER(realbrk_state::dai2kaku_flipscreen_w)
{
	m_disable_video = 0;
}

/***************************************************************************

                                Tilemaps


***************************************************************************/



/***************************************************************************

                            Background Tilemaps

    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Flip Y
                -e-- ---- ---- ----     Flip X
                --dc ba98 7--- ----
                ---- ---- -654 3210     Color

        2.w                             Code

***************************************************************************/

TILE_GET_INFO_MEMBER(realbrk_state::get_tile_info_0)
{
	UINT16 attr = m_vram_0[tile_index * 2 + 0];
	UINT16 code = m_vram_0[tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0,
			code,
			attr & 0x7f,
			TILE_FLIPYX( attr >> 14 ));
}

TILE_GET_INFO_MEMBER(realbrk_state::get_tile_info_1)
{
	UINT16 attr = m_vram_1[tile_index * 2 + 0];
	UINT16 code = m_vram_1[tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0,
			code,
			attr & 0x7f,
			TILE_FLIPYX( attr >> 14 ));
}

WRITE16_MEMBER(realbrk_state::vram_0_w)
{
	COMBINE_DATA(&m_vram_0[offset]);
	m_tilemap_0->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(realbrk_state::vram_1_w)
{
	COMBINE_DATA(&m_vram_1[offset]);
	m_tilemap_1->mark_tile_dirty(offset/2);
}

/***************************************************************************

                                Text Tilemap

    Offset:     Bits:                   Value:

        0.w     fedc ---- ---- ----     Color
                ---- ba98 7654 3210     Code

    The full palette of 0x8000 colors can be used by this tilemap since
    a video register selects the higher bits of the color code.

***************************************************************************/

TILE_GET_INFO_MEMBER(realbrk_state::get_tile_info_2)
{
	UINT16 code = m_vram_2[tile_index];
	SET_TILE_INFO_MEMBER(1,
			code & 0x0fff,
			((code & 0xf000) >> 12) | ((m_vregs[0xa/2] & 0x7f) << 4),
			0);
}

WRITE16_MEMBER(realbrk_state::vram_2_w)
{
	COMBINE_DATA(&m_vram_2[offset]);
	m_tilemap_2->mark_tile_dirty(offset);
}



/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void realbrk_state::video_start()
{
	/* Backgrounds */
	m_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(realbrk_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 16, 16, 0x40, 0x20);
	m_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(realbrk_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 16, 16, 0x40, 0x20);

	/* Text */
	m_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(realbrk_state::get_tile_info_2),this), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x20);

	m_tilemap_0->set_transparent_pen(0);
	m_tilemap_1->set_transparent_pen(0);
	m_tilemap_2->set_transparent_pen(0);

	m_tmpbitmap0 = std::make_unique<bitmap_ind16>(32,32);
	m_tmpbitmap1 = std::make_unique<bitmap_ind16>(32,32);

	save_item(NAME(m_disable_video));
}

/***************************************************************************

                                Sprites Drawing

    Sprites RAM is 0x4000 bytes long with each sprite needing 16 bytes.

    Not all sprites must be displayed: there is a list of words at offset
    0x3000. If the high bit of a word is 0 the low bits form the index
    of a sprite to be drawn. 0x300 items of the list seem to be used.

    Each sprite is made of several 16x16 tiles (from 1 to 32x32) and
    can be zoomed / shrunk in size.

    There are two set of tiles: with 256 or 16 colors.

    Offset:     Bits:                   Value:

        0.w                             Y

        2.w                             X

        4.w     fedc ba98 ---- ----     Number of tiles along Y, minus 1 (5 bits or more ?)
                ---- ---- 7654 3210     Number of tiles along X, minus 1 (5 bits or more ?)

        6.w     fedc ba98 ---- ----     Zoom factor along Y (0x40 = no zoom)
                ---- ---- 7654 3210     Zoom factor along X (0x40 = no zoom)

        8.w     fe-- ---- ---- ----
                --d- ---- ---- ----     Flip Y
                ---c ---- ---- ----     Flip X
                ---- ---- --54 ----     Rotation
                ---- ba98 76-- 3210     Priority?

        A.w     fedc b--- ---- ----
                ---- -a98 7654 3210     Color

        C.w     fedc ba9- ---- ----
                ---- ---8 ---- ----
                ---- ---- 7654 321-
                ---- ---- ---- ---0     1 = Use 16 color sprites, 0 = Use 256 color sprites

        E.w                             Code

***************************************************************************/

void realbrk_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int offs;

	int max_x = m_screen->width();
	int max_y = m_screen->height();

	rectangle spritetile_clip(0, 31, 0, 31);

	for ( offs = 0x3000/2; offs < 0x3600/2; offs += 2/2 )
	{
		int sx, sy, dim, zoom, flip, color, attr, code, flipx, flipy, gfx, rot;

		int x, xdim, xnum, xstart, xend, xinc;
		int y, ydim, ynum, ystart, yend, yinc;

		UINT16 *s;

		if (m_spriteram[offs] & 0x8000) continue;

		s       =       &m_spriteram[(m_spriteram[offs] & 0x3ff) * 16/2];

		sy      =       s[ 0 ];
		sx      =       s[ 1 ];
		dim     =       s[ 2 ];
		zoom    =       s[ 3 ];
		flip    =       s[ 4 ];
		color   =       s[ 5 ];
		attr    =       s[ 6 ];
		code    =       s[ 7 ];

		xnum    =       ((dim >> 0) & 0x1f) + 1;
		ynum    =       ((dim >> 8) & 0x1f) + 1;

		flipx   =       flip & 0x0100;
		flipy   =       flip & 0x0200;
		rot     =       flip & 0x0030;

		gfx     =       (attr & 0x0001) + 2;

		sx      =       ((sx & 0x1ff) - (sx & 0x200)) << 16;
		sy      =       ((sy & 0x0ff) - (sy & 0x100)) << 16;

		xdim    =       ((zoom & 0x00ff) >> 0) << (16-6+4);
		ydim    =       ((zoom & 0xff00) >> 8) << (16-6+4);

		if (flip_screen_x()) {   flipx = !flipx;     sx = (max_x << 16) - sx - xnum * xdim;  }
		if (flip_screen_y()) {   flipy = !flipy;     sy = (max_y << 16) - sy - ynum * ydim;  }

		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }


		// The positioning of the rotated sprites makes it look as if
		// the sprite source is scanned in a constant pattern left to right,
		// top to bottom, and the destination plotting pattern is varied.
		// copyrozbitmap works the other way.

		// Rotating a sprite when drawgfxzoom draws a tile at a time means
		// - rotating each sprite tile
		// - transforming each tile position
		// - compensating for the offset introduced by the difference in
		//   scanning patterns between the original mechanism and copyrozbitmap


		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				int currx = (sx + x * xdim) / 0x10000;
				int curry = (sy + y * ydim) / 0x10000;

				int scalex = (sx + (x + 1) * xdim) / 0x10000 - currx;
				int scaley = (sy + (y + 1) * ydim) / 0x10000 - curry;

				// buffer the tile and rotate it into bitmap
				if( rot )
				{
					m_tmpbitmap0->fill(0, spritetile_clip );
					m_tmpbitmap1->fill(0, spritetile_clip );
					m_gfxdecode->gfx(gfx)->zoom_transpen(*m_tmpbitmap0,spritetile_clip,
									code++,
									color,
									flipx, flipy,
									0,0,
									(rot & 1 ? scaley : scalex) << 12, (rot & 1 ? scalex : scaley) << 12,0);

					// peek at the unrotated sprite
					// copybitmap_trans( bitmap,*m_tmpbitmap0, 0,0, 50+(x * xdim/0x10000),50+(y * ydim/0x10000), cliprect, 0 );
				}

				switch( rot )
				{
					case 0x10: // rot 90
						copyrozbitmap_trans( *m_tmpbitmap1, m_tmpbitmap1->cliprect(), *m_tmpbitmap0,
							(UINT32)0<<16,
							(UINT32)16<<16,
							0 << 16,
							0xffff << 16,
							1 << 16,
							0 << 16,
							0, 0 );

							currx = (sx - (y+1) * ydim) / 0x10000;
							curry = (sy + x * xdim) / 0x10000;

							copybitmap_trans( bitmap,*m_tmpbitmap1, 0,0, currx,curry, cliprect, 0 );
						break;

					case 0x20: // rot 180
						copyrozbitmap_trans( *m_tmpbitmap1, m_tmpbitmap1->cliprect(), *m_tmpbitmap0,
							(UINT32)16<<16,
							(UINT32)16<<16,
							0xffff << 16,
							0 << 16,
							0 << 16,
							0xffff << 16,
							0, 0 );

							currx = (sx - (x+1) * xdim) / 0x10000;
							curry = (sy - (y+1) * ydim) / 0x10000;

							copybitmap_trans( bitmap,*m_tmpbitmap1, 0,0, currx,curry, cliprect, 0 );
						break;

					case 0x30: // rot 270
						copyrozbitmap_trans( *m_tmpbitmap1, m_tmpbitmap1->cliprect(), *m_tmpbitmap0,
							(UINT32)16<<16,
							(UINT32)0<<16,
							0 << 16,
							1 << 16,
							0xffff << 16,
							0 << 16,
							0, 0 );

							currx = (sx + y * ydim) / 0x10000;
							curry = (sy - (x+1) * xdim) / 0x10000;

							copybitmap_trans( bitmap,*m_tmpbitmap1, 0,0, currx,curry, cliprect, 0 );
						break;

					default:
						m_gfxdecode->gfx(gfx)->zoom_transpen(bitmap,cliprect,
										code++,
										color,
										flipx, flipy,
										currx, curry,
										scalex << 12, scaley << 12,0);
						break;
				}

			}
		}
	}
}

/* DaiDaiKakumei */
/* layer : 0== bghigh<spr    1== bglow<spr<bghigh     2==spr<bglow    3==boarder */
void realbrk_state::dai2kaku_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int layer)
{
	int offs;

	int max_x = m_screen->width();
	int max_y = m_screen->height();

	for ( offs = 0x3000/2; offs < 0x3600/2; offs += 2/2 )
	{
		int sx, sy, dim, zoom, flip, color, attr, code, flipx, flipy, gfx;

		int x, xdim, xnum, xstart, xend, xinc;
		int y, ydim, ynum, ystart, yend, yinc;

		UINT16 *s;

		if (m_spriteram[offs] & 0x8000) continue;

		s       =       &m_spriteram[(m_spriteram[offs] & 0x3ff) * 16/2];

		sy      =       s[ 0 ];
		sx      =       s[ 1 ];
		dim     =       s[ 2 ];
		zoom    =       s[ 3 ];
		flip    =       s[ 4 ];
		color   =       s[ 5 ];
		attr    =       s[ 6 ];
		code    =       s[ 7 ];

		if(( flip & 0x03 ) != layer ) continue;

		xnum    =       ((dim >> 0) & 0x1f) + 1;
		ynum    =       ((dim >> 8) & 0x1f) + 1;

		flipx   =       flip & 0x0100;
		flipy   =       flip & 0x0200;

		gfx     =       (attr & 0x0001) + 2;

		sx      =       ((sx & 0x1ff) - (sx & 0x200)) << 16;
		sy      =       ((sy & 0x0ff) - (sy & 0x100)) << 16;

		xdim    =       ((zoom & 0x00ff) >> 0) << (16-6+4);
		ydim    =       ((zoom & 0xff00) >> 8) << (16-6+4);

		if (flip_screen_x()) {   flipx = !flipx;     sx = (max_x << 16) - sx - xnum * xdim;  }
		if (flip_screen_y()) {   flipy = !flipy;     sy = (max_y << 16) - sy - ynum * ydim;  }

		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				int currx = (sx + x * xdim) / 0x10000;
				int curry = (sy + y * ydim) / 0x10000;

				int scalex = (sx + (x + 1) * xdim) / 0x10000 - currx;
				int scaley = (sy + (y + 1) * ydim) / 0x10000 - curry;

				m_gfxdecode->gfx(gfx)->zoom_transpen(bitmap,cliprect,
								code++,
								color,
								flipx, flipy,
								currx, curry,
								scalex << 12, scaley << 12,0);
			}
		}
	}
}


/***************************************************************************

                                Screen Drawing

    Video Registers:

    Offset:     Bits:                   Value:

    0.w                                 Background 0 Scroll Y

    2.w                                 Background 0 Scroll X

    4.w                                 Background 1 Scroll Y

    6.w                                 Background 1 Scroll X

    8.w         fedc ba98 ---- ----     ? bit f = flip
                ---- ---- 7654 3210

    A.w         fedc ba98 7--- ----
                ---- ---- -654 3210     Color codes high bits for the text tilemap

    C.w         f--- ---- ---- ----
                -edc ba98 7654 3210     Index of the background color

***************************************************************************/

WRITE16_MEMBER(realbrk_state::vregs_w)
{
	UINT16 old_data = m_vregs[offset];
	UINT16 new_data = COMBINE_DATA(&m_vregs[offset]);
	if (new_data != old_data)
	{
		if (offset == 0xa/2)
			m_tilemap_0->mark_all_dirty();
	}
}

UINT32 realbrk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	m_tilemap_0->set_scrolly(0, m_vregs[0x0/2]);
	m_tilemap_0->set_scrollx(0, m_vregs[0x2/2]);

	m_tilemap_1->set_scrolly(0, m_vregs[0x4/2]);
	m_tilemap_1->set_scrollx(0, m_vregs[0x6/2]);

#ifdef MAME_DEBUG
if ( machine().input().code_pressed(KEYCODE_Z) )
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (m_disable_video)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}
	else
		bitmap.fill(m_vregs[0xc/2] & 0x7fff, cliprect);

	if (layers_ctrl & 2)    m_tilemap_1->draw(screen, bitmap, cliprect, 0,0);
	if (layers_ctrl & 1)    m_tilemap_0->draw(screen, bitmap, cliprect, 0,0);

	if (layers_ctrl & 8)    draw_sprites(bitmap,cliprect);

	if (layers_ctrl & 4)    m_tilemap_2->draw(screen, bitmap, cliprect, 0,0);

//  popmessage("%04x",m_vregs[0x8/2]);
	return 0;
}

/* DaiDaiKakumei */
UINT32 realbrk_state::screen_update_dai2kaku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;
	int offs, bgx0, bgy0, bgx1, bgy1;

	bgy0 = m_vregs[0x0/2];
	bgx0 = m_vregs[0x2/2];
	bgy1 = m_vregs[0x4/2];
	bgx1 = m_vregs[0x6/2];

	// bg0
	m_tilemap_0->set_scroll_rows(512);
	m_tilemap_0->set_scroll_cols(1);
	if( m_vregs[8/2] & (0x0100)){
		for(offs=0; offs<(512); offs++) {
			m_tilemap_0->set_scrollx(offs, bgx0 - (m_vram_1ras[offs]&0x3ff) );
		}
	} else {
		for(offs=0; offs<(512); offs++) {
			m_tilemap_0->set_scrollx(offs, bgx0 );
		}
	}
	m_tilemap_0->set_scrolly(0, bgy0 );

	// bg1
	m_tilemap_1->set_scroll_rows(512);
	m_tilemap_1->set_scroll_cols(1);
	if( m_vregs[8/2] & (0x0001)){
		for(offs=0; offs<(512); offs++) {
			m_tilemap_1->set_scrollx(offs, bgx1 - (m_vram_1ras[offs]&0x3ff) );
		}
	} else {
		for(offs=0; offs<(512); offs++) {
			m_tilemap_1->set_scrollx(offs, bgx1 );
		}
	}
	m_tilemap_1->set_scrolly(0, bgy1 );

#ifdef MAME_DEBUG
if ( machine().input().code_pressed(KEYCODE_Z) )
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (m_disable_video)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}
	else
		bitmap.fill(m_vregs[0xc/2] & 0x7fff, cliprect);



	// spr 0
	if (layers_ctrl & 8)    dai2kaku_draw_sprites(bitmap,cliprect,2);

	// bglow
	if( m_vregs[8/2] & (0x8000)){
		if (layers_ctrl & 1)    m_tilemap_0->draw(screen, bitmap, cliprect, 0,0);
	} else {
		if (layers_ctrl & 2)    m_tilemap_1->draw(screen, bitmap, cliprect, 0,0);
	}

	// spr 1
	if (layers_ctrl & 8)    dai2kaku_draw_sprites(bitmap,cliprect,1);

	// bghigh
	if( m_vregs[8/2] & (0x8000)){
		if (layers_ctrl & 2)    m_tilemap_1->draw(screen, bitmap, cliprect, 0,0);
	} else {
		if (layers_ctrl & 1)    m_tilemap_0->draw(screen, bitmap, cliprect, 0,0);
	}

	// spr 2
	if (layers_ctrl & 8)    dai2kaku_draw_sprites(bitmap,cliprect,0);

	// fix
	if (layers_ctrl & 4)    m_tilemap_2->draw(screen, bitmap, cliprect, 0,0);

//  usrintf_showmessage("%04x",m_vregs[0x8/2]);
	return 0;
}
