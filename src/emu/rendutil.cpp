// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendutil.c

    Core rendering utilities.
***************************************************************************/

#include "emu.h"
#include "render.h"
#include "rendutil.h"
#include "png.h"

#include "libjpeg/jpeglib.h"

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* utilities */
static void resample_argb_bitmap_average(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color &color, UINT32 dx, UINT32 dy);
static void resample_argb_bitmap_bilinear(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color &color, UINT32 dx, UINT32 dy);
static bool copy_png_to_bitmap(bitmap_argb32 &bitmap, const png_info *png);
static bool copy_png_alpha_to_bitmap(bitmap_argb32 &bitmap, const png_info *png);



/***************************************************************************
    RENDER UTILITIES
***************************************************************************/

/*-------------------------------------------------
    render_resample_argb_bitmap_hq - perform a high
    quality resampling of a texture
-------------------------------------------------*/

void render_resample_argb_bitmap_hq(bitmap_argb32 &dest, bitmap_argb32 &source, const render_color &color, bool force)
{
	if (dest.width() == 0 || dest.height() == 0)
		return;

	/* adjust the source base */
	const UINT32 *sbase = &source.pix32(0);

	/* determine the steppings */
	UINT32 swidth = source.width();
	UINT32 sheight = source.height();
	UINT32 dwidth = dest.width();
	UINT32 dheight = dest.height();
	UINT32 dx = (swidth << 12) / dwidth;
	UINT32 dy = (sheight << 12) / dheight;

	/* if the source is higher res than the target, use full averaging */
	if (dx > 0x1000 || dy > 0x1000 || force)
		resample_argb_bitmap_average(&dest.pix(0), dest.rowpixels(), dwidth, dheight, sbase, source.rowpixels(), swidth, sheight, color, dx, dy);
	else
		resample_argb_bitmap_bilinear(&dest.pix(0), dest.rowpixels(), dwidth, dheight, sbase, source.rowpixels(), swidth, sheight, color, dx, dy);
}


/*-------------------------------------------------
    resample_argb_bitmap_average - resample a texture
    by performing a true weighted average over
    all contributing pixels
-------------------------------------------------*/

static void resample_argb_bitmap_average(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color &color, UINT32 dx, UINT32 dy)
{
	UINT64 sumscale = (UINT64)dx * (UINT64)dy;
	UINT32 r, g, b, a;
	UINT32 x, y;

	/* precompute premultiplied R/G/B/A factors */
	r = color.r * color.a * 256.0f;
	g = color.g * color.a * 256.0f;
	b = color.b * color.a * 256.0f;
	a = color.a * 256.0f;

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

					/* determine the X contribution, clamping to the amount remaining */
					xchunk = 0x1000 - (curx & 0xfff);
					if (xchunk > xremaining)
						xchunk = xremaining;
					xremaining -= xchunk;

					/* total contribution = x * y */
					factor = xchunk * ychunk;

					/* fetch the source pixel */
					rgb_t pix = source[(cury >> 12) * srowpixels + (curx >> 12)];

					/* accumulate the RGBA values */
					sumr += factor * pix.r();
					sumg += factor * pix.g();
					sumb += factor * pix.b();
					suma += factor * pix.a();
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
				rgb_t dpix = dest[y * drowpixels + x];
				suma += dpix.a() * (256 - a);
				sumr += dpix.r() * (256 - a);
				sumg += dpix.g() * (256 - a);
				sumb += dpix.b() * (256 - a);
			}

			/* store the target pixel, dividing the RGBA values by the overall scale factor */
			dest[y * drowpixels + x] = rgb_t(suma, sumr, sumg, sumb);
		}
	}
}


/*-------------------------------------------------
    resample_argb_bitmap_bilinear - perform texture
    sampling via a bilinear filter
-------------------------------------------------*/

