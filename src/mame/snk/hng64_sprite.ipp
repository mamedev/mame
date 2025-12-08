// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner

/* Hyper NeoGeo 64 Sprite bits */

/*
 * Sprite Format
 * ------------------
 *
 * offset | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0  0 | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | x/y position
 *   1  4 | YYYY YYYY YYYY YYYY XXXX XXXX XXXX XXXX | x/y zoom (*)
 *   2  8 | ---- Szzz zzzz zzzz ---- ---I cccc CCCC | S = set on CPU car markers above cars (in roadedge) z = Z-buffer value, i = 'Inline' chain flag, cC = x/y chain
 *   3  c | ---- ---- pppp pppp ---- ---- ---- ---- | palette entry
 *   4 10 | mmmm -cfF aggg tttt tttt tttt tttt tttt | mosaic factor, unknown (x1), checkerboard, flip bits, blend, group?, tile number
 *   5 14 | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   6 18 | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   7 1c | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *
 *  in (4) ggg seems to be either group, or priority against OTHER layers (7 being the lowest, 0 being the highest in normal situations eg most of the time in buriki)
 *
 * (*) Fatal Fury WA standard elements are 0x1000-0x1000, all the other games sets 0x100-0x100, related to the bit 27 of sprite regs 0?
 ** Sprite Global Registers
 * -----------------------
 *
 * offset | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ssss z--f b--- -aap ---- ---- ---- ---- | s = unknown, samsho  z = zooming mode, f = priority sort mode (unset set in roadedge ingame) b = bpp select a = always, p = post, disable?
 *   1    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | global sprite offset (ss64 rankings in attract, roadedge HUD scroll when starting game)
 *   2    | ---- ---- ---- uuuu uuuu uuuu uuuu uuuu | u = unknown, set to 0x000fffff in roadedge ingame, bbust2, samsho - also possible depthfilter related
 *   3    | ---- ---- ---- ---- ---- ---- ---- ---- |
 *   4    | ---- ---- ---- ---- ---- ---- ---- ---- |
 * (anything else is unknown at the current time)
 * Notes:
 * [4]
 * 0x0e0 in Samurai Shodown/Xrally games, 0x1c0 in all the others, zooming factor?

 */


#define PIXEL_OP_REBASE_TRANSPEN(DEST, DESTZ, SOURCE) \
do \
{ \
	const u32 srcdata = (SOURCE); \
	if (xdrawpos <= cliprect.right() && xdrawpos >= cliprect.left()) \
	{ \
		if (zval < (DESTZ)) \
		{ \
			if (srcdata != trans_pen) \
			{ \
				(DESTZ) = zval; \
				(DEST) = color + srcdata; \
			} \
		} \
	} \
} \
while (0)


#define PIXEL_OP_REBASE_TRANSPEN_REV(DEST, DESTZ, SOURCE) \
do \
{ \
	const u32 srcdata = (SOURCE); \
	if (xdrawpos <= cliprect.right() && xdrawpos >= cliprect.left()) \
	{ \
		if (zval >= (DESTZ)) \
		{ \
			if (srcdata != trans_pen) \
			{ \
				(DESTZ) = zval; \
				(DEST) = color + srcdata; \
			} \
		} \
	} \
} \
while (0)

#define PIX_CHECKERBOARD \
do \
{ \
	if (!mosaic) \
		srcpix = srcptr[(cursrcx >> 16) & 0xf]; \
	else \
	{ \
		if (mosaic_count_x == 0) \
		{ \
			srcpix = srcptr[(cursrcx >> 16) & 0xf]; \
			mosaic_count_x = mosaic; \
		} \
		else \
		{ \
			mosaic_count_x--; \
		} \
	} \
	\
	if (checkerboard) \
	{ \
		if (curx & 1) \
		{ \
			if (!(ypos & 1)) \
				srcpix = 0; \
		} \
		else \
		{ \
			if ((ypos & 1)) \
				srcpix = 0; \
		} \
	} \
} \
while (0)

inline void hng64_state::drawline(bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect,
	gfx_element * gfx, u32 code, u32 color, int flipy, s32 xpos,
	s32 dx, s32 dy, u32 trans_pen, u32 zval, bool zrev, bool blend, bool checkerboard, u8 mosaic, u8 &mosaic_count_x, s32 ypos, const u8 *srcdata, s32 srcx, u32 leftovers, int curyy, u16 &srcpix)
{
	auto* destptr = &dest.pix(ypos, 0);
	auto* destzptr = &destz.pix(ypos, 0);

	const u8* srcptr = srcdata + (flipy ? ((15-curyy) & 0xf) : (curyy & 0xf)) * gfx->rowbytes();
	s32 cursrcx = srcx;

	if (zrev)
	{
		// iterate over leftover pixels
		for (s32 curx = 0; curx < leftovers; curx++)
		{
			int xdrawpos = xpos + curx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN_REV(destptr[xdrawpos], destzptr[xdrawpos], srcpix);
			cursrcx += dx;
		}
	}
	else
	{
		// iterate over leftover pixels
		for (s32 curx = 0; curx < leftovers; curx++)
		{
			int xdrawpos = xpos + curx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN(destptr[xdrawpos], destzptr[xdrawpos], srcpix);
			cursrcx += dx;
		}
	}
}

