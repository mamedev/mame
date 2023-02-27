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
			xtopleft = (m_videoram[(0x40000 + (scrollbase << 4)) / 4]);
			xmiddle = (m_videoram[(0x40004 + (scrollbase << 4)) / 4]); // middle screen point

			uint32_t xtopleft2 = (m_videoram[(0x40000 + (line * 0x10) + (scrollbase << 4)) / 4]);
			uint32_t xmiddle2 = (m_videoram[(0x40004 + (line * 0x10) + (scrollbase << 4)) / 4]); // middle screen point

			if ((xtopleft2 & 0xff) == 0x00)// set to 0x00 if we should use the line data?
				xtopleft = xtopleft2;

			if ((xmiddle2 & 0xff) == 0x00) // also set to 0x00 if we should use the line data?
				xmiddle = xmiddle2;

			ytopleft = (m_videoram[(0x40008 + (line * 0x10) + (scrollbase << 4)) / 4]);
			ymiddle = (m_videoram[(0x4000c + (line * 0x10) + (scrollbase << 4)) / 4]); // middle screen point
		}

		if (global_alt_scroll_register_format) // globally selects alt scroll register layout???
		{
			xinc = (xmiddle - xtopleft) / 512;
			yinc = 0;
			xinc2 = 0;
			yinc2 = (ymiddle - ytopleft) / 512;
		}
		else
		{
			xinc = (xmiddle - xtopleft) / 512;
			yinc = (ymiddle - ytopleft) / 512;
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
 *   0    | ---- -Cdd ---- -??Z ---- ---- ---- --uu |  C = global complex zoom
          | 0000 0011  - road edge alt 1            | dd = global tilemap dimension selector
          | 0000 0111  - road edge alt 2            |  ? = Always Set?
          |                                         |  Z = Global Zoom Disable?
		  |                                         |  u = bit 0 is explicitly cleared from initialized value in sams64, both bits turned on for buriki 'split' effect
 *   1    | oooo oooo oooo oooX ---- ---- ---- ---- | unknown - X is sometimes used (1 in demo of xrally, 0 in game) not always initialized  whole register gets set to 0xffff during mosaic bit of roadedge intro. Also buriki intro
 *        | ---- ---- ---- ---- oooo oooo oYoo oooo | unknown - untouched in sams64 games, initialized elsewhere  Y gets set to 4 at some points in xrally attract
 *   2    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap0 per layer flags
 *        | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap1 per layer flags
 *   3    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap2 per layer flags
 *        | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap3 per layer flags
 *   4    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap0 scrollbase when not floor, lineram offset when floor
 *        | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap1 scrollbase when not floor, lineram offset when floor
 *   5    | xxxx xxxx xxxx xxxx ---- ---- ---- ---- | tilemap3 scrollbase when not floor, lineram offset when floor
 *        | ---- ---- ---- ---- xxxx xxxx xxxx xxxx | tilemap4 scrollbase when not floor, lineram offset when floor
 *   6    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 000001ff (fatfurwa)  ---- bfff (xrally, and fatfurwa despite comment? maybe reads MAME initialized value and changes it?)
 *   7    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 000001ff (fatfurwa)  5e00 3fff (xrally ^^ )
 *   8    | oooo oooo oooo oooo oooo oooo oooo oooo | unknown - always seems to be 80008000 (fatfurwa)  9e00 be00 (xrally ^^ )
 *   9    | oooo oooo oooo oooo ---- ---- ---- ---- | some kind of tilemap split effect?
          | ---- ---- ---- ---- oooo oooo oooo oooo | ^
 *   a    | oooo oooo oooo oooo ---- ---- ---- ---- | ^
 *        | ---- ---- ---- ---- oooo oooo oooo oooo | ^
 *   b    | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | auto animation mask for tilemaps
 *   c    | xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | auto animation bits for tilemaps
 *   d    | oooo oooo oooo oooo oooo oooo oooo oooo | not used ??

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

	// set during transitions, could be 'disable all palette output'?
	if ((m_tcram[0x24 / 4] >> 16) & 0x0002)
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

	pen_t const *const clut = &m_palette->pen(0);

	// 3d gets drawn next
	if (!(m_fbcontrol[0] & 0x01))
	{
		// Blit the color buffer into the primary bitmap
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
#ifdef USE_32BIT_3DBUFFER
			const uint32_t *src = &m_poly_renderer->colorBuffer3d().pix(y, cliprect.min_x);
			uint32_t *dst = &bitmap.pix(y, cliprect.min_x);

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				if (*src & 0xff000000)
					*dst = *src;

				dst++;
				src++;
			}
#else
			const uint16_t *src = &m_poly_renderer->colorBuffer3d().pix(y, cliprect.min_x);
			uint32_t *dst = &bitmap.pix(y, cliprect.min_x);

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				uint16_t srcpix = *src;
				if (srcpix & 0x0fff)
				{
					*dst = clut[srcpix & 0x0fff];
				}

				dst++;
				src++;
			}
