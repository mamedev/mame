// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendutil.cpp

    Core rendering utilities.

***************************************************************************/

#include "emu.h"
#include "rendutil.h"

#include "msdib.h"
#include "png.h"

#include "jpeglib.h"
#include "jerror.h"

#include <csetjmp>
#include <cstdlib>


namespace {

struct jpeg_corefile_source : public jpeg_source_mgr
{
	static void source(j_decompress_ptr cinfo, util::random_read &file);

private:
	static constexpr unsigned INPUT_BUF_SIZE = 4096;

	static void do_init(j_decompress_ptr cinfo)
	{
		jpeg_corefile_source &src = *static_cast<jpeg_corefile_source *>(cinfo->src);
		src.start_of_file = true;
	}

	static boolean do_fill(j_decompress_ptr cinfo)
	{
		jpeg_corefile_source &src = *static_cast<jpeg_corefile_source *>(cinfo->src);

		size_t nbytes;
		src.infile->read(src.buffer, INPUT_BUF_SIZE, nbytes); // TODO: check error return

		if (0 >= nbytes)
		{
			if (src.start_of_file)
				ERREXIT(cinfo, JERR_INPUT_EMPTY);
			WARNMS(cinfo, JWRN_JPEG_EOF);
			src.buffer[0] = JOCTET(0xff);
			src.buffer[1] = JOCTET(JPEG_EOI);
			nbytes = 2;
		}

		src.next_input_byte = src.buffer;
		src.bytes_in_buffer = nbytes;
		src.start_of_file = false;

		return TRUE;
	}

	static void do_skip(j_decompress_ptr cinfo, long num_bytes)
	{
		jpeg_corefile_source &src = *static_cast<jpeg_corefile_source *>(cinfo->src);

		if (0 < num_bytes)
		{
			while (long(src.bytes_in_buffer) < num_bytes)
			{
				num_bytes -= long(src.bytes_in_buffer);
				(void)(*src.fill_input_buffer)(cinfo);
			}
			src.next_input_byte += size_t(num_bytes);
			src.bytes_in_buffer -= size_t(num_bytes);
		}
	}

	static void do_term(j_decompress_ptr cinfo)
	{
	}

	util::random_read *infile;
	JOCTET *buffer;
	bool start_of_file;
};

void jpeg_corefile_source::source(j_decompress_ptr cinfo, util::random_read &file)
{
	jpeg_corefile_source *src;
	if (!cinfo->src)
	{
		src = reinterpret_cast<jpeg_corefile_source *>(
				(*cinfo->mem->alloc_small)(
					reinterpret_cast<j_common_ptr>(cinfo),
					JPOOL_PERMANENT,
					sizeof(jpeg_corefile_source)));
		cinfo->src = src;
		src->buffer = reinterpret_cast<JOCTET *>(
				(*cinfo->mem->alloc_small)(
					reinterpret_cast<j_common_ptr>(cinfo),
					JPOOL_PERMANENT,
					INPUT_BUF_SIZE * sizeof(JOCTET)));
	}
	else
	{
		src = static_cast<jpeg_corefile_source *>(cinfo->src);
	}

	src->init_source = &jpeg_corefile_source::do_init;
	src->fill_input_buffer = &jpeg_corefile_source::do_fill;
	src->skip_input_data = &jpeg_corefile_source::do_skip;
	src->resync_to_restart = jpeg_resync_to_restart;
	src->term_source = &jpeg_corefile_source::do_term;
	src->infile = &file;
	src->bytes_in_buffer = 0;
	src->next_input_byte = nullptr;
}


struct jpeg_setjmp_error_mgr : public jpeg_error_mgr
{
	jpeg_setjmp_error_mgr()
	{
		jpeg_std_error(this);
		error_exit = [] (j_common_ptr cinfo) { std::longjmp(static_cast<jpeg_setjmp_error_mgr *>(cinfo->err)->m_jump_buffer, 1); };
	}

	std::jmp_buf m_jump_buffer;
};

} // anonymous namespace


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* utilities */
static void resample_argb_bitmap_average(u32 *dest, u32 drowpixels, u32 dwidth, u32 dheight, const u32 *source, u32 srowpixels, u32 swidth, u32 sheight, const render_color &color, u32 dx, u32 dy);
static void resample_argb_bitmap_bilinear(u32 *dest, u32 drowpixels, u32 dwidth, u32 dheight, const u32 *source, u32 srowpixels, u32 swidth, u32 sheight, const render_color &color, u32 dx, u32 dy);
static bool copy_png_alpha_to_bitmap(bitmap_argb32 &bitmap, const util::png_info &png);



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
	const u32 *sbase = &source.pix(0);

	/* determine the steppings */
	u32 swidth = source.width();
	u32 sheight = source.height();
	u32 dwidth = dest.width();
	u32 dheight = dest.height();
	u32 dx = (swidth << 12) / dwidth;
	u32 dy = (sheight << 12) / dheight;

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

