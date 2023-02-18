// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
#include "emu.h"
#include "hng64.h"

#define HNG64_VIDEO_DEBUG 0


void hng64_state::hng64_mark_all_tiles_dirty(int tilemap)
{
	m_tilemap[tilemap].m_tilemap_8x8->mark_all_dirty();
	m_tilemap[tilemap].m_tilemap_16x16->mark_all_dirty();
	m_tilemap[tilemap].m_tilemap_16x16_alt->mark_all_dirty();
}

void hng64_state::hng64_mark_tile_dirty(int tilemap, int tile_index)
{
	m_tilemap[tilemap].m_tilemap_8x8->mark_tile_dirty(tile_index);
	m_tilemap[tilemap].m_tilemap_16x16->mark_tile_dirty(tile_index);
	m_tilemap[tilemap].m_tilemap_16x16_alt->mark_tile_dirty(tile_index);
}

// make this a template function!
// pppppppp ffattttt tttttttt tttttttt
#define HNG64_GET_TILE_INFO                                                     \
{                                                                               \
	uint16_t tilemapinfo = (m_videoregs[reg]>>shift)&0xffff;                    \
	int tileno,pal, flip;                                                       \
																				\
	tileno = m_videoram[tile_index+(offset/4)];                                 \
																				\
	pal = (tileno&0xff000000)>>24;                                              \
	flip =(tileno&0x00c00000)>>22;                                              \
																				\
	if (tileno&0x200000)                                                        \
	{                                                                           \
		tileno = (tileno & m_videoregs[0x0b]) | m_videoregs[0x0c];              \
	}                                                                           \
																				\
	tileno &= 0x1fffff;                                                         \
																				\
	if (size==0)                                                                \
	{                                                                           \
		if (tilemapinfo&0x400)                                                  \
		{                                                                       \
			tileinfo.set(1,tileno>>1,pal>>4,TILE_FLIPYX(flip));                 \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			tileinfo.set(0,tileno, pal,TILE_FLIPYX(flip));                      \
		}                                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		if (tilemapinfo&0x400)                                                  \
		{                                                                       \
			tileinfo.set(3,tileno>>3,pal>>4,TILE_FLIPYX(flip));                 \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			tileinfo.set(2,tileno>>2, pal,TILE_FLIPYX(flip));                   \
		}                                                                       \
	}                                                                           \
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile0_8x8_info)
{
	int offset = 0x00000;
	int size = 0;
	int reg = 0x02;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile0_16x16_info)
{
	int offset = 0x00000;
	int size = 1;
	int reg = 0x02;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile1_8x8_info)
{
	int offset = 0x10000;
	int size = 0;
	int reg = 0x02;
	int shift = 0;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile1_16x16_info)
{
	int offset = 0x10000;
	int size = 1;
	int reg = 0x02;
	int shift = 0;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile2_8x8_info)
{
	int offset = 0x20000;
	int size = 0;
	int reg = 0x03;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile2_16x16_info)
{
	int offset = 0x20000;
	int size = 1;
	int reg = 0x03;
	int shift = 16;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile3_8x8_info)
{
	int offset = 0x30000;
	int size = 0;
	int reg = 0x03;
	int shift = 0;

	HNG64_GET_TILE_INFO
}

TILE_GET_INFO_MEMBER(hng64_state::get_hng64_tile3_16x16_info)
{
	int offset = 0x30000;
	int size = 1;
	int reg = 0x03;
	int shift = 0;

	HNG64_GET_TILE_INFO
}


void hng64_state::hng64_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const int realoff = (offset * 4);
	COMBINE_DATA(&m_videoram[offset]);

	if ((realoff >= 0) && (realoff < 0x10000))
	{
		hng64_mark_tile_dirty(0, offset & 0x3fff);
	}
	else if ((realoff >= 0x10000) && (realoff < 0x20000))
	{
		hng64_mark_tile_dirty(1, offset & 0x3fff);
	}
	else if ((realoff >= 0x20000) && (realoff < 0x30000))
	{
		hng64_mark_tile_dirty(2, offset & 0x3fff);
	}
	else if ((realoff >= 0x30000) && (realoff < 0x40000))
	{
		hng64_mark_tile_dirty(3, offset & 0x3fff);
	}
	// Offsets 0x40000 - 0x58000 are for "floor" scanline control

	/* 400000 - 7fffff is scroll regs etc. */
}

/* internal set of transparency states for rendering */

/*-------------------------------------------------
    tilemap_draw_roz_core - render the tilemap's
    pixmap to the destination with rotation
    and zoom
-------------------------------------------------*/

#define HNG64_ROZ_PLOT_PIXEL(INPUT_VAL) \
do { \
	if (drawformat == 1) \
		*(uint32_t *)dest = clut[INPUT_VAL]; \
	else if (drawformat == 2) \
		*(uint32_t *)dest = add_blend_r32(*(uint32_t *)dest, clut[INPUT_VAL]); \
	else if (drawformat == 3) \
		*(uint32_t *)dest = alpha_blend_r32(*(uint32_t *)dest, clut[INPUT_VAL], alpha); \
} while (0)

