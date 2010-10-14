/***************************************************************************

    rendersw.c

    Software-only rasterization system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    This file is not to be directly compiled. Rather, the OSD code should
    #define the macros below and then #include this file to generate
    rasterizers that are optimized for a given output format. See
    windows/rendsoft.c for an example.

***************************************************************************/



/***************************************************************************
    USAGE VERIFICATION
***************************************************************************/

#if !defined(FUNC_PREFIX)
#error Must define FUNC_PREFIX!
#endif

#if !defined(PIXEL_TYPE)
#error Must define PIXEL_TYPE!
#endif

#if !defined(SRCSHIFT_R) || !defined(SRCSHIFT_G) || !defined(SRCSHIFT_B)
#error Must define SRCSHIFT_R/SRCSHIFT_G/SRCSHIFT_B!
#endif

#if !defined(DSTSHIFT_R) || !defined(DSTSHIFT_G) || !defined(DSTSHIFT_B)
#error Must define DSTSHIFT_R/DSTSHIFT_G/DSTSHIFT_B!
#endif

#if !defined(NO_DEST_READ)
#define NO_DEST_READ 0
#endif

#if !defined(BILINEAR_FILTER)
#define BILINEAR_FILTER 0
#endif



/***************************************************************************
    ONE-TIME-ONLY DEFINITIONS
***************************************************************************/

#ifndef FIRST_TIME
#define FIRST_TIME

#include "emucore.h"
#include "eminline.h"
#include "video/rgbutil.h"
#include "render.h"



/***************************************************************************
    MACROS
***************************************************************************/

#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)

#define Tinten(intensity, col) \
	MAKE_RGB((RGB_RED(col) * (intensity)) >> 8, (RGB_GREEN(col) * (intensity)) >> 8, (RGB_BLUE(col) * (intensity)) >> 8)

#define IS_OPAQUE(a)		(a >= (NO_DEST_READ ? 0.5f : 1.0f))
#define IS_TRANSPARENT(a)	(a <  (NO_DEST_READ ? 0.5f : 0.0001f))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _quad_setup_data quad_setup_data;
struct _quad_setup_data
{
	INT32			dudx, dvdx, dudy, dvdy;
	INT32			startu, startv;
	INT32			startx, starty;
	INT32			endx, endy;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT32 cosine_table[2049];



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    round_nearest - round to nearest in a
    predictable way
-------------------------------------------------*/

INLINE float round_nearest(float f)
{
	return floor(f + 0.5f);
}


/*------------------------------------------------------------------------
    ycc_to_rgb - convert YCC to RGB; the YCC pixel
    contains Y in the LSB, Cb << 8, and Cr << 16
    This actually a YCbCr conversion,
    details my be found in chapter 6.4 ff of
    http://softwarecommunity.intel.com/isn/downloads/softwareproducts/pdfs/346495.pdf
    The document also contains the constants below as floats.
--------------------------------------------------------------------------*/

INLINE UINT8 clamp16_shift8(UINT32 x)
{
	return (((INT32) x < 0) ? 0 : (x > 65535 ? 255: x >> 8));
}

INLINE UINT32 ycc_to_rgb(UINT32 ycc)
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

        Now combine constants:

        R = clip(( 298 * Y            + 409 * Cr - 56992) >> 8)
        G = clip(( 298 * Y - 100 * Cb - 208 * Cr + 34784) >> 8)
        B = clip(( 298 * Y + 516 * Cb            - 70688) >> 8)

        Define common = 298 * y - 56992. This will save one addition

        R = clip(( common            + 409 * Cr -     0) >> 8)
        G = clip(( common - 100 * Cb - 208 * Cr + 91776) >> 8)
        B = clip(( common + 516 * Cb            - 13696) >> 8)

    */
    UINT8 y = ycc;
    UINT8 cb = ycc >> 8;
    UINT8 cr = ycc >> 16;
	UINT32 r, g, b, common;

	common = 298 * y - 56992;
	r = (common +            409 * cr);
	g = (common - 100 * cb - 208 * cr + 91776);
	b = (common + 516 * cb - 13696);

	/* Now clamp and shift back */
	return MAKE_RGB(clamp16_shift8(r), clamp16_shift8(g), clamp16_shift8(b));
}

/*-------------------------------------------------
    get_texel_palette16_nearest - return the
    nearest neighbor texel from a palettized
    16bpp source
-------------------------------------------------*/

INLINE UINT32 get_texel_palette16_nearest(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base + (curv >> 16) * texture->rowpixels + (curu >> 16);
	return texture->palette[texbase[0]];
}


/*-------------------------------------------------
    get_texel_palette16_bilinear - return a
    bilinear filtered texel from a palettized
    16bpp source
-------------------------------------------------*/

INLINE UINT32 get_texel_palette16_bilinear(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base;
	rgb_t pix00, pix01, pix10, pix11;
	INT32 u0, u1, v0, v1;

	u0 = curu >> 16;
	u1 = 1;
	if (u0 < 0) u0 = u1 = 0;
	else if (u0 + 1 >= texture->width) u0 = texture->width - 1, u1 = 0;
	v0 = curv >> 16;
	v1 = texture->rowpixels;
	if (v0 < 0) v0 = v1 = 0;
	else if (v0 + 1 >= texture->height) v0 = texture->height - 1, v1 = 0;

	texbase += v0 * texture->rowpixels + u0;

	pix00 = texture->palette[texbase[0]];
	pix01 = texture->palette[texbase[u1]];
	pix10 = texture->palette[texbase[v1]];
	pix11 = texture->palette[texbase[u1 + v1]];
	return rgb_bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
}


/*-------------------------------------------------
    get_texel_palette16a_nearest - return the
    nearest neighbor texel from a palettized
    16bpp source
-------------------------------------------------*/

INLINE UINT32 get_texel_palette16a_nearest(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base + (curv >> 16) * texture->rowpixels + (curu >> 16);
	return texture->palette[texbase[0]];
}


/*-------------------------------------------------
    get_texel_palette16a_bilinear - return a
    bilinear filtered texel from a palettized
    16bpp source
-------------------------------------------------*/

INLINE UINT32 get_texel_palette16a_bilinear(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base;
	rgb_t pix00, pix01, pix10, pix11;
	INT32 u0, u1, v0, v1;

	u0 = curu >> 16;
	u1 = 1;
	if (u0 < 0) u0 = u1 = 0;
	else if (u0 + 1 >= texture->width) u0 = texture->width - 1, u1 = 0;
	v0 = curv >> 16;
	v1 = texture->rowpixels;
	if (v0 < 0) v0 = v1 = 0;
	else if (v0 + 1 >= texture->height) v0 = texture->height - 1, v1 = 0;

	texbase += v0 * texture->rowpixels + u0;

	pix00 = texture->palette[texbase[0]];
	pix01 = texture->palette[texbase[u1]];
	pix10 = texture->palette[texbase[v1]];
	pix11 = texture->palette[texbase[u1 + v1]];
	return rgba_bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
}


/*-------------------------------------------------
    get_texel_rgb15_nearest - return the
    nearest neighbor texel from a 15bpp RGB source
-------------------------------------------------*/

INLINE UINT32 get_texel_rgb15_nearest(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base + (curv >> 16) * texture->rowpixels + (curu >> 16);
	return texbase[0];
}


