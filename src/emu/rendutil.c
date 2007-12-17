/***************************************************************************

    rendutil.c

    Core rendering utilities.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "render.h"
#include "rendutil.h"
#include "png.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* utilities */
static void resample_argb_bitmap_average(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color *color, UINT32 dx, UINT32 dy);
static void resample_argb_bitmap_bilinear(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color *color, UINT32 dx, UINT32 dy);
static void copy_png_to_bitmap(mame_bitmap *bitmap, const png_info *png, int *hasalpha);
static void copy_png_alpha_to_bitmap(mame_bitmap *bitmap, const png_info *png, int *hasalpha);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    compute_brightness - compute the effective
    brightness for an RGB pixel
-------------------------------------------------*/

INLINE UINT8 compute_brightness(rgb_t rgb)
{
	return (RGB_RED(rgb) * 222 + RGB_GREEN(rgb) * 707 + RGB_BLUE(rgb) * 71) / 1000;
}



/***************************************************************************
    RENDER UTILITIES
***************************************************************************/

/*-------------------------------------------------
    render_resample_argb_bitmap_hq - perform a high
    quality resampling of a texture
-------------------------------------------------*/

void render_resample_argb_bitmap_hq(void *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const mame_bitmap *source, const rectangle *orig_sbounds, const render_color *color)
{
	UINT32 swidth, sheight;
	const UINT32 *sbase;
	rectangle sbounds;
	UINT32 dx, dy;

	if (dwidth == 0 || dheight == 0)
		return;

	/* compute the real source bounds */
	if (orig_sbounds != NULL)
		sbounds = *orig_sbounds;
	else
	{
		sbounds.min_x = sbounds.min_y = 0;
		sbounds.max_x = source->width;
		sbounds.max_y = source->height;
	}

	/* adjust the source base */
	sbase = (const UINT32 *)source->base + sbounds.min_y * source->rowpixels + sbounds.min_x;

	/* determine the steppings */
	swidth = sbounds.max_x - sbounds.min_x;
	sheight = sbounds.max_y - sbounds.min_y;
	dx = (swidth << 12) / dwidth;
	dy = (sheight << 12) / dheight;

	/* if the source is higher res than the target, use full averaging */
	if (dx > 0x1000 || dy > 0x1000)
		resample_argb_bitmap_average(dest, drowpixels, dwidth, dheight, sbase, source->rowpixels, swidth, sheight, color, dx, dy);
	else
		resample_argb_bitmap_bilinear(dest, drowpixels, dwidth, dheight, sbase, source->rowpixels, swidth, sheight, color, dx, dy);
}


/*-------------------------------------------------
    resample_argb_bitmap_average - resample a texture
    by performing a true weighted average over
    all contributing pixels
-------------------------------------------------*/

static void resample_argb_bitmap_average(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color *color, UINT32 dx, UINT32 dy)
{
	UINT64 sumscale = (UINT64)dx * (UINT64)dy;
	UINT32 r, g, b, a;
	UINT32 x, y;

	/* precompute premultiplied R/G/B/A factors */
	r = color->r * color->a * 256.0;
	g = color->g * color->a * 256.0;
	b = color->b * color->a * 256.0;
	a = color->a * 256.0;

	/* loop over the target vertically */
	for (y = 0; y < dheight; y++)
	{
		UINT32 starty = y * dy;

		/* loop over the target horizontally */
		for (x = 0; x < dwidth; x++)
		{
			UINT64 sumr = 0, sumg = 0, sumb = 0, suma = 0;
			UINT32 startx = x * dx;
			UINT32 xchunk, ychunk;
			UINT32 curx, cury;

			UINT32 yremaining = dy;

			/* accumulate all source pixels that contribute to this pixel */
			for (cury = starty; yremaining; cury += ychunk)
			{
				UINT32 xremaining = dx;

				/* determine the Y contribution, clamping to the amount remaining */
				ychunk = 0x1000 - (cury & 0xfff);
				if (ychunk > yremaining)
					ychunk = yremaining;
				yremaining -= ychunk;

				/* loop over all source pixels in the X direction */
				for (curx = startx; xremaining; curx += xchunk)
				{
					UINT32 factor;
					UINT32 pix;

					/* determine the X contribution, clamping to the amount remaining */
					xchunk = 0x1000 - (curx & 0xfff);
					if (xchunk > xremaining)
						xchunk = xremaining;
					xremaining -= xchunk;

					/* total contribution = x * y */
					factor = xchunk * ychunk;

					/* fetch the source pixel */
					pix = source[(cury >> 12) * srowpixels + (curx >> 12)];

					/* accumulate the RGBA values */
					sumr += factor * RGB_RED(pix);
					sumg += factor * RGB_GREEN(pix);
					sumb += factor * RGB_BLUE(pix);
					suma += factor * RGB_ALPHA(pix);
				}
			}

			/* apply scaling */
			suma = (suma / sumscale) * a / 256;
			sumr = (sumr / sumscale) * r / 256;
			sumg = (sumg / sumscale) * g / 256;
			sumb = (sumb / sumscale) * b / 256;

			/* if we're translucent, add in the destination pixel contribution */
			if (a < 256)
			{
				UINT32 dpix = dest[y * drowpixels + x];
				suma += RGB_ALPHA(dpix) * (256 - a);
				sumr += RGB_RED(dpix) * (256 - a);
				sumg += RGB_GREEN(dpix) * (256 - a);
				sumb += RGB_BLUE(dpix) * (256 - a);
			}

			/* store the target pixel, dividing the RGBA values by the overall scale factor */
			dest[y * drowpixels + x] = MAKE_ARGB(suma, sumr, sumg, sumb);
		}
	}
}