#endif
		}
	}

	// Draw the sprites on top of everything
	draw_sprites_buffer(screen, cliprect);

	// copy sprites into display

	// this correctly allows buriki intro sprites to use regular alpha, not additive
	// while also being correct for sams64, which wants additive, but appears to be
	// incorrect for Fatal Fury's hit effects which want additive
	// 
	// the 6 regs around here have the same values in fatfur and buriki, so are unlikely
	// to control the blend type.
	//uint8_t spriteblendtype = (m_tcram[0x10 / 4] >> 16) & 0x10;

	// would be an odd place for it, after the 'vblank' flag but...
	uint8_t spriteblendtype = (m_tcram[0x4c / 4] >> 16) & 0x01;


	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const uint16_t *src = &m_sprite_bitmap.pix(y, cliprect.min_x);
		uint32_t *dst = &bitmap.pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t srcpix = *src;
			if (srcpix & 0x7fff)
			{
				if (srcpix & 0x8000)
				{
					if (spriteblendtype)
						*dst = alpha_blend_r32(*(uint32_t *)dst, clut[srcpix & 0x7fff], 0x80);
					else
						*dst = add_blend_r32(*dst, clut[srcpix & 0x7fff]);
				}
				else
				{
					*dst = clut[srcpix & 0x7fff];
				}
			}

			dst++;
			src++;
		}
	}