/*-------------------------------------------------
    get_texel_rgb15_bilinear - return the
    bilinear filtered texel from a 15bpp RGB
    source
-------------------------------------------------*/

INLINE UINT32 get_texel_rgb15_bilinear(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base;
	rgb_t pix00, pix01, pix10, pix11, filtered;
	INT32 u0, u1, v0, v1;

	u0 = curu >> 16;
	u1 = 1;
	if (u0 < 0) u0 = u1 = 0;
	else if (u0 + 1 >= texture->width) u0 = texture->width - 1, u1 = 0;
	v0 = curv >> 16;
	v1 = texture->rowpixels;
	if (v0 < 0) v0 = v1 = 0;
	else if (v0 + 1 >= texture->height) v0 = texture->height - 1, v1 = 0;

	texbase += v0 * texture->rowpixels + u0;

	pix00 = texbase[0];
	pix01 = texbase[u1];
	pix10 = texbase[v1];
	pix11 = texbase[u1 + v1];
	pix00 = ((pix00 & 0x7fe0) << 6) | (pix00 & 0x1f);
	pix01 = ((pix01 & 0x7fe0) << 6) | (pix01 & 0x1f);
	pix10 = ((pix10 & 0x7fe0) << 6) | (pix10 & 0x1f);
	pix11 = ((pix11 & 0x7fe0) << 6) | (pix11 & 0x1f);
	filtered = rgb_bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
	return (filtered & 0x1f) | ((filtered & 0x1ff800) >> 6);
}


/*-------------------------------------------------
    get_texel_yuy16_nearest - return the
    nearest neighbor texel from a 16bpp YCbCr
    source (pixel is returned as Cr-Cb-Y
-------------------------------------------------*/

INLINE UINT32 get_texel_yuy16_nearest(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base + (curv >> 16) * texture->rowpixels + (curu >> 17) * 2;
	return (texbase[(curu >> 16) & 1] >> 8) | ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
}


/*-------------------------------------------------
    get_texel_yuy16_bilinear - return the
    nearest neighbor texel from a 16bpp YCbCr
    source (pixel is returned as Cr-Cb-Y
-------------------------------------------------*/

INLINE UINT32 get_texel_yuy16_bilinear(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT16 *texbase = (const UINT16 *)texture->base;
	rgb_t pix00, pix01, pix10, pix11;
	INT32 u0, u1, v0, v1;

	u0 = curu >> 16;
	u1 = 1;
	if (u0 < 0) u0 = u1 = 0;
	else if (u0 + 1 >= texture->width) u0 = texture->width - 1, u1 = 0;
	v0 = curv >> 16;
	v1 = texture->rowpixels;
	if (v0 < 0) v0 = v1 = 0;
	else if (v0 + 1 >= texture->height) v0 = texture->height - 1, v1 = 0;

	texbase += v0 * texture->rowpixels + (u0 & ~1);

	if ((curu & 0x10000) == 0)
	{
		rgb_t cbcr = ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
		pix00 = (texbase[0] >> 8) | cbcr;
		pix01 = (texbase[u1] >> 8) | cbcr;
		cbcr = ((texbase[v1 + 0] & 0xff) << 8) | ((texbase[v1 + 1] & 0xff) << 16);
		pix10 = (texbase[v1 + 0] >> 8) | cbcr;
		pix11 = (texbase[v1 + u1] >> 8) | cbcr;
	}
	else
	{
		rgb_t cbcr = ((texbase[0] & 0xff) << 8) | ((texbase[1] & 0xff) << 16);
		pix00 = (texbase[1] >> 8) | cbcr;
		if (u1 != 0)
		{
			cbcr = ((texbase[2] & 0xff) << 8) | ((texbase[3] & 0xff) << 16);
			pix01 = (texbase[2] >> 8) | cbcr;
		}
		else
			pix01 = pix00;
		cbcr = ((texbase[v1 + 0] & 0xff) << 8) | ((texbase[v1 + 1] & 0xff) << 16);
		pix10 = (texbase[v1 + 1] >> 8) | cbcr;
		if (u1 != 0)
		{
			cbcr = ((texbase[v1 + 2] & 0xff) << 8) | ((texbase[v1 + 3] & 0xff) << 16);
			pix11 = (texbase[v1 + 2] >> 8) | cbcr;
		}
		else
			pix11 = pix10;
	}
	return rgb_bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
}


/*-------------------------------------------------
    get_texel_rgb32_nearest - return the
    nearest neighbor texel from a 32bpp RGB source
-------------------------------------------------*/

INLINE UINT32 get_texel_rgb32_nearest(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT32 *texbase = (const UINT32 *)texture->base + (curv >> 16) * texture->rowpixels + (curu >> 16);
	return texbase[0];
}


/*-------------------------------------------------
    get_texel_rgb32_bilinear - return the
    bilinear filtered texel from a 32bpp RGB
    source
-------------------------------------------------*/

INLINE UINT32 get_texel_rgb32_bilinear(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT32 *texbase = (const UINT32 *)texture->base;
	rgb_t pix00, pix01, pix10, pix11;
	INT32 u0, u1, v0, v1;

	u0 = curu >> 16;
	u1 = 1;
	if (u0 < 0) u0 = u1 = 0;
	else if (u0 + 1 >= texture->width) u0 = texture->width - 1, u1 = 0;
	v0 = curv >> 16;
	v1 = texture->rowpixels;
	if (v0 < 0) v0 = v1 = 0;
	else if (v0 + 1 >= texture->height) v0 = texture->height - 1, v1 = 0;

	texbase += v0 * texture->rowpixels + u0;

	pix00 = texbase[0];
	pix01 = texbase[u1];
	pix10 = texbase[v1];
	pix11 = texbase[u1 + v1];
	return rgb_bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
}


/*-------------------------------------------------
    get_texel_argb32_nearest - return the
    nearest neighbor texel from a 32bpp ARGB
    source
-------------------------------------------------*/

INLINE UINT32 get_texel_argb32_nearest(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT32 *texbase = (const UINT32 *)texture->base + (curv >> 16) * texture->rowpixels + (curu >> 16);
	return texbase[0];
}


/*-------------------------------------------------
    get_texel_argb32_bilinear - return the
    bilinear filtered texel from a 32bpp ARGB
    source
-------------------------------------------------*/

INLINE UINT32 get_texel_argb32_bilinear(const render_texinfo *texture, INT32 curu, INT32 curv)
{
	const UINT32 *texbase = (const UINT32 *)texture->base;
	rgb_t pix00, pix01, pix10, pix11;
	INT32 u0, u1, v0, v1;

	u0 = curu >> 16;
	u1 = 1;
	if (u0 < 0) u0 = u1 = 0;
	else if (u0 + 1 >= texture->width) u0 = texture->width - 1, u1 = 0;
	v0 = curv >> 16;
	v1 = texture->rowpixels;
	if (v0 < 0) v0 = v1 = 0;
	else if (v0 + 1 >= texture->height) v0 = texture->height - 1, v1 = 0;

	texbase += v0 * texture->rowpixels + u0;

	pix00 = texbase[0];
	pix01 = texbase[u1];
	pix10 = texbase[v1];
	pix11 = texbase[u1 + v1];
	return rgba_bilinear_filter(pix00, pix01, pix10, pix11, curu >> 8, curv >> 8);
}


