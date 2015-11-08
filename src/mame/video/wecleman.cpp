// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************
                        WEC Le Mans 24  &   Hot Chase

                          (C)   1986 & 1988 Konami

                    driver by       Luca Elia (l.elia@tin.it)

***************************************************************************/

#include "emu.h"
#include "includes/wecleman.h"

#define BMP_PAD     8
#define BLEND_STEPS 16
#define BLEND_MIN   0
#define BLEND_MAX   (BLEND_STEPS*0x20-1)
#define BLEND_INC   1
#define BLEND_DEC   -8

#define SPRITE_FLIPX    0x01
#define SPRITE_FLIPY    0x02
#define NUM_SPRITES     256

struct sprite
{
	UINT8 *pen_data;    /* points to top left corner of tile data */
	int line_offset;

	const pen_t *pal_data;
	rgb_t pal_base;

	int x_offset, y_offset;
	int tile_width, tile_height;
	int total_width, total_height;  /* in screen coordinates */
	int x, y;
	int shadow_mode, flags;
};


/***************************************************************************

                        Sprite Description and Routines
                        -------------------------------

    Sprites: 256 entries, 16 bytes each, first ten bytes used (and tested)

    Offset  Bits                    Meaning

    00.w    fedc ba98 ---- ----     Screen Y stop
            ---- ---- 7654 3210     Screen Y start

    02.w    fedc ba-- ---- ----     High bits of sprite "address"
            ---- --9- ---- ----     Flip Y ?
            ---- ---8 7654 3210     Screen X start

    04.w    fedc ba98 ---- ----     Color
            ---- ---- 7654 3210     Source Width / 8

    06.w    f--- ---- ---- ----     Flip X
            -edc ba98 7654 3210     Low bits of sprite "address"

    08.w    --dc ba98 ---- ----     Y? Shrink Factor
            ---- ---- --54 3210     X? Shrink Factor

    Sprite "address" is the index of the pixel the hardware has to start
    fetching data from, divided by 8. Only the on-screen height and source data
    width are provided, along with two shrinking factors. So on screen width
    and source height are calculated by the hardware using the shrink factors.
    The factors are in the range 0 (no shrinking) - 3F (half size).

    Hot Chase: shadow of trees is pen 0x0a

***************************************************************************/

void wecleman_state::get_sprite_info()
{
	const pen_t *base_pal = m_palette->pens();
	UINT8 *base_gfx = memregion("gfx1")->base();
	int gfx_max     = memregion("gfx1")->bytes();

	UINT16 *source = m_spriteram;

	struct sprite *sprite = m_sprite_list;
	struct sprite *finish = m_sprite_list + NUM_SPRITES;

	int bank, code, gfx, zoom;

	for (m_spr_count=0; sprite<finish; source+=0x10/2, sprite++)
	{
		if (source[0x00/2] == 0xffff) break;

		sprite->y = source[0x00/2] & 0xff;
		sprite->total_height = (source[0x00/2] >> 8) - sprite->y;
		if (sprite->total_height < 1) continue;

		sprite->x = source[0x02/2] & 0x1ff;
		bank = source[0x02/2] >> 10;
		if (bank == 0x3f) continue;

		sprite->tile_width = source[0x04/2] & 0xff;
		if (sprite->tile_width < 1) continue;

		sprite->shadow_mode = source[0x04/2] & 0x4000;

		code = source[0x06/2];
		zoom = source[0x08/2];

		sprite->pal_base = (source[0x0e/2] & 0xff) << 4;
		sprite->pal_data = base_pal + sprite->pal_base;

		gfx = (m_gfx_bank[bank] << 15) + (code & 0x7fff);

		sprite->flags = 0;
		if (code & 0x8000) { sprite->flags |= SPRITE_FLIPX; gfx += 1-sprite->tile_width; }
		if (source[0x02/2] & 0x0200) sprite->flags |= SPRITE_FLIPY;

		gfx <<= 3;
		sprite->tile_width <<= 3;
		sprite->tile_height = (sprite->total_height * 0x80) / (0x80 - (zoom >> 8)); // needs work

		if ((gfx + sprite->tile_width * sprite->tile_height - 1) >= gfx_max) continue;

		sprite->pen_data = base_gfx + gfx;
		sprite->line_offset = sprite->tile_width;
		sprite->total_width = sprite->tile_width - (sprite->tile_width * (zoom & 0xff)) / 0x80;
		sprite->total_height += 1;
		sprite->x += m_spr_offsx;
		sprite->y += m_spr_offsy;

		if (m_gameid == 0)
		{
			m_spr_idx_list[m_spr_count] = m_spr_count;
			m_spr_pri_list[m_spr_count] = source[0x0e/2] >> 8;
		}

		m_spr_ptr_list[m_spr_count] = sprite;
		m_spr_count++;
	}
}

