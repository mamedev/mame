// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                              -= Metro Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

                Q       Shows Layer 0
                W       Shows Layer 1
                E       Shows Layer 2
                A       Shows Sprites

        Keys can be used together!


                            [ 3 Scrolling Layers ]

        There is memory for a huge layer, but the actual tilemap
        is a smaller window (of fixed size) carved from anywhere
        inside that layer.

        Tile Size:                  8 x 8 x 4
        (later games can switch to  8 x 8 x 8, 16 x 16 x 4/8 at run time)

        Big Layer Size:         2048 x 2048 (8x8 tiles) or 4096 x 4096 (16x16 tiles)

        Tilemap Window Size:    512 x 256 (8x8 tiles) or 1024 x 512 (16x16 tiles)

        The tile codes in memory do not map directly to tiles. They
        are indexes into a table (with 0x200 entries) that defines
        a virtual set of tiles for the 3 layers. Each entry in that
        table adds 16 tiles to the set of available tiles, and decides
        their color code.

        Tile code with their msbit set are different as they mean:
        draw a tile filled with a single color (0-fff)


                            [ 512 Zooming Sprites ]

        The sprites are NOT tile based: the "tile" size can vary from
        8 to 64 (independently for width and height) with an 8 pixel
        granularity. The "tile" address is a multiple of 8x8 pixels.

        Each sprite can be shrinked to ~1/4 or enlarged to ~32x following
        an exponential curve of sizes (with one zoom value for both width
        and height)


***************************************************************************/

#include "emu.h"
#include "includes/metro.h"

TILE_GET_INFO_MEMBER(metro_state::metro_k053936_get_tile_info)
{
	int code = m_k053936_ram[tile_index];

	SET_TILE_INFO_MEMBER(4,
			code & 0x7fff,
			0xe,
			0);
}

TILE_GET_INFO_MEMBER(metro_state::metro_k053936_gstrik2_get_tile_info)
{
	int code = m_k053936_ram[tile_index];

	SET_TILE_INFO_MEMBER(4,
			(code & 0x7fff)>>2,
			0xe,
			0);
}

WRITE16_MEMBER(metro_state::metro_k053936_w)
{
	COMBINE_DATA(&m_k053936_ram[offset]);
	m_k053936_tilemap->mark_tile_dirty(offset);
}

TILEMAP_MAPPER_MEMBER(metro_state::tilemap_scan_gstrik2)
{
	/* logical (col,row) -> memory offset */
	int val;

	val = (row & 0x3f) * (256 * 2) + (col * 2);

	if (row & 0x40) val += 1;
	if (row & 0x80) val += 256;

	return val;
}



/***************************************************************************

                        Tilemaps: Tiles Set & Window

    Each entry in the Tiles Set RAM uses 2 words to specify a starting
    tile code and a color code. This adds 16 consecutive tiles with
    that color code to the set of available tiles.

        Offset:     Bits:                   Value:

        0.w         fedc ---- ---- ----
                    ---- ba98 7654 ----     Color Code*
                    ---- ---- ---- 3210     Code High Bits

        2.w                                 Code Low Bits

* 00-ff, but on later chips supporting it, xf means 256 color tile and palette x

***************************************************************************/


/***************************************************************************


                            Tilemaps: Rendering


***************************************************************************/

// A 2048 x 2048 virtual tilemap
#define BIG_NX      (0x100)
#define BIG_NY      (0x100)

// A smaller 512 x 256 window defines the actual tilemap

#define WIN_NX      (0x40)
#define WIN_NY      (0x20)

/* This looks up a single pixel in a tile, given the tile code.
   The Metro hardware has an indirection table, which is used here.
   Returns if to draw the pixel or not, pixel colour is placed in pix */