#endif


/***************************************************************************
    MACROS
***************************************************************************/

/* source 15-bit pixels are in MAME standardized format */
#define SOURCE15_R(pix)		(((pix) >> (7 + SRCSHIFT_R)) & (0xf8 >> SRCSHIFT_R))
#define SOURCE15_G(pix)		(((pix) >> (2 + SRCSHIFT_G)) & (0xf8 >> SRCSHIFT_G))
#if (SRCSHIFT_B < 3)
#define SOURCE15_B(pix)		(((pix) << (3 - SRCSHIFT_B)) & (0xf8 >> SRCSHIFT_B))
#else
#define SOURCE15_B(pix)		(((pix) >> (SRCSHIFT_B - 3)) & (0xf8 >> SRCSHIFT_B))
#endif

/* source 32-bit pixels are in MAME standardized format */
#define SOURCE32_R(pix)		(((pix) >> (16 + SRCSHIFT_R)) & (0xff >> SRCSHIFT_R))
#define SOURCE32_G(pix)		(((pix) >> (8 + SRCSHIFT_G)) & (0xff >> SRCSHIFT_G))
#define SOURCE32_B(pix)		(((pix) >> (0 + SRCSHIFT_B)) & (0xff >> SRCSHIFT_B))

/* destination pixels are written based on the values of the macros */
#define DEST_ASSEMBLE_RGB(r,g,b)	(((r) << DSTSHIFT_R) | ((g) << DSTSHIFT_G) | ((b) << DSTSHIFT_B))
#define DEST_RGB_TO_PIXEL(r,g,b)	DEST_ASSEMBLE_RGB((r) >> SRCSHIFT_R, (g) >> SRCSHIFT_G, (b) >> SRCSHIFT_B)

/* destination pixel masks are based on the macros as well */
#define DEST_R(pix)			(((pix) >> DSTSHIFT_R) & (0xff >> SRCSHIFT_R))
#define DEST_G(pix)			(((pix) >> DSTSHIFT_G) & (0xff >> SRCSHIFT_G))
#define DEST_B(pix)			(((pix) >> DSTSHIFT_B) & (0xff >> SRCSHIFT_B))

/* direct 15-bit source to destination pixel conversion */
#define SOURCE15_TO_DEST(pix)	DEST_ASSEMBLE_RGB(SOURCE15_R(pix), SOURCE15_G(pix), SOURCE15_B(pix))
#ifndef VARIABLE_SHIFT
#if (SRCSHIFT_R == 3) && (SRCSHIFT_G == 3) && (SRCSHIFT_B == 3) && (DSTSHIFT_R == 10) && (DSTSHIFT_G == 5) && (DSTSHIFT_B == 0)
#undef SOURCE15_TO_DEST
#define SOURCE15_TO_DEST(pix)	(pix)
#endif
#endif

/* direct 32-bit source to destination pixel conversion */
#define SOURCE32_TO_DEST(pix)	DEST_ASSEMBLE_RGB(SOURCE32_R(pix), SOURCE32_G(pix), SOURCE32_B(pix))
#ifndef VARIABLE_SHIFT
#if (SRCSHIFT_R == 0) && (SRCSHIFT_G == 0) && (SRCSHIFT_B == 0) && (DSTSHIFT_R == 16) && (DSTSHIFT_G == 8) && (DSTSHIFT_B == 0)
#undef SOURCE32_TO_DEST
#define SOURCE32_TO_DEST(pix)	(pix)
#endif
#endif

/* texel functions */
#undef GET_TEXEL
#if BILINEAR_FILTER
#define GET_TEXEL(type)				get_texel_##type##_##bilinear
#else
#define GET_TEXEL(type)				get_texel_##type##_##nearest
#endif



/***************************************************************************
    LINE RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_aa_pixel - draw an antialiased pixel
-------------------------------------------------*/

INLINE void FUNC_PREFIX(draw_aa_pixel)(void *dstdata, UINT32 pitch, int x, int y, rgb_t col)
{
	UINT32 dpix, dr, dg, db;
	PIXEL_TYPE *dest;

	dest = (PIXEL_TYPE *)dstdata + y * pitch + x;
	dpix = NO_DEST_READ ? 0 : *dest;
	dr = SOURCE32_R(col) + DEST_R(dpix);
	dg = SOURCE32_G(col) + DEST_G(dpix);
	db = SOURCE32_B(col) + DEST_B(dpix);
	dr = (dr | -(dr >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
	dg = (dg | -(dg >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
	db = (db | -(db >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
	*dest = DEST_ASSEMBLE_RGB(dr, dg, db);
}


/*-------------------------------------------------
    draw_line - draw a line or point
-------------------------------------------------*/

static void FUNC_PREFIX(draw_line)(const render_primitive *prim, void *dstdata, INT32 width, INT32 height, UINT32 pitch)
{
	int dx,dy,sx,sy,cx,cy,bwidth;
	UINT8 a1;
	int x1,x2,y1,y2;
	UINT32 col;
	int xx,yy;
	int beam;

	/* compute the start/end coordinates */
	x1 = (int)(prim->bounds.x0 * 65536.0f);
	y1 = (int)(prim->bounds.y0 * 65536.0f);
	x2 = (int)(prim->bounds.x1 * 65536.0f);
	y2 = (int)(prim->bounds.y1 * 65536.0f);

	/* handle color and intensity */
	col = MAKE_RGB((int)(255.0f * prim->color.r * prim->color.a), (int)(255.0f * prim->color.g * prim->color.a), (int)(255.0f * prim->color.b * prim->color.a));

	if (PRIMFLAG_GET_ANTIALIAS(prim->flags))
	{
		/* build up the cosine table if we haven't yet */
		if (cosine_table[0] == 0)
		{
			int entry;
			for (entry = 0; entry <= 2048; entry++)
				cosine_table[entry] = (int)((double)(1.0 / cos(atan((double)(entry) / 2048.0))) * 0x10000000 + 0.5);
		}

		beam = prim->width * 65536.0f;
		if (beam < 0x00010000)
			beam = 0x00010000;

		/* draw an anti-aliased line */
		dx = abs(x1 - x2);
		dy = abs(y1 - y2);

		if (dx >= dy)
		{
			sx = ((x1 <= x2) ? 1 : -1);
			sy = (dy == 0) ? 0 : div_32x32_shift(y2 - y1, dx, 16);
			if (sy < 0)
				dy--;
			x1 >>= 16;
			xx = x2 >> 16;
			bwidth = mul_32x32_hi(beam << 4, cosine_table[abs(sy) >> 5]);
			y1 -= bwidth >> 1; /* start back half the diameter */
			for (;;)
			{
				if (x1 >= 0 && x1 < width)
				{
					dx = bwidth;    /* init diameter of beam */
					dy = y1 >> 16;
					if (dy >= 0 && dy < height)
						FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, x1, dy, Tinten(0xff & (~y1 >> 8), col));
					dy++;
					dx -= 0x10000 - (0xffff & y1); /* take off amount plotted */
					a1 = (dx >> 8) & 0xff;   /* calc remainder pixel */
					dx >>= 16;                   /* adjust to pixel (solid) count */
					while (dx--)                 /* plot rest of pixels */
					{
						if (dy >= 0 && dy < height)
							FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, x1, dy, col);
						dy++;
					}
					if (dy >= 0 && dy < height)
						FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, x1, dy, Tinten(a1,col));
				}
				if (x1 == xx) break;
				x1 += sx;
				y1 += sy;
			}
		}
		else
		{
			sy = ((y1 <= y2) ? 1: -1);
			sx = (dx == 0) ? 0 : div_32x32_shift(x2 - x1, dy, 16);
			if (sx < 0)
				dx--;
			y1 >>= 16;
			yy = y2 >> 16;
			bwidth = mul_32x32_hi(beam << 4,cosine_table[abs(sx) >> 5]);
			x1 -= bwidth >> 1; /* start back half the width */
			for (;;)
			{
				if (y1 >= 0 && y1 < height)
				{
					dy = bwidth;    /* calc diameter of beam */
					dx = x1 >> 16;
					if (dx >= 0 && dx < width)
						FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, dx, y1, Tinten(0xff & (~x1 >> 8), col));
					dx++;
					dy -= 0x10000 - (0xffff & x1); /* take off amount plotted */
					a1 = (dy >> 8) & 0xff;   /* remainder pixel */
					dy >>= 16;                   /* adjust to pixel (solid) count */
					while (dy--)                 /* plot rest of pixels */
					{
						if (dx >= 0 && dx < width)
							FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, dx, y1, col);
						dx++;
					}
					if (dx >= 0 && dx < width)
						FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, dx, y1, Tinten(a1, col));
				}
				if (y1 == yy) break;
				y1 += sy;
				x1 += sx;
			}
		}
	}
	else /* use good old Bresenham for non-antialiasing 980317 BW */
	{
		x1 = (x1 + 0x8000) >> 16;
		y1 = (y1 + 0x8000) >> 16;
		x2 = (x2 + 0x8000) >> 16;
		y2 = (y2 + 0x8000) >> 16;

		dx = abs(x1 - x2);
		dy = abs(y1 - y2);
		sx = (x1 <= x2) ? 1 : -1;
		sy = (y1 <= y2) ? 1 : -1;
		cx = dx / 2;
		cy = dy / 2;

		if (dx >= dy)
		{
			for (;;)
			{
				if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
					FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, x1, y1, col);
				if (x1 == x2) break;
				x1 += sx;
				cx -= dy;
				if (cx < 0)
				{
					y1 += sy;
					cx += dx;
				}
			}
		}
		else
		{
			for (;;)
			{
				if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
					FUNC_PREFIX(draw_aa_pixel)(dstdata, pitch, x1, y1, col);
				if (y1 == y2) break;
				y1 += sy;
				cy -= dx;
				if (cy < 0)
				{
					x1 += sx;
					cy += dy;
				}
			}
		}
	}
}