static void resample_argb_bitmap_bilinear(UINT32 *dest, UINT32 drowpixels, UINT32 dwidth, UINT32 dheight, const UINT32 *source, UINT32 srowpixels, UINT32 swidth, UINT32 sheight, const render_color &color, UINT32 dx, UINT32 dy)
{
	UINT32 maxx = swidth << 12, maxy = sheight << 12;
	UINT32 r, g, b, a;
	UINT32 x, y;

	/* precompute premultiplied R/G/B/A factors */
	r = color.r * color.a * 256.0f;
	g = color.g * color.a * 256.0f;
	b = color.b * color.a * 256.0f;
	a = color.a * 256.0f;

	/* loop over the target vertically */
	for (y = 0; y < dheight; y++)
	{
		UINT32 starty = y * dy;

		/* loop over the target horizontally */
		for (x = 0; x < dwidth; x++)
		{
			UINT32 startx = x * dx;
			rgb_t pix0, pix1, pix2, pix3;
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
			sumr = factor * pix0.r();
			sumg = factor * pix0.g();
			sumb = factor * pix0.b();
			suma = factor * pix0.a();

			/* contributions from pixel 1 (top,right) */
			factor = curx * (0x1000 - cury);
			sumr += factor * pix1.r();
			sumg += factor * pix1.g();
			sumb += factor * pix1.b();
			suma += factor * pix1.a();

			/* contributions from pixel 2 (bottom,left) */
			factor = (0x1000 - curx) * cury;
			sumr += factor * pix2.r();
			sumg += factor * pix2.g();
			sumb += factor * pix2.b();
			suma += factor * pix2.a();

			/* contributions from pixel 3 (bottom,right) */
			factor = curx * cury;
			sumr += factor * pix3.r();
			sumg += factor * pix3.g();
			sumb += factor * pix3.b();
			suma += factor * pix3.a();

			/* apply scaling */
			suma = (suma >> 24) * a / 256;
			sumr = (sumr >> 24) * r / 256;
			sumg = (sumg >> 24) * g / 256;
			sumb = (sumb >> 24) * b / 256;

			/* if we're translucent, add in the destination pixel contribution */
			if (a < 256)
			{
				rgb_t dpix = dest[y * drowpixels + x];
				suma += dpix.a() * (256 - a);
				sumr += dpix.r() * (256 - a);
				sumg += dpix.g() * (256 - a);
				sumb += dpix.b() * (256 - a);
			}

			/* store the target pixel, dividing the RGBA values by the overall scale factor */
			dest[y * drowpixels + x] = rgb_t(suma, sumr, sumg, sumb);
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
		if (texcoords != nullptr)
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
		if (texcoords != nullptr)
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
		if (texcoords != nullptr)
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
		if (texcoords != nullptr)
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
		float invlength = width / sqrtf(unitx * unitx + unity * unity);
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
    render_load_jpeg - load a JPG file into a
    bitmap
-------------------------------------------------*/

void render_load_jpeg(bitmap_argb32 &bitmap, emu_file &file, const char *dirname, const char *filename)
{
	// deallocate previous bitmap
	bitmap.reset();

	// define file's full name
	std::string fname;

	if (dirname == nullptr)
		fname = filename;
	else
		fname.assign(dirname).append(PATH_SEPARATOR).append(filename);

	if (file.open(fname.c_str()) != osd_file::error::NONE)
		return;

	// define standard JPEG structures
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// allocates a buffer for the image
	UINT32 jpg_size = file.size();
	unsigned char *jpg_buffer = global_alloc_array(unsigned char, jpg_size);

	// read data from the file and set them in the buffer
	file.read(jpg_buffer, jpg_size);
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);

	// read JPEG header and start decompression
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	// allocates the destination bitmap
	int w = cinfo.output_width;
	int h = cinfo.output_height;
	int s = cinfo.output_components;
	bitmap.allocate(w, h);

	// allocates a buffer to receive the information and copy them into the bitmap
	int row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
	buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);

	while ( cinfo.output_scanline < cinfo.output_height )
	{
		int j = cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);

		if (s == 1)
			for (int i = 0; i < w; ++i)
				bitmap.pix32(j, i) = rgb_t(0xFF, buffer[0][i], buffer[0][i], buffer[0][i]);

		else if (s == 3)
			for (int i = 0; i < w; ++i)
				bitmap.pix32(j, i) = rgb_t(0xFF, buffer[0][i * s], buffer[0][i * s + 1], buffer[0][i * s + 2]);
		else
		{
			osd_printf_error("Cannot read JPEG data from %s file.\n", fname.c_str());
			bitmap.reset();
			break;
		}
	}

	// finish decompression and frees the memory
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	file.close();
	free(buffer[0]);
	free(buffer);
	global_free_array(jpg_buffer);
}


/*-------------------------------------------------
    render_load_png - load a PNG file into a
    bitmap
-------------------------------------------------*/

