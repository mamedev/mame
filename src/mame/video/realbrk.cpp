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


void realbrk_state::realbrk_flipscreen_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0,    data & 0x0001);
		machine().bookkeeping().coin_counter_w(1,    data & 0x0004);

		flip_screen_set(    data & 0x0080);
	}

	if (ACCESSING_BITS_8_15)
	{
		m_disable_video =   data & 0x8000;
	}
}

void realbrk_state::dai2kaku_flipscreen_w(u16 data)
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

template<int Layer>
TILE_GET_INFO_MEMBER(realbrk_state::get_tile_info)
{
	const u16 attr = m_vram[Layer][tile_index * 2 + 0];
	const u16 code = m_vram[Layer][tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0,
			code,
			attr & 0x7f,
			TILE_FLIPYX( attr >> 14 ));
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
	const u16 code = m_vram[2][tile_index];
	SET_TILE_INFO_MEMBER(1,
			code & 0x0fff,
			((code & 0xf000) >> 12) | ((m_vregs[0xa/2] & 0x7f) << 4),
			0);
}

void realbrk_state::vram_2_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[2][offset]);
	m_tilemap[2]->mark_tile_dirty(offset);
}

/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void realbrk_state::video_start()
{
	/* Backgrounds */
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(realbrk_state::get_tile_info<0>),this), TILEMAP_SCAN_ROWS, 16, 16, 0x40, 0x20);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(realbrk_state::get_tile_info<1>),this), TILEMAP_SCAN_ROWS, 16, 16, 0x40, 0x20);

	/* Text */
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(realbrk_state::get_tile_info_2),this), TILEMAP_SCAN_ROWS,   8,  8, 0x40, 0x20);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

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
                ---- ba98 76-- ----
                ---- ---- --54 ----     Rotation
                ---- ---- ---- 32--
                ---- ---- ---- --10     Priority

        A.w     fedc b--- ---- ----
                ---- -a98 7654 3210     Color

        C.w     fedc ba9- ---- ----
                ---- ---8 ---- ----
                ---- ---- 7654 321-
                ---- ---- ---- ---0     1 = Use 16 color sprites, 0 = Use 256 color sprites

        E.w                             Code

***************************************************************************/