/***************************************************************************
    RECT RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_rect - draw a solid rectangle
-------------------------------------------------*/

static void FUNC_PREFIX(draw_rect)(const render_primitive *prim, void *dstdata, INT32 width, INT32 height, UINT32 pitch)
{
	render_bounds fpos = prim->bounds;
	INT32 startx, starty, endx, endy;
	INT32 x, y;

	assert(fpos.x0 <= fpos.x1);
	assert(fpos.y0 <= fpos.y1);

	/* clamp to integers */
	startx = round_nearest(fpos.x0);
	starty = round_nearest(fpos.y0);
	endx = round_nearest(fpos.x1);
	endy = round_nearest(fpos.y1);

	/* ensure we fit */
	if (startx < 0) startx = 0;
	if (startx >= width) startx = width;
	if (endx < 0) endx = 0;
	if (endx >= width) endx = width;
	if (starty < 0) starty = 0;
	if (starty >= height) starty = height;
	if (endy < 0) endy = 0;
	if (endy >= height) endy = height;

	/* bail if nothing left */
	if (fpos.x0 > fpos.x1 || fpos.y0 > fpos.y1)
		return;

	/* only support alpha and "none" blendmodes */
	assert(PRIMFLAG_GET_BLENDMODE(prim->flags) == BLENDMODE_NONE ||
	       PRIMFLAG_GET_BLENDMODE(prim->flags) == BLENDMODE_ALPHA);

	/* fast case: no alpha */
	if (PRIMFLAG_GET_BLENDMODE(prim->flags) == BLENDMODE_NONE || IS_OPAQUE(prim->color.a))
	{
		UINT32 r = (UINT32)(256.0f * prim->color.r);
		UINT32 g = (UINT32)(256.0f * prim->color.g);
		UINT32 b = (UINT32)(256.0f * prim->color.b);
		UINT32 pix;

		/* clamp R,G,B to 0-256 range */
		if (r > 0xff) { if ((INT32)r < 0) r = 0; else r = 0xff; }
		if (g > 0xff) { if ((INT32)g < 0) g = 0; else g = 0xff; }
		if (b > 0xff) { if ((INT32)b < 0) b = 0; else b = 0xff; }
		pix = DEST_RGB_TO_PIXEL(r, g, b);

		/* loop over rows */
		for (y = starty; y < endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + startx;

			/* loop over cols */
			for (x = startx; x < endx; x++)
				*dest++ = pix;
		}
	}

	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(prim->color.a))
	{
		UINT32 rmask = DEST_RGB_TO_PIXEL(0xff,0x00,0x00);
		UINT32 gmask = DEST_RGB_TO_PIXEL(0x00,0xff,0x00);
		UINT32 bmask = DEST_RGB_TO_PIXEL(0x00,0x00,0xff);
		UINT32 r = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 g = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 b = (UINT32)(256.0f * prim->color.b * prim->color.a);
		UINT32 inva = (UINT32)(256.0f * (1.0f - prim->color.a));

		/* clamp R,G,B and inverse A to 0-256 range */
		if (r > 0xff) { if ((INT32)r < 0) r = 0; else r = 0xff; }
		if (g > 0xff) { if ((INT32)g < 0) g = 0; else g = 0xff; }
		if (b > 0xff) { if ((INT32)b < 0) b = 0; else b = 0xff; }
		if (inva > 0x100) { if ((INT32)inva < 0) inva = 0; else inva = 0x100; }

		/* pre-shift the RGBA pieces */
		r = DEST_RGB_TO_PIXEL(r, 0, 0) << 8;
		g = DEST_RGB_TO_PIXEL(0, g, 0) << 8;
		b = DEST_RGB_TO_PIXEL(0, 0, b) << 8;

		/* loop over rows */
		for (y = starty; y < endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + startx;

			/* loop over cols */
			for (x = startx; x < endx; x++)
			{
				UINT32 dpix = NO_DEST_READ ? 0 : *dest;
				UINT32 dr = (r + ((dpix & rmask) * inva)) & (rmask << 8);
				UINT32 dg = (g + ((dpix & gmask) * inva)) & (gmask << 8);
				UINT32 db = (b + ((dpix & bmask) * inva)) & (bmask << 8);
				*dest++ = (dr | dg | db) >> 8;
			}
		}
	}
}