inline UINT8 metro_state::get_tile_pix( UINT16 code, UINT8 x, UINT8 y, int big, UINT16 *pix )
{
	int table_index;
	UINT32 tile;

	// Use code as an index into the tiles set table
	table_index = ((code & 0x1ff0) >> 4) * 2;
	tile = (m_tiletable[table_index + 0] << 16) + m_tiletable[table_index + 1];

	if (code & 0x8000) // Special: draw a tile of a single color (i.e. not from the gfx ROMs)
	{
		*pix = code & 0x0fff;

		if ((*pix & 0xf) != 0xf)
			return 1;
		else
			return 0;
	}
	else if (((tile & 0x00f00000) == 0x00f00000)    && (m_support_8bpp)) /* draw tile as 8bpp (e.g. balcube bg) */
	{
		gfx_element *gfx1 = m_gfxdecode->gfx(big?3:1);
		UINT32 tile2 = big ? ((tile & 0xfffff) + 8*(code & 0xf)) :
								((tile & 0xfffff) + 2*(code & 0xf));
		const UINT8* data;
		UINT8 flipxy = (code & 0x6000) >> 13;

		if (tile2 < gfx1->elements())
			data = gfx1->get_data(tile2);
		else
		{
			*pix = 0;
			return 0;
		}

		switch (flipxy)
		{
			default:
			case 0x0: *pix = data[(y              * (big?16:8)) + x];              break;
			case 0x1: *pix = data[(((big?15:7)-y) * (big?16:8)) + x];              break;
			case 0x2: *pix = data[(y              * (big?16:8)) + ((big?15:7)-x)]; break;
			case 0x3: *pix = data[(((big?15:7)-y) * (big?16:8)) + ((big?15:7)-x)]; break;
		}

		*pix |= ((tile & 0x0f000000) >> 24) * 0x100;

		if ((*pix & 0xff) != 0xff)
			return 1;
		else
			return 0;
	}
	else
	{
		gfx_element *gfx1 = m_gfxdecode->gfx(big?2:0);
		UINT32 tile2 = big ? ((tile & 0xfffff) + 4*(code & 0xf)) :
								((tile & 0xfffff) +   (code & 0xf));
		const UINT8* data;
		UINT8 flipxy = (code & 0x6000) >> 13;

		if (tile2 < gfx1->elements())
			data = gfx1->get_data(tile2);
		else
		{
			*pix = 0;
			return 0;
		}

		switch (flipxy)
		{
			default:
			case 0x0: *pix = data[(y              * (big?16:8)) + x];             break;
			case 0x1: *pix = data[(((big?15:7)-y) * (big?16:8)) + x];             break;
			case 0x2: *pix = data[(y              * (big?16:8)) + ((big?15:7)-x)]; break;
			case 0x3: *pix = data[(((big?15:7)-y) * (big?16:8)) + ((big?15:7)-x)]; break;
		}

		*pix |= (((tile & 0x0ff00000) >> 20)) * 0x10;

		if ((*pix & 0xf) != 0xf)
			return 1;
		else
			return 0;
	}
}


inline void metro_state::metro_vram_w( offs_t offset, UINT16 data, UINT16 mem_mask, int layer, UINT16 *vram )
{
	COMBINE_DATA(&vram[offset]);
}

WRITE16_MEMBER(metro_state::metro_vram_0_w){ metro_vram_w(offset, data, mem_mask, 0, m_vram_0); }
WRITE16_MEMBER(metro_state::metro_vram_1_w){ metro_vram_w(offset, data, mem_mask, 1, m_vram_1); }
WRITE16_MEMBER(metro_state::metro_vram_2_w){ metro_vram_w(offset, data, mem_mask, 2, m_vram_2); }



WRITE16_MEMBER(metro_state::metro_window_w)
{
	COMBINE_DATA(&m_window[offset]);
}



/***************************************************************************


                            Video Init Routines


***************************************************************************/

/*
 Sprites are not tile based, so we decode their graphics at runtime.
*/

void metro_state::expand_gfx1()
{
	UINT8 *base_gfx =   memregion("gfx1")->base();
	UINT32 length   =   memregion("gfx1")->bytes() * 2;

	m_expanded_gfx1 = auto_alloc_array(machine(), UINT8, length);

	for (int i = 0; i < length; i += 2)
	{
		UINT8 src = base_gfx[i / 2];

		m_expanded_gfx1[i + 0] = src & 0xf;
		m_expanded_gfx1[i + 1] = src >> 4;
	}
}

VIDEO_START_MEMBER(metro_state,metro_i4100)
{
	expand_gfx1();

	m_support_8bpp = 0;
	m_support_16x16 = 0;
	m_has_zoom = 0;

	m_tilemap_scrolldx[0] = 0;
	m_tilemap_scrolldx[1] = 0;
	m_tilemap_scrolldx[2] = 0;

	m_sprite_xoffs_dx = 0;
}

VIDEO_START_MEMBER(metro_state,metro_i4220)
{
	VIDEO_START_CALL_MEMBER(metro_i4100);

	m_support_8bpp = 1;     // balcube
	m_support_16x16 = 1;    // vmetal
}
VIDEO_START_MEMBER(metro_state,metro_i4220_dx_tmap)
{
	VIDEO_START_CALL_MEMBER(metro_i4220);

	m_tilemap_scrolldx[0] = -2;
	m_tilemap_scrolldx[1] = -2;
	m_tilemap_scrolldx[2] = -2;
}
VIDEO_START_MEMBER(metro_state,metro_i4220_dx_sprite)
{
	VIDEO_START_CALL_MEMBER(metro_i4220);

	m_sprite_xoffs_dx = 8;
}