bool render_load_png(bitmap_argb32 &bitmap, emu_file &file, const char *dirname, const char *filename, bool load_as_alpha_to_existing)
{
	// deallocate if we're not overlaying alpha
	if (!load_as_alpha_to_existing)
		bitmap.reset();

	// open the file
	std::string fname;
	if (dirname == nullptr)
		fname.assign(filename);
	else
		fname.assign(dirname).append(PATH_SEPARATOR).append(filename);
	osd_file::error filerr = file.open(fname.c_str());
	if (filerr != osd_file::error::NONE)
		return false;

	// read the PNG data
	png_info png;
	png_error result = png_read_file(file, &png);
	file.close();
	if (result != PNGERR_NONE)
		return false;

	// verify we can handle this PNG
	if (png.bit_depth > 8)
	{
		osd_printf_error("%s: Unsupported bit depth %d (8 bit max)\n", filename, png.bit_depth);
		png_free(&png);
		return false;
	}
	if (png.interlace_method != 0)
	{
		osd_printf_error("%s: Interlace unsupported\n", filename);
		png_free(&png);
		return false;
	}
	if (png.color_type != 0 && png.color_type != 3 && png.color_type != 2 && png.color_type != 6)
	{
		osd_printf_error("%s: Unsupported color type %d\n", filename, png.color_type);
		png_free(&png);
		return false;
	}

	// if less than 8 bits, upsample
	png_expand_buffer_8bit(&png);

	// non-alpha case
	bool hasalpha = false;
	if (!load_as_alpha_to_existing)
	{
		bitmap.allocate(png.width, png.height);
		hasalpha = copy_png_to_bitmap(bitmap, &png);
	}

	// alpha case
	else if (png.width == bitmap.width() && png.height == bitmap.height())
		hasalpha = copy_png_alpha_to_bitmap(bitmap, &png);

	// free PNG data
	png_free(&png);
	return hasalpha;
}


/*-------------------------------------------------
    copy_png_to_bitmap - copy the PNG data to a
    bitmap
-------------------------------------------------*/

static bool copy_png_to_bitmap(bitmap_argb32 &bitmap, const png_info *png)
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
				bitmap.pix32(y, x) = rgb_t(alpha, png->palette[*src * 3], png->palette[*src * 3 + 1], png->palette[*src * 3 + 2]);
			}
	}

	/* handle 8bpp grayscale case */
	else if (png->color_type == 0)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src++)
				bitmap.pix32(y, x) = rgb_t(0xff, *src, *src, *src);
	}

	/* handle 32bpp non-alpha case */
	else if (png->color_type == 2)
	{
		/* loop over width/height */
		src = png->image;
		for (y = 0; y < png->height; y++)
			for (x = 0; x < png->width; x++, src += 3)
				bitmap.pix32(y, x) = rgb_t(0xff, src[0], src[1], src[2]);
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
				bitmap.pix32(y, x) = rgb_t(src[3], src[0], src[1], src[2]);
			}
	}

	/* set the hasalpha flag */
	return (accumalpha != 0xff);
}


/*-------------------------------------------------
    copy_png_alpha_to_bitmap - copy the PNG data
    to the alpha channel of a bitmap
-------------------------------------------------*/

static bool copy_png_alpha_to_bitmap(bitmap_argb32 &bitmap, const png_info *png)
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
				rgb_t pixel = bitmap.pix32(y, x);
				UINT8 alpha = rgb_t(png->palette[*src * 3], png->palette[*src * 3 + 1], png->palette[*src * 3 + 2]).brightness();
				accumalpha &= alpha;
				bitmap.pix32(y, x) = rgb_t(alpha, pixel.r(), pixel.g(), pixel.b());
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
				rgb_t pixel = bitmap.pix32(y, x);
				accumalpha &= *src;
				bitmap.pix32(y, x) = rgb_t(*src, pixel.r(), pixel.g(), pixel.b());
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
				rgb_t pixel = bitmap.pix32(y, x);
				UINT8 alpha = rgb_t(src[0], src[1], src[2]).brightness();
				accumalpha &= alpha;
				bitmap.pix32(y, x) = rgb_t(alpha, pixel.r(), pixel.g(), pixel.b());
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
				rgb_t pixel = bitmap.pix32(y, x);
				UINT8 alpha = rgb_t(src[0], src[1], src[2]).brightness();
				accumalpha &= alpha;
				bitmap.pix32(y, x) = rgb_t(alpha, pixel.r(), pixel.g(), pixel.b());
			}
	}

	/* set the hasalpha flag */
	return (accumalpha != 0xff);
}