#if HNG64_VIDEO_DEBUG
	if (0)
		popmessage("%08x %08x %08x %08x %08x", m_spriteregs[0], m_spriteregs[1], m_spriteregs[2], m_spriteregs[3], m_spriteregs[4]);

	// see notes at top for more detailed info on these
	if (0)
		popmessage("%08x %08x\nTR(%04x %04x %04x %04x)\nSB(%04x %04x %04x %04x)\n%08x %08x %08x\nSPLIT?(%04x %04x %04x %04x)\nAA(%08x %08x)\n%08x",
			// global tilemap control regs?
			m_videoregs[0x00], m_videoregs[0x01],
			// general per-tilemap regs
			(m_videoregs[0x02] >> 16) & 0xffff, (m_videoregs[0x02] >> 0) & 0xffff, (m_videoregs[0x03] >> 16) & 0xffff, (m_videoregs[0x03] >> 0) & 0xffff,
			// scrollbase regs
			(m_videoregs[0x04] >> 16) & 0xffff, (m_videoregs[0x04] >> 0) & 0xffff, (m_videoregs[0x05] >> 16) & 0xffff, (m_videoregs[0x05] >> 0) & 0xffff,
			// initialized to fixed values?
			m_videoregs[0x06], m_videoregs[0x07], m_videoregs[0x08],
			// used when a single tilemap gets 'split' on Buriki player entrances?
			(m_videoregs[0x09] >> 16) & 0xffff, (m_videoregs[0x09] >> 0) & 0xffff, (m_videoregs[0x0a] >> 16) & 0xffff, (m_videoregs[0x0a] >> 0) & 0xffff,
			// Auto Animation registers
			m_videoregs[0x0b], m_videoregs[0x0c],
			// unused?
			m_videoregs[0x0d]);

	if (1)
		popmessage("TC: %08x MINX(%d) MINY(%d) MAXX(%d) MAXY(%d)\nBLEND ENABLES? %02x %02x %02x | %02x %02x %02x\nUNUSED?(%04x)\n%04x\n%04x\nMASTER FADES - ADD?(%08x) - SUBTRACT?(%08x)\nUNUSED?(%08x)\nBITFIELD REGS(%04x)\nPALEFFECT_ENABLES(%d %d %d %d %d %d %d %d)\n PALFADES?(%08x %08x : %08x %08x : %08x %08x : %08x %08x)\n % 08x % 08x : % 08x % 08x % 08x % 08x",
			m_tcram[0x00 / 4], // 0007 00e4 (fatfurwa, bbust2)
			(m_tcram[0x04 / 4] >> 16) & 0xffff, (m_tcram[0x04 / 4] >> 0) & 0xffff, // 0000 0010 (fatfurwa) 0000 0000 (bbust2, xrally)
			(m_tcram[0x08 / 4] >> 16) & 0xffff, (m_tcram[0x08 / 4] >> 0) & 0xffff, // 0200 01b0 (fatfurwa) 0200 01c0 (bbust2, xrally)

			// is this 2 groups of 3 regS?
			(m_tcram[0x0c / 4] >> 24) & 0xff, // 04 = 'blend' on tm1  
			(m_tcram[0x0c / 4] >> 16) & 0xff, // 04 = blend all sprites? (buriki intro, text fades?)
			(m_tcram[0x0c / 4] >> 8) & 0xff,  // 10 gets set here in some cases when blended sprites are used too (blend type against a different input layer?)
			// 2nd group?
			(m_tcram[0x0c / 4] >> 0) & 0xff, //  04 = 'blend' on tm3  (used in transitions?)
			(m_tcram[0x10 / 4] >> 24) & 0xff,
			(m_tcram[0x10 / 4] >> 16) & 0xff, // 10 being set seems to change sprite blend mode (maybe blend type against one input layer)

			m_tcram[0x10 / 4] & 0xffff, // unused?

			(m_tcram[0x14 / 4] >> 16) & 0xffff,  // typically 0007 or 0001, - 0011 on ss64 ingame, 0009 on continue screen
			(m_tcram[0x14 / 4] >> 0) & 0xffff,   // 0xxx ?  (often 0555 or 0fff, seen 56a, 57f too)

			// these are used for 'fade to black' in most cases, but
			// in xrally attract, when one image is meant to fade into another, one value increases while the other decreases
			m_tcram[0x18 / 4],  // xRGB fade values? (roadedge attract)
			m_tcram[0x1c / 4],  // xRGB fade values? (roadedge attract) fades on fatfurwa before buildings in intro

			m_tcram[0x20 / 4],  //  unused?

			// some kind of bitfields
			(m_tcram[0x24 / 4] >> 16) & 0xffff, // 0002 gets set in roadedge during some transitions (layers are disabled? blacked out?) 0001 set on SNK logo in roadedge 
			(m_tcram[0x24 / 4] >> 0) & 0x3, // 0001 gets set when in a tunnel on roadedge in 1st person mode (state isn't updated otherwise, switching back to 3rd person in a tunnel leaves it set until you flick back to 1st person)  briefly set to 3c on roadedge car select during 'no fb clear' effect?
			(m_tcram[0x24 / 4] >> 2) & 0x3,
			(m_tcram[0x24 / 4] >> 4) & 0x3,
			(m_tcram[0x24 / 4] >> 6) & 0x3,
			(m_tcram[0x24 / 4] >> 8) & 0x3,
			(m_tcram[0x24 / 4] >> 10) & 0x3,
			(m_tcram[0x24 / 4] >> 12) & 0x3,
			(m_tcram[0x24 / 4] >> 14) & 0x3,

			// 7 of these fade during the buriki SNK logo (probably redundant)
			// in roadedge they're just set to
			// 0x00000000, 0x01000000, 0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000
			// the first 8 bits seem to be which palette this affects (so 0x0b is colours 0xb00-0xbff as you see with xrally 1st person?)
			// enabled with m_tcram[0x24 / 4] regs above? (see note)

			m_tcram[0x28 / 4],  // ?RGB fade values (buriki jumbotron)  fades on fatfurwa before high score table etc. + bottom value only on 'fade to red' part of fatfurywa intro
			m_tcram[0x2c / 4],  // ?RGB fade values (buriki jumbotron)
			m_tcram[0x30 / 4],
			m_tcram[0x34 / 4],
			m_tcram[0x38 / 4],
			m_tcram[0x3c / 4],  // gets used for the 1st person view on xrally - maybe some fade effect on the front of the car?
			m_tcram[0x40 / 4],
			m_tcram[0x44 / 4],


			m_tcram[0x48 / 4], // this is where the 'vblank' thing lives
			m_tcram[0x4c / 4], // 0001 0000 or 0000 0000 - works(?) as blend type selection for sprite mixing?
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

	
	/*
		palette manipulation note
	 
		pal7 pal6 pal5 pal4 pal3 pal2 pal1 pal0  // which fade register those bits relate to?
 		00   00   11   00   00   00   10   10    // bits in  (m_tcram[0x24 / 4] >> 0)  (set to 0c0a in this example)

		an entry of 00 means palette effect not in use?
		an entry of 11 means subtractive?
		an entry of 10 means addition?
		an entry of 01 means??
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

void hng64_state::update_palette_entry(int entry)
{
	uint32_t data = m_paletteram[entry];

	int16_t r = (data >> 16) & 0xff;
	int16_t g = (data >> 8) & 0xff;
	int16_t b = (data >> 0) & 0xff;

	for (int i = 0; i < 8; i++)
	{
		uint32_t tcdata = m_tcram[(0x28 / 4) + i];

		// correct for buriki (jumbotron fading images) and xrally / roadedge (palette effect on front of car in first person view)
		//
		// however this logic fails on fatal fury fades, as the msb is 00, indicating apply to palette entries 000-0ff but the actual
		// palette needing the changes is 0x900 - 0x9ff (hng64 logo white flash) so there must be further ways in which this is
		// configured
		uint8_t tcregion = (tcdata & 0x0f000000) >> 24;

		if (tcregion == (entry >> 8))
		{
			uint8_t bmod = (tcdata >> 16) & 0xff;
			uint8_t gmod = (tcdata >> 8) & 0xff;
			uint8_t rmod = (tcdata >> 0) & 0xff;

			uint8_t tcreg = (m_tcram[0x24 / 4] >> (i << 1)) & 3;
			switch (tcreg)
			{
			case 0x00: // this entry is disabled
				break;
			case 0x01: // this entry is disabled(?)
				break;
			case 0x02: // additive blending
				r = r + rmod;
				if (r > 0xff)
					r = 0xff;
				g = g + gmod;
				if (g > 0xff)
					g = 0xff;
				b = b + bmod;
				if (b > 0xff)
					b = 0xff;
				break;
			case 0x03: // subtractive blending
				r = r - rmod;
				if (r < 0x00)
					r = 0x00;
				g = g - gmod;
				if (g < 0x00)
					g = 0x00;
				b = b - bmod;
				if (b < 0x00)
					b = 0x00;
				break;
			}
		}
	}

	m_palette->set_pen_color(entry, r,g,b);
}

void hng64_state::pal_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette_entry(offset);
}

// Transition Control memory.
void hng64_state::tcram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t *hng64_tcram = m_tcram;

	uint32_t old_data = hng64_tcram[offset];
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

	if ((offset >= (0x28 / 4)) && (offset < (0x48 / 4)))
	{
		uint32_t new_data = hng64_tcram[offset];
		if (old_data != new_data)
		{
			// the old blend value no longer applies, so update the colours referenced by that
			uint8_t old_tcregion = (old_data & 0x0f000000) >> 24;
			for (int i = 0; i < 0x100; i++)
			{
				update_palette_entry((old_tcregion * 0x100) + i);
			}
			// update the colours referenced by the new blend register
			uint8_t new_tcregion = (new_data & 0x0f000000) >> 24;
			for (int i = 0; i < 0x100; i++)
			{
				update_palette_entry((new_tcregion * 0x100) + i);
			}
		}
	}

	if (offset == (0x24 / 4))
	{
		// lazy, just update the lot
		uint32_t new_data = hng64_tcram[offset];
		if (old_data != new_data)
		{
			for (int i = 0; i < 0x1000; i++)
			{
				update_palette_entry(i);
			}
		}
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

	m_screen->register_screen_bitmap(m_sprite_bitmap);
	m_screen->register_screen_bitmap(m_sprite_zbuffer);

}


#include "hng64_3d.ipp"
#include "hng64_sprite.ipp"
