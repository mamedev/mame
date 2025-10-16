// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
#include "emu.h"
#include "hng64.h"

#include <algorithm>

#define LOG_3D           (1 << 1)
#define LOG_FRAMEBUFFER  (1 << 2)
#define LOG_DISPLAY_LIST (1 << 3)
#define LOG_TEXTURE      (1 << 4)

#define VERBOSE (0)

#include "logmacro.h"

#define LOG3D(...)           LOGMASKED(LOG_3D, __VA_ARGS__)
#define LOGFRAMEBUFFER(...)  LOGMASKED(LOG_FRAMEBUFFER, __VA_ARGS__)
#define LOGDISPLAYLIST(...)  LOGMASKED(LOG_DISPLAY_LIST, __VA_ARGS__)
#define LOGTEXTURE(...)      LOGMASKED(LOG_TEXTURE, __VA_ARGS__)

/*
    final mix can clearly only process 2 possibilities for any screen pixel; a 'top' and 'bottom' pixel option
    one of those can be blended.
    blended pixels can't be stacked (only one still appears as blended, the other becomes solid)

    many examples can be found where using alpha effects just cuts holes in sprites/3D or erases other alpha
    tilemap layers due to this
*/

#define HNG64_VIDEO_DEBUG 0


void hng64_state::mark_all_tiles_dirty(int tilemap)
{
	m_tilemap[tilemap].m_tilemap_8x8->mark_all_dirty();
	m_tilemap[tilemap].m_tilemap_16x16->mark_all_dirty();
	m_tilemap[tilemap].m_tilemap_16x16_alt->mark_all_dirty();
}

void hng64_state::mark_tile_dirty(int tilemap, int tile_index)
{
	m_tilemap[tilemap].m_tilemap_8x8->mark_tile_dirty(tile_index);
	m_tilemap[tilemap].m_tilemap_16x16->mark_tile_dirty(tile_index);
	m_tilemap[tilemap].m_tilemap_16x16_alt->mark_tile_dirty(tile_index);
}

// pppppppp ffattttt tttttttt tttttttt
template <u8 Which, u8 Size>
TILE_GET_INFO_MEMBER(hng64_state::get_tile_info)
{
	const u16 tilemapinfo = get_tileregs(Which);

	u32 tileno = m_videoram[tile_index + (Which << 14)];

	const u32 pal = (tileno & 0xff000000) >> 24;
	const int flip =(tileno & 0x00c00000) >> 22;

	if (BIT(tileno, 21))
	{
		if (BIT(m_videoregs[0x01], 16))
			tileno = (tileno & m_videoregs[0x0b]) | m_videoregs[0x0c];
	}

	tileno &= 0x1fffff;

	if (Size == 0)
	{
		if (BIT(tilemapinfo, 10))
		{
			tileinfo.set(1, tileno >> 1, pal >> 4, TILE_FLIPYX(flip));
		}
		else
		{
			tileinfo.set(0, tileno, pal, TILE_FLIPYX(flip));
		}
	}
	else
	{
		if (BIT(tilemapinfo, 10))
		{
			tileinfo.set(3, tileno >> 3, pal >> 4, TILE_FLIPYX(flip));
		}
		else
		{
			tileinfo.set(2, tileno >> 2, pal, TILE_FLIPYX(flip));
		}
	}
}