// priority sorting, silly but good for smaller arrays
void wecleman_state::sortsprite(int *idx_array, int *key_array, int size)
{
	int i, j, tgt_val, low_val, low_pos, src_idx, tgt_idx, hi_idx;

	idx_array += size;

	for (j=-size; j<-1; j++)
	{
		src_idx = idx_array[j];
		low_pos = j;
		low_val = key_array[src_idx];
		hi_idx = src_idx;
		for (i=j+1; i; i++)
		{
			tgt_idx = idx_array[i];
			tgt_val = key_array[tgt_idx];
			if (low_val > tgt_val)
				{ low_val = tgt_val; low_pos = i; }
			else if ((low_val == tgt_val) && (hi_idx <= tgt_idx))
				{ hi_idx = tgt_idx; low_pos = i; }
		}
		low_val = idx_array[low_pos];
		idx_array[low_pos] = src_idx;
		idx_array[j] = low_val;
	}
}

// draws a 8bpp palette sprites on a 16bpp direct RGB target (sub-par implementation)
template<class _BitmapClass>
void wecleman_state::do_blit_zoom32(_BitmapClass &bitmap, const rectangle &cliprect, struct sprite *sprite)
{
#define PRECISION_X 20
#define PRECISION_Y 20
#define FPY_HALF (1<<(PRECISION_Y-1))

	const pen_t *pal_base;
	int src_f0y, src_fdy, src_f0x, src_fdx, src_fpx;
	int x1, x2, y1, y2, dx, dy, sx, sy;
	int xcount0=0, ycount0=0;

	if (sprite->flags & SPRITE_FLIPX)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < cliprect.min_x) x2 = cliprect.min_x;
		if (x1 > cliprect.max_x )
		{
			xcount0 = x1 - cliprect.max_x;
			x1 = cliprect.max_x;
		}
		if (x2 >= x1) return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < cliprect.min_x )
		{
			xcount0 = cliprect.min_x - x1;
			x1 = cliprect.min_x;
		}
		if (x2 > cliprect.max_x ) x2 = cliprect.max_x;
		if (x1 >= x2) return;
	}

	if (sprite->flags & SPRITE_FLIPY)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < cliprect.min_y ) y2 = cliprect.min_y;
		if (y1 > cliprect.max_y )
		{
			ycount0 = cliprect.max_y;
			y1 = cliprect.max_y;
		}
		if (y2 >= y1) return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < cliprect.min_y )
		{
			ycount0 = cliprect.min_y - y1;
			y1 = cliprect.min_y;
		}
		if (y2 > cliprect.max_y) y2 = cliprect.max_y;
		if (y1 >= y2) return;
	}

	// calculate entry point decimals
	src_fdy = (sprite->tile_height<<PRECISION_Y) / sprite->total_height;
	src_f0y = src_fdy * ycount0 + FPY_HALF;

	src_fdx = (sprite->tile_width<<PRECISION_X) / sprite->total_width;
	src_f0x = src_fdx * xcount0;

	// pre-loop assignments and adjustments
	pal_base = sprite->pal_data;

	if (x1 > cliprect.min_x)
	{
		x1 -= dx;
		x2 -= dx;
	}

	for (sy = y1; sy != y2; sy += dy)
	{
		UINT8 *row_base = sprite->pen_data + (src_f0y>>PRECISION_Y) * sprite->line_offset;
		src_fpx = src_f0x;
		typename _BitmapClass::pixel_t *dst_ptr = &bitmap.pix(sy);

		if (bitmap.format() == BITMAP_FORMAT_RGB32) // Wec Le Mans
		{
			if (!sprite->shadow_mode)
			{
				for (sx = x1; sx != x2; sx += dx)
				{
					int pix = row_base[src_fpx >> PRECISION_X];
					if (pix & 0x80) break;
					if (pix)
						dst_ptr[sx] = pal_base[pix];
					src_fpx += src_fdx;
				}
			}
			else
			{
				for (sx = x1; sx != x2; sx += dx)
				{
					int pix = row_base[src_fpx >> PRECISION_X];
					if (pix & 0x80) break;
					if (pix)
					{
						if (pix != 0xa)
							dst_ptr[sx] = pal_base[pix];
						else
							dst_ptr[sx] = (dst_ptr[sx] >> 1) & rgb_t(0x7f,0x7f,0x7f);
					}
					src_fpx += src_fdx;
				}
			}
		}
		else    // Hot Chase
		{
			pen_t base = sprite->pal_base;

			if (!sprite->shadow_mode)
			{
				for (sx = x1; sx != x2; sx += dx)
				{
					int pix = row_base[src_fpx >> PRECISION_X];
					if (pix & 0x80) break;
					if (pix)
						dst_ptr[sx] = base + pix;
					src_fpx += src_fdx;
				}
			}
			else
			{
				for (sx = x1; sx != x2; sx += dx)
				{
					int pix = row_base[src_fpx >> PRECISION_X];
					if (pix & 0x80) break;
					if (pix)
					{
						if (pix != 0xa)
							dst_ptr[sx] = base + pix;
						else
						{
							if (dst_ptr[sx] != m_black_pen)
								dst_ptr[sx] |= 0x800;
						}
					}
					src_fpx += src_fdx;
				}
			}
		}

		src_f0y += src_fdy;
	}
}