/*-------------------------------------------------
    resample_argb_bitmap_bilinear - perform texture
    sampling via a bilinear filter
-------------------------------------------------*/

static void resample_argb_bitmap_bilinear(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color *color, UINT32 dx, UINT32 dy)
{
	UINT32 maxx = swidth << 12, maxy = sheight << 12;
	UINT32 r, g, b, a;
	UINT32 x, y;

	/* precompute premultiplied R/G/B/A factors */
	r = color->r * color->a * 256.0;
	g = color->g * color->a * 256.0;
	b = color->b * color->a * 256.0;
	a = color->a * 256.0;

	/* loop over the target vertically */
	for (y = 0; y < dheight; y++)
	{
		UINT32 starty = y * dy;

		/* loop over the target horizontally */
		for (x = 0; x < dwidth; x++)
		{
			UINT32 startx = x * dx;
			UINT32 pix0, pix1, pix2, pix3;
			UINT32 sumr, sumg, sumb, suma;
			UINT32 nextx, nexty;
			UINT32 curx, cury;
			UINT32 factor;

			/* adjust start to the center; note that this math will tend to produce */
			/* negative results on the first pixel, which is why we clamp below */
			curx = startx + dx / 2 - 0x800;
			cury = starty + dy / 2 - 0x800;

			/* compute the neighboring pixel */
			nextx = curx + 0x1000;
			nexty = cury + 0x1000;

			/* fetch the four relevant pixels */
			pix0 = pix1 = pix2 = pix3 = 0;
			if ((INT32)cury >= 0 && cury < maxy && (INT32)curx >= 0 && curx < maxx)
				pix0 = source[(cury >> 12) * srowpixels + (curx >> 12)];
			if ((INT32)cury >= 0 && cury < maxy && (INT32)nextx >= 0 && nextx < maxx)
				pix1 = source[(cury >> 12) * srowpixels + (nextx >> 12)];
			if ((INT32)nexty >= 0 && nexty < maxy && (INT32)curx >= 0 && curx < maxx)
				pix2 = source[(nexty >> 12) * srowpixels + (curx >> 12)];
			if ((INT32)nexty >= 0 && nexty < maxy && (INT32)nextx >= 0 && nextx < maxx)
				pix3 = source[(nexty >> 12) * srowpixels + (nextx >> 12)];

			/* compute the x/y scaling factors */
			curx &= 0xfff;
			cury &= 0xfff;

			/* contributions from pixel 0 (top,left) */
			factor = (0x1000 - curx) * (0x1000 - cury);
			sumr = factor * RGB_RED(pix0);
			sumg = factor * RGB_GREEN(pix0);
			sumb = factor * RGB_BLUE(pix0);
			suma = factor * RGB_ALPHA(pix0);

			/* contributions from pixel 1 (top,right) */
			factor = curx * (0x1000 - cury);
			sumr += factor * RGB_RED(pix1);
			sumg += factor * RGB_GREEN(pix1);
			sumb += factor * RGB_BLUE(pix1);
			suma += factor * RGB_ALPHA(pix1);

			/* contributions from pixel 2 (bottom,left) */
			factor = (0x1000 - curx) * cury;
			sumr += factor * RGB_RED(pix2);
			sumg += factor * RGB_GREEN(pix2);
			sumb += factor * RGB_BLUE(pix2);
			suma += factor * RGB_ALPHA(pix2);

			/* contributions from pixel 3 (bottom,right) */
			factor = curx * cury;
			sumr += factor * RGB_RED(pix3);
			sumg += factor * RGB_GREEN(pix3);
			sumb += factor * RGB_BLUE(pix3);
			suma += factor * RGB_ALPHA(pix3);

			/* apply scaling */
			suma = (suma >> 24) * a / 256;
			sumr = (sumr >> 24) * r / 256;
			sumg = (sumg >> 24) * g / 256;
			sumb = (sumb >> 24) * b / 256;

			/* if we're translucent, add in the destination pixel contribution */
			if (a < 256)
			{
				UINT32 dpix = dest[y * drowpixels + x];
				suma += RGB_ALPHA(dpix) * (256 - a);
				sumr += RGB_RED(dpix) * (256 - a);
				sumg += RGB_GREEN(dpix) * (256 - a);
				sumb += RGB_BLUE(dpix) * (256 - a);
			}

			/* store the target pixel, dividing the RGBA values by the overall scale factor */
			dest[y * drowpixels + x] = MAKE_ARGB(suma, sumr, sumg, sumb);
		}
	}
}