static void resample_argb_bitmap_average(u32 *dest, u32 drowpixels, u32 dwidth, u32 dheight, const u32 *source, u32 srowpixels, u32 swidth, u32 sheight, const render_color &color, u32 dx, u32 dy)
{
	u64 sumscale = u64(dx) * u64(dy);
	u32 r, g, b, a;
	u32 x, y;

	/* precompute premultiplied R/G/B/A factors */
	r = color.r * color.a * 256.0f;
	g = color.g * color.a * 256.0f;
	b = color.b * color.a * 256.0f;
	a = color.a * 256.0f;

	/* loop over the target vertically */
	for (y = 0; y < dheight; y++)
	{
		u32 starty = y * dy;

		/* loop over the target horizontally */
		for (x = 0; x < dwidth; x++)
		{
			u64 sumr = 0, sumg = 0, sumb = 0, suma = 0;
			u32 startx = x * dx;
			u32 xchunk, ychunk;
			u32 curx, cury;

			u32 yremaining = dy;

			/* accumulate all source pixels that contribute to this pixel */
			for (cury = starty; yremaining; cury += ychunk)
			{
				u32 xremaining = dx;

				/* determine the Y contribution, clamping to the amount remaining */
				ychunk = 0x1000 - (cury & 0xfff);
				if (ychunk > yremaining)
					ychunk = yremaining;
				yremaining -= ychunk;

				/* loop over all source pixels in the X direction */
				for (curx = startx; xremaining; curx += xchunk)
				{
					u32 factor;

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

static void resample_argb_bitmap_bilinear(u32 *dest, u32 drowpixels, u32 dwidth, u32 dheight, const u32 *source, u32 srowpixels, u32 swidth, u32 sheight, const render_color &color, u32 dx, u32 dy)
{
	u32 maxx = swidth << 12, maxy = sheight << 12;
	u32 r, g, b, a;
	u32 x, y;

	/* precompute premultiplied R/G/B/A factors */
	r = color.r * color.a * 256.0f;
	g = color.g * color.a * 256.0f;
	b = color.b * color.a * 256.0f;
	a = color.a * 256.0f;

	/* loop over the target vertically */
	for (y = 0; y < dheight; y++)
	{
		u32 starty = y * dy;

		/* loop over the target horizontally */
		for (x = 0; x < dwidth; x++)
		{
			u32 startx = x * dx;
			rgb_t pix0, pix1, pix2, pix3;
			u32 sumr, sumg, sumb, suma;
			u32 nextx, nexty;
			u32 curx, cury;
			u32 factor;

			/* adjust start to the center; note that this math will tend to produce */
			/* negative results on the first pixel, which is why we clamp below */
			curx = startx + dx / 2 - 0x800;
			cury = starty + dy / 2 - 0x800;

			/* compute the neighboring pixel */
			nextx = curx + 0x1000;
			nexty = cury + 0x1000;

			/* fetch the four relevant pixels */
			pix0 = pix1 = pix2 = pix3 = 0;
			if (s32(cury) >= 0 && cury < maxy && s32(curx) >= 0 && curx < maxx)
				pix0 = source[(cury >> 12) * srowpixels + (curx >> 12)];
			if (s32(cury) >= 0 && cury < maxy && s32(nextx) >= 0 && nextx < maxx)
				pix1 = source[(cury >> 12) * srowpixels + (nextx >> 12)];
			if (s32(nexty) >= 0 && nexty < maxy && s32(curx) >= 0 && curx < maxx)
				pix2 = source[(nexty >> 12) * srowpixels + (curx >> 12)];
			if (s32(nexty) >= 0 && nexty < maxy && s32(nextx) >= 0 && nextx < maxx)
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

bool render_clip_line(render_bounds *bounds, const render_bounds *clip)
{
	/* loop until we get a final result */
	while (1)
	{
		u8 code0 = 0, code1 = 0;
		u8 thiscode;
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

		/* trivial accept: just return false */
		if ((code0 | code1) == 0)
			return false;

		/* trivial reject: just return true */
		if ((code0 & code1) != 0)
			return true;

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

bool render_clip_quad(render_bounds *bounds, const render_bounds *clip, render_quad_texuv *texcoords)
{
	/* ensure our assumptions about the bounds are correct */
	assert(bounds->x0 <= bounds->x1);
	assert(bounds->y0 <= bounds->y1);

	/* trivial reject */
	if (bounds->y1 < clip->y0)
		return true;
	if (bounds->y0 > clip->y1)
		return true;
	if (bounds->x1 < clip->x0)
		return true;
	if (bounds->x0 > clip->x1)
		return true;

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
	return false;
}


/*-------------------------------------------------
    render_line_to_quad - convert a line and a
    width to four points
-------------------------------------------------*/

void render_line_to_quad(const render_bounds *bounds, float width, float length_extension, render_bounds *bounds0, render_bounds *bounds1)
{
	render_bounds modbounds = *bounds;

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
	float half_width = width * 0.5f;

	/* compute a vector from point 0 to point 1 */
	float unitx = modbounds.x1 - modbounds.x0;
	float unity = modbounds.y1 - modbounds.y0;

	/* points just use a +1/+1 unit vector; this gives a nice diamond pattern */
	if (unitx == 0 && unity == 0)
	{
		/* length of a unit vector (1,1) */
		float unit_length = 0.70710678f;

		unitx = unity = unit_length * half_width;
		modbounds.x0 -= unitx;
		modbounds.y0 -= unity;
		modbounds.x1 += unitx;
		modbounds.y1 += unity;
	}

	/* lines need to be divided by their length */
	else
	{
		float length = sqrtf(unitx * unitx + unity * unity);

		/* extend line length */
		if (length_extension > 0.0f)
		{
			float half_length_extension = length_extension *0.5f;

			float directionx = unitx / length;
			float directiony = unity / length;

			modbounds.x0 -= directionx * half_length_extension;
			modbounds.y0 -= directiony * half_length_extension;
			modbounds.x1 += directionx * half_length_extension;
			modbounds.y1 += directiony * half_length_extension;
		}

		/* prescale unitx and unity by the half-width */
		float invlength = half_width / length;
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

	/* rotate the unit vector by -90 degrees and add to point 1 */
	bounds1->x1 = modbounds.x1 + unity;
	bounds1->y1 = modbounds.y1 - unitx;
}


/*-------------------------------------------------
    render_load_msdib - load a Microsoft DIB file
    into a bitmap
-------------------------------------------------*/

void render_load_msdib(bitmap_argb32 &bitmap, util::random_read &file)
{
	// deallocate previous bitmap
	bitmap.reset();

	// read the DIB data
	util::msdib_error const result = util::msdib_read_bitmap(file, bitmap);
	if (result != util::msdib_error::NONE)
	{
		osd_printf_error("Error reading Microsoft DIB file\n");
		bitmap.reset();
	}
}


/*-------------------------------------------------
    render_load_jpeg - load a JPEG file into a
    bitmap
-------------------------------------------------*/

void render_load_jpeg(bitmap_argb32 &bitmap, util::random_read &file)
{
	// deallocate previous bitmap
	bitmap.reset();

	// set up context for error handling
	jpeg_decompress_struct cinfo;
	jpeg_setjmp_error_mgr jerr;
	cinfo.err = &jerr;
	JSAMPARRAY buffer = nullptr;
	int w, h, s, row_stride, j, i;
	if (setjmp(jerr.m_jump_buffer)) // setjmp must be used in control expression
	{
		char msg[1024];
		(cinfo.err->format_message)(reinterpret_cast<j_common_ptr>(&cinfo), msg);
		osd_printf_error("JPEG error reading data from file: %s\n", msg);
		bitmap.reset();
		goto cleanup; // use goto to ensure longjmp can't cross an initialisation
	}

	// create a JPEG source for the file
	jpeg_create_decompress(&cinfo);
	cinfo.mem->max_memory_to_use = 128 * 1024 * 1024;
	jpeg_corefile_source::source(&cinfo, file);

	// read JPEG header and start decompression
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	// allocates the destination bitmap
	w = cinfo.output_width;
	h = cinfo.output_height;
	s = cinfo.output_components;
	bitmap.allocate(w, h);

	// allocates a buffer to receive the information and copy them into the bitmap
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = reinterpret_cast<JSAMPARRAY>(std::malloc(sizeof(JSAMPROW)));
	buffer[0] = reinterpret_cast<JSAMPROW>(std::malloc(sizeof(JSAMPLE) * row_stride));

	while (cinfo.output_scanline < cinfo.output_height)
	{
		j = cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);

		if (s == 1)
		{
			for (i = 0; i < w; ++i)
				bitmap.pix(j, i) = rgb_t(0xff, buffer[0][i], buffer[0][i], buffer[0][i]);

		}
		else if (s == 3)
		{
			for (i = 0; i < w; ++i)
				bitmap.pix(j, i) = rgb_t(0xff, buffer[0][i * s], buffer[0][i * s + 1], buffer[0][i * s + 2]);
		}
		else
		{
			osd_printf_error("Cannot read JPEG data from file.\n");
			bitmap.reset();
			break;
		}
	}

	// finish decompression and free the memory
	jpeg_finish_decompress(&cinfo);
cleanup:
	jpeg_destroy_decompress(&cinfo);
	if (buffer)
	{
		if (buffer[0])
			std::free(buffer[0]);
		std::free(buffer);
	}
}


/*-------------------------------------------------
    render_load_png - load a PNG file into a
    bitmap
-------------------------------------------------*/

bool render_load_png(bitmap_argb32 &bitmap, util::random_read &file, bool load_as_alpha_to_existing)
{
	// deallocate if we're not overlaying alpha
	if (!load_as_alpha_to_existing)
		bitmap.reset();

	// read the PNG data
	util::png_info png;
	std::error_condition const result = png.read_file(file);
	if (result)
	{
		osd_printf_error("Error reading PNG file\n");
		return false;
	}

	// if less than 8 bits, upsample
	if (png.expand_buffer_8bit())
	{
		osd_printf_error("Error upsampling PNG bitmap\n");
		return false;
	}

	bool hasalpha = false;
	if (!load_as_alpha_to_existing)
	{
		// non-alpha case
		if (png.copy_to_bitmap(bitmap, hasalpha))
		{
			osd_printf_error("Error copying PNG bitmap to MAME bitmap\n");
			return false;
		}
	}
	else if (png.width == bitmap.width() && png.height == bitmap.height())
	{
		// verify we can handle this PNG
		if (png.bit_depth > 8)
		{
			osd_printf_error("Unsupported bit depth %d (8 bit max)\n", png.bit_depth);
			return false;
		}

		// alpha case
		hasalpha = copy_png_alpha_to_bitmap(bitmap, png);
	}

	// free PNG data
	return hasalpha;
}


/*-------------------------------------------------
    copy_png_alpha_to_bitmap - copy the PNG data
    to the alpha channel of a bitmap
-------------------------------------------------*/

static bool copy_png_alpha_to_bitmap(bitmap_argb32 &bitmap, const util::png_info &png)
{
	// FIXME: this function is basically copy/pasted from the PNG code in util, and should be unified with it
	u8 accumalpha = 0xff;

	// colour format table
	static constexpr unsigned samples[] = { 1, 0, 3, 1, 2, 0, 4 };

	// adam7 interlace tables
	static constexpr unsigned x_bias[7] = { 7, 3, 3, 1, 1, 0, 0 };
	static constexpr unsigned y_bias[7] = { 7, 7, 3, 3, 1, 1, 0 };
	static constexpr unsigned x_shift[7] = { 3, 3, 2, 2, 1, 1, 0 };
	static constexpr unsigned y_shift[7] = { 3, 3, 3, 2, 2, 1, 1 };

	unsigned const pass_count(png.interlace_method ? 7 : 1);
	u32 pass_offset[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (unsigned pass = 0; pass_count > pass; ++pass)
	{
		// calculate offset for next interlace pass
		u32 const width(png.interlace_method ? ((png.width + x_bias[pass]) >> x_shift[pass]) : png.width);
		u32 const height(png.interlace_method ? ((png.height + y_bias[pass]) >> y_shift[pass]) : png.height);
		u32 const rowbytes(((width * samples[png.color_type] * png.bit_depth) + 7) >> 3);
		pass_offset[pass + 1] = pass_offset[pass] + (height * (rowbytes + 1));
		u8 const *src(&png.image[pass_offset[pass]]);
		auto const x_trans = [offs = (1 << x_shift[pass]) - x_bias[pass] - 1, shift = x_shift[pass]] (u32 x) { return (x << shift) + offs; };
		auto const y_trans = [offs = (1 << y_shift[pass]) - y_bias[pass] - 1, shift = y_shift[pass]] (u32 y) { return (y << shift) + offs; };

		if (png.color_type == 3)
		{
			// handle 8bpp palettized case
			for (u32 y = 0; height > y; ++y)
			{
				for (u32 x = 0; width > x; ++x, ++src)
				{
					bitmap_argb32::pixel_t &dest(png.interlace_method ? bitmap.pix(y_trans(y), x_trans(x)) : bitmap.pix(y, x));
					rgb_t const pixel(dest);
					u8 const alpha(rgb_t(png.palette[*src * 3], png.palette[*src * 3 + 1], png.palette[*src * 3 + 2]).brightness());
					accumalpha &= alpha;
					dest = rgb_t(alpha, pixel.r(), pixel.g(), pixel.b());
				}
			}
		}
		else if (png.color_type == 0)
		{
			// handle 8bpp grayscale non-alpha case
			for (u32 y = 0; height > y; ++y)
			{
				for (u32 x = 0; width > x; ++x, ++src)
				{
					bitmap_argb32::pixel_t &dest(png.interlace_method ? bitmap.pix(y_trans(y), x_trans(x)) : bitmap.pix(y, x));
					rgb_t const pixel(dest);
					accumalpha &= *src;
					dest = rgb_t(*src, pixel.r(), pixel.g(), pixel.b());
				}
			}
		}
		else if (png.color_type == 4)
		{
			// handle 8bpp grayscale alpha case
			for (u32 y = 0; height > y; ++y)
			{
				for (u32 x = 0; width > x; ++x, src += 2)
				{
					bitmap_argb32::pixel_t &dest(png.interlace_method ? bitmap.pix(y_trans(y), x_trans(x)) : bitmap.pix(y, x));
					rgb_t const pixel(dest);
					accumalpha &= *src;
					dest = rgb_t(*src, pixel.r(), pixel.g(), pixel.b());
				}
			}
		}
		else if (png.color_type == 2)
		{
			// handle 32bpp non-alpha case
			for (u32 y = 0; height > y; ++y)
			{
				for (u32 x = 0; width > x; ++x, src += 3)
				{
					bitmap_argb32::pixel_t &dest(png.interlace_method ? bitmap.pix(y_trans(y), x_trans(x)) : bitmap.pix(y, x));
					rgb_t const pixel(dest);
					u8 const alpha(rgb_t(src[0], src[1], src[2]).brightness());
					accumalpha &= alpha;
					dest = rgb_t(alpha, pixel.r(), pixel.g(), pixel.b());
				}
			}
		}
		else
		{
			// handle 32bpp alpha case
			for (u32 y = 0; height > y; ++y)
			{
				for (u32 x = 0; width > x; ++x, src += 4)
				{
					bitmap_argb32::pixel_t &dest(png.interlace_method ? bitmap.pix(y_trans(y), x_trans(x)) : bitmap.pix(y, x));
					rgb_t const pixel(dest);
					u8 const alpha(rgb_t(src[0], src[1], src[2]).brightness());
					accumalpha &= alpha;
					dest = rgb_t(alpha, pixel.r(), pixel.g(), pixel.b());
				}
			}
		}
	}

	// set the hasalpha flag
	return (accumalpha != 0xff);
}


/*-------------------------------------------------
    render_detect_image - detect image format
-------------------------------------------------*/

ru_imgformat render_detect_image(util::random_read &file)
{
	// PNG: check for valid header
	{
		std::error_condition const png = util::png_info::verify_header(file);
		file.seek(0, SEEK_SET); // TODO: check error return
		if (!png)
			return RENDUTIL_IMGFORMAT_PNG;
	}

	// JPEG: attempt to read header with libjpeg
	{
		jpeg_decompress_struct cinfo;
		jpeg_setjmp_error_mgr jerr;
		cinfo.err = &jerr;
		if (setjmp(jerr.m_jump_buffer)) // setjmp must be used in control expression
			goto notjpeg; // use goto to ensure longjmp can't cross an initialisation

		jpeg_create_decompress(&cinfo);
		cinfo.mem->max_memory_to_use = 128 * 1024 * 1024;
		jpeg_corefile_source::source(&cinfo, file);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_destroy_decompress(&cinfo);
		file.seek(0, SEEK_SET); // TODO: check error return
		return RENDUTIL_IMGFORMAT_JPEG;

	notjpeg:
		jpeg_destroy_decompress(&cinfo);
		file.seek(0, SEEK_SET);
	}

	// Microsoft DIB: check for valid header
	{
		util::msdib_error const msdib = util::msdib_verify_header(file);
		file.seek(0, SEEK_SET);
		if (util::msdib_error::NONE == msdib)
			return RENDUTIL_IMGFORMAT_MSDIB;
	}

	// TODO: add more as necessary
	return RENDUTIL_IMGFORMAT_UNKNOWN;
}