template<class _BitmapClass>
void wecleman_state::sprite_draw(_BitmapClass &bitmap, const rectangle &cliprect)
{
	int i;

	if (m_gameid == 0)   // Wec Le Mans
	{
		sortsprite(m_spr_idx_list, m_spr_pri_list, m_spr_count);

		for (i=0; i<m_spr_count; i++) do_blit_zoom32(bitmap, cliprect, m_spr_ptr_list[m_spr_idx_list[i]]);
	}
	else    // Hot Chase
	{
		for (i=0; i<m_spr_count; i++) do_blit_zoom32(bitmap, cliprect, m_spr_ptr_list[i]);
	}
}


/***************************************************************************

                    Background Description and Routines
                    -----------------------------------

                            [WEC Le Mans 24]

[ 2 Scrolling Layers ]
    [Background]
    [Foreground]
        Tile Size:              8x8

        Tile Format:            see wecleman_get_bg_tile_info()

        Layer Size:             4 Pages - Page0 Page1 Page2 Page3
                                each page is 512 x 256 (64 x 32 tiles)

        Page Selection Reg.:    108efe  [Bg]
                                108efc  [Fg]
                                4 pages to choose from

        Scrolling Columns:      1
        Scrolling Columns Reg.: 108f26  [Bg]
                                108f24  [Fg]

        Scrolling Rows:         224 / 8 (Screen-wise scrolling)
        Scrolling Rows Reg.:    108f82/4/6..    [Bg]
                                108f80/2/4..    [Fg]

[ 1 Text Layer ]
        Tile Size:              8x8

        Tile Format:            see wecleman_get_txt_tile_info()

        Layer Size:             1 Page: 512 x 256 (64 x 32 tiles)

        Scrolling:              -

[ 1 Road Layer ]

[ 256 Sprites ]
    Zooming Sprites, see below


                                [Hot Chase]

[ 3 Zooming Layers ]
    [Background]
    [Foreground (text)]
    [Road]

[ 256 Sprites ]
    Zooming Sprites, see below

***************************************************************************/

/***************************************************************************
                                WEC Le Mans 24
***************************************************************************/

#define PAGE_GFX        (0)
#define PAGE_NX         (0x40)
#define PAGE_NY         (0x20)
#define TILEMAP_DIMY    (PAGE_NY * 2 * 8)

/*------------------------------------------------------------------------
                [ Frontmost (text) layer + video registers ]
------------------------------------------------------------------------*/

TILE_GET_INFO_MEMBER(wecleman_state::wecleman_get_txt_tile_info)
{
	int code = m_txtram[tile_index];
	SET_TILE_INFO_MEMBER(PAGE_GFX, code&0xfff, (code>>5&0x78)+(code>>12), 0);
}

WRITE16_MEMBER(wecleman_state::wecleman_txtram_w)
{
	UINT16 old_data = m_txtram[offset];
	UINT16 new_data = COMBINE_DATA(&m_txtram[offset]);

	if ( old_data != new_data )
	{
		if (offset >= 0xE00/2 ) /* Video registers */
		{
			/* pages selector for the background */
			if (offset == 0xEFE/2)
			{
				m_bgpage[0] = (new_data >> 0x4) & 3;
				m_bgpage[1] = (new_data >> 0x0) & 3;
				m_bgpage[2] = (new_data >> 0xc) & 3;
				m_bgpage[3] = (new_data >> 0x8) & 3;
				m_bg_tilemap->mark_all_dirty();
			}

			/* pages selector for the foreground */
			if (offset == 0xEFC/2)
			{
				m_fgpage[0] = (new_data >> 0x4) & 3;
				m_fgpage[1] = (new_data >> 0x0) & 3;
				m_fgpage[2] = (new_data >> 0xc) & 3;
				m_fgpage[3] = (new_data >> 0x8) & 3;
				m_fg_tilemap->mark_all_dirty();
			}

			/* Parallactic horizontal scroll registers follow */
		}
		else
			m_txt_tilemap->mark_tile_dirty(offset);
	}
}

/*------------------------------------------------------------------------
                            [ Background ]
------------------------------------------------------------------------*/