/***************************************************************************
    16-BIT PALETTE RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_quad_palette16_none - perform
    rasterization of a 16bpp palettized texture
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_palette16_none)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* ensure all parameters are valid */
	assert(prim->texture.palette != NULL);

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16)(&prim->texture, curu, curv);
				*dest++ = SOURCE32_TO_DEST(pix);
				curu += dudx;
				curv += dvdx;
			}
		}
	}

	/* coloring-only case */
	else if (IS_OPAQUE(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);

		/* clamp R,G,B to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16)(&prim->texture, curu, curv);
				UINT32 r = (SOURCE32_R(pix) * sr) >> 8;
				UINT32 g = (SOURCE32_G(pix) * sg) >> 8;
				UINT32 b = (SOURCE32_B(pix) * sb) >> 8;

				*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
				curu += dudx;
				curv += dvdx;
			}
		}
	}

	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 sg = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 sb = (UINT32)(256.0f * prim->color.b * prim->color.a);
		UINT32 invsa = (UINT32)(256.0f * (1.0f - prim->color.a));

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (invsa > 0x100) { if ((INT32)invsa < 0) invsa = 0; else invsa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16)(&prim->texture, curu, curv);
				UINT32 dpix = NO_DEST_READ ? 0 : *dest;
				UINT32 r = (SOURCE32_R(pix) * sr + DEST_R(dpix) * invsa) >> 8;
				UINT32 g = (SOURCE32_G(pix) * sg + DEST_G(dpix) * invsa) >> 8;
				UINT32 b = (SOURCE32_B(pix) * sb + DEST_B(dpix) * invsa) >> 8;

				*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
				curu += dudx;
				curv += dvdx;
			}
		}
	}
}


/*-------------------------------------------------
    draw_quad_palette16_add - perform
    rasterization of a 16bpp palettized texture
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_palette16_add)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* ensure all parameters are valid */
	assert(prim->texture.palette != NULL);

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16)(&prim->texture, curu, curv);
				if ((pix & 0xffffff) != 0)
				{
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = SOURCE32_R(pix) + DEST_R(dpix);
					UINT32 g = SOURCE32_G(pix) + DEST_G(dpix);
					UINT32 b = SOURCE32_B(pix) + DEST_B(dpix);
					r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
					g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
					b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
					*dest = DEST_ASSEMBLE_RGB(r, g, b);
				}
				dest++;
				curu += dudx;
				curv += dvdx;
			}
		}
	}

	/* alpha and/or coloring case */
	else
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 sg = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 sb = (UINT32)(256.0f * prim->color.b * prim->color.a);

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16)(&prim->texture, curu, curv);
				if ((pix & 0xffffff) != 0)
				{
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = ((SOURCE32_R(pix) * sr) >> 8) + DEST_R(dpix);
					UINT32 g = ((SOURCE32_G(pix) * sg) >> 8) + DEST_G(dpix);
					UINT32 b = ((SOURCE32_B(pix) * sb) >> 8) + DEST_B(dpix);
					r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
					g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
					b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}



/***************************************************************************
    16-BIT ALPHA PALETTE RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_quad_palettea16_alpha - perform
    rasterization using standard alpha blending
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_palettea16_alpha)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* ensure all parameters are valid */
	assert(prim->texture.palette != NULL);

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16a)(&prim->texture, curu, curv);
				UINT32 ta = pix >> 24;
				if (ta != 0)
				{
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 invta = 0x100 - ta;
					UINT32 r = (SOURCE32_R(pix) * ta + DEST_R(dpix) * invta) >> 8;
					UINT32 g = (SOURCE32_G(pix) * ta + DEST_G(dpix) * invta) >> 8;
					UINT32 b = (SOURCE32_B(pix) * ta + DEST_B(dpix) * invta) >> 8;

					*dest = DEST_ASSEMBLE_RGB(r, g, b);
				}
				dest++;
				curu += dudx;
				curv += dvdx;
			}
		}
	}

	/* alpha and/or coloring case */
	else
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);
		UINT32 sa = (UINT32)(256.0f * prim->color.a);

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (sa > 0x100) { if ((INT32)sa < 0) sa = 0; else sa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* loop over cols */
			for (x = setup->startx; x < endx; x++)
			{
				UINT32 pix = GET_TEXEL(palette16a)(&prim->texture, curu, curv);
				UINT32 ta = (pix >> 24) * sa;
				if (ta != 0)
				{
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 invsta = (0x10000 - ta) << 8;
					UINT32 r = (SOURCE32_R(pix) * sr * ta + DEST_R(dpix) * invsta) >> 24;
					UINT32 g = (SOURCE32_G(pix) * sg * ta + DEST_G(dpix) * invsta) >> 24;
					UINT32 b = (SOURCE32_B(pix) * sb * ta + DEST_B(dpix) * invsta) >> 24;

					*dest = DEST_ASSEMBLE_RGB(r, g, b);
				}
				dest++;
				curu += dudx;
				curv += dvdx;
			}
		}
	}
}