/*-------------------------------------------------
    render_clip_line - clip a line to a rectangle
-------------------------------------------------*/

int render_clip_line(render_bounds *bounds, const render_bounds *clip)
{
	/* loop until we get a final result */
	while (1)
	{
		UINT8 code0 = 0, code1 = 0;
		UINT8 thiscode;
		float x, y;

		/* compute Cohen Sutherland bits for first coordinate */
		if (bounds->y0 > clip->y1)
			code0 |= 1;
		if (bounds->y0 < clip->y0)
			code0 |= 2;
		if (bounds->x0 > clip->x1)
			code0 |= 4;
		if (bounds->x0 < clip->x0)
			code0 |= 8;

		/* compute Cohen Sutherland bits for second coordinate */
		if (bounds->y1 > clip->y1)
			code1 |= 1;
		if (bounds->y1 < clip->y0)
			code1 |= 2;
		if (bounds->x1 > clip->x1)
			code1 |= 4;
		if (bounds->x1 < clip->x0)
			code1 |= 8;

		/* trivial accept: just return FALSE */
		if ((code0 | code1) == 0)
			return FALSE;

		/* trivial reject: just return TRUE */
		if ((code0 & code1) != 0)
			return TRUE;

		/* fix one of the OOB cases */
		thiscode = code0 ? code0 : code1;

		/* off the bottom */
		if (thiscode & 1)
		{
			x = bounds->x0 + (bounds->x1 - bounds->x0) * (clip->y1 - bounds->y0) / (bounds->y1 - bounds->y0);
			y = clip->y1;
		}

		/* off the top */
		else if (thiscode & 2)
		{
			x = bounds->x0 + (bounds->x1 - bounds->x0) * (clip->y0 - bounds->y0) / (bounds->y1 - bounds->y0);
			y = clip->y0;
		}

		/* off the right */
		else if (thiscode & 4)
		{
			y = bounds->y0 + (bounds->y1 - bounds->y0) * (clip->x1 - bounds->x0) / (bounds->x1 - bounds->x0);
			x = clip->x1;
		}

		/* off the left */
		else
		{
			y = bounds->y0 + (bounds->y1 - bounds->y0) * (clip->x0 - bounds->x0) / (bounds->x1 - bounds->x0);
			x = clip->x0;
		}

		/* fix the appropriate coordinate */
		if (thiscode == code0)
		{
			bounds->x0 = x;
			bounds->y0 = y;
		}
		else
		{
			bounds->x1 = x;
			bounds->y1 = y;
		}
	}
}