TILE_GET_INFO_MEMBER(wecleman_state::wecleman_get_bg_tile_info)
{
	int page = m_bgpage[((tile_index&0x7f)>>6) + ((tile_index>>12)<<1)];
	int code = m_pageram[(tile_index&0x3f) + ((tile_index>>7&0x1f)<<6) + (page<<11)];

	SET_TILE_INFO_MEMBER(PAGE_GFX, code&0xfff, (code>>5&0x78)+(code>>12), 0);
}

/*------------------------------------------------------------------------
                            [ Foreground ]
------------------------------------------------------------------------*/

TILE_GET_INFO_MEMBER(wecleman_state::wecleman_get_fg_tile_info)
{
	int page = m_fgpage[((tile_index&0x7f)>>6) + ((tile_index>>12)<<1)];
	int code = m_pageram[(tile_index&0x3f) + ((tile_index>>7&0x1f)<<6) + (page<<11)];

	if (!code || code==0xffff) code = 0x20;
	SET_TILE_INFO_MEMBER(PAGE_GFX, code&0xfff, (code>>5&0x78)+(code>>12), 0);
}

/*------------------------------------------------------------------------
                    [ Pages (Background & Foreground) ]
------------------------------------------------------------------------*/

/* Pages that compose both the background and the foreground */
WRITE16_MEMBER(wecleman_state::wecleman_pageram_w)
{
	COMBINE_DATA(&m_pageram[offset]);

	{
		int page,col,row;

		page = ( offset ) / (PAGE_NX * PAGE_NY);
		col  = ( offset ) % PAGE_NX;
		row  = ( offset / PAGE_NX ) % PAGE_NY;

		/* background */
		if (m_bgpage[0] == page) m_bg_tilemap->mark_tile_dirty((col+PAGE_NX*0) + (row+PAGE_NY*0)*PAGE_NX*2 );
		if (m_bgpage[1] == page) m_bg_tilemap->mark_tile_dirty((col+PAGE_NX*1) + (row+PAGE_NY*0)*PAGE_NX*2 );
		if (m_bgpage[2] == page) m_bg_tilemap->mark_tile_dirty((col+PAGE_NX*0) + (row+PAGE_NY*1)*PAGE_NX*2 );
		if (m_bgpage[3] == page) m_bg_tilemap->mark_tile_dirty((col+PAGE_NX*1) + (row+PAGE_NY*1)*PAGE_NX*2 );

		/* foreground */
		if (m_fgpage[0] == page) m_fg_tilemap->mark_tile_dirty((col+PAGE_NX*0) + (row+PAGE_NY*0)*PAGE_NX*2 );
		if (m_fgpage[1] == page) m_fg_tilemap->mark_tile_dirty((col+PAGE_NX*1) + (row+PAGE_NY*0)*PAGE_NX*2 );
		if (m_fgpage[2] == page) m_fg_tilemap->mark_tile_dirty((col+PAGE_NX*0) + (row+PAGE_NY*1)*PAGE_NX*2 );
		if (m_fgpage[3] == page) m_fg_tilemap->mark_tile_dirty((col+PAGE_NX*1) + (row+PAGE_NY*1)*PAGE_NX*2 );
	}
}

/*------------------------------------------------------------------------
                                Road Drawing

    This layer is composed of horizontal lines gfx elements
    There are 256 lines in ROM, each is 512 pixels wide

    Offset:         Elements:       Data:
    0000-01ff       100 Words       Code

        fedcba98--------    Priority?
        --------76543210    Line Number

    0200-03ff       100 Words       Horizontal Scroll
    0400-05ff       100 Words       Color
    0600-07ff       100 Words       ??

    We draw each line using a bunch of 64x1 tiles

------------------------------------------------------------------------*/