/***************************************************************************
    16-BIT YUY RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_quad_yuy16_none - perform
    rasterization of a 16bpp YUY image
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_yuy16_none)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = ycc_to_rgb(GET_TEXEL(yuy16)(&prim->texture, curu, curv));
					*dest++ = SOURCE32_TO_DEST(pix);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = ycc_to_rgb(GET_TEXEL(yuy16)(&prim->texture, curu, curv));
					*dest++ = SOURCE32_TO_DEST(pix);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* coloring-only case */
	else if (IS_OPAQUE(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);

		/* clamp R,G,B to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = ycc_to_rgb(GET_TEXEL(yuy16)(&prim->texture, curu, curv));
					UINT32 r = (SOURCE32_R(pix) * sr) >> 8;
					UINT32 g = (SOURCE32_G(pix) * sg) >> 8;
					UINT32 b = (SOURCE32_B(pix) * sb) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = ycc_to_rgb(GET_TEXEL(yuy16)(&prim->texture, curu, curv));
					UINT32 r = (SOURCE32_R(pix) * sr) >> 8;
					UINT32 g = (SOURCE32_G(pix) * sg) >> 8;
					UINT32 b = (SOURCE32_B(pix) * sb) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 sg = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 sb = (UINT32)(256.0f * prim->color.b * prim->color.a);
		UINT32 invsa = (UINT32)(256.0f * (1.0f - prim->color.a));

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (invsa > 0x100) { if ((INT32)invsa < 0) invsa = 0; else invsa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = ycc_to_rgb(GET_TEXEL(yuy16)(&prim->texture, curu, curv));
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (SOURCE32_R(pix) * sr + DEST_R(dpix) * invsa) >> 8;
					UINT32 g = (SOURCE32_G(pix) * sg + DEST_G(dpix) * invsa) >> 8;
					UINT32 b = (SOURCE32_B(pix) * sb + DEST_B(dpix) * invsa) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = ycc_to_rgb(GET_TEXEL(yuy16)(&prim->texture, curu, curv));
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (SOURCE32_R(pix) * sr + DEST_R(dpix) * invsa) >> 8;
					UINT32 g = (SOURCE32_G(pix) * sg + DEST_G(dpix) * invsa) >> 8;
					UINT32 b = (SOURCE32_B(pix) * sb + DEST_B(dpix) * invsa) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}


/***************************************************************************
    15-BIT RGB QUAD RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_quad_rgb15 - perform rasterization of
    a 15bpp RGB texture
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_rgb15)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb15)(&prim->texture, curu, curv);
					*dest++ = SOURCE15_TO_DEST(pix);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb15)(&prim->texture, curu, curv);
					UINT32 r = palbase[(pix >> 10) & 0x1f] >> SRCSHIFT_R;
					UINT32 g = palbase[(pix >> 5) & 0x1f] >> SRCSHIFT_G;
					UINT32 b = palbase[(pix >> 0) & 0x1f] >> SRCSHIFT_B;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* coloring-only case */
	else if (IS_OPAQUE(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);

		/* clamp R,G,B to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb15)(&prim->texture, curu, curv);
					UINT32 r = (SOURCE15_R(pix) * sr) >> 8;
					UINT32 g = (SOURCE15_G(pix) * sg) >> 8;
					UINT32 b = (SOURCE15_B(pix) * sb) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb15)(&prim->texture, curu, curv);
					UINT32 r = (palbase[(pix >> 10) & 0x1f] * sr) >> (8 + SRCSHIFT_R);
					UINT32 g = (palbase[(pix >> 5) & 0x1f] * sg) >> (8 + SRCSHIFT_G);
					UINT32 b = (palbase[(pix >> 0) & 0x1f] * sb) >> (8 + SRCSHIFT_B);

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 sg = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 sb = (UINT32)(256.0f * prim->color.b * prim->color.a);
		UINT32 invsa = (UINT32)(256.0f * (1.0f - prim->color.a));

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (invsa > 0x100) { if ((INT32)invsa < 0) invsa = 0; else invsa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb15)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (SOURCE15_R(pix) * sr + DEST_R(dpix) * invsa) >> 8;
					UINT32 g = (SOURCE15_G(pix) * sg + DEST_G(dpix) * invsa) >> 8;
					UINT32 b = (SOURCE15_B(pix) * sb + DEST_B(dpix) * invsa) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb15)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = ((palbase[(pix >> 10) & 0x1f] >> SRCSHIFT_R) * sr + DEST_R(dpix) * invsa) >> 8;
					UINT32 g = ((palbase[(pix >> 5) & 0x1f] >> SRCSHIFT_G) * sg + DEST_G(dpix) * invsa) >> 8;
					UINT32 b = ((palbase[(pix >> 0) & 0x1f] >> SRCSHIFT_B) * sb + DEST_B(dpix) * invsa) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}



/***************************************************************************
    32-BIT RGB QUAD RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_quad_rgb32 - perform rasterization of
    a 32bpp RGB texture
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_rgb32)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb32)(&prim->texture, curu, curv);
					*dest++ = SOURCE32_TO_DEST(pix);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb32)(&prim->texture, curu, curv);
					UINT32 r = palbase[(pix >> 16) & 0xff] >> SRCSHIFT_R;
					UINT32 g = palbase[(pix >> 8) & 0xff] >> SRCSHIFT_G;
					UINT32 b = palbase[(pix >> 0) & 0xff] >> SRCSHIFT_B;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* coloring-only case */
	else if (IS_OPAQUE(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);

		/* clamp R,G,B to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb32)(&prim->texture, curu, curv);
					UINT32 r = (SOURCE32_R(pix) * sr) >> 8;
					UINT32 g = (SOURCE32_G(pix) * sg) >> 8;
					UINT32 b = (SOURCE32_B(pix) * sb) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb32)(&prim->texture, curu, curv);
					UINT32 r = (palbase[(pix >> 16) & 0xff] * sr) >> (8 + SRCSHIFT_R);
					UINT32 g = (palbase[(pix >> 8) & 0xff] * sg) >> (8 + SRCSHIFT_G);
					UINT32 b = (palbase[(pix >> 0) & 0xff] * sb) >> (8 + SRCSHIFT_B);

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(prim->color.a))
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 sg = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 sb = (UINT32)(256.0f * prim->color.b * prim->color.a);
		UINT32 invsa = (UINT32)(256.0f * (1.0f - prim->color.a));

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (invsa > 0x100) { if ((INT32)invsa < 0) invsa = 0; else invsa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (SOURCE32_R(pix) * sr + DEST_R(dpix) * invsa) >> 8;
					UINT32 g = (SOURCE32_G(pix) * sg + DEST_G(dpix) * invsa) >> 8;
					UINT32 b = (SOURCE32_B(pix) * sb + DEST_B(dpix) * invsa) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(rgb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = ((palbase[(pix >> 16) & 0xff] >> SRCSHIFT_R) * sr + DEST_R(dpix) * invsa) >> 8;
					UINT32 g = ((palbase[(pix >> 8) & 0xff] >> SRCSHIFT_G) * sg + DEST_G(dpix) * invsa) >> 8;
					UINT32 b = ((palbase[(pix >> 0) & 0xff] >> SRCSHIFT_B) * sb + DEST_B(dpix) * invsa) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}


/*-------------------------------------------------
    draw_quad_rgb32_add - perform
    rasterization by using RGB add
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_rgb32_add)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* simply can't do this without reading from the dest */
	if (NO_DEST_READ)
		return;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = SOURCE32_R(pix) + DEST_R(dpix);
					UINT32 g = SOURCE32_G(pix) + DEST_G(dpix);
					UINT32 b = SOURCE32_B(pix) + DEST_B(dpix);
					r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
					g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
					b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (palbase[(pix >> 16) & 0xff] >> SRCSHIFT_R) + DEST_R(dpix);
					UINT32 g = (palbase[(pix >> 8) & 0xff] >> SRCSHIFT_G) + DEST_G(dpix);
					UINT32 b = (palbase[(pix >> 0) & 0xff] >> SRCSHIFT_B) + DEST_B(dpix);
					r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
					g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
					b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);
		UINT32 sa = (UINT32)(256.0f * prim->color.a);

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (sa > 0x100) { if ((INT32)sa < 0) sa = 0; else sa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = ((SOURCE32_R(pix) * sr * sa) >> 16) + DEST_R(dpix);
					UINT32 g = ((SOURCE32_G(pix) * sg * sa) >> 16) + DEST_G(dpix);
					UINT32 b = ((SOURCE32_B(pix) * sb * sa) >> 16) + DEST_B(dpix);
					r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
					g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
					b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = ((palbase[(pix >> 16) & 0xff] * sr * sa) >> (16 + SRCSHIFT_R)) + DEST_R(dpix);
					UINT32 g = ((palbase[(pix >> 8) & 0xff] * sr * sa) >> (16 + SRCSHIFT_R)) + DEST_G(dpix);
					UINT32 b = ((palbase[(pix >> 0) & 0xff] * sr * sa) >> (16 + SRCSHIFT_R)) + DEST_B(dpix);
					r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
					g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
					b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}



/***************************************************************************
    32-BIT ARGB QUAD RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    draw_quad_argb32_alpha - perform
    rasterization using standard alpha blending
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_argb32_alpha)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = pix >> 24;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 invta = 0x100 - ta;
						UINT32 r = (SOURCE32_R(pix) * ta + DEST_R(dpix) * invta) >> 8;
						UINT32 g = (SOURCE32_G(pix) * ta + DEST_G(dpix) * invta) >> 8;
						UINT32 b = (SOURCE32_B(pix) * ta + DEST_B(dpix) * invta) >> 8;

						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = pix >> 24;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 invta = 0x100 - ta;
						UINT32 r = ((palbase[(pix >> 16) & 0xff] >> SRCSHIFT_R) * ta + DEST_R(dpix) * invta) >> 8;
						UINT32 g = ((palbase[(pix >> 8) & 0xff] >> SRCSHIFT_G) * ta + DEST_G(dpix) * invta) >> 8;
						UINT32 b = ((palbase[(pix >> 0) & 0xff] >> SRCSHIFT_B) * ta + DEST_B(dpix) * invta) >> 8;

						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);
		UINT32 sa = (UINT32)(256.0f * prim->color.a);

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (sa > 0x100) { if ((INT32)sa < 0) sa = 0; else sa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = (pix >> 24) * sa;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 invsta = (0x10000 - ta) << 8;
						UINT32 r = (SOURCE32_R(pix) * sr * ta + DEST_R(dpix) * invsta) >> 24;
						UINT32 g = (SOURCE32_G(pix) * sg * ta + DEST_G(dpix) * invsta) >> 24;
						UINT32 b = (SOURCE32_B(pix) * sb * ta + DEST_B(dpix) * invsta) >> 24;

						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}


			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = (pix >> 24) * sa;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 invsta = (0x10000 - ta) << 8;
						UINT32 r = ((palbase[(pix >> 16) & 0xff] >> SRCSHIFT_R) * sr * ta + DEST_R(dpix) * invsta) >> 24;
						UINT32 g = ((palbase[(pix >> 8) & 0xff] >> SRCSHIFT_G) * sg * ta + DEST_G(dpix) * invsta) >> 24;
						UINT32 b = ((palbase[(pix >> 0) & 0xff] >> SRCSHIFT_B) * sb * ta + DEST_B(dpix) * invsta) >> 24;

						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}


/*-------------------------------------------------
    draw_quad_argb32_multiply - perform
    rasterization using RGB multiply
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_argb32_multiply)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* simply can't do this without reading from the dest */
	if (NO_DEST_READ)
		return;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (SOURCE32_R(pix) * DEST_R(dpix)) >> (8 - SRCSHIFT_R);
					UINT32 g = (SOURCE32_G(pix) * DEST_G(dpix)) >> (8 - SRCSHIFT_G);
					UINT32 b = (SOURCE32_B(pix) * DEST_B(dpix)) >> (8 - SRCSHIFT_B);

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (palbase[(pix >> 16) & 0xff] * DEST_R(dpix)) >> 8;
					UINT32 g = (palbase[(pix >> 8) & 0xff] * DEST_G(dpix)) >> 8;
					UINT32 b = (palbase[(pix >> 0) & 0xff] * DEST_B(dpix)) >> 8;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r * prim->color.a);
		UINT32 sg = (UINT32)(256.0f * prim->color.g * prim->color.a);
		UINT32 sb = (UINT32)(256.0f * prim->color.b * prim->color.a);

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (SOURCE32_R(pix) * sr * DEST_R(dpix)) >> (16 - SRCSHIFT_R);
					UINT32 g = (SOURCE32_G(pix) * sg * DEST_G(dpix)) >> (16 - SRCSHIFT_G);
					UINT32 b = (SOURCE32_B(pix) * sb * DEST_B(dpix)) >> (16 - SRCSHIFT_B);

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 dpix = NO_DEST_READ ? 0 : *dest;
					UINT32 r = (palbase[(pix >> 16) & 0xff] * sr * DEST_R(dpix)) >> 16;
					UINT32 g = (palbase[(pix >> 8) & 0xff] * sg * DEST_G(dpix)) >> 16;
					UINT32 b = (palbase[(pix >> 0) & 0xff] * sb * DEST_B(dpix)) >> 16;

					*dest++ = DEST_ASSEMBLE_RGB(r, g, b);
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}