void hng64_state::videoram_w(offs_t offset, u32 data, u32 mem_mask)
{
	const int realoff = (offset * 4);
	COMBINE_DATA(&m_videoram[offset]);

	if ((realoff >= 0) && (realoff < 0x10000))
	{
		mark_tile_dirty(0, offset & 0x3fff);
	}
	else if ((realoff >= 0x10000) && (realoff < 0x20000))
	{
		mark_tile_dirty(1, offset & 0x3fff);
	}
	else if ((realoff >= 0x20000) && (realoff < 0x30000))
	{
		mark_tile_dirty(2, offset & 0x3fff);
	}
	else if ((realoff >= 0x30000) && (realoff < 0x40000))
	{
		mark_tile_dirty(3, offset & 0x3fff);
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
		*dest = clut[INPUT_VAL]; \
	else if (drawformat == 2) \
		*dest = add_blend_r32(*dest, clut[INPUT_VAL]); \
	else if (drawformat == 3) \
		*dest = alpha_blend_r32(*dest, clut[INPUT_VAL], alpha); \
} while (0)

void hng64_state::tilemap_draw_roz_core_line(
		screen_device &screen, bitmap_rgb32 &destbitmap, const rectangle &cliprect, tilemap_t *tmap,
		int wraparound, u8 drawformat, u8 alpha, u8 mosaic, u8 tm, int splitside)
{
	int source_line_to_use = cliprect.min_y;
	source_line_to_use = (source_line_to_use / (mosaic+1)) * (mosaic+1);

	int xinc, xinc2, yinc, yinc2;
	s32 xtopleft;
	s32 ytopleft;
	const u16 scrollbase = get_scrollbase(tm);
	const u16 tileregs = get_tileregs(tm);
	const u8 not_line_mode = (tileregs & 0x0800) >> 11;

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
			const s32 xalt = (m_videoram[(0x40004 + (scrollbase << 4)) / 4]); // middle screen point
			const s32 xmiddle = (m_videoram[(0x40010 + (scrollbase << 4)) / 4]);

			ytopleft = (m_videoram[(0x40008 + (scrollbase << 4)) / 4]);
			const s32 yalt = (m_videoram[(0x40018 + (scrollbase << 4)) / 4]); // middle screen point
			const s32 ymiddle = (m_videoram[(0x4000c + (scrollbase << 4)) / 4]);

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

			s32 xmiddle;
			s32 ymiddle;

			const u32& global_tileregs = m_videoregs[0x00];
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

				if (splitside == 1)
				{
					xtopleft += ((m_videoregs[0x0a] >> 0) & 0xffff) << 16;
					ytopleft += ((m_videoregs[0x0a] >> 16) & 0xffff) << 16;

					xmiddle += ((m_videoregs[0x0a] >> 0) & 0xffff) << 16;
					ymiddle += ((m_videoregs[0x0a] >> 16) & 0xffff) << 16;
				}
				else if (splitside == 2)
				{
					xtopleft += ((m_videoregs[0x9] >> 0) & 0xffff) << 16;
					ytopleft += ((m_videoregs[0x9] >> 16) & 0xffff) << 16;

					xmiddle += ((m_videoregs[0x09] >> 0) & 0xffff) << 16;
					ymiddle += ((m_videoregs[0x09] >> 16) & 0xffff) << 16;
				}
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
		s32 xmiddle;
		s32 ymiddle;

		const u32& global_tileregs = m_videoregs[0x00];
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
			u16 line = source_line_to_use;
			xtopleft = (m_videoram[(0x40000 + (scrollbase << 4)) / 4]);
			xmiddle = (m_videoram[(0x40004 + (scrollbase << 4)) / 4]); // middle screen point

			u32 xtopleft2 = (m_videoram[(0x40000 + (line * 0x10) + (scrollbase << 4)) / 4]);
			u32 xmiddle2 = (m_videoram[(0x40004 + (line * 0x10) + (scrollbase << 4)) / 4]); // middle screen point

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

	u32 startx = xtopleft;
	u32 starty = ytopleft;
	const int incxx = xinc << 1;
	const int incxy = yinc2 << 1;
	const int incyx = xinc2 << 1;
	const int incyy = yinc << 1;

	// we have the scroll values for the current line, draw

	const pen_t *clut;

	// allow one of the 2 pairs of fade values to be used (complete guess! but fatfurwa turns this bit on whenever it wants to do a fade effect and off when it's done)
	if (BIT(get_tileregs(tm), 7))
	{
		// which one depends on target layer? (also complete guess!)
		// (buriki intro suggests this is wrong, it should be fading tilemaps to black on the discipline name screens with a full subtractive blend, but picks the wrong register?
		//  as we're already using this to decide which layer we're on for blending, it's probably not got a double use)
		if (BIT(get_tileregs(tm), 5))
		{
			clut = &m_palette_fade0->pen(0);
		}
		else
		{
			clut = &m_palette_fade1->pen(0);
		}
	}
	else
	{
		clut = &m_palette->pen(0);
	}

	bitmap_ind8 &priority_bitmap = screen.priority();
	const bitmap_ind16 &srcbitmap = tmap->pixmap();
	const bitmap_ind8 &flagsmap = tmap->flagsmap();
	const int xmask = srcbitmap.width()-1;
	const int ymask = srcbitmap.height()-1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;
	u32 priority = 0;
	u8 mask = 0x1f;
	u8 value = 0x10;

	/* pre-advance based on the cliprect */
	startx += cliprect.min_x * incxx + source_line_to_use * incyx;
	starty += cliprect.min_x * incxy + source_line_to_use * incyy;

	/* extract start/end points */
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;

	if (incxy == 0 && incyx == 0 && !wraparound)
	{
		/* optimized loop for the not rotated case */
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
			u32 cx = startx;
			u32 cy = starty >> 16;

			/* get source and priority pointers */
			u8 *pri = &priority_bitmap.pix(sy, sx);
			u16 const *const src = &srcbitmap.pix(cy);
			u8 const *const maskptr = &flagsmap.pix(cy);
			u32 *dest = &destbitmap.pix(sy, sx);

			/* loop over columns */
			int mosaic_counter = 0;
			u16 masksrc = 0;
			u16 datasrc = 0;

			while (x <= ex && cx < widthshifted)
			{
				u16 srcoffset = cx >> 16;

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
	else if (wraparound)
	{
		/* wraparound case */
		/* initialize X counters */
		int x = sx;
		u32 cx = startx;
		u32 cy = starty;

		/* get dest and priority pointers */
		u32 *dest = &destbitmap.pix(sy, sx);
		u8 *pri = &priority_bitmap.pix(sy, sx);

		/* loop over columns */
		int mosaic_counter = 0;
		u16 masksrc = 0;
		u16 datasrc = 0;

		while (x <= ex)
		{
			u16 srcoffset = (cx >> 16) & xmask;
			u16 srcyoffset = (cy >> 16) & ymask;

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
		u32 cx = startx;
		u32 cy = starty;

		/* get dest and priority pointers */
		u32 *dest = &destbitmap.pix(sy, sx);
		u8 *pri = &priority_bitmap.pix(sy, sx);

		int mosaic_counter = 0;
		u16 masksrc = 0;
		u16 datasrc = 0;

		/* loop over columns */
		while (x <= ex)
		{
			u16 srcoffset = cx >> 16;
			u16 srcyoffset = cy >> 16;

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
 * u32    | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ---- -Cdd ---- -??Z ---- ---- ---- --uu |  C = global complex zoom
          | 0000 0011  - road edge alt 1            | dd = global tilemap dimension selector
          | 0000 0111  - road edge alt 2            |  ? = Always Set?
          |                                         |  Z = Global Zoom Disable?
          |                                         |  u = bit 0 is explicitly cleared from initialized value in sams64, both bits turned on for buriki 'split' effect
 *   1    | oooo oooo oooo oooA ---- ---- ---- ---- |  A = tile animation enable? (buriki intro sets this to 0, then expects tile anim to stop while still updating registers below)  whole register gets set to 0xffff during mosaic bit of roadedge intro.
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
 */



u16 hng64_state::get_tileregs(int tm)
{
	return (m_videoregs[0x02 + BIT(tm, 1)] >> (BIT(tm, 0) ? 0 : 16)) & 0x0000ffff;
}

u16 hng64_state::get_scrollbase(int tm)
{
	return (m_videoregs[0x04 + BIT(tm, 1)] >> (BIT(tm, 0) ? 0 : 16)) & 0x00003fff;
}

int hng64_state::get_blend_mode(int tm)
{
	// this is based on xrally and sams64/sams64_2 use, it could be incorrect
	// it doesn't seem to be 100% on sams64_2 select screen when the mode select circle moves down - that is a blended sprite and blended tilemap mixing in a weird way to produce a darker than expected image as there are can be no other components in said mix

	u8 blendmode = 1;
	if (BIT(get_tileregs(tm), 5)) // for layers with L(1)
	{
		if (BIT(m_tcram[0x0c / 4], 2))
			blendmode = 2;
	}
	else // for layers with L(0)
	{
		if (BIT(m_tcram[0x0c / 4], 26))
			blendmode = 2;
	}

	return blendmode;
}


void hng64_state::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tm, int flags, int line)
{
	// Useful bits from the global tilemap flags
	const int global_dimensions = (m_videoregs[0x00] & 0x03000000) >> 24;

#if HNG64_VIDEO_DEBUG
	if ((global_dimensions != 0) && (global_dimensions != 3))
		popmessage("unsupported global_dimensions on tilemaps");
#endif

	// Determine which tilemap registers and scroll base this tilemap uses
	const u16 tileregs = get_tileregs(tm);

	// Useful bits from the tilemap registers
	const u8 mosaic = (tileregs & 0xf000) >> 12;
	//const u8 bpp = BIT(tileregs, 10); // not used?
	const u8 big = BIT(tileregs, 9);
	const u8 enable = BIT(tileregs, 6);
	const u8 wrap = BIT(tileregs, 8);

	// this bit is used to disable tilemaps, note on fatfurwa a raster interrupt is used to change this mid-screen, swapping between
	// the background layers and the floor being enabled!
	if (!enable)
	{
		return;
	}

	// Select the proper tilemap size
	tilemap_t* tilemap = nullptr;
	if (global_dimensions == 0)
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

	auto profile = g_profiler.start(PROFILER_TILEMAP_DRAW_ROZ);
	/* get the full pixmap for the tilemap */
	tilemap->pixmap();

	/* then do the roz copy */

	// buriki also turns on bit 0x00000002 and the effect is expected to apply to tm2 and tm3 at least (tm0/tm1 not used at this time)
	// sams64 does not initialize bit 0x00000002 though, only 0x00000001 to 0
	const int global_split_format = BIT(m_videoregs[0x00], 0);

	if (global_split_format)
	{
		clip.min_x = 256;
		clip.max_x = 512;
		tilemap_draw_roz_core_line(screen, bitmap, clip, tilemap, wrap, get_blend_mode(tm), 0x80, mosaic, tm, 1);
		clip.min_x = 0;
		clip.max_x = 256;
		tilemap_draw_roz_core_line(screen, bitmap, clip, tilemap, wrap, get_blend_mode(tm), 0x80, mosaic, tm, 2);

	}
	else
	{
		tilemap_draw_roz_core_line(screen, bitmap, clip, tilemap, wrap, get_blend_mode(tm), 0x80, mosaic, tm, 0);
	}
}

void hng64_state::mixsprites_test(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect, u16 priority, int y)
{
	// this correctly allows buriki intro sprites to use regular alpha, not additive
	// while also being correct for sams64, which wants additive, but appears to be
	// incorrect for Fatal Fury's hit effects which want additive
	//
	// the 6 regs around here have the same values in fatfur and buriki, so are unlikely
	// to control the blend type.
	//u8 spriteblendtype = (m_tcram[0x10 / 4] >> 16) & 0x10;

	// would be an odd place for it, after the 'vblank' flag but...
	const u8 spriteblendtype = BIT(m_tcram[0x4c / 4], 16);
	pen_t const* const spriteclut = &m_palette->pen(0);

	if (true)
	{
		const u16 *spritesrc = &m_sprite_bitmap.pix(y, cliprect.min_x);
		u32 *spritedst = &bitmap.pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u16 srcpix = *spritesrc;
			if (srcpix & 0x0fff)
			{
				// if ((srcpix & 0x7000) == 0x7000) // buriki jumbotron (behind all tilemaps, behind 3d) (behind tilemap prio 1b)
				// if ((srcpix & 0x7000) == 0x6000) // people behind arena walls (above lowest tilemap, behind others, behind 3d) (behind tilemap prio 16)

				//if ((srcpix & 0x7000) == 0x5000) // not used on buriki?
				//if ((srcpix & 0x7000) == 0x4000) // character portraits on buriki select screen (still behind 3d)

				//if ((srcpix & 0x7000) == 0x3000) // character names and bio (above 3D graphics)

				//if ((srcpix & 0x7000) == 0x2000) // select cursor, second system graphic, timers etc. (above ring tilemap, above 3d)

				//if ((srcpix & 0x7000) == 0x1000) // credit text etc.
				//if ((srcpix & 0x7000) == 0x0000 // not used on buriki ?

				if ((srcpix & 0x7000) == priority)
				{
					if (srcpix & 0x8000)
					{
						if (spriteblendtype)
							*spritedst = alpha_blend_r32(*spritedst, spriteclut[srcpix & 0x0fff], 0x80);
						else
							*spritedst = add_blend_r32(*spritedst, spriteclut[srcpix & 0x0fff]);
					}
					else
					{
						*spritedst = spriteclut[srcpix & 0x0fff];
					}
				}
			}

			spritedst++;
			spritesrc++;
		}

	}
}

u32 hng64_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

	// Draw sprites to buffer for later mixing
	draw_sprites_buffer(screen, cliprect);

	// set during transitions, could be 'disable all palette output'?
	if (BIT(m_tcram[0x24 / 4], 17))
		return 0;


	// If the auto-animation mask or bits have changed search for tiles using them and mark as dirty
	u32 animmask;

	// this bit either disables the feature entirely, or maybe disables the latching of these animation registers (leaving them in last used state)
	if (BIT(m_videoregs[0x01], 16))
		animmask = m_videoregs[0x0b];
	else
		animmask = 0xffffffff;

	const u32 animbits = m_videoregs[0x0c];
	if ((m_old_animmask != animmask) || (m_old_animbits != animbits))
	{
		int tile_index;
		for (tile_index = 0; tile_index < 128 * 128; tile_index++)
		{
			if (m_videoram[tile_index + (0x00000 / 4)] & 0x200000)
			{
				mark_tile_dirty(0, tile_index);
			}
			if (m_videoram[tile_index + (0x10000 / 4)] & 0x200000)
			{
				mark_tile_dirty(1, tile_index);
			}
			if (m_videoram[tile_index + (0x20000 / 4)] & 0x200000)
			{
				mark_tile_dirty(2, tile_index);
			}
			if (m_videoram[tile_index + (0x30000 / 4)] & 0x200000)
			{
				mark_tile_dirty(3, tile_index);
			}
		}

		m_old_animmask = animmask;
		m_old_animbits = animbits;
	}

	// If any magic bits have been touched, mark every tilemap dirty
	u16 tileflags[4];
	tileflags[0] = m_videoregs[0x02] >> 16;
	tileflags[1] = m_videoregs[0x02] & 0xffff;
	tileflags[2] = m_videoregs[0x03] >> 16;
	tileflags[3] = m_videoregs[0x03] & 0xffff;
	const u16 IMPORTANT_DIRTY_TILEFLAG_MASK = 0x0600;
	for (int i = 0; i < 4; i++)
	{
		if ((m_old_tileflags[i] & IMPORTANT_DIRTY_TILEFLAG_MASK) != (tileflags[i] & IMPORTANT_DIRTY_TILEFLAG_MASK))
		{
			mark_all_tiles_dirty(i);
			m_old_tileflags[i] = tileflags[i];
		}
	}

	// NOTE: buriki 'how to play' might be a sprite vs 3D edge case
	// while the 3D is currently positioned badly due to unhandled 3D packet registers, it should still go behind the shutters, not in front
	// it could also be a unique mixing case

	// tilemaps with 'priority' 0x10 - 0x1f are always behind the 3d?
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int i = 0x1f; i >= 0x10; i--)
		{
			for (int j = 0; j < 4; j++)
			{
				const u16 pri = get_tileregs(j) & 0x1f;

				if (pri == i)
					draw_tilemap(screen, bitmap, cliprect, j, 0, y);
			}

			if ((i & 3) == 0)
				mixsprites_test(screen, bitmap, cliprect, (i/4)<<12, y);
		}
	}

	/*
	   Each framebuffer has enough RAM for 24 bits of data for a 512x256
	   layer (screen is interlaced, so it doesn't really have 448 pixels
	   in height)

	   theory: 11 bit palette index (can use either half of palette)
	            1 bit 'blend'
	            4 bit 'light'
	            8 bit depth?
	*/

	// 3d gets drawn next
	u16 palbase = 0x000;
	if (BIT(m_fbcontrol[2], 5))
	{
		if (!m_roadedge_3d_hack)
			palbase = 0x800;
	}

	rectangle visarea = m_screen->visible_area();
	const int ysize = visarea.max_y - visarea.min_y;

	if (ysize)
	{
		const int yinc = (512 << 16) / ysize;

		pen_t const* const clut_3d = &m_palette_3d->pen(0);
		if (!(m_fbcontrol[0] & 0x01))
		{
			// this moves the car in the xrally selection screen the correct number of pixels to the left
			int xscroll = (m_fbscroll[0] >> 21);
			if (xscroll & 0x400)
				xscroll -= 0x800;

			// value is midpoint?
			xscroll += 256;

			// Blit the color buffer into the primary bitmap
			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				const int realy = (((y-visarea.min_y) * yinc) >> 16);

				const u16 *src = &m_poly_renderer->colorBuffer3d()[((realy) & 0x1ff) * 512];
				u32 *dst = &bitmap.pix(y, cliprect.min_x);

				for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					const u16 srcpix = src[((cliprect.min_x + x) + xscroll) & 0x1ff];
					if (srcpix & 0x07ff)
					{
						// format in our framebuffer is llll appp pppp pppp
						// where l = lighting, a = alpha, p = palindex
						// our m_palette_3d has 16 copies of the palette at different brightness levels, so we can pass directly
						if (srcpix & 0x0800)
						{
							*dst = alpha_blend_r32(*dst, clut_3d[(srcpix & 0xf7ff) | palbase], 0x80);
						}
						else
						{
							*dst = clut_3d[(srcpix & 0xf7ff) | palbase];
						}
					}

					dst++;
				}
			}
		}
	}

	// tilemaps with 'priority' 0x00 - 0x0f are always above the 3d? - could bit 0x10 really be a 'relative to 3d' bit, rather than a 'relative to other tilemaps' bit?
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int i = 0x0f; i >= 0x00; i--)
		{
			for (int j = 0; j < 4; j++)
			{
				const u16 pri = get_tileregs(j) & 0x1f;

				if (pri == i)
					draw_tilemap(screen, bitmap, cliprect, j, 0, y);
			}

			if ((i & 3) == 0)
				mixsprites_test(screen, bitmap, cliprect, (i/4)<<12, y);
		}
	}