void wecleman_state::wecleman_draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority)
{
// must be powers of 2
#define XSIZE 512
#define YSIZE 256

#define YMASK (YSIZE-1)

#define DST_WIDTH 320
#define DST_HEIGHT 224

#define MIDCURB_DY 5
#define TOPCURB_DY 7

	static const pen_t road_color[48] =
	{
		0x3f1,0x3f3,0x3f5,0x3fd,0x3fd,0x3fb,0x3fd,0x7ff,    // road color 0
		0x3f0,0x3f2,0x3f4,0x3fc,0x3fc,0x3fb,0x3fc,0x7fe,    // road color 1
			0,    0,    0,0x3f9,0x3f9,    0,    0,    0,    // midcurb color 0
			0,    0,    0,0x3f8,0x3f8,    0,    0,    0,    // midcurb color 1
			0,    0,    0,0x3f7,    0,    0,    0,    0,    // topcurb color 0
			0,    0,    0,0x3f6,    0,    0,    0,    0     // topcutb color 1
	};


	const UINT8 *src_ptr;
	const pen_t *pal_ptr, *rgb_ptr;

	int scrollx, sy, sx;
	int mdy, tdy, i;

	rgb_ptr = m_palette->pens();

	if (priority == 0x02)
	{
		// draw sky; each scanline is assumed to be dword aligned
		for (sy=cliprect.min_y-BMP_PAD; sy<DST_HEIGHT; sy++)
		{
			UINT32 *dst = &bitmap.pix32(sy+BMP_PAD, BMP_PAD);
			UINT32 pix;
			UINT16 road;

			road = m_roadram[sy];
			if ((road>>8) != 0x02) continue;

			pix = rgb_ptr[(m_roadram[sy+(YSIZE*2)] & 0xf) + 0x7f0];

			for (sx = 0; sx < DST_WIDTH; sx++)
				dst[sx] = pix;
		}
	}
	else if (priority == 0x04)
	{
		// draw road
		pen_t road_rgb[48];

		for (i=0; i<48; i++)
		{
			int color = road_color[i];
			road_rgb[i] = color ? rgb_ptr[color] : 0xffffffff;
		}

		for (sy=cliprect.min_y-BMP_PAD; sy<DST_HEIGHT; sy++)
		{
			UINT32 *dst = &bitmap.pix32(sy+BMP_PAD, BMP_PAD);
			UINT32 pix;
			UINT16 road;

			road = m_roadram[sy];
			if ((road>>8) != 0x04) continue;
			road &= YMASK;

			src_ptr = m_gfxdecode->gfx(1)->get_data((road << 3));
			m_gfxdecode->gfx(1)->get_data((road << 3) + 1);
			m_gfxdecode->gfx(1)->get_data((road << 3) + 2);
			m_gfxdecode->gfx(1)->get_data((road << 3) + 3);
			m_gfxdecode->gfx(1)->get_data((road << 3) + 4);
			m_gfxdecode->gfx(1)->get_data((road << 3) + 5);
			m_gfxdecode->gfx(1)->get_data((road << 3) + 6);
			m_gfxdecode->gfx(1)->get_data((road << 3) + 7);
			mdy = ((road * MIDCURB_DY) >> 8) * bitmap.rowpixels();
			tdy = ((road * TOPCURB_DY) >> 8) * bitmap.rowpixels();

			scrollx = m_roadram[sy+YSIZE] + (0x18 - 0xe00);

			pal_ptr = road_rgb + ((m_roadram[sy+(YSIZE*2)]<<3) & 8);

			for (sx = 0; sx < DST_WIDTH; sx++, scrollx++)
			{
				if (scrollx >= 0 && scrollx < XSIZE)
				{
					pen_t temp;

					pix = src_ptr[scrollx];
					dst[sx] = pal_ptr[pix];

					temp = pal_ptr[pix + 16];
					if (temp != 0xffffffff) dst[sx - mdy] = temp;

					temp = pal_ptr[pix + 32];
					if (temp != 0xffffffff) dst[sx - tdy] = temp;
				}
				else
					dst[sx] = pal_ptr[7];
			}
		}
	}

#undef YSIZE
#undef XSIZE
}

/*------------------------------------------------------------------------
                                Sky Drawing
------------------------------------------------------------------------*/

