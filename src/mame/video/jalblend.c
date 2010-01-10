/***************************************************************************

    Jaleco color blend emulation

****************************************************************************

    This implements the behaviour of color blending/alpha hardware
    found in a small set of machines from the late 80's.

    Thus far, Psychic 5, Argus, and Valtric are presumed to use it.

****************************************************************************/

#include "emu.h"
#include "jalblend.h"


/* each palette entry contains a fourth 'alpha' value */
UINT8 *jal_blend_table;

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
rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, UINT8 alpha)
{
	int r, g, b;
	int ir, ig, ib;

	r = (int)RGB_RED  (dest);
	g = (int)RGB_GREEN(dest);
	b = (int)RGB_BLUE (dest);

	ir = (int)RGB_RED  (addMe);
	ig = (int)RGB_GREEN(addMe);
	ib = (int)RGB_BLUE (addMe);

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

	return MAKE_RGB(r,g,b);
}

void jal_blend_drawgfx(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{
	if (jal_blend_table == NULL)
	{
		drawgfx_transpen(dest_bmp,clip,gfx,code,color,flipx,flipy,offsx,offsy,transparent_color);
		return;
	}

	/* Start drawing */
	if (gfx)
	{
		const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
		const UINT8 *alpha = &jal_blend_table[gfx->color_granularity * (color % gfx->total_colors)];
		const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
		int x_index_base, y_index, sx, sy, ex, ey;
		int xinc, yinc;

		xinc = flipx ? -1 : 1;
		yinc = flipy ? -1 : 1;

		x_index_base = flipx ? gfx->width-1 : 0;
		y_index = flipy ? gfx->height-1 : 0;

		/* start coordinates */
		sx = offsx;
		sy = offsy;

		/* end coordinates */
		ex = sx + gfx->width;
		ey = sy + gfx->height;

		if (clip)
		{
			if (sx < clip->min_x)
			{ /* clip left */
				int pixels = clip->min_x-sx;
				sx += pixels;
				x_index_base += xinc*pixels;
			}
			if (sy < clip->min_y)
			{ /* clip top */
				int pixels = clip->min_y-sy;
				sy += pixels;
				y_index += yinc*pixels;
			}
			/* NS 980211 - fixed incorrect clipping */
			if (ex > clip->max_x+1)
			{ /* clip right */
				ex = clip->max_x+1;
			}
			if (ey > clip->max_y+1)
			{ /* clip bottom */
				ey = clip->max_y+1;
			}
		}

		if (ex > sx)
		{ /* skip if inner loop doesn't draw anything */
			int x, y;

			/* 32-bit destination bitmap */
			if (dest_bmp->bpp == 32)
			{
				/* taken from case 7: TRANSPARENCY_ALPHARANGE */
				for (y = sy; y < ey; y++)
				{
					const UINT8 *source = source_base + y_index*gfx->line_modulo;
					UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
					int x_index = x_index_base;
					for (x = sx; x < ex; x++)
					{
						int c = source[x_index];
						if (c != transparent_color)
						{
							if (alpha[c] & 8)
							{
								/* Comp with clamp */
								dest[x] = jal_blend_func(dest[x], pal[c], alpha[c]);
							}
							else
							{
								/* Skip the costly alpha step altogether */
								dest[x] = pal[c];
							}
						}
						x_index += xinc;
					}
					y_index += yinc;
				}
			}

			/* 16-bit destination bitmap */
			else
			{
				/* taken from case 7: TRANSPARENCY_ALPHARANGE */
				for (y = sy; y < ey; y++)
				{
					const UINT8 *source = source_base + y_index*gfx->line_modulo;
					UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);
					int x_index = x_index_base;
					for (x = sx; x < ex; x++)
					{
						int c = source[x_index];
						if (c != transparent_color)
						{
							if (alpha[c] & 8)
							{
								/* Comp with clamp */
								dest[x] = jal_blend_func(dest[x], pal[c], alpha[c]);
							}
							else
							{
								/* Skip the costly alpha step altogether */
								dest[x] = pal[c];
							}
						}
						x_index += xinc;
					}
					y_index += yinc;
				}
			}
		}
	}
}