/*-------------------------------------------------
    render_clip_quad - clip a quad to a rectangle
-------------------------------------------------*/

int render_clip_quad(render_bounds *bounds, const render_bounds *clip, render_quad_texuv *texcoords)
{
	/* ensure our assumptions about the bounds are correct */
	assert(bounds->x0 <= bounds->x1);
	assert(bounds->y0 <= bounds->y1);

	/* trivial reject */
	if (bounds->y1 < clip->y0)
		return TRUE;
	if (bounds->y0 > clip->y1)
		return TRUE;
	if (bounds->x1 < clip->x0)
		return TRUE;
	if (bounds->x0 > clip->x1)
		return TRUE;

	/* clip top (x0,y0)-(x1,y1) */
	if (bounds->y0 < clip->y0)
	{
		float frac = (clip->y0 - bounds->y0) / (bounds->y1 - bounds->y0);
		bounds->y0 = clip->y0;
		if (texcoords != NULL)
		{
			texcoords->tl.u += (texcoords->bl.u - texcoords->tl.u) * frac;
			texcoords->tl.v += (texcoords->bl.v - texcoords->tl.v) * frac;
			texcoords->tr.u += (texcoords->br.u - texcoords->tr.u) * frac;
			texcoords->tr.v += (texcoords->br.v - texcoords->tr.v) * frac;
		}
	}

	/* clip bottom (x3,y3)-(x2,y2) */
	if (bounds->y1 > clip->y1)
	{
		float frac = (bounds->y1 - clip->y1) / (bounds->y1 - bounds->y0);
		bounds->y1 = clip->y1;
		if (texcoords != NULL)
		{
			texcoords->bl.u -= (texcoords->bl.u - texcoords->tl.u) * frac;
			texcoords->bl.v -= (texcoords->bl.v - texcoords->tl.v) * frac;
			texcoords->br.u -= (texcoords->br.u - texcoords->tr.u) * frac;
			texcoords->br.v -= (texcoords->br.v - texcoords->tr.v) * frac;
		}
	}

	/* clip left (x0,y0)-(x3,y3) */
	if (bounds->x0 < clip->x0)
	{
		float frac = (clip->x0 - bounds->x0) / (bounds->x1 - bounds->x0);
		bounds->x0 = clip->x0;
		if (texcoords != NULL)
		{
			texcoords->tl.u += (texcoords->tr.u - texcoords->tl.u) * frac;
			texcoords->tl.v += (texcoords->tr.v - texcoords->tl.v) * frac;
			texcoords->bl.u += (texcoords->br.u - texcoords->bl.u) * frac;
			texcoords->bl.v += (texcoords->br.v - texcoords->bl.v) * frac;
		}
	}

	/* clip right (x1,y1)-(x2,y2) */
	if (bounds->x1 > clip->x1)
	{
		float frac = (bounds->x1 - clip->x1) / (bounds->x1 - bounds->x0);
		bounds->x1 = clip->x1;
		if (texcoords != NULL)
		{
			texcoords->tr.u -= (texcoords->tr.u - texcoords->tl.u) * frac;
			texcoords->tr.v -= (texcoords->tr.v - texcoords->tl.v) * frac;
			texcoords->br.u -= (texcoords->br.u - texcoords->bl.u) * frac;
			texcoords->br.v -= (texcoords->br.v - texcoords->bl.v) * frac;
		}
	}
	return FALSE;
}


/*-------------------------------------------------
    render_line_to_quad - convert a line and a
    width to four points
-------------------------------------------------*/