// blends two 8x8x16bpp direct RGB tilemaps
void wecleman_state::draw_cloud(bitmap_rgb32 &bitmap,
					gfx_element *gfx,
					UINT16 *tm_base,
					int x0, int y0,             // target coordinate
					int xcount, int ycount,     // number of tiles to draw in x and y
					int scrollx, int scrolly,       // tilemap scroll position
					int tmw_l2, int tmh_l2,     // tilemap width and height in log(2)
					int alpha, int pal_offset ) // alpha(0-3f), # of color codes to shift
{
	const UINT8 *src_ptr;
	UINT16 *tmap_ptr;
	UINT32 *dst_base, *dst_ptr;
	const pen_t *pal_base, *pal_ptr;

	int tilew, tileh;
	int tmskipx, tmskipy, tmscanx, tmmaskx, tmmasky;
	int dx, dy;
	int i, j, tx, ty;

	if (alpha > 0x1f) return;

	tilew = gfx->width();
	tileh = gfx->height();

	tmmaskx = (1<<tmw_l2) - 1;
	tmmasky = (1<<tmh_l2) - 1;

	scrollx &= ((tilew<<tmw_l2) - 1);
	scrolly &= ((tileh<<tmh_l2) - 1);

	tmskipx = scrollx / tilew;
	dx = -(scrollx & (tilew-1));
	tmskipy = scrolly / tileh;
	dy = -(scrolly & (tileh-1));

	dst_base = &bitmap.pix32(y0+dy, x0+dx);

	pal_base = m_palette->pens() + pal_offset * gfx->granularity();

	alpha <<= 6;

	dst_base += 8;
	for (i = 0; i < ycount; i++)
	{
		tmap_ptr = tm_base + ((tmskipy++ & tmmasky)<<tmw_l2);
		tmscanx = tmskipx;

		for (j = 0; j < xcount; j++)
		{
			UINT16 tiledata = tmap_ptr[tmscanx++ & tmmaskx];

			// Wec Le Mans specific: decodes tile index in EBX
			UINT16 tile_index = tiledata & 0xfff;

			// Wec Le Mans specific: decodes tile color in EAX
			UINT16 tile_color = ((tiledata >> 5) & 0x78) + (tiledata >> 12);

			src_ptr = gfx->get_data(tile_index);
			pal_ptr = pal_base + tile_color * gfx->granularity();
			dst_ptr = dst_base + j * tilew;

			/* alpha case */
			if (alpha > 0)
			{
				for (ty = 0; ty < tileh; ty++)
				{
					for (tx = 0; tx < tilew; tx++)
					{
						UINT8 srcpix = *src_ptr++;
						pen_t srcrgb = pal_ptr[srcpix];
						UINT32 dstrgb = dst_ptr[tx];
						int sr, sg, sb, dr, dg, db;

						sr = (srcrgb >> 3) & 0x1f;
						sg = (srcrgb >> 11) & 0x1f;
						sb = (srcrgb >> 19) & 0x1f;

						dr = (dstrgb >> 3) & 0x1f;
						dg = (dstrgb >> 11) & 0x1f;
						db = (dstrgb >> 19) & 0x1f;

						dr = (m_t32x32pm[dr - sr + alpha] >> 5) + dr;
						dg = (m_t32x32pm[dg - sg + alpha] >> 5) + dg;
						db = (m_t32x32pm[db - sb + alpha] >> 5) + db;

						dst_ptr[tx] = rgb_t(pal5bit(db), pal5bit(dg), pal5bit(dr));
					}
					dst_ptr += bitmap.rowpixels();
				}
			}

			/* non-alpha case */
			else
			{
				for (ty = 0; ty < tileh; ty++)
				{
					for (tx = 0; tx < tilew; tx++)
						dst_ptr[tx] = pal_ptr[*src_ptr++];
					dst_ptr += bitmap.rowpixels();
				}
			}
		}

		dst_base += bitmap.rowpixels() * tileh;
	}
}

/***************************************************************************
                                Hot Chase
***************************************************************************/

/*------------------------------------------------------------------------
                                Road Drawing

    This layer is composed of horizontal lines gfx elements
    There are 512 lines in ROM, each is 512 pixels wide

    Offset:         Elements:       Data:
    0000-03ff       00-FF           Code (4 bytes)

    Code:
        00.w
            fedc ba98 ---- ----     Unused?
            ---- ---- 7654 ----     color
            ---- ---- ---- 3210     scroll x
        02.w
            fedc ba-- ---- ----     scroll x
            ---- --9- ---- ----     ?
            ---- ---8 7654 3210     code

    We draw each line using a bunch of 64x1 tiles

------------------------------------------------------------------------*/

void wecleman_state::hotchase_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/* Referred to what's in the ROMs */
#define XSIZE 512
#define YSIZE 512

	int sx, sy;
	const rectangle &visarea = m_screen->visible_area();

	/* Let's draw from the top to the bottom of the visible screen */
	for (sy = visarea.min_y;sy <= visarea.max_y;sy++)
	{
		int code    = m_roadram[sy*4/2+2/2] + (m_roadram[sy*4/2+0/2] << 16);
		int color   = ((code & 0x00f00000) >> 20) + 0x70;
		int scrollx = ((code & 0x0007fc00) >> 10) * 2;

		/* convert line number in gfx element number: */
		/* code is the tile code of the start of this line */
		code &= 0x1ff;
		code *= XSIZE / 32;

		for (sx=0; sx<2*XSIZE; sx+=64)
		{
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code++,
					color,
					0,0,
					((sx-scrollx)&0x3ff)-(384-32),sy,0);
		}
	}

#undef XSIZE
#undef YSIZE
}


/***************************************************************************
                            Palette Routines
***************************************************************************/

// new video and palette code
WRITE16_MEMBER(wecleman_state::wecleman_videostatus_w)
{
	COMBINE_DATA(m_videostatus);

	// bit0-6: background transition, 0=off, 1=on
	// bit7: palette being changed, 0=no, 1=yes
	if (ACCESSING_BITS_0_7)
	{
		if ((data & 0x7f) == 0 && !m_cloud_ds)
			m_cloud_ds = BLEND_INC;
		else
		if ((data & 0x7f) == 1 && !m_cloud_visible)
		{
			data ^= 1;
			m_cloud_ds = BLEND_DEC;
			m_cloud_visible = 1;
		}
	}
}