// DaiDaiKakumei
// layer : 0== bghigh<spr    1== bglow<spr<bghigh     2==spr<bglow    3==boarder
template <bool Rotatable>
void realbrk_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority)
{
	constexpr u32 PRI_MAP[4]{
			GFX_PMASK_4,                                // over m_tilemap[1], over m_tilemap[0], under m_tilemap[2]
			GFX_PMASK_4 | GFX_PMASK_2,                  // over m_tilemap[1], under m_tilemap[0], under m_tilemap[2]
			GFX_PMASK_4 | GFX_PMASK_2 | GFX_PMASK_1,    // under m_tilemap[1], under m_tilemap[0], under m_tilemap[2]
			GFX_PMASK_4 | GFX_PMASK_2 | GFX_PMASK_1 };  // unknown

	const int max_x(m_screen->width());
	const int max_y(m_screen->height());

	for (int offs = (0x3600 - 2) / 2; offs >= 0x3000 / 2; offs -= 2 / 2)
	{
		if (BIT(m_spriteram[offs], 15))
			continue;

		const u16 *const s(&m_spriteram[(m_spriteram[offs] & 0x3ff) * 16 / 2]);

		int sy          = s[0];

		int sx          = s[1];

		const int xnum  = ((s[2] >> 0) & 0x001f) + 1;
		const int ynum  = ((s[2] >> 8) & 0x001f) + 1;

		const int xdim  = ((s[3] >> 0) & 0x00ff) << (16 - 6 + 4);
		const int ydim  = ((s[3] >> 8) & 0x00ff) << (16 - 6 + 4);

		int flipx       = BIT(s[4], 8);
		int flipy       = BIT(s[4], 9);
		const u8 rot    = (s[4] >> 4) & 0x0003;
		const u8 pri    = (s[4] >> 0) & 0x0003;

		const u32 color = s[5];

		const u8 gfx    = m_gfxdecode->gfx(3) ? (s[6] & 0x0001) + 2 : 2;

		u32 code        = s[7];

		sx              = ((sx & 0x1ff) - (sx & 0x200)) << 16;
		sy              = ((sy & 0x0ff) - (sy & 0x100)) << 16;

		if (flip_screen_x()) { flipx = !flipx;     sx = (max_x << 16) - sx - xnum * xdim; }
		if (flip_screen_y()) { flipy = !flipy;     sy = (max_y << 16) - sy - ynum * ydim; }

		int xstart, xend, xinc;
		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		int ystart, yend, yinc;
		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		/*
		  The positioning of the rotated sprites makes it look as if
		  the sprite source is scanned in a constant pattern left to right,
		  top to bottom, and the destination plotting pattern is varied.
		  copyrozbitmap works the other way.

		  Rotating a sprite when drawgfxzoom draws a tile at a time means
		  - rotating each sprite tile
		  - transforming each tile position
		  - compensating for the offset introduced by the difference in
		    scanning patterns between the original mechanism and copyrozbitmap
		*/
		for (int y = ystart; y != yend; y += yinc)
		{
			for (int x = xstart; x != xend; x += xinc)
			{
				int currx = (sx + x * xdim) / 0x10000;
				int curry = (sy + y * ydim) / 0x10000;

				const int scalex = (sx + (x + 1) * xdim) / 0x10000 - currx;
				const int scaley = (sy + (y + 1) * ydim) / 0x10000 - curry;

				if (Rotatable && rot)
				{
					// buffer the tile and rotate it into bitmap
					constexpr rectangle spritetile_clip(0, 31, 0, 31);
					m_tmpbitmap0->fill(0, spritetile_clip);
					m_tmpbitmap1->fill(0, spritetile_clip);
					m_gfxdecode->gfx(gfx)->zoom_transpen(*m_tmpbitmap0, spritetile_clip,
							code++,
							color,
							flipx, flipy,
							0, 0,
							((rot & 1) ? scaley : scalex) << 12, ((rot & 1) ? scalex : scaley) << 12,
							0);

					// peek at the unrotated sprite
					// copybitmap_trans(bitmap, *m_tmpbitmap0, 0,0, 50 + (x * xdim / 0x10000), 50 + (y * ydim/0x10000), cliprect, 0);
					switch (rot)
					{
					case 0x1: // rot 90
						copyrozbitmap_trans(*m_tmpbitmap1, m_tmpbitmap1->cliprect(), *m_tmpbitmap0,
								u32(0) << 16,
								u32(16) << 16,
								0 << 16,
								0xffff << 16,
								1 << 16,
								0 << 16,
								0, 0);

						currx = (sx - (y + 1) * ydim) / 0x10000;
						curry = (sy + x * xdim) / 0x10000;
						break;

					case 0x2: // rot 180
						copyrozbitmap_trans(*m_tmpbitmap1, m_tmpbitmap1->cliprect(), *m_tmpbitmap0,
								u32(16) << 16,
								u32(16) << 16,
								0xffff << 16,
								0 << 16,
								0 << 16,
								0xffff << 16,
								0, 0);

						currx = (sx - (x + 1) * xdim) / 0x10000;
						curry = (sy - (y + 1) * ydim) / 0x10000;
						break;

					case 0x3: // rot 270
						copyrozbitmap_trans(*m_tmpbitmap1, m_tmpbitmap1->cliprect(), *m_tmpbitmap0,
								u32(16) << 16,
								u32(0) << 16,
								0 << 16,
								1 << 16,
								0xffff << 16,
								0 << 16,
								0, 0);

						currx = (sx + y * ydim) / 0x10000;
						curry = (sy - (x + 1) * xdim) / 0x10000;
						break;
					}

					prio_copybitmap_trans(bitmap, *m_tmpbitmap1, 0, 0, currx, curry, cliprect, priority, PRI_MAP[pri], 0);
				}
				else
				{
					m_gfxdecode->gfx(gfx)->prio_zoom_transpen(bitmap, cliprect,
							code++,
							color,
							flipx, flipy,
							currx, curry,
							scalex << 12, scaley << 12,
							priority, PRI_MAP[pri],
							0);
				}
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

void realbrk_state::vregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old_data = m_vregs[offset];
	data = COMBINE_DATA(&m_vregs[offset]);
	if (data != old_data)
	{
		if ((offset == 0xa/2) && ((old_data ^ data) & 0x7f))
			m_tilemap[0]->mark_all_dirty();
	}
}

u32 realbrk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrolly(0, m_vregs[0x0/2]);
	m_tilemap[0]->set_scrollx(0, m_vregs[0x2/2]);

	m_tilemap[1]->set_scrolly(0, m_vregs[0x4/2]);
	m_tilemap[1]->set_scrollx(0, m_vregs[0x6/2]);

	if (m_disable_video)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	bitmap.fill(m_vregs[0xc / 2] & 0x7fff, cliprect);
	screen.priority().fill(0, cliprect);

	int layers_ctrl(-1);
#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk(0);
		if (machine().input().code_pressed(KEYCODE_Q)) msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W)) msk |= 2;
		if (machine().input().code_pressed(KEYCODE_E)) msk |= 4;
		if (machine().input().code_pressed(KEYCODE_A)) msk |= 8;
		if (msk) layers_ctrl &= msk;
	}
