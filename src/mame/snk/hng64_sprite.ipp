// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner

/* Hyper NeoGeo 64 Sprite bits */

/*
 * Sprite Format
 * ------------------
 *
 * uint32_t | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | x/y position
 *   1    | YYYY YYYY YYYY YYYY XXXX XXXX XXXX XXXX | x/y zoom (*)
 *   2    | ---- -zzz zzzz zzzz ---- ---I cccc CCCC | Z-buffer value, 'Inline' chain flag, x/y chain
 *   3    | ---- ---- pppp pppp ---- ---- ---- ---- | palette entry
 *   4    | mmmm -?fF a??? tttt tttt tttt tttt tttt | mosaic factor, unknown (**) , flip bits, additive blending, unknown (***), tile number
 *   5    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   6    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   7    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *
 * (*) Fatal Fury WA standard elements are 0x1000-0x1000, all the other games sets 0x100-0x100, related to the bit 27 of sprite regs 0?
 * (**) setted by black squares in ranking screen in Samurai Shodown 64 1, sprite disable?
 * (***) bit 22 is setted on some Fatal Fury WA snow (not all of them), bit 21 is setted on Xrally how to play elements in attract mode
 ** Sprite Global Registers
 * -----------------------
 *
 * uint32_t | Bits                                    | Use
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
		u32 srcdata = (SOURCE); \
		if (srcdata != trans_pen) \
		{ \
			(DESTZ) = zval; \
			(DEST) = color + srcdata; \
		} \
	} \
} \
while (0)


#define PIXEL_OP_REBASE_TRANSPEN_REV(DEST, DESTZ, SOURCE) \
do \
{ \
	if (zval > (DESTZ)) \
	{ \
		u32 srcdata = (SOURCE); \
		if (srcdata != trans_pen) \
		{ \
			(DESTZ) = zval; \
			(DEST) = color + srcdata; \
		} \
	} \
} \
while (0)


inline void hng64_state::drawgfxzoom_core(bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect, gfx_element *gfx, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 trans_pen, u32 color, u32 zval, bool zrev)
{
	g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute scaled size
		u32 dstwidth = (scalex * gfx->width() + 0x8000) >> 16;
		u32 dstheight = (scaley * gfx->height() + 0x8000) >> 16;
		if (dstwidth < 1 || dstheight < 1)
			break;

		// compute 16.16 source steps in dx and dy
		s32 dx = (gfx->width() << 16) / dstwidth;
		s32 dy = (gfx->height() << 16) / dstheight;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + dstwidth - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		s32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = (cliprect.left() - destx) * dx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + dstheight - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		s32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = (cliprect.top() - desty) * dy;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

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
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// iterate over pixels in Y
		for (s32 cury = desty; cury <= destendy; cury++)
		{
			auto *destptr = &dest.pix(cury, destx);
			auto *destzptr = &destz.pix(cury, destx);

			const u8 *srcptr = srcdata + (srcy >> 16) * gfx->rowbytes();
			s32 cursrcx = srcx;
			srcy += dy;

			// iterate over unrolled blocks of 4
			if (zrev)
			{
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					PIXEL_OP_REBASE_TRANSPEN_REV(destptr[0], destzptr[0], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					PIXEL_OP_REBASE_TRANSPEN_REV(destptr[1], destzptr[1], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					PIXEL_OP_REBASE_TRANSPEN_REV(destptr[2], destzptr[2], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					PIXEL_OP_REBASE_TRANSPEN_REV(destptr[3], destzptr[3], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					destptr += 4;
					destzptr += 4;

				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					PIXEL_OP_REBASE_TRANSPEN_REV(destptr[0], destzptr[0], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					destptr++;
					destzptr++;
				}
			}
			else
			{
				for (s32 curx = 0; curx < numblocks; curx++)
				{
					PIXEL_OP_REBASE_TRANSPEN(destptr[0], destzptr[0], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					PIXEL_OP_REBASE_TRANSPEN(destptr[1], destzptr[1], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					PIXEL_OP_REBASE_TRANSPEN(destptr[2], destzptr[2], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					PIXEL_OP_REBASE_TRANSPEN(destptr[3], destzptr[3], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					destptr += 4;
					destzptr += 4;

				}

				// iterate over leftover pixels
				for (s32 curx = 0; curx < leftovers; curx++)
				{
					PIXEL_OP_REBASE_TRANSPEN(destptr[0], destzptr[0], srcptr[cursrcx >> 16]);
					cursrcx += dx;
					destptr++;
					destzptr++;
				}
			}
		}
	} while (0);
	g_profiler.stop();
}

void hng64_state::zoom_transpen(bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect,
		gfx_element *gfx, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_pen, u32 zval, bool zrev, bool blend)
{
#if 0
	// use pen usage to optimize
	code %= gfx->elements();
	if (gfx->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}
#endif

	// render
	color = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());

	if (blend)
		color |= 0x8000;

	drawgfxzoom_core(dest, destz, cliprect, gfx, code, flipx, flipy, destx, desty, scalex, scaley, trans_pen, color, zval, zrev);
}



void hng64_state::draw_sprites_buffer(screen_device& screen, const rectangle& cliprect)
{
	m_sprite_bitmap.fill(0x0000, cliprect);
	gfx_element* gfx;

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

	uint32_t* source = m_spriteram;
	uint32_t* end = m_spriteram + (0xc000 / 4);

	while (source < end)
	{
		int tileno, chainx, chainy, xflip;
		int pal, xinc, yinc, yflip;
		uint16_t xpos, ypos;
		int xdrw, ydrw;
		int chaini;
		uint32_t zoomx, zoomy;
		float foomX, foomY;
		bool blend;

		// disable bit? ss64 rankings ? - not disable!, used in buriki for sprites with a dithering effect (fake transparency?)
		if (source[4] & 0x04000000) 
		{
			source += 8;
			continue;
		}

		uint16_t zval = (source[2] & 0x07ff0000) >> 16;

		ypos = (source[0] & 0xffff0000) >> 16;
		xpos = (source[0] & 0x0000ffff) >> 0;
		xpos += (spriteoffsx);
		ypos += (spriteoffsy);

		tileno = (source[4] & 0x0007ffff);
		blend = (source[4] & 0x00800000);
		yflip = (source[4] & 0x01000000) >> 24;
		xflip = (source[4] & 0x02000000) >> 25;

		pal = (source[3] & 0x00ff0000) >> 16;

		chainy = (source[2] & 0x0000000f);
		chainx = (source[2] & 0x000000f0) >> 4;
		chaini = (source[2] & 0x00000100);

		zoomy = (source[1] & 0xffff0000) >> 16;
		zoomx = (source[1] & 0x0000ffff) >> 0;

		/* Calculate the zoom */
		{
			int zoom_factor;

			/* FIXME: regular zoom mode has precision bugs, can be easily seen in Samurai Shodown 64 intro */
			zoom_factor = (m_spriteregs[0] & 0x08000000) ? 0x1000 : 0x100;
			if (!zoomx) zoomx = zoom_factor;
			if (!zoomy) zoomy = zoom_factor;

			/* First, prevent any possible divide by zero errors */
			foomX = (float)(zoom_factor) / (float)zoomx;
			foomY = (float)(zoom_factor) / (float)zoomy;

			zoomx = ((int)foomX) << 16;
			zoomy = ((int)foomY) << 16;

			zoomx += (int)((foomX - floor(foomX)) * (float)0x10000);
			zoomy += (int)((foomY - floor(foomY)) * (float)0x10000);
		}

		if (m_spriteregs[0] & 0x00800000) //bpp switch
		{
			gfx = m_gfxdecode->gfx(4);
		}
		else
		{
			gfx = m_gfxdecode->gfx(5);
			tileno >>= 1;
			pal &= 0xf;
		}

		// Accommodate for chaining and flipping
		if (xflip)
		{
			xinc = -(int)(16.0f * foomX);
			xpos -= xinc * chainx;
		}
		else
		{
			xinc = (int)(16.0f * foomX);
		}

		if (yflip)
		{
			yinc = -(int)(16.0f * foomY);
			ypos -= yinc * chainy;
		}
		else
		{
			yinc = (int)(16.0f * foomY);
		}


		for (ydrw = 0; ydrw <= chainy; ydrw++)
		{
			for (xdrw = 0; xdrw <= chainx; xdrw++)
			{
				int16_t drawx = xpos + (xinc * xdrw);
				int16_t drawy = ypos + (yinc * ydrw);

				// 0x3ff (0x200 sign bit) based on sams64_2 char select
				drawx &= 0x3ff;
				drawy &= 0x3ff;

				if (drawx & 0x0200)drawx -= 0x400;
				if (drawy & 0x0200)drawy -= 0x400;

				if (!chaini)
				{
					zoom_transpen(m_sprite_bitmap, m_sprite_zbuffer, cliprect, gfx, tileno, pal, xflip, yflip, drawx, drawy, zoomx, zoomy/*0x10000*/, 0, zval, zsort, blend);
					tileno++;
				}
				else // inline chain mode, used by ss64
				{

					tileno = (source[4] & 0x0007ffff);
					pal = (source[3] & 0x00ff0000) >> 16;

					if (m_spriteregs[0] & 0x00800000) //bpp switch
					{
						gfx = m_gfxdecode->gfx(4);
					}
					else
					{
						gfx = m_gfxdecode->gfx(5);
						tileno >>= 1;
						pal &= 0xf;
					}

					zoom_transpen(m_sprite_bitmap, m_sprite_zbuffer, cliprect, gfx, tileno, pal, xflip, yflip, drawx, drawy, zoomx, zoomy/*0x10000*/, 0, zval, zsort, blend);
					source += 8;
				}
			}
		}
		if (!chaini)
			source += 8;
	}
}