/*-------------------------------------------------
    draw_quad_argb32_add - perform
    rasterization by using RGB add
-------------------------------------------------*/

static void FUNC_PREFIX(draw_quad_argb32_add)(const render_primitive *prim, void *dstdata, UINT32 pitch, quad_setup_data *setup)
{
	const rgb_t *palbase = prim->texture.palette;
	INT32 dudx = setup->dudx;
	INT32 dvdx = setup->dvdx;
	INT32 endx = setup->endx;
	INT32 x, y;

	/* simply can't do this without reading from the dest */
	if (NO_DEST_READ)
		return;

	/* fast case: no coloring, no alpha */
	if (prim->color.r >= 1.0f && prim->color.g >= 1.0f && prim->color.b >= 1.0f && IS_OPAQUE(prim->color.a))
	{
		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = pix >> 24;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 r = ((SOURCE32_R(pix) * ta) >> 8) + DEST_R(dpix);
						UINT32 g = ((SOURCE32_G(pix) * ta) >> 8) + DEST_G(dpix);
						UINT32 b = ((SOURCE32_B(pix) * ta) >> 8) + DEST_B(dpix);
						r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
						g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
						b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = pix >> 24;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 r = ((palbase[(pix >> 16) & 0xff] * ta) >> (8 + SRCSHIFT_R)) + DEST_R(dpix);
						UINT32 g = ((palbase[(pix >> 8) & 0xff] * ta) >> (8 + SRCSHIFT_G)) + DEST_G(dpix);
						UINT32 b = ((palbase[(pix >> 0) & 0xff] * ta) >> (8 + SRCSHIFT_B)) + DEST_B(dpix);
						r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
						g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
						b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}

	/* alpha and/or coloring case */
	else
	{
		UINT32 sr = (UINT32)(256.0f * prim->color.r);
		UINT32 sg = (UINT32)(256.0f * prim->color.g);
		UINT32 sb = (UINT32)(256.0f * prim->color.b);
		UINT32 sa = (UINT32)(256.0f * prim->color.a);

		/* clamp R,G,B and inverse A to 0-256 range */
		if (sr > 0x100) { if ((INT32)sr < 0) sr = 0; else sr = 0x100; }
		if (sg > 0x100) { if ((INT32)sg < 0) sg = 0; else sg = 0x100; }
		if (sb > 0x100) { if ((INT32)sb < 0) sb = 0; else sb = 0x100; }
		if (sa > 0x100) { if ((INT32)sa < 0) sa = 0; else sa = 0x100; }

		/* loop over rows */
		for (y = setup->starty; y < setup->endy; y++)
		{
			PIXEL_TYPE *dest = (PIXEL_TYPE *)dstdata + y * pitch + setup->startx;
			INT32 curu = setup->startu + (y - setup->starty) * setup->dudy;
			INT32 curv = setup->startv + (y - setup->starty) * setup->dvdy;

			/* no lookup case */
			if (palbase == NULL)
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = (pix >> 24) * sa;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 r = ((SOURCE32_R(pix) * sr * ta) >> 24) + DEST_R(dpix);
						UINT32 g = ((SOURCE32_G(pix) * sg * ta) >> 24) + DEST_G(dpix);
						UINT32 b = ((SOURCE32_B(pix) * sb * ta) >> 24) + DEST_B(dpix);
						r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
						g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
						b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}

			/* lookup case */
			else
			{
				/* loop over cols */
				for (x = setup->startx; x < endx; x++)
				{
					UINT32 pix = GET_TEXEL(argb32)(&prim->texture, curu, curv);
					UINT32 ta = (pix >> 24) * sa;
					if (ta != 0)
					{
						UINT32 dpix = NO_DEST_READ ? 0 : *dest;
						UINT32 r = ((palbase[(pix >> 16) & 0xff] * sr * ta) >> (24 + SRCSHIFT_R)) + DEST_R(dpix);
						UINT32 g = ((palbase[(pix >> 8) & 0xff] * sr * ta) >> (24 + SRCSHIFT_R)) + DEST_G(dpix);
						UINT32 b = ((palbase[(pix >> 0) & 0xff] * sr * ta) >> (24 + SRCSHIFT_R)) + DEST_B(dpix);
						r = (r | -(r >> (8 - SRCSHIFT_R))) & (0xff >> SRCSHIFT_R);
						g = (g | -(g >> (8 - SRCSHIFT_G))) & (0xff >> SRCSHIFT_G);
						b = (b | -(b >> (8 - SRCSHIFT_B))) & (0xff >> SRCSHIFT_B);
						*dest = DEST_ASSEMBLE_RGB(r, g, b);
					}
					dest++;
					curu += dudx;
					curv += dvdx;
				}
			}
		}
	}
}



