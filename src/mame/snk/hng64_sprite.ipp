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
 *   0    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | x/y position
 *   1    | YYYY YYYY YYYY YYYY XXXX XXXX XXXX XXXX | x/y zoom (*)
 *   2    | ---- Szzz zzzz zzzz ---- ---I cccc CCCC | S = set on CPU car markers above cars (in roadedge) z = Z-buffer value, i = 'Inline' chain flag, cC = x/y chain
 *   3    | ---- ---- pppp pppp ---- ---- ---- ---- | palette entry
 *   4    | mmmm -cfF aggg tttt tttt tttt tttt tttt | mosaic factor, unknown (x1), checkerboard, flip bits, blend, group?, tile number
 *   5    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   6    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   7    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
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
	if (zval < (DESTZ)) \
	{ \
		uint32_t srcdata = (SOURCE); \
		if (srcdata != trans_pen) \
		{ \
			if (cury > cliprect.bottom() || cury < cliprect.top()) \
			{ \
			} \
			else \
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
	if (zval > (DESTZ)) \
	{ \
		uint32_t srcdata = (SOURCE); \
		if (srcdata != trans_pen) \
		{ \
			if (cury > cliprect.bottom() || cury < cliprect.top()) \
			{ \
			} \
			else \
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
	if (!checkerboard) \
	{ \
		srcpix = srcptr[cursrcx >> 16]; \
	} \
	else \
	{ \
		if (cb & 1) \
		{ \
			srcpix = (cury & 1) ? srcptr[cursrcx >> 16] : 0; \
		} \
		else \
		{ \
			srcpix = (cury & 1) ? 0 : srcptr[cursrcx >> 16]; \
		} \
		cb++; \
	} \
} \
while (0)

void hng64_state::drawline(bitmap_ind16 & dest, bitmap_ind16 & destz, const rectangle & cliprect,
	gfx_element * gfx, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty,
	int32_t dx, int32_t dy, uint32_t dstwidth, uint32_t dstheight, uint32_t trans_pen, uint32_t zval, bool zrev, bool blend, bool checkerboard, uint8_t mosaic, int cury, const u8 *srcdata, int32_t srcx, int32_t srcy_copy, uint32_t numblocks, uint32_t leftovers, int line)
{
	int srcy = dy * line + srcy_copy;

	auto* destptr = &dest.pix(cury, destx);
	auto* destzptr = &destz.pix(cury, destx);

	const u8* srcptr = srcdata + (srcy >> 16) * gfx->rowbytes();
	int32_t cursrcx = srcx;

	// iterate over unrolled blocks of 4
	if (zrev)
	{
		uint8_t cb = 0;
		for (int32_t curx = 0; curx < numblocks; curx++)
		{
			uint16_t srcpix;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN_REV(destptr[0], destzptr[0], srcpix);
			cursrcx += dx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN_REV(destptr[1], destzptr[1], srcpix);
			cursrcx += dx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN_REV(destptr[2], destzptr[2], srcpix);
			cursrcx += dx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN_REV(destptr[3], destzptr[3], srcpix);
			cursrcx += dx;
			destptr += 4;
			destzptr += 4;

		}

		// iterate over leftover pixels
		for (int32_t curx = 0; curx < leftovers; curx++)
		{
			uint16_t srcpix;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN_REV(destptr[0], destzptr[0], srcpix);
			cursrcx += dx;
			destptr++;
			destzptr++;
		}
	}
	else
	{
		uint8_t cb = 0;
		for (int32_t curx = 0; curx < numblocks; curx++)
		{
			uint16_t srcpix;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN(destptr[0], destzptr[0], srcpix);
			cursrcx += dx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN(destptr[1], destzptr[1], srcpix);
			cursrcx += dx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN(destptr[2], destzptr[2], srcpix);
			cursrcx += dx;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN(destptr[3], destzptr[3], srcpix);
			cursrcx += dx;
			destptr += 4;
			destzptr += 4;

		}

		// iterate over leftover pixels
		for (int32_t curx = 0; curx < leftovers; curx++)
		{
			uint16_t srcpix;
			PIX_CHECKERBOARD;
			PIXEL_OP_REBASE_TRANSPEN(destptr[0], destzptr[0], srcpix);
			cursrcx += dx;
			destptr++;
			destzptr++;
		}
	}
}

void hng64_state::zoom_transpen(bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect,
		gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty,
		int32_t dx, int32_t dy, uint32_t dstwidth, uint32_t dstheight, uint32_t trans_pen, uint32_t zval, bool zrev, bool blend, bool checkerboard, uint8_t mosaic)
{
	// use pen usage to optimize
	code %= gfx->elements();
	if (gfx->has_pen_usage())
	{
		// fully transparent; do nothing
		uint32_t usage = gfx->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}

	// render
	color = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());

	if (blend)
		color |= 0x8000;

	assert(dest.valid());
	assert(dest.cliprect().contains(cliprect));

	// ignore empty/invalid cliprects
	if (cliprect.empty())
		return;

	if (dstwidth < 1 || dstheight < 1)
		return;

	// compute final pixel in X and exit if we are entirely clipped
	int32_t destendx = destx + dstwidth - 1;
	if (destx > cliprect.right() || destendx < cliprect.left())
		return;

	// apply left clip
	int32_t srcx = 0;
	if (destx < cliprect.left())
	{
		srcx = (cliprect.left() - destx) * dx;
		destx = cliprect.left();
	}

	// apply right clip
	if (destendx > cliprect.right())
		destendx = cliprect.right();

	// compute final pixel in Y and exit if we are entirely clipped
	int32_t destendy = desty + dstheight - 1;

	// apply top clip
	int32_t srcy = 0;

	// apply X flipping
	if (flipx)
	{
		srcx = (dstwidth - 1) * dx - srcx;
		dx = -dx;
	}

	// apply Y flipping
	if (flipy)
	{
		srcy = (dstheight - 1) * dy - srcy;
		dy = -dy;
	}

	// fetch the source data
	const u8 *srcdata = gfx->get_data(code);

	// compute how many blocks of 4 pixels we have
	uint32_t numblocks = (destendx + 1 - destx) / 4;
	uint32_t leftovers = (destendx + 1 - destx) - 4 * numblocks;

	// iterate over pixels in Y
	int line = 0;
	int32_t srcycopy = srcy;

	for (int32_t cury = desty; cury <= destendy; cury++)
	{
		drawline(dest, destz, cliprect,
			gfx, code, color, flipx, flipy, destx, desty,
			dx, dy, dstwidth, dstheight, trans_pen, zval, zrev, blend, checkerboard, mosaic, cury, srcdata, srcx, srcycopy, numblocks, leftovers, line);

		line++;
	}

}

void hng64_state::get_tile_details(bool chain, uint16_t spritenum, uint8_t xtile, uint8_t ytile, uint8_t xsize, uint8_t ysize, bool xflip, bool yflip, uint32_t& tileno, uint16_t& pal, uint8_t &gfxregion)
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
	int spriteoffsx = (m_spriteregs[1] >> 0) & 0xffff;
	int spriteoffsy = (m_spriteregs[1] >> 16) & 0xffff;

	// This flips between ingame and other screens for roadedge, where the sprites which are filtered definitely needs to change and the game explicitly swaps the values in the sprite list at the same time.
	// m_spriteregs[2] could also play a part as it also flips between 0x00000000 and 0x000fffff at the same time
	// Samsho games also set the upper 3 bits which could be related, samsho games still have some unwanted sprites (but also use the other 'sprite clear' mechanism)
	// Could also be draw order related, check if it inverts the z value?
	bool zsort = !(m_spriteregs[0] & 0x01000000);

	if (zsort)
		m_sprite_zbuffer.fill(0x0000, cliprect);
	else
		m_sprite_zbuffer.fill(0x07ff, cliprect);

	int nextsprite = 0;
	int currentsprite = 0;

	while (currentsprite < ((0xc000 / 4) / 8))
	{
		uint16_t zval = (m_spriteram[(currentsprite * 8) + 2] & 0x07ff0000) >> 16;

		uint16_t ypos = (m_spriteram[(currentsprite * 8) + 0] & 0xffff0000) >> 16;
		uint16_t xpos = (m_spriteram[(currentsprite * 8) + 0] & 0x0000ffff) >> 0;
		xpos += (spriteoffsx);
		ypos += (spriteoffsy);

		bool blend = (m_spriteram[(currentsprite * 8) + 4] & 0x00800000);
		bool checkerboard = (m_spriteram[(currentsprite * 8) + 4] & 0x04000000);
		uint8_t mosaic = (m_spriteram[(currentsprite * 8) + 4] & 0xf0000000) >> 28;

		int yflip = (m_spriteram[(currentsprite * 8) + 4] & 0x01000000) >> 24;
		int xflip = (m_spriteram[(currentsprite * 8) + 4] & 0x02000000) >> 25;

		int chainy = (m_spriteram[(currentsprite * 8) + 2] & 0x0000000f);
		int chainx = (m_spriteram[(currentsprite * 8) + 2] & 0x000000f0) >> 4;
		int chaini = (m_spriteram[(currentsprite * 8) + 2] & 0x00000100);

		if (!chaini)
		{
			nextsprite = currentsprite + 1;
		}
		else
		{
			nextsprite = currentsprite + ((chainx + 1) * (chainy + 1));
		}

		uint32_t zoomy = (m_spriteram[(currentsprite * 8) + 1] & 0xffff0000) >> 16;
		uint32_t zoomx = (m_spriteram[(currentsprite * 8) + 1] & 0x0000ffff) >> 0;

		/* Calculate the zoom */
		int zoom_factor = (m_spriteregs[0] & 0x08000000) ? 0x1000 : 0x100;
		if (!zoomx) zoomx = zoom_factor;
		if (!zoomy) zoomy = zoom_factor;

		int32_t dx, dy;

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

		int16_t drawy = ypos;
		uint32_t srcpix_y = 0;
		for (int ydrw = 0; ydrw <= chainy; ydrw++)
		{
			uint32_t dstheight = 0;
			do
			{
				srcpix_y += dy;
				dstheight++;
			} while (srcpix_y < 0x100000);
			srcpix_y &= 0x0fffff;

			int16_t drawx = xpos;
			uint32_t srcpix_x = 0;

			for (int xdrw = 0; xdrw <= chainx; xdrw++)
			{
				uint32_t dstwidth = 0;
				do
				{
					srcpix_x += dx;
					dstwidth++;
				} while (srcpix_x < 0x100000);
				srcpix_x &= 0x0fffff;

				// 0x3ff (0x200 sign bit) based on sams64_2 char select
				drawx &= 0x3ff;
				drawy &= 0x3ff;

				if (drawx & 0x0200)drawx -= 0x400;
				if (drawy & 0x0200)drawy -= 0x400;

				uint32_t tileno;
				uint16_t pal;
				uint8_t gfxregion;

				get_tile_details(chaini, currentsprite, xdrw, ydrw, chainx, chainy, xflip, yflip, tileno, pal, gfxregion);

				zoom_transpen(m_sprite_bitmap, m_sprite_zbuffer, cliprect, m_gfxdecode->gfx(gfxregion), tileno, pal, xflip, yflip, drawx, drawy, dx, dy, dstwidth, dstheight, 0, zval, zsort, blend, checkerboard, mosaic);

				drawx += dstwidth;
			}

			drawy += dstheight;
		}
		currentsprite = nextsprite;
	}
}