void hng64_state::hng64_tilemap_draw_roz_core_line(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		int wraparound, uint8_t drawformat, uint8_t alpha, uint8_t mosaic, uint8_t tm)
{
	int source_line_to_use = cliprect.min_y;
	source_line_to_use = (source_line_to_use / (mosaic+1)) * (mosaic+1);

	int xinc, xinc2, yinc, yinc2;
	int32_t xtopleft;
	int32_t ytopleft;
	uint16_t scrollbase = get_scrollbase(tm);
	uint16_t tileregs = get_tileregs(tm);
	const uint8_t not_line_mode = (tileregs & 0x0800) >> 11;

	if (not_line_mode)
	{
		// 0x1000 is set up the buriki 2nd title screen with rotating logo and in fatal fury at various times?
		const int global_alt_scroll_register_format = m_videoregs[0x00] & 0x04000000;

		if (global_alt_scroll_register_format) // globally selects alt scroll register layout???
		{
			// xrally 'mist in tunnel' does NOT use this mode
			// buriki title screen tm==3 DOES use this mode

			/* complex zoom mode? */
			/* with this scroll register layout rotation effects are possible
			    the most obvious use of rotation is the Buriki One logo after
			    attract mode; the text around the outside of the logo is rotated
			    onto the screen

			    see 1:32 in http://www.youtube.com/watch?v=PoYaHOILuGs

			    Xtreme Rally seems to have an issue with this mode on the communication check
			    screen at startup, however during the period in which the values are invalid
			    it looks like the display shouldn't even be enabled (only gets enabled when
			    the value starts counting up)

			*/
#if HNG64_VIDEO_DEBUG
			if (0)
				if (tm == 3)
					popmessage("X %08x X %08x X %08x X(?) %08x | Y %08x Y %08x Y %08x Y(?) %08x",
						m_videoram[(0x40000 + (scrollbase << 4)) / 4],
						m_videoram[(0x40004 + (scrollbase << 4)) / 4],
						m_videoram[(0x40010 + (scrollbase << 4)) / 4],
						m_videoram[(0x40014 + (scrollbase << 4)) / 4],  // unused? (dupe value on fatfurwa, 00 on rest)

						m_videoram[(0x40008 + (scrollbase << 4)) / 4],
						m_videoram[(0x40018 + (scrollbase << 4)) / 4],
						m_videoram[(0x4000c + (scrollbase << 4)) / 4],
						m_videoram[(0x4001c + (scrollbase << 4)) / 4]); // unused? (dupe value on fatfurwa, 00 on rest)
#endif

			xtopleft = (m_videoram[(0x40000 + (scrollbase << 4)) / 4]);
			const int32_t xalt = (m_videoram[(0x40004 + (scrollbase << 4)) / 4]); // middle screen point
			const int32_t xmiddle = (m_videoram[(0x40010 + (scrollbase << 4)) / 4]);

			ytopleft = (m_videoram[(0x40008 + (scrollbase << 4)) / 4]);
			const int32_t yalt = (m_videoram[(0x40018 + (scrollbase << 4)) / 4]); // middle screen point
			const int32_t ymiddle = (m_videoram[(0x4000c + (scrollbase << 4)) / 4]);

			xinc = (xmiddle - xtopleft) / 512;
			yinc = (ymiddle - ytopleft) / 512;
			xinc2 = (xalt - xtopleft) / 512;
			yinc2 = (yalt - ytopleft) / 512;
		}
		else
		{
			/* simple zoom mode? - only 4 regs? */
			/* in this mode they can only specify the top left and middle screen points for each tilemap,
			    this allows simple zooming, but not rotation */

#if HNG64_VIDEO_DEBUG
			if (0)
				if (tm == 3)
					popmessage("%08x %08x %08x %08x",
						m_videoram[(0x40010 + (scrollbase << 4)) / 4],
						m_videoram[(0x40014 + (scrollbase << 4)) / 4],
						m_videoram[(0x40018 + (scrollbase << 4)) / 4],
						m_videoram[(0x4001c + (scrollbase << 4)) / 4]);
#endif

			int32_t xmiddle;
			int32_t ymiddle;

			const uint32_t& global_tileregs = m_videoregs[0x00];
			const int global_zoom_disable = global_tileregs & 0x00010000;
			if (global_zoom_disable) // disable all scrolling / zoom (test screen) (maybe)
			{
				/* If this bit is active the scroll registers don't seem valid at all?
				    It either disables zooming, or disables use of the scroll registers completely
				    - used at startup
				*/

				xtopleft = 0;
				xmiddle = 256 << 16;

				ytopleft = 0;
				ymiddle = 256 << 16;
			}
			else
			{
				xtopleft = (m_videoram[(0x40000 + (scrollbase << 4)) / 4]);
				xmiddle = (m_videoram[(0x40004 + (scrollbase << 4)) / 4]); // middle screen point
				ytopleft = (m_videoram[(0x40008 + (scrollbase << 4)) / 4]);
				ymiddle = (m_videoram[(0x4000c + (scrollbase << 4)) / 4]); // middle screen point
			}

			xinc = (xmiddle - xtopleft) / 512;
			yinc = (ymiddle - ytopleft) / 512;
			xinc2 = 0;
			yinc2 = 0;
		}
	}
	else // line mode
	{
		const int global_alt_scroll_register_format = m_videoregs[0x00] & 0x04000000;
		if (global_alt_scroll_register_format) // globally selects alt scroll register layout???
		{
			//popmessage("global_alt_scroll_register_format in linemode");
		}
		//else
		{
			int32_t xmiddle;
			int32_t ymiddle;

			const uint32_t& global_tileregs = m_videoregs[0x00];
			const int global_zoom_disable = global_tileregs & 0x00010000;
			if (global_zoom_disable) // disable all scrolling / zoom (test screen) (maybe)
			{
				// If this bit is active the scroll registers don't seem valid at all?
				// It either disables zooming, or disables use of the scroll registers completely
				// - used at startup

				xtopleft = 0;
				xmiddle = 256 << 16;

				ytopleft = 0;
				ymiddle = 256 << 16;
			}
			else
			{
				uint16_t line = source_line_to_use;
				xtopleft = (m_videoram[(0x40000 + (line * 0x10) + (scrollbase << 4)) / 4]);
				xmiddle = (m_videoram[(0x40004 + (line * 0x10) + (scrollbase << 4)) / 4]); // middle screen point
				ytopleft = (m_videoram[(0x40008 + (line * 0x10) + (scrollbase << 4)) / 4]);
				ymiddle = (m_videoram[(0x4000c + (line * 0x10) + (scrollbase << 4)) / 4]); // middle screen point
			}

			xinc = (xmiddle - xtopleft) / 512;
			yinc = (ymiddle - ytopleft) / 512;
			// TODO: if global_alt_scroll_register_format is enabled uses incxy / incyx into calculation somehow ...
			xinc2 = 0;
			yinc2 = 0;
		}
	}

	uint32_t startx = xtopleft;
	uint32_t starty = ytopleft;
	const int incxx = xinc << 1;
	const int incxy = yinc2 << 1;
	const int incyx = xinc2 << 1;
	const int incyy = yinc << 1;

	// we have the scroll values for the current line, draw

	pen_t const *const clut = &m_palette->pen(0);
	bitmap_ind8 &priority_bitmap = screen.priority();
	bitmap_rgb32 &destbitmap = dest;
	const bitmap_ind16 &srcbitmap = tmap->pixmap();
	const bitmap_ind8 &flagsmap = tmap->flagsmap();
	const int xmask = srcbitmap.width()-1;
	const int ymask = srcbitmap.height()-1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;
	uint32_t priority = 0;
	uint8_t mask = 0x1f;
	uint8_t value = 0x10;

	/* pre-advance based on the cliprect */
	startx += cliprect.min_x * incxx + source_line_to_use * incyx;
	starty += cliprect.min_x * incxy + source_line_to_use * incyy;

	/* extract start/end points */
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;

	/* optimized loop for the not rotated case */
	if (incxy == 0 && incyx == 0 && !wraparound)
	{
		/* skip without drawing until we are within the bitmap */
		while (startx >= widthshifted && sx <= ex)
		{
			startx += incxx;
			sx++;
		}

		/* early exit if we're done already */
		if (sx > ex)
			return;

		/* only draw if Y is within range */
		if (starty < heightshifted)
		{
			/* initialize X counters */
			int x = sx;
			uint32_t cx = startx;
			uint32_t cy = starty >> 16;

			/* get source and priority pointers */
			uint8_t *pri = &priority_bitmap.pix(sy, sx);
			uint16_t const *const src = &srcbitmap.pix(cy);
			uint8_t const *const maskptr = &flagsmap.pix(cy);
			uint32_t *dest = &destbitmap.pix(sy, sx);

			/* loop over columns */
			int mosaic_counter = 0;
			uint16_t masksrc = 0;
			uint16_t datasrc = 0;

			while (x <= ex && cx < widthshifted)
			{
				uint16_t srcoffset = cx >> 16;

				if (mosaic_counter == 0)
				{
					masksrc = (maskptr[srcoffset] & mask);
					datasrc = src[srcoffset];
					mosaic_counter = mosaic;
				}
				else
				{
					mosaic_counter--;
				}

				/* plot if we match the mask */
				if (masksrc == value)
				{
					HNG64_ROZ_PLOT_PIXEL(datasrc);
					*pri = (*pri & (priority >> 8)) | priority;
				}

				/* advance in X */
				cx += incxx;
				x++;
				dest++;
				pri++;
			}
		}
	}
	/* wraparound case */
	else if (wraparound)
	{
		/* initialize X counters */
		int x = sx;
		uint32_t cx = startx;
		uint32_t cy = starty;

		/* get dest and priority pointers */
		uint32_t *dest = &destbitmap.pix(sy, sx);
		uint8_t *pri = &priority_bitmap.pix(sy, sx);

		/* loop over columns */
		int mosaic_counter = 0;
		uint16_t masksrc = 0;
		uint16_t datasrc = 0;

		while (x <= ex)
		{
			uint16_t srcoffset = (cx >> 16) & xmask;
			uint16_t srcyoffset = (cy >> 16) & ymask;

			if (mosaic_counter == 0)
			{
				masksrc = (flagsmap.pix(srcyoffset, srcoffset) & mask);
				datasrc = srcbitmap.pix(srcyoffset, srcoffset);
				mosaic_counter = mosaic;
			}
			else
			{
				mosaic_counter--;
			}
			/* plot if we match the mask */
			if (masksrc == value)
			{
				HNG64_ROZ_PLOT_PIXEL(datasrc);
				*pri = (*pri & (priority >> 8)) | priority;
			}

			/* advance in X */
			cx += incxx;
			cy += incxy;
			x++;
			dest++;
			pri++;
		}
	}
	/* non-wraparound case */
	else
	{
		/* initialize X counters */
		int x = sx;
		uint32_t cx = startx;
		uint32_t cy = starty;

		/* get dest and priority pointers */
		uint32_t *dest = &destbitmap.pix(sy, sx);
		uint8_t *pri = &priority_bitmap.pix(sy, sx);

		int mosaic_counter = 0;
		uint16_t masksrc = 0;
		uint16_t datasrc = 0;

		/* loop over columns */
		while (x <= ex)
		{
			uint16_t srcoffset = cx >> 16;
			uint16_t srcyoffset = cy >> 16;

			/* plot if we're within the bitmap and we match the mask */

			if (cx < widthshifted && cy < heightshifted)
			{
				if (mosaic_counter == 0)
				{
					masksrc = (flagsmap.pix(srcyoffset, srcoffset) & mask);
					datasrc = srcbitmap.pix(srcyoffset, srcoffset);
					mosaic_counter = mosaic;
				}
				else
				{
					mosaic_counter--;
				}

				if (masksrc == value)
				{
					HNG64_ROZ_PLOT_PIXEL(datasrc);
					*pri = (*pri & (priority >> 8)) | priority;
				}

			}
			/* advance in X */
			cx += incxx;
			cy += incxy;
			x++;
			dest++;
			pri++;
		}
	}
}