inline void hng64_state::zoom_transpen(bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect,
		gfx_element *gfx, u32 code, u32 color, int flipx, int flipy, s32 xpos, s32 ypos,
		s32 dx, s32 dy, u32 dstwidth, u32 trans_pen, u32 zval, bool zrev, bool blend, u16 group, bool checkerboard, u8 mosaic, u8 &mosaic_count_x, int curyy, u16 &srcpix)
{
	// use pen usage to optimize
	code %= gfx->elements();
	if (gfx->has_pen_usage())
	{
		// fully transparent; do nothing
		const u32 usage = gfx->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}

	// render
	color = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());

	if (blend)
		color |= 0x8000;

	color |= group;

	assert(dest.valid());
	assert(dest.cliprect().contains(cliprect));

	// ignore empty/invalid cliprects
	if (cliprect.empty())
		return;

	if (dstwidth < 1)
		return;

	s32 srcx = 0;
	// apply X flipping
	if (flipx)
	{
		srcx = (dstwidth - 1) * dx - srcx;
		dx = -dx;
	}

	// fetch the source data
	const u8 *srcdata = gfx->get_data(code);

	s32 destendx = xpos + dstwidth - 1;
	u32 leftovers = (destendx + 1 - xpos);

	drawline(dest, destz, cliprect,
		gfx, code, color, flipy, xpos,
		dx, dy, trans_pen, zval, zrev, blend, checkerboard, mosaic, mosaic_count_x, ypos, srcdata, srcx, leftovers, curyy, srcpix);

}

inline void hng64_state::get_tile_details(bool chain, u16 spritenum, u8 xtile, u8 ytile, u8 xsize, u8 ysize, bool xflip, bool yflip, u32& tileno, u16& pal, u8 &gfxregion)
{
	int offset;
	if (!xflip)
	{
		if (!yflip)
			offset = (xtile + (ytile * (xsize + 1)));
		else
			offset = (xtile + ((ysize - ytile) * (xsize + 1)));
	}
	else
	{
		if (!yflip)
			offset = ((xsize - xtile) + (ytile * (xsize + 1)));
		else
			offset = ((xsize - xtile) + ((ysize - ytile) * (xsize + 1)));
	}

	if (!chain)
	{
		tileno = (m_spriteram[(spritenum * 8) + 4] & 0x0007ffff);
		pal = (m_spriteram[(spritenum * 8) + 3] & 0x00ff0000) >> 16;

		if (m_spriteregs[0] & 0x00800000) //bpp switch
		{
			gfxregion = 4;
		}
		else
		{
			gfxregion = 5;
			tileno >>= 1;
			pal &= 0xf;
		}

		tileno += offset;
	}
	else
	{
		tileno = (m_spriteram[((spritenum + offset) * 8) + 4] & 0x0007ffff);
		pal = (m_spriteram[((spritenum + offset) * 8) + 3] & 0x00ff0000) >> 16;
		if (m_spriteregs[0] & 0x00800000) //bpp switch
		{
			gfxregion = 4;
		}
		else
		{
			gfxregion = 5;
			tileno >>= 1;
			pal &= 0xf;
		}
	}
}