#endif

	if (layers_ctrl & 2) m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);

	if (layers_ctrl & 1) m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 2);

	if (layers_ctrl & 4) m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 4);

	if (layers_ctrl & 8) draw_sprites<true>(bitmap,cliprect, screen.priority());

//  popmessage("%04x",m_vregs[0x8/2]);
	return 0;
}

/* DaiDaiKakumei */
u32 realbrk_state::screen_update_dai2kaku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// bg0
	const int bgy0(m_vregs[0x0 / 2]);
	const int bgx0(m_vregs[0x2 / 2]);
	if (BIT(m_vregs[8 / 2], 8))
	{
		m_tilemap[0]->set_scroll_rows(512);
		for (int offs = 0; offs < 512; offs++)
			m_tilemap[0]->set_scrollx(offs, bgx0 - (m_vram_ras[1][offs] & 0x3ff));
	}
	else
	{
		m_tilemap[0]->set_scroll_rows(1);
		m_tilemap[0]->set_scrollx(0, bgx0);
	}
	m_tilemap[0]->set_scrolly(0, bgy0);

	// bg1
	const int bgy1(m_vregs[0x4 / 2]);
	const int bgx1(m_vregs[0x6 / 2]);
	if (BIT(m_vregs[8 / 2], 0))
	{
		m_tilemap[1]->set_scroll_rows(512);
		for (int offs = 0; offs < 512; offs++)
			m_tilemap[1]->set_scrollx(offs, bgx1 - (m_vram_ras[1][offs] & 0x3ff));
	}
	else
	{
		m_tilemap[1]->set_scroll_rows(1);
		m_tilemap[1]->set_scrollx(0, bgx1);
	}
	m_tilemap[1]->set_scrolly(0, bgy1);

	if (m_disable_video)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	bitmap.fill(m_vregs[0xc/2] & 0x7fff, cliprect);
	screen.priority().fill(0, cliprect);

	int layers_ctrl(-1);
#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk(0);
		if (machine().input().code_pressed(KEYCODE_Q)) msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W)) msk |= 2;
		if (machine().input().code_pressed(KEYCODE_E)) msk |= 4;
		if (machine().input().code_pressed(KEYCODE_A)) msk |= 8;
		if (msk) layers_ctrl &= msk;
	}
#endif

	const bool bgpri(BIT(m_vregs[8 / 2], 15));

	// bglow
	if (layers_ctrl & (bgpri ? 1 : 2)) m_tilemap[bgpri ? 0 : 1]->draw(screen, bitmap, cliprect, 0, 1);

	// bghigh
	if (layers_ctrl & (bgpri ? 2 : 1)) m_tilemap[bgpri ? 1 : 0]->draw(screen, bitmap, cliprect, 0, 2);

	// fix
	if (layers_ctrl & 4) m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 4);

	// spr
	if (layers_ctrl & 8) draw_sprites<false>(bitmap, cliprect, screen.priority());

//  usrintf_showmessage("%04x",m_vregs[0x8/2]);
	return 0;
}