/*
 * Video Regs Format (appear to just be tilemap regs)
 * --------------------------------------------------
 *
 * uint32_t | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ---- -Cdd ---- -??Z ---- ---- ---- ---- |  C = global complex zoom
          | 0000 0011  - road edge alt 1            | dd = global tilemap dimension selector
          | 0000 0111  - road edge alt 2            |  ? = Always Set?
          |                                         |  Z = Global Zoom Disable?
 *   1    | oooo oooo oooo oooo ---- ---- ---- ---- | unknown - 0001 is a popular value.  Explore.
 *   1    | ---- ---- ---- ---- oooo oooo oooo oooo | unknown - untouched in sams64 games, initialized elsewhere
 *   2    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap0 per layer flags
 *   2    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap1 per layer flags
 *   3    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap2 per layer flags
 *   3    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap3 per layer flags
 *   4    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap0 scrollbase when not floor, lineram offset when floor
 *   4    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap1 scrollbase when not floor, lineram offset when floor
 *   5    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap3 scrollbase when not floor, lineram offset when floor
 *   5    | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap4 scrollbase when not floor, lineram offset when floor
 *   6    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 000001ff (fatfurwa)
 *   7    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 000001ff (fatfurwa)
 *   8    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 80008000 (fatfurwa)
 *   9    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 00000000 (fatfurwa)
 *   a    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 00000000 (fatfurwa)
 *   b    | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | auto animation mask for tilemaps
 *   c    | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | auto animation bits for tilemaps
 *   d    | oooo oooo oooo oooo oooo oooo oooo oooo | not used ??
 *   e    | oooo oooo oooo oooo oooo oooo oooo oooo | not used ??

    // tilemap0 per layer flags
    // 0840 - startup tests, 8x8x4 layer
    // 0cc0 - beast busters 2, 8x8x8 layer
    // 0860 - fatal fury wa
    // 08e0 - fatal fury wa during transitions
    // 0940 - samurai shodown 64
    // 0880 - buriki

    // Individual tilemap regs format
    // ------------------------------
    // mmmm dbr? ??ez zzzz
    // m = Tilemap mosaic level [0-15] - confirmed in sams64 demo mode
    //  -- they seem to enable mosaic at the same time as rowscroll in several cases (floor in buriki / ff)
    //     and also on the rotating logo in buriki.. does it cause some kind of aliasing side-effect, or.. ?
    // d = line (floor) mode - buriki, fatafurwa, some backgrounds in ss64_2
    // b = 4bpp/8bpp (seems correct) (beast busters, samsh64, sasm64 2, xrally switch it for some screens)
    // r = tile size (seems correct)
    // e = tilemap enable bit according to sams64_2
    // z = z depth/priority? tilemaps might also be affected by min / max clip values somewhere?
    //              (debug layer on buriki has priority 0x020, which would be highest)
 */