VIDEO_START_MEMBER(metro_state,metro_i4300)
{
	VIDEO_START_CALL_MEMBER(metro_i4220);

	// any additional feature?
}

VIDEO_START_MEMBER(metro_state,blzntrnd)
{
	VIDEO_START_CALL_MEMBER(metro_i4220);

	m_has_zoom = 1;

	m_k053936_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(metro_state::metro_k053936_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 512);

	m_tilemap_scrolldx[0] = 8;
	m_tilemap_scrolldx[1] = 8;
	m_tilemap_scrolldx[2] = 8;
}

VIDEO_START_MEMBER(metro_state,gstrik2)
{
	VIDEO_START_CALL_MEMBER(metro_i4220);

	m_has_zoom = 1;

	m_k053936_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(metro_state::metro_k053936_gstrik2_get_tile_info),this), tilemap_mapper_delegate(FUNC(metro_state::tilemap_scan_gstrik2),this), 16, 16, 128, 256);

	m_tilemap_scrolldx[0] = 8;
	m_tilemap_scrolldx[1] = 0;
	m_tilemap_scrolldx[2] = 8;
}

/***************************************************************************

                                Video Registers


        Offset:     Bits:                   Value:

        0.w                                 Number Of Sprites To Draw
        2.w         f--- ---- ---- ----     Disable Sprites Layer Priority
                    -edc ---- ---- ----
                    ---- ba-- ---- ----     Sprites Masked Layer
                    ---- --98 ---- ----     Sprites Priority
                    ---- ---- 765- ----
                    ---- ---- ---4 3210     Sprites Masked Number
        4.w                                 Sprites Y Offset
        6.w                                 Sprites X Offset
        8.w                                 Sprites Color Codes Start

        -

        10.w        fedc ba98 76-- ----
                    ---- ---- --54 ----     Layer 2 Priority (3 backmost, 0 frontmost)
                    ---- ---- ---- 32--     Layer 1 Priority
                    ---- ---- ---- --10     Layer 0 Priority

        12.w                                Backround Color


***************************************************************************/



/***************************************************************************


                                Sprites Drawing


        Offset:     Bits:                   Value:

        0.w         fedc b--- ---- ----     Priority (0 = Max)
                    ---- -a98 7654 3210     X

        2.w         fedc ba-- ---- ----     Zoom (Both X & Y)
                    ---- --98 7654 3210     Y

        4.w         f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc b--- ---- ----     Size X *
                    ---- -a98 ---- ----     Size Y *
                    ---- ---- 7654 ----     Color
                    ---- ---- ---- 3210     Code High Bits **

        6.w                                 Code Low Bits  **

*  8 pixel increments
** 8x8 pixel increments

***************************************************************************/