#if HNG64_VIDEO_DEBUG
	if (0)
		popmessage("%08x %08x %08x %08x %08x", m_spriteregs[0], m_spriteregs[1], m_spriteregs[2], m_spriteregs[3], m_spriteregs[4]);

	if (1)
		popmessage("Nv(%02x) CZ(%d), DIM(%d) Nv(%02x) Al(%d) GZ(%d)  %04x %04x AE(%d) %04x\n"
				   "TR0(MO(%01x) NL(%d) BPP(%d) TSIZE(%d) W(%d) F(%d) E(%d) L(%d) DEPTH( %d %d %d %d %d (%02x))\n"
				   "TR1(MO(%01x) NL(%d) BPP(%d) TSIZE(%d) W(%d) F(%d) E(%d) L(%d) DEPTH( %d %d %d %d %d (%02x))\n"
				   "TR2(MO(%01x) NL(%d) BPP(%d) TSIZE(%d) W(%d) F(%d) E(%d) L(%d) DEPTH( %d %d %d %d %d (%02x))\n"
				   "TR3(MO(%01x) NL(%d) BPP(%d) TSIZE(%d) W(%d) F(%d) E(%d) L(%d) DEPTH( %d %d %d %d %d (%02x))\n"
				   "tSB(%04x %04x %04x %04x)\n"
				   "%08x %08x %08x\n"
				   "SPLIT?(%04x %04x %04x %04x)\n"
				   "AA(%08x %08x)\n%08x\n",
			// global tilemap control regs?
			(m_videoregs[0x00] & 0xf8000000) >> 24, (m_videoregs[0x00] & 0x04000000) >> 26, (m_videoregs[0x00] & 0x03000000) >> 24, (m_videoregs[0x00] & 0x00f80000) >> 16, (m_videoregs[0x00] & 0x00060000) >> 17, (m_videoregs[0x00] & 0x00010000) >> 16,  m_videoregs[0x00] & 0xffff, (m_videoregs[0x01] & 0xfffe0000) >> 16, (m_videoregs[0x01] & 0x00010000) >> 16, m_videoregs[0x01] & 0xffff,
			// general per-tilemap regs
			(get_tileregs(0) & 0xf000)>>12, (get_tileregs(0) & 0x0800)>>11, (get_tileregs(0) & 0x0400)>>10,(get_tileregs(0) & 0x0200)>>9,(get_tileregs(0) & 0x0100)>>8,(get_tileregs(0) & 0x0080)>>7,(get_tileregs(0) & 0x0040)>>6,(get_tileregs(0) & 0x0020)>>5,(get_tileregs(0) & 0x0010)>>4,(get_tileregs(0) & 0x0008)>>3,(get_tileregs(0) & 0x0004)>>2,(get_tileregs(0) & 0x0002)>>1,(get_tileregs(0) & 0x0001)>>0,(get_tileregs(0) & 0x001f)>>0,
			(get_tileregs(1) & 0xf000)>>12, (get_tileregs(1) & 0x0800)>>11, (get_tileregs(1) & 0x0400)>>10,(get_tileregs(1) & 0x0200)>>9,(get_tileregs(1) & 0x0100)>>8,(get_tileregs(1) & 0x0080)>>7,(get_tileregs(1) & 0x0040)>>6,(get_tileregs(1) & 0x0020)>>5,(get_tileregs(1) & 0x0010)>>4,(get_tileregs(1) & 0x0008)>>3,(get_tileregs(1) & 0x0004)>>2,(get_tileregs(1) & 0x0002)>>1,(get_tileregs(1) & 0x0001)>>0,(get_tileregs(1) & 0x001f)>>0,
			(get_tileregs(2) & 0xf000)>>12, (get_tileregs(2) & 0x0800)>>11, (get_tileregs(2) & 0x0400)>>10,(get_tileregs(2) & 0x0200)>>9,(get_tileregs(2) & 0x0100)>>8,(get_tileregs(2) & 0x0080)>>7,(get_tileregs(2) & 0x0040)>>6,(get_tileregs(2) & 0x0020)>>5,(get_tileregs(2) & 0x0010)>>4,(get_tileregs(2) & 0x0008)>>3,(get_tileregs(2) & 0x0004)>>2,(get_tileregs(2) & 0x0002)>>1,(get_tileregs(2) & 0x0001)>>0,(get_tileregs(2) & 0x001f)>>0,
			(get_tileregs(3) & 0xf000)>>12, (get_tileregs(3) & 0x0800)>>11, (get_tileregs(3) & 0x0400)>>10,(get_tileregs(3) & 0x0200)>>9,(get_tileregs(3) & 0x0100)>>8,(get_tileregs(3) & 0x0080)>>7,(get_tileregs(3) & 0x0040)>>6,(get_tileregs(3) & 0x0020)>>5,(get_tileregs(3) & 0x0010)>>4,(get_tileregs(3) & 0x0008)>>3,(get_tileregs(3) & 0x0004)>>2,(get_tileregs(3) & 0x0002)>>1,(get_tileregs(3) & 0x0001)>>0,(get_tileregs(3) & 0x001f)>>0,
			// scrollbase regs
			(m_videoregs[0x04] >> 16) & 0xffff, (m_videoregs[0x04] >> 0) & 0xffff, (m_videoregs[0x05] >> 16) & 0xffff, (m_videoregs[0x05] >> 0) & 0xffff,
			// initialized to fixed values?
			m_videoregs[0x06], m_videoregs[0x07], m_videoregs[0x08],
			// used when a single tilemap gets 'split' on Buriki player entrances?
			(m_videoregs[0x09] >> 16) & 0xffff, (m_videoregs[0x09] >> 0) & 0xffff, (m_videoregs[0x0a] >> 16) & 0xffff, (m_videoregs[0x0a] >> 0) & 0xffff,
			// Auto Animation registers
			m_videoregs[0x0b], m_videoregs[0x0c],
			// unused?
			m_videoregs[0x0d]
		);

	// Individual tilemap regs format
	// ------------------------------
	// mmmm dbrW FELz zzzz
	// m = Tilemap mosaic level [0-15] - confirmed in sams64 demo mode
	//  -- they seem to enable mosaic at the same time as rowscroll in several cases (floor in buriki / ff)
	//     and also on the rotating logo in buriki.. does it cause some kind of aliasing side-effect, or.. ?
	// d = line (floor) mode - buriki, fatafurwa, some backgrounds in ss64_2
	// b = 4bpp/8bpp (seems correct) (beast busters, samsh64, sasm64 2, xrally switch it for some screens)
	// r = tile size (seems correct)
	// W = allow wraparound? (not set on Fatal Fury title logo, or rotating 'name entry' in Roads Edge, both confirmed to need wraparound disabled)
	// F = allow fade1 or fade2 to apply? (complete guess!)
	// E = tilemap enable bit according to sams64_2
	// L = related to output layer? seems to be tied to which mixing bits are used to enable blending
	// z = z depth/priority? tilemaps might also be affected by min / max clip values somewhere?
	//              (debug layer on buriki has priority 0x020, which would be highest)



	if (0)
		popmessage("TC: %08x MINX(%d) MINY(%d) MAXX(%d) MAXY(%d)\n"
				   "MIX BITSA Nv(%d%d%d%d) P:%d B:%d Nv(%d) Al(%d) : Nv(%d) p:%d Nv(%d%d%d) b:%d Nv(%d%d)  : Nv(%d) (%d) (%d %d %d) (%d %d %d)\n"
				   "MIX BITSB Nv(%d%d%d%d) P:%d B:%d Nv(%d) Al(%d) : Nv(%d) p:%d Nv(%d%d%d) b:%d Nv(%d%d)  : Nv(%d) (%d) (%d %d %d) (%d %d %d)\n"
				   "UNUSED?(%04x)\n%04x\nUNUSED?(%d %d)\n"
				   "For FADE1 or 1st in PALFADES group per-RGB blend modes(%d %d %d)\n"
				   "For FADE2 or 2nd in PALFADES group per-RGB blend modes(%d %d %d)\n"
				   "MASTER FADES - FADE1?(%08x)\n"
				   "MASTER FADES - FADE2?(%08x)\n"
				   "UNUSED?(%08x)\n"
				   "UNUSED?&0xfffc(%04x) DISABLE_DISPLAY(%d) ALSO USE REGS BELOW FOR MASTER FADE(%d)\n"
				   "PALEFFECT_ENABLES(%d %d %d %d %d %d %d %d)\n PALFADES?(%08x %08x : %08x %08x : %08x %08x : %08x %08x)\n"
				   "%08x SPRITE_BLEND_TYPE?(%08x) : %08x %08x %08x %08x",
			m_tcram[0x00 / 4], // 0007 00e4 (fatfurwa, bbust2)
			(m_tcram[0x04 / 4] >> 16) & 0xffff, (m_tcram[0x04 / 4] >> 0) & 0xffff, // 0000 0010 (fatfurwa) 0000 0000 (bbust2, xrally)
			(m_tcram[0x08 / 4] >> 16) & 0xffff, (m_tcram[0x08 / 4] >> 0) & 0xffff, // 0200 01b0 (fatfurwa) 0200 01c0 (bbust2, xrally)

			// is this 2 groups of 3 regs?
			//(m_tcram[0x0c / 4] >> 24) & 0xff, // 04 = 'blend' on tm1
			//(m_tcram[0x0c / 4] >> 16) & 0xff, // 04 = set when fades are going on with blended sprites in buriki intro? otherwise usually 00
			//(m_tcram[0x0c / 4] >> 8) & 0xff,  // upper bit not used? value usually 2x, 4x, 5x or 6x
			(m_tcram[0x0c / 4] >> 31) & 0x1,
			(m_tcram[0x0c / 4] >> 30) & 0x1,
			(m_tcram[0x0c / 4] >> 29) & 0x1,
			(m_tcram[0x0c / 4] >> 28) & 0x1,
			(m_tcram[0x0c / 4] >> 27) & 0x1,
			(m_tcram[0x0c / 4] >> 26) & 0x1, // set for blends in on tm1 (pink bits on xrally etc.)  (U 1 1) E(1) L(0) DEPTH (0 1 0 0 1)   in sams64 intro it expects it on tm2, but it gets applied to tm1 TM2 = U(1 1) E(1) L(0) DEPTH(1 0 1 0 1)

			(m_tcram[0x0c / 4] >> 25) & 0x1,
			(m_tcram[0x0c / 4] >> 24) & 0x1, // always set

			(m_tcram[0x0c / 4] >> 23) & 0x1,
			(m_tcram[0x0c / 4] >> 22) & 0x1, // set on POST in xrally etc.
			(m_tcram[0x0c / 4] >> 21) & 0x1,
			(m_tcram[0x0c / 4] >> 20) & 0x1,
			(m_tcram[0x0c / 4] >> 19) & 0x1,
			(m_tcram[0x0c / 4] >> 18) & 0x1, // set on some fades in buriki attract intro (related to the sprites being blended during the fade?)
			(m_tcram[0x0c / 4] >> 17) & 0x1,
			(m_tcram[0x0c / 4] >> 16) & 0x1,

			(m_tcram[0x0c / 4] >> 15) & 0x1,
			(m_tcram[0x0c / 4] >> 14) & 0x1,
			(m_tcram[0x0c / 4] >> 13) & 0x1,
			(m_tcram[0x0c / 4] >> 12) & 0x1,
			(m_tcram[0x0c / 4] >> 11) & 0x1,
			(m_tcram[0x0c / 4] >> 10) & 0x1,
			(m_tcram[0x0c / 4] >> 9) & 0x1,
			(m_tcram[0x0c / 4] >> 8) & 0x1,

			// 2nd group?
			//(m_tcram[0x0c / 4] >> 0) & 0xff, //  04 = 'blend' on tm3  (used in transitions?)
			//(m_tcram[0x10 / 4] >> 24) & 0xff, // usually (always?) 00
			//(m_tcram[0x10 / 4] >> 16) & 0xff, // upper bit not used? value usually 2x, 4x, 5x or 6x
			(m_tcram[0x0c / 4] >> 7) & 0x1,
			(m_tcram[0x0c / 4] >> 6) & 0x1,
			(m_tcram[0x0c / 4] >> 5) & 0x1,
			(m_tcram[0x0c / 4] >> 4) & 0x1,
			(m_tcram[0x0c / 4] >> 3) & 0x1, // set in POST on xrally etc.
			(m_tcram[0x0c / 4] >> 2) & 0x1, // set on buriki when blends are used. tm0 blends on fatfurwa frontmost layer U(0 1) E(1) L(1) DEPTH ( 0 0 0 0 0 )   set on SS64 Portrait Win blend TM1 (U1 1 ) E(1) L(1) DEPT( 0 0 1 0 1) (other blend enabled here too!)

			(m_tcram[0x0c / 4] >> 1) & 0x1,
			(m_tcram[0x0c / 4] >> 0) & 0x1, // always set

			(m_tcram[0x10 / 4] >> 31) & 0x1,
			(m_tcram[0x10 / 4] >> 30) & 0x1,
			(m_tcram[0x10 / 4] >> 29) & 0x1,
			(m_tcram[0x10 / 4] >> 28) & 0x1,
			(m_tcram[0x10 / 4] >> 27) & 0x1,
			(m_tcram[0x10 / 4] >> 26) & 0x1,
			(m_tcram[0x10 / 4] >> 25) & 0x1,
			(m_tcram[0x10 / 4] >> 24) & 0x1,

			(m_tcram[0x10 / 4] >> 23) & 0x1,
			(m_tcram[0x10 / 4] >> 22) & 0x1,
			(m_tcram[0x10 / 4] >> 21) & 0x1,
			(m_tcram[0x10 / 4] >> 20) & 0x1,
			(m_tcram[0x10 / 4] >> 19) & 0x1,
			(m_tcram[0x10 / 4] >> 18) & 0x1,
			(m_tcram[0x10 / 4] >> 17) & 0x1,
			(m_tcram[0x10 / 4] >> 16) & 0x1,

			m_tcram[0x10 / 4] & 0xffff, // unused?

			// also seems fade mode related?
			(m_tcram[0x14 / 4] >> 16) & 0xffff,  // typically 0007 or 0001, - 0011 on ss64 ingame, 0009 on continue screen

			// 0xxx ?  (often 0555 or 0fff, seen 56a, 57f too) -  register split into 2 bits - typically a bit will be 3 or 1 depending if the effect is additive / subtractive
			// usually relate to the RGB pairings at m_tcram[0x18 / 4] & m_tcram[0x1c / 4] but m_tcram[0x24 / 4] & 1 may cause it to use the registers at m_tcram[0x28 / 4] instead?
			(m_tcram[0x14 / 4] >> 14) & 0x3, (m_tcram[0x14 / 4] >> 12) & 0x3, // unused?
			(m_tcram[0x14 / 4] >> 10) & 0x3, (m_tcram[0x14 / 4] >> 8) & 0x3, (m_tcram[0x14 / 4] >> 6) & 0x3, // for 'fade1' or first register in group of 8?
			(m_tcram[0x14 / 4] >> 4) & 0x3, (m_tcram[0x14 / 4] >> 2) & 0x3, (m_tcram[0x14 / 4] >> 0) & 0x3, // for 'fade2' or 2nd register in group of 8?

			// these are used for 'fade to black' in most cases, but
			// in xrally attract, when one image is meant to fade into another, one value increases while the other decreases
			m_tcram[0x18 / 4],  // xRGB fade values? (roadedge attract)
			m_tcram[0x1c / 4],  // xRGB fade values? (roadedge attract) fades on fatfurwa before buildings in intro

			m_tcram[0x20 / 4], // unused?

			(m_tcram[0x24 / 4] >> 16) & 0xfffc,
			((m_tcram[0x24 / 4] >> 16) & 0x0002)>>1, // 0002 gets set in roadedge during some transitions (layers are disabled? blacked out?) 0001
			(m_tcram[0x24 / 4] >> 16) & 0x0001, // 0001 may indicate if to use the 8 below for standard fade, set on SNK logo in roadedge, in FFWA

			// some kind of bitfields, these appear related to fade mode for the registers at 0x28 / 4, set to either 3 or 2 which is additive or subtractive
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

void hng64_state::screen_vblank(int state)
{
	// rising edge and buffer swap
	if (state && BIT(m_tcram[0x50 / 4], 16))
		clear3d();
}


/* Transition Control Video Registers  **OUTDATED, see notes with popmessage**
 * ----------------------------------
 *
 * u32    | Bits                                    | Use
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



inline void hng64_state::set_palette_entry_with_faderegs(int entry, u8 r, u8 g, u8 b, u32 rgbfade, u8 r_mode, u8 g_mode, u8 b_mode, palette_device *palette)
{
	const int r_fadeval = (rgbfade >> 16) & 0xff;
	const int g_fadeval = (rgbfade >> 8) & 0xff;
	const int b_fadeval = (rgbfade >> 0) & 0xff;

	int r_new = r;
	int g_new = g;
	int b_new = b;

	switch (r_mode)
	{
	case 0x00:
	case 0x02:
		break;

	case 0x01: // additive
		r_new = r_new + r_fadeval;
		if (r_new > 255)
			r_new = 255;
		break;

	case 0x03: // subtractive
		r_new = r_new - r_fadeval;
		if (r_new < 0)
			r_new = 0;
		break;
	}

	switch (g_mode)
	{
	case 0x00:
	case 0x02:
		break;

	case 0x01: // additive
		g_new = g_new + g_fadeval;
		if (g_new > 255)
			g_new = 255;
		break;

	case 0x03: // subtractive
		g_new = g_new - g_fadeval;
		if (g_new < 0)
			g_new = 0;
		break;
	}

	switch (b_mode)
	{
	case 0x00:
	case 0x02:
		break;

	case 0x01: // additive
		b_new = b_new + b_fadeval;
		if (b_new > 255)
			b_new = 255;
		break;

	case 0x03: // subtractive
		b_new = b_new - b_fadeval;
		if (b_new < 0)
			b_new = 0;
		break;
	}

	palette->set_pen_color(entry, r_new, g_new, b_new);
}

inline void hng64_state::set_single_palette_entry(int entry, u8 r, u8 g, u8 b)
{
	m_palette->set_pen_color(entry, r, g, b);

	set_palette_entry_with_faderegs(entry, r, g, b, m_tcram[0x18 / 4], (m_tcram[0x14 / 4] >> 10) & 0x3, (m_tcram[0x14 / 4] >> 8) & 0x3, (m_tcram[0x14 / 4] >> 6) & 0x3, m_palette_fade0);
	set_palette_entry_with_faderegs(entry, r, g, b, m_tcram[0x1c / 4], (m_tcram[0x14 / 4] >> 4) & 0x3, (m_tcram[0x14 / 4] >> 2) & 0x3, (m_tcram[0x14 / 4] >> 0) & 0x3, m_palette_fade1);

	// our code assumes the 'lighting' values from the 3D framebuffer can be 4-bit precision
	// based on 'banding' seen in buriki reference videos.  precalculate those here to avoid
	// having to do it in the video update function
	//
	// note, it is unlikely intensity is meant to be added, probably instead it should
	// be multipled, reducing overall brightness, not increasing beyond max?
	for (int intensity = 0; intensity < 0x10; intensity++)
	{
		u16 newr = r + (intensity << 2);
		u16 newg = g + (intensity << 2);
		u16 newb = b + (intensity << 2);

		if (newr > 255)
			newr = 255;
		if (newg > 255)
			newg = 255;
		if (newb > 255)
			newb = 255;

		//m_palette_3d->set_pen_color((intensity * 0x1000) + entry, newr, newg, newb);
		// always use 1nd fade register for 3D (very unlikely!) (allows 3d to flash correctly in sams64 how to play, but applies wrong fade register on 3d in buriki discipline intro screens, assuming those are meant to be blacked out this way)
		set_palette_entry_with_faderegs((intensity * 0x1000) + entry, newr, newg, newb, m_tcram[0x18 / 4], (m_tcram[0x14 / 4] >> 10) & 0x3, (m_tcram[0x14 / 4] >> 8) & 0x3, (m_tcram[0x14 / 4] >> 6) & 0x3, m_palette_3d);
		// always use 2nd fade register for 3D (very unlikely!)
		//set_palette_entry_with_faderegs((intensity * 0x1000) + entry, newr, newg, newb, m_tcram[0x1c / 4], (m_tcram[0x14 / 4] >> 4) & 0x3, (m_tcram[0x14 / 4] >> 2) & 0x3, (m_tcram[0x14 / 4] >> 0) & 0x3, m_palette_3d);
	}
}

void hng64_state::update_palette_entry(int entry)
{
	const u32 data = m_paletteram[entry];

	s16 r = (data >> 16) & 0xff;
	s16 g = (data >> 8) & 0xff;
	s16 b = (data >> 0) & 0xff;

	for (int i = 0; i < 8; i++)
	{
		const u32 tcdata = m_tcram[(0x28 / 4) + i];

		// correct for buriki (jumbotron fading images) and xrally / roadedge (palette effect on front of car in first person view)
		//
		// however this logic fails on fatal fury fades, as the msb is 00, indicating apply to palette entries 000-0ff but the actual
		// palette needing the changes is 0x900 - 0x9ff (hng64 logo white flash) so there must be further ways in which this is
		// configured ** NO, see below!
		//
		// ** this is instead meant to apply to the tm0 in fatfurwa, basically a full screen blended layer so IS being applied to the
		// correct palettes, it's currently a blend / priority issue causing it to not show
		const u8 tcregion = (tcdata & 0x0f000000) >> 24;

		if (tcregion == (entry >> 8))
		{
			const u8 bmod = (tcdata >> 16) & 0xff;
			const u8 gmod = (tcdata >> 8) & 0xff;
			const u8 rmod = (tcdata >> 0) & 0xff;

			const u8 tcreg = (m_tcram[0x24 / 4] >> (i << 1)) & 3;
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

	set_single_palette_entry(entry, r, g, b);
}

void hng64_state::pal_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette_entry(offset);
}

// Transition Control memory.
void hng64_state::tcram_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old_data = m_tcram[offset];
	COMBINE_DATA(&m_tcram[offset]);

	if (offset == 0x02)
	{
		rectangle visarea = m_screen->visible_area();

		const u16 min_x = (m_tcram[1] & 0xffff0000) >> 16;
		const u16 min_y = (m_tcram[1] & 0x0000ffff) >> 0;
		const u16 max_x = (m_tcram[2] & 0xffff0000) >> 16;
		const u16 max_y = (m_tcram[2] & 0x0000ffff) >> 0;

		if (max_x == 0 || max_y == 0) // bail out if values are invalid, Fatal Fury WA sets this to disable the screen.
		{
			m_screen_dis = 1;
			return;
		}

		m_screen_dis = 0;

		visarea.set(min_x, min_x + max_x - 1, min_y, min_y + max_y - 1);

		// TODO: properly calculate this from screen params
		attoseconds_t period;
		if (max_y == 448)
			period = HZ_TO_ATTOSECONDS(59.430077); // everything apart from fatfurwa uses a 512x448 resolution, and 59.43hz appears to sync with hardware
		else
			period = HZ_TO_ATTOSECONDS(61.651673); // fatfurwa uses 512x432, sync frequency not verified

		m_screen->configure(HTOTAL, VTOTAL, visarea, period);
	}

	if ((offset >= (0x28 / 4)) && (offset < (0x48 / 4)))
	{
		const u32 new_data = m_tcram[offset];
		if (old_data != new_data)
		{
			// the old blend value no longer applies, so update the colours referenced by that
			u8 old_tcregion = (old_data & 0x0f000000) >> 24;
			for (int i = 0; i < 0x100; i++)
			{
				update_palette_entry((old_tcregion * 0x100) + i);
			}
			// update the colours referenced by the new blend register
			u8 new_tcregion = (new_data & 0x0f000000) >> 24;
			for (int i = 0; i < 0x100; i++)
			{
				update_palette_entry((new_tcregion * 0x100) + i);
			}
		}
	}

	if ((offset == (0x24 / 4)) || (offset == (0x14 / 4)) || (offset == (0x18 / 4)) || (offset == (0x1c / 4)))
	{
		// lazy, just update the lot
		const u32 new_data = m_tcram[offset];
		if (old_data != new_data)
		{
			for (int i = 0; i < 0x1000; i++)
			{
				update_palette_entry(i);
			}
		}
	}
}

u32 hng64_state::tcram_r(offs_t offset)
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

	m_tilemap[0].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<0, 0>))), TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[0].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<0, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[0].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<0, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	m_tilemap[1].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<1, 0>))), TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[1].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<1, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[1].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<1, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	m_tilemap[2].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<2, 0>))), TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[2].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<2, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[2].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<2, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	m_tilemap[3].m_tilemap_8x8       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<3, 0>))), TILEMAP_SCAN_ROWS,  8,  8, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[3].m_tilemap_16x16     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<3, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 128, 128); // 128x128x4 = 0x10000
	m_tilemap[3].m_tilemap_16x16_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&hng64_state::get_tile_info<3, 1>))), TILEMAP_SCAN_ROWS, 16, 16, 256,  64); // 128x128x4 = 0x10000

	for (auto &elem : m_tilemap)
	{
		elem.m_tilemap_8x8->set_transparent_pen(0);
		elem.m_tilemap_16x16->set_transparent_pen(0);
		elem.m_tilemap_16x16_alt->set_transparent_pen(0);
	}

	// Rasterizer creation
	m_poly_renderer = std::make_unique<hng64_poly_renderer>(*this);

	// 3d information
	m_dl = std::make_unique<u16[]>(0x100);
	m_polys.resize(HNG64_MAX_POLYGONS);

	m_screen->register_screen_bitmap(m_sprite_bitmap);
	m_screen->register_screen_bitmap(m_sprite_zbuffer);
}


#include "hng64_3d.ipp"
#include "hng64_sprite.ipp"