uint16_t hng64_state::get_tileregs(int tm)
{
	return (m_videoregs[0x02 + BIT(tm, 1)] >> (BIT(tm, 0) ? 0 : 16)) & 0x0000ffff;
}

uint16_t hng64_state::get_scrollbase(int tm)
{
	return (m_videoregs[0x04 + BIT(tm, 1)] >> (BIT(tm, 0) ? 0 : 16)) & 0x00003fff;
}

int hng64_state::get_blend_mode(int tm)
{
	// this is based on xrally and sams64/sams64_2 use, it could be incorrect
	// it doesn't seem to be 100% on sams64_2 select screen when the mode select circle moves down

	// m_tcram[0x14/4] may be some additional per layer for this?
	uint8_t blendmode = 1;
	if ((m_tcram[0x0c / 4] & 0x04000000) && (tm == 1)) // only enable it for the 2nd tilemap right now, find other use cases!
		blendmode = 2;


	// the bit below also gets set for certain blending effects
	// for example a mist effect in the long tunnel on the South America course
	// ( https://youtu.be/9rOPkNHTmYA?t=403 6:43 )
	// this is the only course which has this bit set (it is set all the time) and is the only course using blending
	//
	// the problem here however is that the tilemap used for blending has a lower tilemap priority than the background tilemap?!
	// the bit also gets set on the buriki title screen, and how that blends is unclear even with reference footage
	//if ((m_tcram[0x0c / 4] & 0x00000004) && (tm == 3))
	//  blendmode = 2;

	return blendmode;
}


