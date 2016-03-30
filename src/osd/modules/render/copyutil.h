// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  copyutil.h - bitmap-conversion functions
//
//============================================================

#pragma once

#ifndef __RENDER_COPYUTIL__
#define __RENDER_COPYUTIL__

#include "emu.h"

class copy_util
{
public:
	static inline void copyline_palette16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette)
	{
		for (int x = 0; x < width; x++)
		{
			rgb_t srcpixel = palette[*src++];
			*dst++ = 0xff000000 | (srcpixel.b() << 16) | (srcpixel.g() << 8) | srcpixel.r();
		}
	}

	static inline void copyline_palettea16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette)
	{
		for (int x = 0; x < width; x++)
		{
			rgb_t srcpixel = palette[*src++];
			*dst++ = (srcpixel.a() << 24) | (srcpixel.b() << 16) | (srcpixel.g() << 8) | srcpixel.r();
		}
	}

	static inline void copyline_rgb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette)
	{
		int x;

		// palette (really RGB map) case
		if (palette != nullptr)
		{
			for (x = 0; x < width; x++)
			{
				rgb_t srcpix = *src++;
				*dst++ = 0xff000000 | palette[0x200 + srcpix.b()] | palette[0x100 + srcpix.g()] | palette[srcpix.r()];
			}
		}

		// direct case
		else
		{
			for (x = 0; x < width; x++)
			{
				rgb_t srcpix = *src++;
				*dst++ = 0xff000000 | (srcpix.b() << 16) | (srcpix.g() << 8) | srcpix.r();
			}
		}
	}

	static inline void copyline_argb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette)
	{
		int x;
		// palette (really RGB map) case
		if (palette != nullptr)
		{
			for (x = 0; x < width; x++)
			{
				rgb_t srcpix = *src++;
				*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.b()] | palette[0x100 + srcpix.g()] | palette[srcpix.r()];
			}
		}

		// direct case
		else
		{
			for (x = 0; x < width; x++)
			{
				rgb_t srcpix = *src++;
				*dst++ = (srcpix.a() << 24) | (srcpix.b() << 16) | (srcpix.g() << 8) | srcpix.r();
			}
		}
	}

	static inline UINT32 ycc_to_rgb(UINT8 y, UINT8 cb, UINT8 cr)
	{
		/* original equations:

		C = Y - 16
		D = Cb - 128
		E = Cr - 128

		R = clip(( 298 * C           + 409 * E + 128) >> 8)
		G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
		B = clip(( 298 * C + 516 * D           + 128) >> 8)

		R = clip(( 298 * (Y - 16)                    + 409 * (Cr - 128) + 128) >> 8)
		G = clip(( 298 * (Y - 16) - 100 * (Cb - 128) - 208 * (Cr - 128) + 128) >> 8)
		B = clip(( 298 * (Y - 16) + 516 * (Cb - 128)                    + 128) >> 8)

		R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
		G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
		B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)

		R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
		G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
		B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)
		*/
		int r, g, b, common;

		common = 298 * y - 298 * 16;
		r = (common + 409 * cr - 409 * 128 + 128) >> 8;
		g = (common - 100 * cb + 100 * 128 - 208 * cr + 208 * 128 + 128) >> 8;
		b = (common + 516 * cb - 516 * 128 + 128) >> 8;

		if (r < 0) r = 0;
		else if (r > 255) r = 255;
		if (g < 0) g = 0;
		else if (g > 255) g = 255;
		if (b < 0) b = 0;
		else if (b > 255) b = 255;

		return 0xff000000 | (b << 16) | (g << 8) | r;
	}

	static inline void copyline_yuy16_to_argb(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xprescale)
	{
		int x;

		assert(width % 2 == 0);

		// palette (really RGB map) case
		if (palette != nullptr)
		{
			for (x = 0; x < width / 2; x++)
			{
				UINT16 srcpix0 = *src++;
				UINT16 srcpix1 = *src++;
				UINT8 cb = srcpix0 & 0xff;
				UINT8 cr = srcpix1 & 0xff;
				for (int x2 = 0; x2 < xprescale; x2++)
					*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
				for (int x2 = 0; x2 < xprescale; x2++)
					*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
			}
		}

		// direct case
		else
		{
			for (x = 0; x < width; x += 2)
			{
				UINT16 srcpix0 = *src++;
				UINT16 srcpix1 = *src++;
				UINT8 cb = srcpix0 & 0xff;
				UINT8 cr = srcpix1 & 0xff;
				for (int x2 = 0; x2 < xprescale; x2++)
					*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
				for (int x2 = 0; x2 < xprescale; x2++)
					*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
			}
		}
	}
};

#endif // __RENDER_COPYUTIL__