WRITE16_MEMBER(wecleman_state::hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w)
{
	int newword, r, g, b;

	newword = COMBINE_DATA(&m_generic_paletteram_16[offset]);

	r = ((newword << 1) & 0x1E ) | ((newword >> 12) & 0x01);
	g = ((newword >> 3) & 0x1E ) | ((newword >> 13) & 0x01);
	b = ((newword >> 7) & 0x1E ) | ((newword >> 14) & 0x01);

	m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
	r>>=1; g>>=1; b>>=1;
	m_palette->set_pen_color(offset+0x800, pal5bit(r)/2, pal5bit(g)/2, pal5bit(b)/2);
}

WRITE16_MEMBER(wecleman_state::wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w)
{
	int newword = COMBINE_DATA(&m_generic_paletteram_16[offset]);

	// the highest nibble has some unknown functions
//  if (newword & 0xf000) logerror("MSN set on color %03x: %1x\n", offset, newword>>12);
	m_palette->set_pen_color(offset, pal4bit(newword >> 0), pal4bit(newword >> 4), pal4bit(newword >> 8));
}


/***************************************************************************
                            Initializations
***************************************************************************/

VIDEO_START_MEMBER(wecleman_state,wecleman)
{
	/*
	    Sprite banking - each bank is 0x20000 bytes (we support 0x40 bank codes)
	    This game has ROMs for 16 banks
	*/
	static const int bank[0x40] =
	{
		0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,
		8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,
		0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,
		8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15
	};

	UINT8 *buffer;
	int i, j;

	assert(m_screen->format() == BITMAP_FORMAT_RGB32);
	buffer = auto_alloc_array(machine(), UINT8, 0x12c00);   // working buffer for sprite operations

	m_gameid = 0;
	m_gfx_bank = bank;
	m_spr_offsx = -0xbc + BMP_PAD;
	m_spr_offsy = 1 + BMP_PAD;
	m_cloud_blend = BLEND_MAX;
	m_cloud_ds = 0;
	m_cloud_visible = 0;
	m_black_pen = m_palette->black_pen();

	m_rgb_half     =          (UINT16*)(buffer + 0x00000);
	m_t32x32pm     =             (int*)(buffer + 0x10020);
	m_spr_ptr_list = (struct sprite **)(buffer + 0x12000);
	m_spr_idx_list =            (int *)(buffer + 0x12400);
	m_spr_pri_list =            (int *)(buffer + 0x12800);

	for (i=0; i<0x8000; i++)
	{
		j = i>>1;
		m_rgb_half[i] = (j&0xf) | (j&0x1e0) | (j&0x3c00);
	}

	for (j=0; j<0x20; j++)
	{
		for (i=-0x1f; i<0x20; i++)
		{
			*(m_t32x32pm + (j<<6) + i) = i * j;
		}
	}

	m_sprite_list = auto_alloc_array_clear(machine(), struct sprite, NUM_SPRITES);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wecleman_state::wecleman_get_bg_tile_info),this),
								TILEMAP_SCAN_ROWS,
									/* We draw part of the road below */
								8,8,
								PAGE_NX * 2, PAGE_NY * 2 );

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wecleman_state::wecleman_get_fg_tile_info),this),
								TILEMAP_SCAN_ROWS,

								8,8,
								PAGE_NX * 2, PAGE_NY * 2);

	m_txt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wecleman_state::wecleman_get_txt_tile_info),this),
									TILEMAP_SCAN_ROWS,

									8,8,
									PAGE_NX * 1, PAGE_NY * 1);

	m_bg_tilemap->set_scroll_rows(TILEMAP_DIMY);    /* Screen-wise scrolling */
	m_bg_tilemap->set_scroll_cols(1);
	m_bg_tilemap->set_transparent_pen(0);

	m_fg_tilemap->set_scroll_rows(TILEMAP_DIMY);    /* Screen-wise scrolling */
	m_fg_tilemap->set_scroll_cols(1);
	m_fg_tilemap->set_transparent_pen(0);

	m_txt_tilemap->set_scroll_rows(1);
	m_txt_tilemap->set_scroll_cols(1);
	m_txt_tilemap->set_transparent_pen(0);

	m_txt_tilemap->set_scrollx(0, 512-320-16 -BMP_PAD);
	m_txt_tilemap->set_scrolly(0, -BMP_PAD );

	// patches out a mysterious pixel floating in the sky (tile decoding bug?)
	*const_cast<UINT8 *>(m_gfxdecode->gfx(0)->get_data(0xaca)+7) = 0;
}

//  Callbacks for the K051316

K051316_CB_MEMBER(wecleman_state::hotchase_zoom_callback_1)
{
	*code |= (*color & 0x03) << 8;
	*color = (*color & 0xfc) >> 2;
}

K051316_CB_MEMBER(wecleman_state::hotchase_zoom_callback_2)
{
	*color = ((*color & 0x3f) << 1) | ((*code & 0x80) >> 7);
}