void hng64_state::hng64_drawtilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tm, int flags, int line)
{
	// Useful bits from the global tilemap flags
	const int global_dimensions = (m_videoregs[0x00] & 0x03000000) >> 24;

#if HNG64_VIDEO_DEBUG
	if ((global_dimensions != 0) && (global_dimensions != 3))
		popmessage("unsupported global_dimensions on tilemaps");
#endif

	// Determine which tilemap registers and scroll base this tilemap uses
	uint16_t tileregs = get_tileregs(tm);

	// Useful bits from the tilemap registers
	const uint8_t mosaic = (tileregs & 0xf000) >> 12;
	const uint8_t not_line_mode = (tileregs & 0x0800) >> 11;
	//const uint8_t bpp = (tileregs & 0x0400) >> 10; // not used?
	const uint8_t big = (tileregs & 0x0200) >>  9;
	const uint8_t enable = (tileregs & 0x0040) >>  6;

	// Tilemap drawing enable?
	//
	// Definitely used to disable tilemaps in sams64_2 demo mode
	// and is also set on the 'debug layer' of buriki
	// however the floor layer of fatfurwa also has it set, and that must be enabled!
	//
	// Speculate that floormode ignores this flag, because lines can be disabled on
	// a per-line basis?
	//
	// Could also just be another priority bits with some priorities being filtered
	if (!enable && not_line_mode)
	{
		return;
	}

	// Select the proper tilemap size
	tilemap_t* tilemap = nullptr;
	if (global_dimensions==0)
	{
		if (big) tilemap = m_tilemap[tm].m_tilemap_16x16;
		else tilemap = m_tilemap[tm].m_tilemap_8x8;
	}
	else
	{
		if (big) tilemap = m_tilemap[tm].m_tilemap_16x16_alt;
		else tilemap = m_tilemap[tm].m_tilemap_8x8; // _alt
	}

	rectangle clip = cliprect;
	clip.min_y = clip.max_y = line;

g_profiler.start(PROFILER_TILEMAP_DRAW_ROZ);
	/* get the full pixmap for the tilemap */
	tilemap->pixmap();

	/* then do the roz copy */
	hng64_tilemap_draw_roz_core_line(screen, bitmap, clip, tilemap, 1, get_blend_mode(tm), 0x80, mosaic, tm);
g_profiler.stop();

}


uint32_t hng64_state::screen_update_hng64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#if 0
	// press in sams64_2 attract mode for a nice debug screen from the game
	// not sure how functional it is, and it doesn't appear to test everything (rowscroll modes etc.)
	// but it could be useful
	if (machine().input().code_pressed_once(KEYCODE_L))
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		if (!strcmp(machine().system().name, "sams64_2"))
		{
			space.write_byte(0x2f27c8, 0x2);
		}
	}