void render_line_to_quad(const render_bounds *bounds, float width, render_bounds *bounds0, render_bounds *bounds1)
{
	render_bounds modbounds = *bounds;
	float unitx, unity;

	/*
        High-level logic -- due to math optimizations, this info is lost below.

        Imagine a thick line of width (w), drawn from (p0) to (p1), with a unit
        vector (u) indicating the direction from (p0) to (p1).

          B                                              C
            +----------------  ...   ------------------+
            |                                        ^ |
            |                                        | |
            |                                        | |
            * (p0)        ------------>           (w)| * (p1)
            |                  (u)                   | |
            |                                        | |
            |                                        v |
            +----------------  ...   ------------------+
          A                                              D

        To convert this into a quad, we need to compute the four points A, B, C
        and D.

        Starting with point A. We first multiply the unit vector by 0.5w and then
        rotate the result 90 degrees. Thus, we have:

            A.x = p0.x + 0.5 * w * u.x * cos(90) - 0.5 * w * u.y * sin(90)
            A.y = p0.y + 0.5 * w * u.x * sin(90) + 0.5 * w * u.y * cos(90)

        Conveniently, sin(90) = 1, and cos(90) = 0, so this simplifies to:

            A.x = p0.x - 0.5 * w * u.y
            A.y = p0.y + 0.5 * w * u.x

        Working clockwise around the polygon, the same fallout happens all around as
        we rotate the unit vector by -90 (B), -90 (C), and 90 (D) degrees:

            B.x = p0.x + 0.5 * w * u.y
            B.y = p0.y - 0.5 * w * u.x

            C.x = p1.x - 0.5 * w * u.y
            C.y = p1.y + 0.5 * w * u.x

            D.x = p1.x + 0.5 * w * u.y
            D.y = p1.y - 0.5 * w * u.x
    */

	/* we only care about the half-width */
	width *= 0.5f;

	/* compute a vector from point 0 to point 1 */
	unitx = modbounds.x1 - modbounds.x0;
	unity = modbounds.y1 - modbounds.y0;

	/* points just use a +1/+1 unit vector; this gives a nice diamond pattern */
	if (unitx == 0 && unity == 0)
	{
		unitx = unity = 0.70710678f * width;
		modbounds.x0 -= 0.5f * unitx;
		modbounds.y0 -= 0.5f * unity;
		modbounds.x1 += 0.5f * unitx;
		modbounds.y1 += 0.5f * unity;
	}

	/* lines need to be divided by their length */
	else
	{
		/* prescale unitx and unity by the half-width */
		float invlength = width / sqrt(unitx * unitx + unity * unity);
		unitx *= invlength;
		unity *= invlength;
	}

	/* rotate the unit vector by 90 degrees and add to point 0 */
	bounds0->x0 = modbounds.x0 - unity;
	bounds0->y0 = modbounds.y0 + unitx;

	/* rotate the unit vector by -90 degrees and add to point 0 */
	bounds0->x1 = modbounds.x0 + unity;
	bounds0->y1 = modbounds.y0 - unitx;

	/* rotate the unit vector by 90 degrees and add to point 1 */
	bounds1->x0 = modbounds.x1 - unity;
	bounds1->y0 = modbounds.y1 + unitx;

	/* rotate the unit vector by -09 degrees and add to point 1 */
	bounds1->x1 = modbounds.x1 + unity;
	bounds1->y1 = modbounds.y1 - unitx;
}


/*-------------------------------------------------
    render_load_png - load a PNG file into a
    mame_bitmap
-------------------------------------------------*/