void hng64_state::draw_sprites_buffer(screen_device& screen, const rectangle& cliprect)
{
	m_sprite_bitmap.fill(0x0000, cliprect);

	// global offsets in sprite regs
	const int spriteoffsx = (m_spriteregs[1] >> 0) & 0xffff;
	const int spriteoffsy = (m_spriteregs[1] >> 16) & 0xffff;

	// This flips between ingame and other screens for roadedge, where the sprites which are filtered definitely needs to change and the game explicitly swaps the values in the sprite list at the same time.
	// m_spriteregs[2] could also play a part as it also flips between 0x00000000 and 0x000fffff at the same time
	// Samsho games also set the upper 3 bits which could be related, samsho games still have some unwanted sprites (but also use the other 'sprite clear' mechanism)
	// Could also be draw order related, check if it inverts the z value?
	const bool zsort = !(m_spriteregs[0] & 0x01000000);

	if (zsort)
		m_sprite_zbuffer.fill(0x0000, cliprect);
	else
		m_sprite_zbuffer.fill(0x07ff, cliprect);

	int nextsprite = 0;
	int currentsprite = 0;

	while (currentsprite < ((0xc000 / 4) / 8))
	{
		const u16 zval = (m_spriteram[(currentsprite * 8) + 2] & 0x07ff0000) >> 16;

		s16 ypos = (m_spriteram[(currentsprite * 8) + 0] & 0xffff0000) >> 16;
		s16 xpos = (m_spriteram[(currentsprite * 8) + 0] & 0x0000ffff) >> 0;

		// should the offsets also be sign extended?
		xpos += (spriteoffsx);
		ypos += (spriteoffsy);

		// sams64_2 wants bit 0x200 to be the sign bit on character select screen
		xpos = util::sext(xpos, 10);
		ypos = util::sext(ypos, 10);

		const bool blend = (m_spriteram[(currentsprite * 8) + 4] & 0x00800000);
		const u16 group = (m_spriteram[(currentsprite * 8) + 4] & 0x00700000) >> 8;
		const bool checkerboard = (m_spriteram[(currentsprite * 8) + 4] & 0x04000000);
		const u8 mosaic = (m_spriteram[(currentsprite * 8) + 4] & 0xf0000000) >> 28;

		const int yflip = (m_spriteram[(currentsprite * 8) + 4] & 0x01000000) >> 24;
		const int xflip = (m_spriteram[(currentsprite * 8) + 4] & 0x02000000) >> 25;

		const int chainy = (m_spriteram[(currentsprite * 8) + 2] & 0x0000000f);
		const int chainx = (m_spriteram[(currentsprite * 8) + 2] & 0x000000f0) >> 4;
		const int chaini = (m_spriteram[(currentsprite * 8) + 2] & 0x00000100);

		if (!chaini)
		{
			nextsprite = currentsprite + 1;
		}
		else
		{
			nextsprite = currentsprite + ((chainx + 1) * (chainy + 1));
		}

		const u32 zoomy = (m_spriteram[(currentsprite * 8) + 1] & 0xffff0000) >> 16;
		const u32 zoomx = (m_spriteram[(currentsprite * 8) + 1] & 0x0000ffff) >> 0;

		/* Calculate the zoom */
		int zoom_factor = (m_spriteregs[0] & 0x08000000) ? 0x1000 : 0x100;

		/* Sprites after 'Fair and Square' have a zoom of 0 in sams64 for one frame, they shouldn't be seen? */
		if (!zoomx || !zoomy)
		{
			currentsprite = nextsprite;
			continue;
		};

		// skip the lowest sprite priority depending on the zsort mode
		// it was previously assumed the default buffer fill would take care of this
		// but unless there's a sign bit, the roadedge name entry screen disagrees as it
		// requires a >= check on the sprite draw, not >
		//
		// for the non-zsort/zrev case, fatfurywa char selectappears requires <, not <=
		if (zsort && (zval == 0))
		{
			currentsprite = nextsprite;
			continue;
		}

		s32 dx, dy;

		if (zoom_factor == 0x100)
		{
			dx = zoomx << 8;
			dy = zoomy << 8;
		}
		else
		{
			dx = zoomx << 4;
			dy = zoomy << 4;
		}


		u32 full_srcpix_y = 0;
		u32 full_dstheight = 0;
		do
		{
			full_srcpix_y += dy;
			full_dstheight++;
		} while (full_srcpix_y < ((chainy+1) * 0x100000));

		int realline = 0;
		int mosaic_y_counter = 0;
		for (s32 curyy = 0; curyy <= full_dstheight - 1; curyy++)
		{
			if (!mosaic)
			{
				realline = curyy;
			}
			else
			{
				if (mosaic_y_counter == 0)
				{
					realline = curyy;
					mosaic_y_counter = mosaic;
				}
				else
				{
					mosaic_y_counter--;
				}
			}

			if (ypos <= cliprect.bottom() && ypos >= cliprect.top())
			{
				const u32 full_srcpix_y2 = realline * dy;
				const int used_ysource_pos = full_srcpix_y2 >> 16;
				const int ytilebbb = used_ysource_pos / 0x10;
				const int use_tile_line = used_ysource_pos & 0xf;
				draw_sprite_line(screen, cliprect, use_tile_line, ypos, xpos, chainx, dx, dy, ytilebbb, chaini, currentsprite, chainy, xflip, yflip, zval, zsort, blend, group, checkerboard, mosaic);
			}
			ypos++;
		}

		currentsprite = nextsprite;
	}
}


inline void hng64_state::draw_sprite_line(screen_device& screen, const rectangle& cliprect, s32 curyy, s16 ypos, s16 xpos, int chainx, s32 dx, s32 dy, int ytileblock, int chaini, int currentsprite, int chainy, int xflip, int yflip, u16 zval, bool zsort, bool blend, u16 group, bool checkerboard, u8 mosaic)
{
	u32 srcpix_x = 0;
	u16 srcpix = 0;

	u8 mosaic_count_x = 0;

	for (int xdrw = 0; xdrw <= chainx; xdrw++)
	{
		u32 dstwidth = 0;
		do
		{
			srcpix_x += dx;
			dstwidth++;
		} while (srcpix_x < 0x100000);
		srcpix_x &= 0x0fffff;

		u32 tileno;
		u16 pal;
		u8 gfxregion;

		get_tile_details(chaini, currentsprite, xdrw, ytileblock, chainx, chainy, xflip, yflip, tileno, pal, gfxregion);
		zoom_transpen(m_sprite_bitmap, m_sprite_zbuffer, cliprect, m_gfxdecode->gfx(gfxregion), tileno, pal, xflip, yflip, xpos, ypos, dx, dy, dstwidth, 0, zval, zsort, blend, group, checkerboard, mosaic, mosaic_count_x, curyy, srcpix);
		xpos += dstwidth;
	}

}