#endif

	// Initialize some buffers
	if (m_fbcontrol[0] & 0x01) //FIXME: Is the register correct? check with HW tests
		bitmap.fill(m_palette->pen(0), cliprect);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	screen.priority().fill(0x00, cliprect);

	// If the screen is disabled, don't draw anything (m_screen_dis is a shady variable at best)
	if (m_screen_dis)
		return 0;

	// If the auto-animation mask or bits have changed search for tiles using them and mark as dirty
	const uint32_t animmask = m_videoregs[0x0b];
	const uint32_t animbits = m_videoregs[0x0c];
	if ((m_old_animmask != animmask) || (m_old_animbits != animbits))
	{
		int tile_index;
		for (tile_index = 0; tile_index < 128 * 128; tile_index++)
		{
			if (m_videoram[tile_index + (0x00000 / 4)] & 0x200000)
			{
				hng64_mark_tile_dirty(0, tile_index);
			}
			if (m_videoram[tile_index + (0x10000 / 4)] & 0x200000)
			{
				hng64_mark_tile_dirty(1, tile_index);
			}
			if (m_videoram[tile_index + (0x20000 / 4)] & 0x200000)
			{
				hng64_mark_tile_dirty(2, tile_index);
			}
			if (m_videoram[tile_index + (0x30000 / 4)] & 0x200000)
			{
				hng64_mark_tile_dirty(3, tile_index);
			}
		}

		m_old_animmask = animmask;
		m_old_animbits = animbits;
	}

	// If any magic bits have been touched, mark every tilemap dirty
	uint16_t tileflags[4];
	tileflags[0] = m_videoregs[0x02] >> 16;
	tileflags[1] = m_videoregs[0x02] & 0xffff;
	tileflags[2] = m_videoregs[0x03] >> 16;
	tileflags[3] = m_videoregs[0x03] & 0xffff;
	const uint16_t IMPORTANT_DIRTY_TILEFLAG_MASK = 0x0600;
	for (int i = 0; i < 4; i++)
	{
		if ((m_old_tileflags[i] & IMPORTANT_DIRTY_TILEFLAG_MASK) != (tileflags[i] & IMPORTANT_DIRTY_TILEFLAG_MASK))
		{
			hng64_mark_all_tiles_dirty(i);
			m_old_tileflags[i] = tileflags[i];
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int i = 0x3f; i >= 0; i--)
		{
			for (int j = 0; j < 4; j++)
			{
				uint16_t pri = get_tileregs(j) & 0x3f;

				if (pri == i)
					hng64_drawtilemap(screen, bitmap, cliprect, j, 0, y);
			}
		}
	}

	// 3d gets drawn next
	if (!(m_fbcontrol[0] & 0x01))
	{
		// Blit the color buffer into the primary bitmap
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			const uint32_t *src = &m_poly_renderer->colorBuffer3d().pix(y, cliprect.min_x);
			uint32_t *dst = &bitmap.pix(y, cliprect.min_x);

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				if (*src & 0xff000000)
					*dst = *src;

				dst++;
				src++;
			}
		}
	}

	// Draw the sprites on top of everything
	draw_sprites(screen, bitmap, cliprect);

	// Layer the global frame buffer operations on top of everything
	// transition_control(bitmap, cliprect);


#if HNG64_VIDEO_DEBUG
	if (0)
		popmessage("%08x %08x %08x %08x %08x", m_spriteregs[0], m_spriteregs[1], m_spriteregs[2], m_spriteregs[3], m_spriteregs[4]);

	if (1)
		popmessage("%08x %08x TR(%04x %04x %04x %04x) SB(%04x %04x %04x %04x) %08x %08x %08x %08x %08x AA(%08x %08x) %08x",
			m_videoregs[0x00],
			m_videoregs[0x01],
			(m_videoregs[0x02] >> 16) & 0xffff,
			(m_videoregs[0x02] >> 0) & 0xffff,  //       ss64_2 debug mode indicates that 0x0040 is enable!
			(m_videoregs[0x03] >> 16) & 0xffff, //       buriki agrees (debug data on text layer) xrally agress (pink layer)
			(m_videoregs[0x03] >> 0) & 0xffff,  //       fatal fury doesn't (all backgrounds have it set) joy
			(m_videoregs[0x04] >> 16) & 0xffff,
			(m_videoregs[0x04] >> 0) & 0xffff,
			(m_videoregs[0x05] >> 16) & 0xffff,
			(m_videoregs[0x05] >> 0) & 0xffff,
			m_videoregs[0x06],
			m_videoregs[0x07],
			m_videoregs[0x08],
			m_videoregs[0x09],
			m_videoregs[0x0a],
			m_videoregs[0x0b],
			m_videoregs[0x0c],
			m_videoregs[0x0d]);

	if (0)
		popmessage("TC: %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x : %08x %08x %08x %08x",
			m_tcram[0x00 / 4],
			m_tcram[0x04 / 4],
			m_tcram[0x08 / 4], // tilemaps 0/1 ?
			m_tcram[0x0c / 4], // ss64_2 debug 04000000 = 'half' on tm1   00000004 = 'half' on tm3  (used in transitions?)
			m_tcram[0x10 / 4],
			m_tcram[0x14 / 4],
			m_tcram[0x18 / 4],
			m_tcram[0x1c / 4],
			m_tcram[0x20 / 4],
			m_tcram[0x24 / 4],
			m_tcram[0x28 / 4],
			m_tcram[0x2c / 4],
			m_tcram[0x30 / 4],
			m_tcram[0x34 / 4],
			m_tcram[0x38 / 4],
			m_tcram[0x3c / 4],
			m_tcram[0x40 / 4],
			m_tcram[0x44 / 4],
			m_tcram[0x48 / 4],
			m_tcram[0x4c / 4],
			m_tcram[0x50 / 4],
			m_tcram[0x54 / 4],
			m_tcram[0x58 / 4],
			m_tcram[0x5c / 4]);

	/*
	    m_tcram[0x0c/4]
	    05002201  blending?
	    01002201

	    m_tcram[0x14/4],
	    0011057f  blending?
	    0001057f
	*/
