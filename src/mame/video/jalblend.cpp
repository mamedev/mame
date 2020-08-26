// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/***************************************************************************

    Jaleco color blend emulation

****************************************************************************

    This implements the behaviour of color blending/alpha hardware
    found in a small set of machines from the late 80's.

    Thus far, Psychic 5, Argus, and Valtric are presumed to use it.

****************************************************************************/

#include "emu.h"
#include "jalblend.h"

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/



DEFINE_DEVICE_TYPE(JALECO_BLEND, jaleco_blend_device, "jaleco_blend", "Jaleco Blending Device")

jaleco_blend_device::jaleco_blend_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, JALECO_BLEND, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jaleco_blend_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void jaleco_blend_device::device_reset()
{
}

/*
 * 'Alpha' Format
 * ------------------
 *
 * Bytes     | Use
 * -76543210-+----------------
 *  ----x--- | blend enable flag (?)
 *  -----x-- | red add/subtract
 *  ------x- | green add/subtract
 *  -------x | blue add/subtract
 */

rgb_t jaleco_blend_device::func(rgb_t dest, rgb_t addMe)
{
	// Comp with clamp
	if (addMe.a() & 8)
		return func(dest, addMe, addMe.a());

	// Skip the costly alpha step altogether
	return addMe;
}

/* basically an add/subtract function with clamping */
rgb_t jaleco_blend_device::func(rgb_t dest, rgb_t addMe, u8 alpha)
{
	int r = (int)dest.r();
	int g = (int)dest.g();
	int b = (int)dest.b();

	const u8 ir = addMe.r();
	const u8 ig = addMe.g();
	const u8 ib = addMe.b();

	if (alpha & 4)
		{ r -= ir; if (r < 0) r = 0; }
	else
		{ r += ir; if (r > 255) r = 255; }
	if (alpha & 2)
		{ g -= ig; if (g < 0) g = 0; }
	else
		{ g += ig; if (g > 255) g = 255; }
	if (alpha & 1)
		{ b -= ib; if (b < 0) b = 0; }
	else
		{ b += ib; if (b > 255) b = 255; }

	return rgb_t(r,g,b);
}

template<class BitmapClass>
void jaleco_blend_device::drawgfx_common(palette_device &palette,BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color)
{
	/* Start drawing */
	const pen_t *pal = &palette.pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
	const u8 *source_base = gfx->get_data(code % gfx->elements());

	const int xinc = flipx ? -1 : 1;
	const int yinc = flipy ? -1 : 1;

	int x_index_base = flipx ? gfx->width() - 1 : 0;
	int y_index = flipy ? gfx->height() - 1 : 0;

	// start coordinates
	int sx = offsx;
	int sy = offsy;

	// end coordinates
	int ex = sx + gfx->width();
	int ey = sy + gfx->height();

	if (sx < clip.min_x)
	{ // clip left
		int pixels = clip.min_x - sx;
		sx += pixels;
		x_index_base += xinc * pixels;
	}
	if (sy < clip.min_y)
	{ // clip top
		int pixels = clip.min_y - sy;
		sy += pixels;
		y_index += yinc * pixels;
	}
	// NS 980211 - fixed incorrect clipping
	if (ex > clip.max_x + 1)
	{ // clip right
		ex = clip.max_x + 1;
	}
	if (ey > clip.max_y + 1)
	{ // clip bottom
		ey = clip.max_y + 1;
	}

	if (ex > sx)
	{ // skip if inner loop doesn't draw anything

		// taken from case : TRANSPARENCY_ALPHARANGE
		for (int y = sy; y < ey; y++)
		{
			const u8 *source = source_base + y_index*gfx->rowbytes();
			typename BitmapClass::pixel_t *dest = &dest_bmp.pix(y);
			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				const u8 c = source[x_index];
				if (c != transparent_color)
				{
					dest[x] = jaleco_blend_device::func(dest[x], pal[c]);
				}
				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}

void jaleco_blend_device::drawgfx(palette_device &palette,bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color)
{ jaleco_blend_device::drawgfx_common(palette,dest_bmp, clip, gfx, code, color, flipx, flipy, offsx, offsy, transparent_color); }
void jaleco_blend_device::drawgfx(palette_device &palette,bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color)
{ jaleco_blend_device::drawgfx_common(palette,dest_bmp, clip, gfx, code, color, flipx, flipy, offsx, offsy, transparent_color); }