/***************************************************************************
    CORE QUAD RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    setup_and_draw_textured_quad - perform setup
    and then dispatch to a texture-mode-specific
    drawing routine
-------------------------------------------------*/

static void FUNC_PREFIX(setup_and_draw_textured_quad)(const render_primitive *prim, void *dstdata, INT32 width, INT32 height, UINT32 pitch)
{
	float fdudx, fdvdx, fdudy, fdvdy;
	quad_setup_data setup;

	assert(prim->bounds.x0 <= prim->bounds.x1);
	assert(prim->bounds.y0 <= prim->bounds.y1);

	/* determine U/V deltas */
	fdudx = (prim->texcoords.tr.u - prim->texcoords.tl.u) / (prim->bounds.x1 - prim->bounds.x0);
	fdvdx = (prim->texcoords.tr.v - prim->texcoords.tl.v) / (prim->bounds.x1 - prim->bounds.x0);
	fdudy = (prim->texcoords.bl.u - prim->texcoords.tl.u) / (prim->bounds.y1 - prim->bounds.y0);
	fdvdy = (prim->texcoords.bl.v - prim->texcoords.tl.v) / (prim->bounds.y1 - prim->bounds.y0);

	/* clamp to integers */
	setup.startx = round_nearest(prim->bounds.x0);
	setup.starty = round_nearest(prim->bounds.y0);
	setup.endx = round_nearest(prim->bounds.x1);
	setup.endy = round_nearest(prim->bounds.y1);

	/* ensure we fit */
	if (setup.startx < 0) setup.startx = 0;
	if (setup.startx >= width) setup.startx = width;
	if (setup.endx < 0) setup.endx = 0;
	if (setup.endx >= width) setup.endx = width;
	if (setup.starty < 0) setup.starty = 0;
	if (setup.starty >= height) setup.starty = height;
	if (setup.endy < 0) setup.endy = 0;
	if (setup.endy >= height) setup.endy = height;

	/* compute start and delta U,V coordinates now */
	setup.dudx = round_nearest(65536.0f * (float)prim->texture.width * fdudx);
	setup.dvdx = round_nearest(65536.0f * (float)prim->texture.height * fdvdx);
	setup.dudy = round_nearest(65536.0f * (float)prim->texture.width * fdudy);
	setup.dvdy = round_nearest(65536.0f * (float)prim->texture.height * fdvdy);
	setup.startu = round_nearest(65536.0f * (float)prim->texture.width * prim->texcoords.tl.u);
	setup.startv = round_nearest(65536.0f * (float)prim->texture.height * prim->texcoords.tl.v);

	/* advance U/V to the middle of the first texel */
	setup.startu += (setup.dudx + setup.dudy) / 2;
	setup.startv += (setup.dvdx + setup.dvdy) / 2;

	/* if we're bilinear filtering, we need to offset u/v by half a texel */
	if (BILINEAR_FILTER)
	{
		setup.startu -= 0x8000;
		setup.startv -= 0x8000;
	}

	/* render based on the texture coordinates */
	switch (prim->flags & (PRIMFLAG_TEXFORMAT_MASK | PRIMFLAG_BLENDMODE_MASK))
	{
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
			FUNC_PREFIX(draw_quad_palette16_none)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
			FUNC_PREFIX(draw_quad_palette16_add)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTEA16) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
			FUNC_PREFIX(draw_quad_palettea16_alpha)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
			FUNC_PREFIX(draw_quad_yuy16_none)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB15) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB15) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
			FUNC_PREFIX(draw_quad_rgb15)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
			FUNC_PREFIX(draw_quad_rgb32)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
			FUNC_PREFIX(draw_quad_rgb32_add)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
			FUNC_PREFIX(draw_quad_argb32_alpha)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY):
			FUNC_PREFIX(draw_quad_argb32_multiply)(prim, dstdata, pitch, &setup);
			break;

		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32) | PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
			FUNC_PREFIX(draw_quad_argb32_add)(prim, dstdata, pitch, &setup);
			break;

		default:
			fatalerror("Unknown texformat(%d)/blendmode(%d) combo\n", PRIMFLAG_GET_TEXFORMAT(prim->flags), PRIMFLAG_GET_BLENDMODE(prim->flags));
			break;
	}
}



/***************************************************************************
    PRIMARY ENTRY POINT
***************************************************************************/

/*-------------------------------------------------
    draw_primitives - draw a series of primitives
    using a software rasterizer
-------------------------------------------------*/

static void FUNC_PREFIX(draw_primitives)(const render_primitive_list &primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch)
{
	const render_primitive *prim;

	/* loop over the list and render each element */
	for (prim = primlist.first(); prim != NULL; prim = prim->next())
		switch (prim->type)
		{
			case render_primitive::LINE:
				FUNC_PREFIX(draw_line)(prim, dstdata, width, height, pitch);
				break;

			case render_primitive::QUAD:
				if (!prim->texture.base)
					FUNC_PREFIX(draw_rect)(prim, dstdata, width, height, pitch);
				else
					FUNC_PREFIX(setup_and_draw_textured_quad)(prim, dstdata, width, height, pitch);
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
}



/***************************************************************************
    MACRO UNDOING
***************************************************************************/

#undef SOURCE15_R
#undef SOURCE15_G
#undef SOURCE15_B

#undef SOURCE32_R
#undef SOURCE32_G
#undef SOURCE32_B

#undef DEST_ASSEMBLE_RGB
#undef DEST_RGB_TO_PIXEL

#undef DEST_R
#undef DEST_G
#undef DEST_B

#undef SOURCE15_TO_DEST
#undef SOURCE32_TO_DEST

#undef FUNC_PREFIX
#undef PIXEL_TYPE

#undef SRCSHIFT_R
#undef SRCSHIFT_G
#undef SRCSHIFT_B

#undef DSTSHIFT_R
#undef DSTSHIFT_G
#undef DSTSHIFT_B

#undef NO_DEST_READ

#undef VARIABLE_SHIFT