VIDEO_START_MEMBER(wecleman_state,hotchase)
{
	/*
	    Sprite banking - each bank is 0x20000 bytes (we support 0x40 bank codes)
	    This game has ROMs for 0x30 banks
	*/
	static const int bank[0x40] =
	{
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
		16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	};

	UINT8 *buffer;

	buffer = auto_alloc_array(machine(), UINT8, 0x400); // reserve 1k for sprite list

	m_gameid = 1;
	m_gfx_bank = bank;
	m_spr_offsx = -0xc0;
	m_spr_offsy = 0;
	m_black_pen = m_palette->black_pen();

	m_spr_ptr_list = (struct sprite **)buffer;

	m_sprite_list = auto_alloc_array_clear(machine(), struct sprite, NUM_SPRITES);
}


/***************************************************************************
                            Video Updates
***************************************************************************/

UINT32 wecleman_state::screen_update_wecleman(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *mrct;
	int video_on;
	int fg_x, bg_x, fg_y, bg_y;
	int cloud_sx, cloud_sy;
	int i, j, k;

	mrct = m_palette->pens();

	video_on = m_irqctrl & 0x40;

	set_led_status(machine(), 0, m_selected_ip & 0x04); // Start lamp

	fg_y = (m_txtram[0x0f24>>1] & (TILEMAP_DIMY - 1));
	bg_y = (m_txtram[0x0f26>>1] & (TILEMAP_DIMY - 1));

	cloud_sx = m_txtram[0xfee>>1] + 0xb0;
	cloud_sy = bg_y;

	m_bg_tilemap->set_scrolly(0, bg_y -BMP_PAD);
	m_fg_tilemap->set_scrolly(0, fg_y -BMP_PAD);

	for (i=0; i<(28<<2); i+=4)
	{
		fg_x = m_txtram[(i+0xf80)>>1] + (0xb0 -BMP_PAD);
		bg_x = m_txtram[(i+0xf82)>>1] + (0xb0 -BMP_PAD);

		k = i<<1;
		for (j=0; j<8; j++)
		{
			m_fg_tilemap->set_scrollx((fg_y + k + j) & (TILEMAP_DIMY - 1), fg_x);
			m_bg_tilemap->set_scrollx((bg_y + k + j) & (TILEMAP_DIMY - 1), bg_x);
		}
	}

	// temporary fix for ranking screen tile masking
	/* palette hacks! */
	((pen_t *)mrct)[0x27] = mrct[0x24];

	get_sprite_info();

	bitmap.fill(m_black_pen, cliprect);

	/* Draw the road (lines which have priority 0x02) */
	if (video_on) wecleman_draw_road(bitmap, cliprect, 0x02);

	/* Draw the background */
	if (video_on) m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draws the cloud layer; needs work
	if (m_cloud_visible)
	{
		/* palette hacks! */
		((pen_t *)mrct)[0] = ((pen_t *)mrct)[0x40] = ((pen_t *)mrct)[0x200] = ((pen_t *)mrct)[0x205];

		if (video_on)
			draw_cloud(bitmap,
			m_gfxdecode->gfx(0),
			m_pageram+0x1800,
			BMP_PAD, BMP_PAD,
			41, 20,
			cloud_sx, cloud_sy,
			6, 5,
			m_cloud_blend/BLEND_STEPS, 0);

		m_cloud_blend += m_cloud_ds;

		if (m_cloud_blend < BLEND_MIN)
			{ m_cloud_blend = BLEND_MIN; m_cloud_ds = 0; *m_videostatus |= 1; }
		else if (m_cloud_blend > BLEND_MAX)
			{ m_cloud_blend = BLEND_MAX; m_cloud_ds = 0; m_cloud_visible = 0; }
	}

	/* Draw the foreground */
	if (video_on) m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* Draw the road (lines which have priority 0x04) */
	if (video_on) wecleman_draw_road(bitmap,cliprect, 0x04);

	/* Draw the sprites */
	if (video_on) sprite_draw(bitmap,cliprect);

	/* Draw the text layer */
	if (video_on) m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************
                                Hot Chase
***************************************************************************/

UINT32 wecleman_state::screen_update_hotchase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int video_on;

	video_on = m_irqctrl & 0x40;

	set_led_status(machine(), 0, m_selected_ip & 0x04); // Start lamp

	get_sprite_info();

	bitmap.fill(m_black_pen, cliprect);

	/* Draw the background */
	if (video_on)
		m_k051316_1->zoom_draw(screen, bitmap, cliprect, 0, 0);

	/* Draw the road */
	if (video_on)
		hotchase_draw_road(bitmap, cliprect);

	/* Draw the sprites */
	if (video_on)
		sprite_draw(bitmap,cliprect);

	/* Draw the foreground (text) */
	if (video_on)
		m_k051316_2->zoom_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