mame_bitmap *render_load_png(const char *dirname, const char *filename, mame_bitmap *alphadest, int *hasalpha)
{
	mame_bitmap *bitmap = NULL;
	file_error filerr;
	mame_file *file;
	png_info png;
	astring *fname;
	png_error result;

	/* open the file */
	if (dirname == NULL)
		fname = astring_dupc(filename);
	else
		fname = astring_assemble_3(astring_alloc(), dirname, PATH_SEPARATOR, filename);
	filerr = mame_fopen(SEARCHPATH_ARTWORK, astring_c(fname), OPEN_FLAG_READ, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
		return NULL;

	/* read the PNG data */
	result = png_read_file(mame_core_file(file), &png);
	mame_fclose(file);
	if (result != PNGERR_NONE)
		return NULL;

	/* verify we can handle this PNG */
	if (png.bit_depth > 8)
	{
		logerror("%s: Unsupported bit depth %d (8 bit max)\n", filename, png.bit_depth);
		png_free(&png);
		return NULL;
	}
	if (png.interlace_method != 0)
	{
		logerror("%s: Interlace unsupported\n", filename);
		png_free(&png);
		return NULL;
	}
	if (png.color_type != 0 && png.color_type != 3 && png.color_type != 2 && png.color_type != 6)
	{
		logerror("%s: Unsupported color type %d\n", filename, png.color_type);
		png_free(&png);
		return NULL;
	}

	/* if less than 8 bits, upsample */
	png_expand_buffer_8bit(&png);

	/* non-alpha case */
	if (alphadest == NULL)
	{
		bitmap = bitmap_alloc(png.width, png.height, BITMAP_FORMAT_ARGB32);
		if (bitmap != NULL)
			copy_png_to_bitmap(bitmap, &png, hasalpha);
	}

	/* alpha case */
	else
	{
		if (png.width == alphadest->width && png.height == alphadest->height)
		{
			bitmap = alphadest;
			copy_png_alpha_to_bitmap(bitmap, &png, hasalpha);
		}
	}

	/* free PNG data */
	png_free(&png);
	return bitmap;
}


/*-------------------------------------------------
    copy_png_to_bitmap - copy the PNG data to a
    bitmap
-------------------------------------------------*/

static void copy_png_to_bitmap(mame_bitmap *bitmap, const png_info *png, int *hasalpha)
{
	UINT8 accumalpha = 0xff;
	UINT8 *src;
	int x, y;

	/* handle 8bpp palettized case */
	if (png->color_type == 3)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src++)
			{
				/* determine alpha and expand to 32bpp */
				UINT8 alpha = (*src < png->num_trans) ? png->trans[*src] : 0xff;
				accumalpha &= alpha;
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(alpha, png->palette[*src * 3], png->palette[*src * 3 + 1], png->palette[*src * 3 + 2]);
			}
	}

	/* handle 8bpp grayscale case */
	else if (png->color_type == 0)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src++)
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(0xff, *src, *src, *src);
	}

	/* handle 32bpp non-alpha case */
	else if (png->color_type == 2)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src += 3)
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(0xff, src[0], src[1], src[2]);
	}

	/* handle 32bpp alpha case */
	else
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src += 4)
			{
				accumalpha &= src[3];
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(src[3], src[0], src[1], src[2]);
			}
	}

	/* set the hasalpha flag */
	if (hasalpha != NULL)
		*hasalpha = (accumalpha != 0xff);
}


/*-------------------------------------------------
    copy_png_alpha_to_bitmap - copy the PNG data
    to the alpha channel of a bitmap
-------------------------------------------------*/

static void copy_png_alpha_to_bitmap(mame_bitmap *bitmap, const png_info *png, int *hasalpha)
{
	UINT8 accumalpha = 0xff;
	UINT8 *src;
	int x, y;

	/* handle 8bpp palettized case */
	if (png->color_type == 3)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src++)
			{
				rgb_t pixel = *BITMAP_ADDR32(bitmap, y, x);
				UINT8 alpha = compute_brightness(MAKE_RGB(png->palette[*src * 3], png->palette[*src * 3 + 1], png->palette[*src * 3 + 2]));
				accumalpha &= alpha;
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(alpha, RGB_RED(pixel), RGB_GREEN(pixel), RGB_BLUE(pixel));
			}
	}

	/* handle 8bpp grayscale case */
	else if (png->color_type == 0)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src++)
			{
				rgb_t pixel = *BITMAP_ADDR32(bitmap, y, x);
				accumalpha &= *src;
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(*src, RGB_RED(pixel), RGB_GREEN(pixel), RGB_BLUE(pixel));
			}
	}

	/* handle 32bpp non-alpha case */
	else if (png->color_type == 2)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src += 3)
			{
				rgb_t pixel = *BITMAP_ADDR32(bitmap, y, x);
				UINT8 alpha = compute_brightness(MAKE_RGB(src[0], src[1], src[2]));
				accumalpha &= alpha;
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(alpha, RGB_RED(pixel), RGB_GREEN(pixel), RGB_BLUE(pixel));
			}
	}

	/* handle 32bpp alpha case */
	else
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src += 4)
			{
				rgb_t pixel = *BITMAP_ADDR32(bitmap, y, x);
				UINT8 alpha = compute_brightness(MAKE_RGB(src[0], src[1], src[2]));
				accumalpha &= alpha;
				*BITMAP_ADDR32(bitmap, y, x) = MAKE_ARGB(alpha, RGB_RED(pixel), RGB_GREEN(pixel), RGB_BLUE(pixel));
			}
	}

	/* set the hasalpha flag */
	if (hasalpha != NULL)
		*hasalpha = (accumalpha != 0xff);
}