#endif

	return 0;
}

WRITE_LINE_MEMBER(hng64_state::screen_vblank_hng64)
{
	// rising edge and buffer swap
	if (state && (m_tcram[0x50 / 4] & 0x10000))
		clear3d();
}


/* Transition Control Video Registers
 * ----------------------------------
 *
 * uint32_t | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *      0 |                                         |
 *      1 | xxxx xxxx xxxx xxxx yyyy yyyy yyyy yyyy | Min X / Min Y visible area rectangle values
 *      2 | xxxx xxxx xxxx xxxx yyyy yyyy yyyy yyyy | Max X / Max Y visible area rectangle values (added up with the Min X / Min Y)
 *      3 |                                         |
 *      4 |                                         |
 *      5 | ---- ---- ---- ---? ---- --?? ???? ???? | Global Fade In/Fade Out control
 *      6 |                                         |
 *      7 | ---- ---- xxxx xxxx xxxx xxxx xxxx xxxx | Port A of RGB fade (subtraction)
 *      8 |                                         |
 *      9 | ---- ---- ---- ---? ---- ---- ---- ???? | Per-layer Fade In/Fade Out control
 *     10 | ---- ---- xxxx xxxx xxxx xxxx xxxx xxxx | Port B of RGB fade (additive)
 *     11 | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | Unknown - looks like an ARGB value - it seems to change when the scene changes
 *     12 |                                         |
 *     13 |                                         |
 *     14 |                                         |
 *     15 |                                         |
 *     16 |                                         |
 *     17 |                                         |
 *     18 | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | V-Blank related stuff
 *     19 |                                         |
 *     20 | ---- ---- ---- ---x ---- ---- ---- ---- | Back layer control register?
 *     21 |                                         |
 *     22 |                                         |
 *     23 |                                         |
 *     24 |                                         |
 *
 *  Various bits change depending on what is happening in the scene.
 *  These bits may set which 'layer' is affected by the blending.
 *  Or maybe they adjust the scale of the lightening and darkening...
 *  Or maybe it switches from fading by scaling to fading using absolute addition and subtraction...
 *  Or maybe they set transition type (there seems to be a cute scaling-squares transition in there somewhere)...
 */

// Transition Control memory.
void hng64_state::tcram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t *hng64_tcram = m_tcram;

	COMBINE_DATA(&hng64_tcram[offset]);

	if (offset == 0x02)
	{
		rectangle visarea = m_screen->visible_area();

		const uint16_t min_x = (hng64_tcram[1] & 0xffff0000) >> 16;
		const uint16_t min_y = (hng64_tcram[1] & 0x0000ffff) >> 0;
		const uint16_t max_x = (hng64_tcram[2] & 0xffff0000) >> 16;
		const uint16_t max_y = (hng64_tcram[2] & 0x0000ffff) >> 0;

		if (max_x == 0 || max_y == 0) // bail out if values are invalid, Fatal Fury WA sets this to disable the screen.
		{
			m_screen_dis = 1;
			return;
		}

		m_screen_dis = 0;

		visarea.set(min_x, min_x + max_x - 1, min_y, min_y + max_y - 1);
		m_screen->configure(HTOTAL, VTOTAL, visarea, m_screen->frame_period().attoseconds());
	}
}

uint32_t hng64_state::tcram_r(offs_t offset)
{
	/* is this really a port? this seems treated like RAM otherwise, check if there's code anywhere
	   to write the desired value here instead */
	if ((offset * 4) == 0x48)
		return m_vblank->read();

	return m_tcram[offset];
}

