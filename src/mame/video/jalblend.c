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



const device_type JALECO_BLEND = &device_creator<jaleco_blend_device>;

jaleco_blend_device::jaleco_blend_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, JALECO_BLEND, "Jaleco Blending Device", tag, owner, clock, "jaleco_blend", __FILE__),
	m_table(NULL)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jaleco_blend_device::device_start()
{
	m_table = auto_alloc_array_clear(machine(), UINT8, 0xc00);

	save_pointer(NAME(m_table), 0xc00);
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void jaleco_blend_device::device_reset()
{
	memset(m_table, 0, 0xc00);
}

void jaleco_blend_device::set(int color, UINT8 val)
{
	m_table[color] = val;
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

/* basically an add/subtract function with clamping */
rgb_t jaleco_blend_device::func(rgb_t dest, rgb_t addMe, UINT8 alpha)
{
	int r, g, b;
	int ir, ig, ib;

	r = (int)dest.r();
	g = (int)dest.g();
	b = (int)dest.b();

	ir = (int)addMe.r();
	ig = (int)addMe.g();
	ib = (int)addMe.b();

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

template<class _BitmapClass>
void jaleco_blend_device::drawgfx_common(palette_device &palette,_BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{
	/* Start drawing */
	const pen_t *pal = &palette.pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
	const UINT8 *alpha = &m_table[gfx->granularity() * (color % gfx->colors())];
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());
	int x_index_base, y_index, sx, sy, ex, ey;
	int xinc, yinc;

	xinc = flipx ? -1 : 1;
	yinc = flipy ? -1 : 1;

	x_index_base = flipx ? gfx->width()-1 : 0;
	y_index = flipy ? gfx->height()-1 : 0;

	// start coordinates
	sx = offsx;
	sy = offsy;

	// end coordinates
	ex = sx + gfx->width();
	ey = sy + gfx->height();

	if (sx < clip.min_x)
	{ // clip left
		int pixels = clip.min_x-sx;
		sx += pixels;
		x_index_base += xinc*pixels;
	}
	if (sy < clip.min_y)
	{ // clip top
		int pixels = clip.min_y-sy;
		sy += pixels;
		y_index += yinc*pixels;
	}
	// NS 980211 - fixed incorrect clipping
	if (ex > clip.max_x+1)
	{ // clip right
		ex = clip.max_x+1;
	}
	if (ey > clip.max_y+1)
	{ // clip bottom
		ey = clip.max_y+1;
	}

	if (ex > sx)
	{ // skip if inner loop doesn't draw anything
		int x, y;

		// taken from case 7: TRANSPARENCY_ALPHARANGE
		for (y = sy; y < ey; y++)
		{
			const UINT8 *source = source_base + y_index*gfx->rowbytes();
			typename _BitmapClass::pixel_t *dest = &dest_bmp.pix(y);
			int x_index = x_index_base;
			for (x = sx; x < ex; x++)
			{
				int c = source[x_index];
				if (c != transparent_color)
				{
					if (alpha[c] & 8)
					{
						// Comp with clamp
						dest[x] = jaleco_blend_device::func(dest[x], pal[c], alpha[c]);
					}
					else
					{
						// Skip the costly alpha step altogether
						dest[x] = pal[c];
					}
				}
				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}

void jaleco_blend_device::drawgfx(palette_device &palette,bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{ jaleco_blend_device::drawgfx_common(palette,dest_bmp, clip, gfx, code, color, flipx, flipy, offsx, offsy, transparent_color); }
void jaleco_blend_device::drawgfx(palette_device &palette,bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{ jaleco_blend_device::drawgfx_common(palette,dest_bmp, clip, gfx, code, color, flipx, flipy, offsx, offsy, transparent_color); }