void metro_state::metro_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *base_gfx4 = m_expanded_gfx1;
	UINT8 *base_gfx8 = memregion("gfx1")->base();
	UINT32 gfx_size = memregion("gfx1")->bytes();

	int max_x = m_screen->width();
	int max_y = m_screen->height();

	int max_sprites = m_spriteram.bytes() / 8;
	int sprites     = m_videoregs[0x00/2] % max_sprites;

	int color_start = (m_videoregs[0x08/2] & 0x0f) << 4;

	int i, j, pri;
	static const int primask[4] = { 0x0000, 0xff00, 0xff00 | 0xf0f0, 0xff00 | 0xf0f0 | 0xcccc };

	UINT16 *src;
	int inc;

	if (sprites == 0)
		return;

	for (i = 0; i < 0x20; i++)
	{
		if (!(m_videoregs[0x02/2] & 0x8000))
		{
			src = m_spriteram + (sprites - 1) * (8 / 2);
			inc = -(8 / 2);
		} else {
			src = m_spriteram;
			inc = (8 / 2);
		}

		for (j = 0; j < sprites; j++)
		{
			int x, y, attr, code, color, flipx, flipy, zoom, curr_pri, width, height;

			// Exponential zoom table extracted from daitoride
			static const int zoomtable[0x40] =
			{   0xAAC,0x800,0x668,0x554,0x494,0x400,0x390,0x334,
				0x2E8,0x2AC,0x278,0x248,0x224,0x200,0x1E0,0x1C8,
				0x1B0,0x198,0x188,0x174,0x164,0x154,0x148,0x13C,
				0x130,0x124,0x11C,0x110,0x108,0x100,0x0F8,0x0F0,
				0x0EC,0x0E4,0x0DC,0x0D8,0x0D4,0x0CC,0x0C8,0x0C4,
				0x0C0,0x0BC,0x0B8,0x0B4,0x0B0,0x0AC,0x0A8,0x0A4,
				0x0A0,0x09C,0x098,0x094,0x090,0x08C,0x088,0x080,
				0x078,0x070,0x068,0x060,0x058,0x050,0x048,0x040 };

			x = src[0];
			curr_pri = (x & 0xf800) >> 11;

			if ((curr_pri == 0x1f) || (curr_pri != i))
			{
				src += inc;
				continue;
			}

			pri = (m_videoregs[0x02/2] & 0x0300) >> 8;

			if (!(m_videoregs[0x02/2] & 0x8000))
			{
				if (curr_pri > (m_videoregs[0x02/2] & 0x1f))
					pri = (m_videoregs[0x02/2] & 0x0c00) >> 10;
			}

			y     = src[1];
			attr  = src[2];
			code  = src[3];

			flipx =  attr & 0x8000;
			flipy =  attr & 0x4000;
			color = (attr & 0xf0) >> 4;

			zoom = zoomtable[(y & 0xfc00) >> 10] << (16 - 8);

			x = (x & 0x07ff) - m_sprite_xoffs;
			y = (y & 0x03ff) - m_sprite_yoffs;

			width  = (((attr >> 11) & 0x7) + 1) * 8;
			height = (((attr >>  8) & 0x7) + 1) * 8;

			UINT32 gfxstart = (8 * 8 * 4 / 8) * (((attr & 0x000f) << 16) + code);

			if (m_flip_screen)
			{
				flipx = !flipx;     x = max_x - x - width;
				flipy = !flipy;     y = max_y - y - height;
			}

			if (m_support_8bpp && color == 0xf)  /* 8bpp */
			{
				/* Bounds checking */
				if ((gfxstart + width * height - 1) >= gfx_size)
					continue;

				gfx_element gfx(m_palette, base_gfx8 + gfxstart, width, height, width, m_palette->entries(), 0, 256);

				gfx.prio_zoom_transpen(bitmap,cliprect,
								0,
								color_start >> 4,
								flipx, flipy,
								x, y,
								zoom, zoom,
								screen.priority(),primask[pri], 255);
			}
			else
			{
				/* Bounds checking */
				if ((gfxstart + width / 2 * height - 1) >= gfx_size)
					continue;

				gfx_element gfx(m_palette, base_gfx4 + 2 * gfxstart, width, height, width, m_palette->entries(),0, 16);

				gfx.prio_zoom_transpen(bitmap,cliprect,
								0,
								color + color_start,
								flipx, flipy,
								x, y,
								zoom, zoom,
								screen.priority(),primask[pri], 15);
			}
#if 0
{   /* Display priority + zoom on each sprite */
	char buf[80];
	sprintf(buf, "%02X %02X", ((src[0] & 0xf800) >> 11) ^ 0x1f, ((src[1] & 0xfc00) >> 10));
	ui_draw_text(buf, x, y);
}
#endif
			src += inc;
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

// Copy a 'window' from the large 2048x2048 (or 4096x4096 for 16x16 tiles) tilemap

void metro_state::draw_tilemap( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32 flags, UINT32 pcode,
							int sx, int sy, int wx, int wy, int big, UINT16 *tilemapram, int layer )
{
	int y;

	bitmap_ind8 &priority_bitmap = m_screen->priority();

	int width  = big ? 4096 : 2048;
	int height = big ? 4096 : 2048;

	int scrwidth  = bitmap.width();
	int scrheight = bitmap.height();

	int windowwidth  = width >> 2;
	int windowheight = height >> 3;

	sx += m_tilemap_scrolldx[layer] * (m_flip_screen ? 1 : -1);

	for (y = 0; y < scrheight; y++)
	{
		int scrolly = (sy+y-wy)&(windowheight-1);
		int x;
		UINT16 *dst;
		UINT8 *priority_baseaddr;
		int srcline = (wy+scrolly)&(height-1);
		int srctilerow = srcline >> (big ? 4 : 3);

		if (!m_flip_screen)
		{
			dst = &bitmap.pix16(y);
			priority_baseaddr = &priority_bitmap.pix8(y);

			for (x = 0; x < scrwidth; x++)
			{
				int scrollx = (sx+x-wx)&(windowwidth-1);
				int srccol = (wx+scrollx)&(width-1);
				int srctilecol = srccol >> (big ? 4 : 3);
				int tileoffs = srctilecol + srctilerow * BIG_NX;

				UINT16 dat = 0;

				UINT16 tile = tilemapram[tileoffs];
				UINT8 draw = get_tile_pix(tile, big ? (srccol&0xf) : (srccol&0x7), big ? (srcline&0xf) : (srcline&0x7), big, &dat);

				if (draw)
				{
					dst[x] = dat;
					priority_baseaddr[x] = (priority_baseaddr[x] & (pcode >> 8)) | pcode;
				}
			}
		}
		else // flipped case
		{
			dst = &bitmap.pix16(scrheight-y-1);
			priority_baseaddr = &priority_bitmap.pix8(scrheight-y-1);

			for (x = 0; x < scrwidth; x++)
			{
				int scrollx = (sx+x-wx)&(windowwidth-1);
				int srccol = (wx+scrollx)&(width-1);
				int srctilecol = srccol >> (big ? 4 : 3);
				int tileoffs = srctilecol + srctilerow * BIG_NX;

				UINT16 dat = 0;

				UINT16 tile = tilemapram[tileoffs];
				UINT8 draw = get_tile_pix(tile, big ? (srccol&0xf) : (srccol&0x7), big ? (srcline&0xf) : (srcline&0x7), big, &dat);

				if (draw)
				{
					dst[scrwidth-x-1] = dat;
					priority_baseaddr[scrwidth-x-1] = (priority_baseaddr[scrwidth-x-1] & (pcode >> 8)) | pcode;
				}
			}
		}
	}
}

// Draw all the layers that match the given priority

void metro_state::draw_layers( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int layers_ctrl )
{
	UINT16 layers_pri = m_videoregs[0x10 / 2];
	int layer;

	// Draw all the layers with priority == pri
	for (layer = 2; layer >= 0; layer--)
	{
		if (pri == ((layers_pri >> (layer * 2)) & 3))
		{
			// Scroll and Window values
			UINT16 sy = m_scroll[layer * 2 + 0]; UINT16 sx = m_scroll[layer * 2 + 1];
			UINT16 wy = m_window[layer * 2 + 0]; UINT16 wx = m_window[layer * 2 + 1];

			if (BIT(layers_ctrl, layer))    // for debug
			{
				UINT16 *tilemapram = 0;

				switch (layer)
				{
					case 0: tilemapram = m_vram_0;   break;
					case 1: tilemapram = m_vram_1;   break;
					case 2: tilemapram = m_vram_2;   break;
				}

				int big = m_support_16x16 && (*m_screenctrl & (0x0020 << layer));

				draw_tilemap(screen, bitmap, cliprect, 0, 1 << (3 - pri), sx, sy, wx, wy, big, tilemapram, layer);
			}
		}
	}
}


UINT32 metro_state::screen_update_metro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pri, layers_ctrl = -1;
	UINT16 screenctrl = *m_screenctrl;

	m_sprite_xoffs = m_videoregs[0x06 / 2] - screen.width()  / 2 + m_sprite_xoffs_dx;
	m_sprite_yoffs = m_videoregs[0x04 / 2] - screen.height() / 2;

	screen.priority().fill(0, cliprect);

	// The background color is selected by a register
	bitmap.fill(m_videoregs[0x12/2] & 0x0fff, cliprect);

	/*  Screen Control Register:

	    f--- ---- ---- ----     ?
	    -edc b--- ---- ----
	    ---- -a98 ---- ----     ? Leds (see gakusai attract)
	    ---- ---- 765- ----     16x16 Tiles  (Layer 2-1-0)
	    ---- ---- ---4 32--
	    ---- ---- ---- --1-     Blank Screen
	    ---- ---- ---- ---0     Flip  Screen    */
	if (screenctrl & 2)
		return 0;

	m_flip_screen = screenctrl & 1;

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		layers_ctrl &= msk;
	}

	popmessage( "lyr: %x-%x-%x spr: %04x clr: %04x scr: %04x",
				(m_videoregs[0x10/2] & 0x30) >> 4, (m_videoregs[0x10/2] & 0xc) >> 2, m_videoregs[0x10/2] & 3,
				m_videoregs[0x02/2], m_videoregs[0x12/2],
				*m_screenctrl);
}
#endif

	if (m_has_zoom)
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_k053936_tilemap, 0, 0, 1);

	for (pri = 3; pri >= 0; pri--)
		draw_layers(screen, bitmap, cliprect, pri, layers_ctrl);

	if (layers_ctrl & 0x08)
		metro_draw_sprites(screen, bitmap, cliprect);

	return 0;
}