// Very much a work in progress - no hard testing has been done
void hng64_state::transition_control(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// If either of the fading memory regions is non-zero...
	if (m_tcram[0x00000007] != 0x00000000 || m_tcram[0x0000000a] != 0x00000000)
	{
		const int32_t darkR = int32_t(m_tcram[0x00000007] & 0xff);
		const int32_t darkG = int32_t((m_tcram[0x00000007] >> 8) & 0xff);
		const int32_t darkB = int32_t((m_tcram[0x00000007] >> 16) & 0xff);

		const int32_t brigR = int32_t(m_tcram[0x0000000a] & 0xff);
		const int32_t brigG = int32_t((m_tcram[0x0000000a] >> 8) & 0xff);
		const int32_t brigB = int32_t((m_tcram[0x0000000a] >> 16) & 0xff);

		for (int i = cliprect.min_x; i < cliprect.max_x; i++)
		{
			for (int j = cliprect.min_y; j < cliprect.max_y; j++)
			{
				rgb_t *thePixel = reinterpret_cast<rgb_t *>(&bitmap.pix(j, i));

				int32_t finR = (int32_t)thePixel->r();
				int32_t finG = (int32_t)thePixel->g();
				int32_t finB = (int32_t)thePixel->b();

#if 0
				float colorScaleR, colorScaleG, colorScaleB;

				// Apply the darkening pass (0x07)...
				colorScaleR = 1.0f - (float)(m_tcram[0x00000007] & 0xff) / 255.0f;
				colorScaleG = 1.0f - (float)((m_tcram[0x00000007] >> 8) & 0xff) / 255.0f;
				colorScaleB = 1.0f - (float)((m_tcram[0x00000007] >> 16) & 0xff) / 255.0f;

				finR = ((float)thePixel->r() * colorScaleR);
				finG = ((float)thePixel->g() * colorScaleG);
				finB = ((float)thePixel->b() * colorScaleB);

				// Apply the lightening pass (0x0a)...
				colorScaleR = 1.0f + (float)(m_tcram[0x0000000a] & 0xff) / 255.0f;
				colorScaleG = 1.0f + (float)((m_tcram[0x0000000a] >> 8) & 0xff) / 255.0f;
				colorScaleB = 1.0f + (float)((m_tcram[0x0000000a] >> 16) & 0xff) / 255.0f;

				finR *= colorScaleR;
				finG *= colorScaleG;
				finB *= colorScaleB;

				// Clamp
				if (finR > 255.0f) finR = 255.0f;
				if (finG > 255.0f) finG = 255.0f;
				if (finB > 255.0f) finB = 255.0f;
#endif

				// Subtractive fading
				if (m_tcram[0x00000007] != 0x00000000)
				{
					finR -= darkR;
					finG -= darkG;
					finB -= darkB;
				}

				// Additive fading
				if (m_tcram[0x0000000a] != 0x00000000)
				{
					finR += brigR;
					finG += brigG;
					finB += brigB;
				}

				// Clamp the high end
				if (finR > 255) finR = 255;
				if (finG > 255) finG = 255;
				if (finB > 255) finB = 255;

				// Clamp the low end
				if (finR < 0) finR = 0;
				if (finG < 0) finG = 0;
				if (finB < 0) finB = 0;

				*thePixel = rgb_t(255, (uint8_t)finR, (uint8_t)finG, (uint8_t)finB);
			}
		}
	}
}

void hng64_state::video_start()
{
	m_old_animmask = -1;
	m_old_animbits = -1;
	m_old_tileflags[0] = -1;
	m_old_tileflags[1] = -1;
	m_old_tileflags[2] = -1;
	m_old_tileflags[3] = -1;

	m_tilemap[0].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile0_8x8_info)),   TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[0].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile0_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[0].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile0_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	m_tilemap[1].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile1_8x8_info)),   TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[1].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile1_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[1].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile1_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	m_tilemap[2].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile2_8x8_info)),   TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[2].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile2_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[2].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile2_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	m_tilemap[3].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile3_8x8_info)),   TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[3].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile3_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[3].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hng64_state::get_hng64_tile3_16x16_info)), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	for (auto &elem : m_tilemap)
	{
		elem.m_tilemap_8x8->set_transparent_pen(0);
		elem.m_tilemap_16x16->set_transparent_pen(0);
		elem.m_tilemap_16x16_alt->set_transparent_pen(0);
	}

	// Rasterizer creation
	m_poly_renderer = std::make_unique<hng64_poly_renderer>(*this);

	// 3d information
	m_dl = std::make_unique<uint16_t[]>(0x100);
	m_polys.resize(HNG64_MAX_POLYGONS);

	m_texturerom = memregion("textures")->base();
	m_vertsrom = (uint16_t*)memregion("verts")->base();
	m_vertsrom_size = memregion("verts")->bytes();
}

#include "hng64_3d.ipp"
#include "hng64_sprite.ipp"
