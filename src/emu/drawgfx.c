/*********************************************************************

    drawgfx.c

    Generic graphic functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "drawgfxm.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* if this line errors during compile, the size of NO_PRIORITY is wrong and I need to use something else */
UINT8 no_priority_size_is_wrong[2 * (sizeof(NO_PRIORITY) == 3) - 1];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void decodechar(const gfx_element *gfx, UINT32 code, const UINT8 *src);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    readbit - read a single bit from a base
    offset
-------------------------------------------------*/

INLINE int readbit(const UINT8 *src, unsigned int bitnum)
{
	return src[bitnum / 8] & (0x80 >> (bitnum % 8));
}


/*-------------------------------------------------
    normalize_xscroll - normalize an X scroll
    value for a bitmap to be positive and less
    than the width
-------------------------------------------------*/

INLINE INT32 normalize_xscroll(bitmap_t *bitmap, INT32 xscroll)
{
	return (xscroll >= 0) ? xscroll % bitmap->width : (bitmap->width - (-xscroll) % bitmap->width);
}


/*-------------------------------------------------
    normalize_yscroll - normalize a Y scroll
    value for a bitmap to be positive and less
    than the height
-------------------------------------------------*/

INLINE INT32 normalize_yscroll(bitmap_t *bitmap, INT32 yscroll)
{
	return (yscroll >= 0) ? yscroll % bitmap->height : (bitmap->height - (-yscroll) % bitmap->height);
}



/***************************************************************************
    GRAPHICS ELEMENTS
***************************************************************************/

/*-------------------------------------------------
    gfx_init - allocate memory for the graphics
    elements referenced by a machine
-------------------------------------------------*/

void gfx_init(running_machine *machine)
{
	const gfx_decode_entry *gfxdecodeinfo = machine->config->m_gfxdecodeinfo;
	int curgfx;

	/* skip if nothing to do */
	if (gfxdecodeinfo == NULL)
		return;

	/* loop over all elements */
	for (curgfx = 0; curgfx < MAX_GFX_ELEMENTS && gfxdecodeinfo[curgfx].gfxlayout != NULL; curgfx++)
	{
		const gfx_decode_entry *gfxdecode = &gfxdecodeinfo[curgfx];
		const region_info *region = (gfxdecode->memory_region != NULL) ? machine->region(gfxdecode->memory_region) : NULL;
		UINT32 region_length = (region != NULL) ? (8 * region->bytes()) : 0;
		const UINT8 *region_base = (region != NULL) ? region->base() : NULL;
		UINT32 xscale = (gfxdecode->xscale == 0) ? 1 : gfxdecode->xscale;
		UINT32 yscale = (gfxdecode->yscale == 0) ? 1 : gfxdecode->yscale;
		UINT32 *extpoffs, extxoffs[MAX_ABS_GFX_SIZE], extyoffs[MAX_ABS_GFX_SIZE];
		const gfx_layout *gl = gfxdecode->gfxlayout;
		int israw = (gl->planeoffset[0] == GFX_RAW);
		int planes = gl->planes;
		UINT32 width = gl->width;
		UINT32 height = gl->height;
		UINT32 total = gl->total;
		UINT32 charincrement = gl->charincrement;
		gfx_layout glcopy;
		int j;

		/* make a copy of the layout */
		glcopy = *gfxdecode->gfxlayout;

		/* copy the X and Y offsets into temporary arrays */
		memcpy(extxoffs, glcopy.xoffset, sizeof(glcopy.xoffset));
		memcpy(extyoffs, glcopy.yoffset, sizeof(glcopy.yoffset));

		/* if there are extended offsets, copy them over top */
		if (glcopy.extxoffs != NULL)
			memcpy(extxoffs, glcopy.extxoffs, glcopy.width * sizeof(extxoffs[0]));
		if (glcopy.extyoffs != NULL)
			memcpy(extyoffs, glcopy.extyoffs, glcopy.height * sizeof(extyoffs[0]));

		/* always use the extended offsets here */
		glcopy.extxoffs = extxoffs;
		glcopy.extyoffs = extyoffs;

		extpoffs = glcopy.planeoffset;

		/* expand X and Y by the scale factors */
		if (xscale > 1)
		{
			width *= xscale;
			for (j = width - 1; j >= 0; j--)
				extxoffs[j] = extxoffs[j / xscale];
		}
		if (yscale > 1)
		{
			height *= yscale;
			for (j = height - 1; j >= 0; j--)
				extyoffs[j] = extyoffs[j / yscale];
		}

		/* if the character count is a region fraction, compute the effective total */
		if (IS_FRAC(total))
		{
			assert(region_length != 0);
			total = region_length / charincrement * FRAC_NUM(total) / FRAC_DEN(total);
		}

		/* for non-raw graphics, decode the X and Y offsets */
		if (!israw)
		{
			/* loop over all the planes, converting fractions */
			for (j = 0; j < planes; j++)
			{
				UINT32 value1 = extpoffs[j];
				if (IS_FRAC(value1))
				{
					assert(region_length != 0);
					extpoffs[j] = FRAC_OFFSET(value1) + region_length * FRAC_NUM(value1) / FRAC_DEN(value1);
				}
			}

			/* loop over all the X/Y offsets, converting fractions */
			for (j = 0; j < width; j++)
			{
				UINT32 value2 = extxoffs[j];
				if (IS_FRAC(value2))
				{
					assert(region_length != 0);
					extxoffs[j] = FRAC_OFFSET(value2) + region_length * FRAC_NUM(value2) / FRAC_DEN(value2);
				}
			}

			for (j = 0; j < height; j++)
			{
				UINT32 value3 = extyoffs[j];
				if (IS_FRAC(value3))
				{
					assert(region_length != 0);
					extyoffs[j] = FRAC_OFFSET(value3) + region_length * FRAC_NUM(value3) / FRAC_DEN(value3);
				}
			}
		}

		/* otherwise, just use the line modulo */
		else
		{
			int base = gfxdecode->start;
			int end = region_length/8;
			int linemod = gl->yoffset[0];
			while (total > 0)
			{
				int elementbase = base + (total - 1) * charincrement / 8;
				int lastpixelbase = elementbase + height * linemod / 8 - 1;
				if (lastpixelbase < end)
					break;
				total--;
			}
		}

		/* update glcopy */
		glcopy.width = width;
		glcopy.height = height;
		glcopy.total = total;

		/* allocate the graphics */
		machine->gfx[curgfx] = gfx_element_alloc(machine, &glcopy, (region_base != NULL) ? region_base + gfxdecode->start : NULL, gfxdecode->total_color_codes, gfxdecode->color_codes_start);
	}
}



/*-------------------------------------------------
    gfx_element_alloc - allocate a gfx_element structure
    based on a given layout
-------------------------------------------------*/

gfx_element *gfx_element_alloc(running_machine *machine, const gfx_layout *gl, const UINT8 *srcdata, UINT32 total_colors, UINT32 color_base)
{
	int israw = (gl->planeoffset[0] == GFX_RAW);
	int planes = gl->planes;
	UINT16 width = gl->width;
	UINT16 height = gl->height;
	UINT32 total = gl->total;
	gfx_element *gfx;

	/* allocate memory for the gfx_element structure */
	gfx = auto_alloc_clear(machine, gfx_element);

	/* fill in the data */
	gfx->width = width;
	gfx->height = height;

	gfx->origwidth = width;
	gfx->origheight = height;
	gfx->total_elements = total;

	gfx->color_base = color_base;
	gfx->color_depth = 1 << planes;
	gfx->color_granularity = 1 << planes;
	gfx->total_colors = total_colors;

	gfx->srcdata = srcdata;
	gfx->machine = machine;

	/* copy the layout */
	gfx->layout = *gl;
	if (gfx->layout.extxoffs != NULL)
	{
		if (gfx->layout.width < ARRAY_LENGTH(gfx->layout.xoffset))
		{
			memcpy(gfx->layout.xoffset, gfx->layout.extxoffs, sizeof(gfx->layout.xoffset[0]) * gfx->layout.width);
			gfx->layout.extxoffs = NULL;
		}
		else
		{
			UINT32 *buffer = auto_alloc_array(machine, UINT32, gfx->layout.width);
			memcpy(buffer, gfx->layout.extxoffs, sizeof(gfx->layout.extxoffs[0]) * gfx->layout.width);
			gfx->layout.extxoffs = buffer;
		}
	}

	if (gfx->layout.extyoffs != NULL)
	{
		if (gfx->layout.height < ARRAY_LENGTH(gfx->layout.yoffset))
		{
			memcpy(gfx->layout.yoffset, gfx->layout.extyoffs, sizeof(gfx->layout.yoffset[0]) * gfx->layout.height);
			gfx->layout.extyoffs = NULL;
		}
		else
		{
			UINT32 *buffer = auto_alloc_array(machine, UINT32, gfx->layout.height);
			memcpy(buffer, gfx->layout.extyoffs, sizeof(gfx->layout.extyoffs[0]) * gfx->layout.height);
			gfx->layout.extyoffs = buffer;
		}
	}

	/* allocate a pen usage array for entries with 32 pens or less */
	if (gfx->color_depth <= 32)
		gfx->pen_usage = auto_alloc_array(machine, UINT32, gfx->total_elements);

	/* allocate a dirty array */
	gfx->dirty = auto_alloc_array(machine, UINT8, gfx->total_elements);
	memset(gfx->dirty, 1, gfx->total_elements * sizeof(*gfx->dirty));

	/* raw graphics case */
	if (israw)
	{
		/* modulos are determined for us by the layout */
		gfx->line_modulo = (gl->extyoffs ? gl->extyoffs[0] : gl->yoffset[0]) / 8;
		gfx->char_modulo = gl->charincrement / 8;

		/* don't free the data because we will get a pointer at decode time */
		gfx->flags |= GFX_ELEMENT_DONT_FREE;
		if (planes <= 4)
			gfx->flags |= GFX_ELEMENT_PACKED;

		/* RAW graphics must have a pointer up front */
		gfx->gfxdata = (UINT8 *)gfx->srcdata;
	}

	/* decoded graphics case */
	else
	{
		/* we get to pick our own modulos */
		gfx->line_modulo = gfx->origwidth;
		gfx->char_modulo = gfx->line_modulo * gfx->origheight;

		/* allocate memory for the data */
		gfx->gfxdata = auto_alloc_array(machine, UINT8, gfx->total_elements * gfx->char_modulo);
	}

	return gfx;
}


/*-------------------------------------------------
    gfx_element_decode - update a single code in
    a gfx_element
-------------------------------------------------*/

void gfx_element_decode(const gfx_element *gfx, UINT32 code)
{
	decodechar(gfx, code, gfx->srcdata);
}


/*-------------------------------------------------
    gfx_element_free - free a gfx_element
-------------------------------------------------*/

void gfx_element_free(gfx_element *gfx)
{
	/* ignore NULL frees */
	if (gfx == NULL)
		return;

	/* free our data */
	auto_free(gfx->machine, gfx->layout.extyoffs);
	auto_free(gfx->machine, gfx->layout.extxoffs);
	auto_free(gfx->machine, gfx->pen_usage);
	auto_free(gfx->machine, gfx->dirty);
	auto_free(gfx->machine, gfx->gfxdata);
	auto_free(gfx->machine, gfx);
}


/*-------------------------------------------------
    gfx_element_build_temporary - create a
    temporary one-off gfx_element
-------------------------------------------------*/

void gfx_element_build_temporary(gfx_element *gfx, running_machine *machine, UINT8 *base, UINT32 width, UINT32 height, UINT32 rowbytes, UINT32 color_base, UINT32 color_granularity, UINT32 flags)
{
	static UINT8 not_dirty = 0;

	gfx->width = width;
	gfx->height = height;
	gfx->startx = 0;
	gfx->starty = 0;

	gfx->origwidth = width;
	gfx->origheight = height;
	gfx->flags = flags;
	gfx->total_elements = 1;

	gfx->color_base = color_base;
	gfx->color_depth = color_granularity;
	gfx->color_granularity = color_granularity;
	gfx->total_colors = (machine->total_colors() - color_base) / color_granularity;

	gfx->pen_usage = NULL;

	gfx->gfxdata = base;
	gfx->line_modulo = rowbytes;
	gfx->char_modulo = 0;
	gfx->srcdata = base;
	gfx->dirty = &not_dirty;
	gfx->dirtyseq = 0;

	gfx->machine = machine;
}


/*-------------------------------------------------
    calc_penusage - calculate the pen usage for
    a given graphics tile
-------------------------------------------------*/

static void calc_penusage(const gfx_element *gfx, UINT32 code)
{
	const UINT8 *dp = gfx->gfxdata + code * gfx->char_modulo;
	UINT32 usage = 0;
	int x, y;

	/* if nothing allocated, don't do it */
	if (gfx->pen_usage == NULL)
		return;

	/* packed case */
	if (gfx->flags & GFX_ELEMENT_PACKED)
		for (y = 0; y < gfx->origheight; y++)
		{
			for (x = 0; x < gfx->origwidth/2; x++)
				usage |= (1 << (dp[x] & 0x0f)) | (1 << (dp[x] >> 4));

			dp += gfx->line_modulo;
		}

	/* unpacked case */
	else
		for (y = 0; y < gfx->origheight; y++)
		{
			for (x = 0; x < gfx->origwidth; x++)
				usage |= 1 << dp[x];

			dp += gfx->line_modulo;
		}

	/* store the final result */
	gfx->pen_usage[code] = usage;
}


/*-------------------------------------------------
    decodechar - decode a single character based
    on a specified layout
-------------------------------------------------*/

void decodechar(const gfx_element *gfx, UINT32 code, const UINT8 *src)
{
	const gfx_layout *gl = &gfx->layout;
	int israw = (gl->planeoffset[0] == GFX_RAW);
	int planes = gl->planes;
	UINT32 charincrement = gl->charincrement;
	const UINT32 *poffset = gl->planeoffset;
	const UINT32 *xoffset = gl->extxoffs ? gl->extxoffs : gl->xoffset;
	const UINT32 *yoffset = gl->extyoffs ? gl->extyoffs : gl->yoffset;
	UINT8 *dp = gfx->gfxdata + code * gfx->char_modulo;
	int plane, x, y;

	if (!israw)
	{
		/* zap the data to 0 */
		memset(dp, 0, gfx->char_modulo);

		/* packed case */
		if (gfx->flags & GFX_ELEMENT_PACKED)
			for (plane = 0; plane < planes; plane++)
			{
				int planebit = 1 << (planes - 1 - plane);
				int planeoffs = code * charincrement + poffset[plane];

				for (y = 0; y < gfx->origheight; y++)
				{
					int yoffs = planeoffs + yoffset[y];

					dp = gfx->gfxdata + code * gfx->char_modulo + y * gfx->line_modulo;
					for (x = 0; x < gfx->origwidth; x += 2)
					{
						if (readbit(src, yoffs + xoffset[x+0]))
							dp[x+0] |= planebit;
						if (readbit(src, yoffs + xoffset[x+1]))
							dp[x+1] |= planebit;
					}
				}
			}

		/* unpacked case */
		else
			for (plane = 0; plane < planes; plane++)
			{
				int planebit = 1 << (planes - 1 - plane);
				int planeoffs = code * charincrement + poffset[plane];

				for (y = 0; y < gfx->origheight; y++)
				{
					int yoffs = planeoffs + yoffset[y];

					dp = gfx->gfxdata + code * gfx->char_modulo + y * gfx->line_modulo;
					for (x = 0; x < gfx->origwidth; x++)
						if (readbit(src, yoffs + xoffset[x]))
							dp[x] |= planebit;
				}
			}
	}

	/* compute pen usage */
	calc_penusage(gfx, code);

	/* no longer dirty */
	gfx->dirty[code] = 0;
}


/***************************************************************************
    DRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    drawgfx_opaque - render a gfx element with
    no transparency
-------------------------------------------------*/

void drawgfx_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfx_transpen - render a gfx element with
    a single transparent pen
-------------------------------------------------*/

void drawgfx_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 transpen)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* special case invalid pens to opaque */
	if (transpen > 0xff)
	{
		drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~(1 << transpen)) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & (1 << transpen)) == 0)
		{
			drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);
			return;
		}
	}

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN, NO_PRIORITY);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfx_transpen_raw - render a gfx element
    with a single transparent pen and no color
    lookups
-------------------------------------------------*/

void drawgfx_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 transpen)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfx_transmask - render a gfx element
    with a multiple transparent pens provided as
    a mask
-------------------------------------------------*/

void drawgfx_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 transmask)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* special case 0 mask to opaque */
	if (transmask == 0)
	{
		drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~transmask) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & transmask) == 0)
		{
			drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);
			return;
		}
	}

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSMASK, NO_PRIORITY);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfx_transtable - render a gfx element
    using a table to look up which pens are
    transparent, opaque, or shadowing
-------------------------------------------------*/

void drawgfx_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		const UINT8 *pentable, const pen_t *shadowtable)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);
	assert(pentable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSTABLE16, NO_PRIORITY);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfx_alpha - render a gfx element with
    a single transparent pen, alpha blending the
    remaining pixels with a fixed alpha value
-------------------------------------------------*/

void drawgfx_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 transpen, UINT8 alpha)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* special case alpha = 0xff */
	if (alpha == 0xff)
	{
		drawgfx_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, transpen);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN_ALPHA16, NO_PRIORITY);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32, NO_PRIORITY);
}



/***************************************************************************
    DRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    drawgfxzoom_opaque - render a scaled gfx
    element with no transparency
-------------------------------------------------*/

void drawgfxzoom_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfxzoom_transpen - render a scaled gfx
    element with a single transparent pen
-------------------------------------------------*/

void drawgfxzoom_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 transpen)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		drawgfx_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, transpen);
		return;
	}

	/* special case invalid pens to opaque */
	if (transpen > 0xff)
	{
		drawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~(1 << transpen)) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & (1 << transpen)) == 0)
		{
			drawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley);
			return;
		}
	}

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN, NO_PRIORITY);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfxzoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups
-------------------------------------------------*/

void drawgfxzoom_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 transpen)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		drawgfx_transpen_raw(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, transpen);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfxzoom_transmask - render a scaled gfx
    element with a multiple transparent pens
    provided as a mask
-------------------------------------------------*/

void drawgfxzoom_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 transmask)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		drawgfx_transmask(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, transmask);
		return;
	}

	/* special case 0 mask to opaque */
	if (transmask == 0)
	{
		drawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~transmask) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & transmask) == 0)
		{
			drawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley);
			return;
		}
	}

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSMASK, NO_PRIORITY);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfxzoom_transtable - render a scaled gfx
    element using a table to look up which pens
    are transparent, opaque, or shadowing
-------------------------------------------------*/

void drawgfxzoom_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, const UINT8 *pentable, const pen_t *shadowtable)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		drawgfx_transtable(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, pentable, shadowtable);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);
	assert(pentable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSTABLE16, NO_PRIORITY);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32, NO_PRIORITY);
}


/*-------------------------------------------------
    drawgfxzoom_alpha - render a scaled gfx element
    with a single transparent pen, alpha blending
    the remaining pixels with a fixed alpha value
-------------------------------------------------*/

void drawgfxzoom_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 transpen, UINT8 alpha)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		drawgfx_alpha(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, transpen, alpha);
		return;
	}

	/* special case alpha = 0xff */
	if (alpha == 0xff)
	{
		drawgfxzoom_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley, transpen);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN_ALPHA16, NO_PRIORITY);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32, NO_PRIORITY);
}



/***************************************************************************
    PDRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    pdrawgfx_opaque - render a gfx element with
    no transparency, checking against the priority
    bitmap
-------------------------------------------------*/

void pdrawgfx_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_t *priority, UINT32 pmask)
{
	const pen_t *paldata;

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE_PRIORITY, UINT8);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfx_transpen - render a gfx element with
    a single transparent pen, checking against the
    priority bitmap
-------------------------------------------------*/

void pdrawgfx_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_t *priority, UINT32 pmask, UINT32 transpen)
{
	const pen_t *paldata;

	/* special case invalid pens to opaque */
	if (transpen > 0xff)
	{
		pdrawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~(1 << transpen)) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & (1 << transpen)) == 0)
		{
			pdrawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask);
			return;
		}
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN_PRIORITY, UINT8);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfx_transpen_raw - render a gfx element
    with a single transparent pen and no color
    lookups, checking against the priority bitmap
-------------------------------------------------*/

void pdrawgfx_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_t *priority, UINT32 pmask, UINT32 transpen)
{
	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfx_transmask - render a gfx element
    with a multiple transparent pens provided as
    a mask, checking against the priority bitmap
-------------------------------------------------*/

void pdrawgfx_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_t *priority, UINT32 pmask, UINT32 transmask)
{
	const pen_t *paldata;

	/* special case 0 mask to opaque */
	if (transmask == 0)
	{
		pdrawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~transmask) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & transmask) == 0)
		{
			pdrawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask);
			return;
		}
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSMASK_PRIORITY, UINT8);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfx_transtable - render a gfx element
    using a table to look up which pens are
    transparent, opaque, or shadowing, checking
    against the priority bitmap
-------------------------------------------------*/

void pdrawgfx_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_t *priority, UINT32 pmask, const UINT8 *pentable, const pen_t *shadowtable)
{
	const pen_t *paldata;

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);
	assert(pentable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSTABLE16_PRIORITY, UINT8);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfx_alpha - render a gfx element with
    a single transparent pen, alpha blending the
    remaining pixels with a fixed alpha value,
    checking against the priority bitmap
-------------------------------------------------*/

void pdrawgfx_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_t *priority, UINT32 pmask, UINT32 transpen, UINT8 alpha)
{
	const pen_t *paldata;

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* special case alpha = 0xff */
	if (alpha == 0xff)
	{
		pdrawgfx_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, transpen);
		return;
	}

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFX_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN_ALPHA16_PRIORITY, UINT8);
	else
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY, UINT8);
}



/***************************************************************************
    PDRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    pdrawgfxzoom_opaque - render a scaled gfx
    element with no transparency, checking against
    the priority bitmap
-------------------------------------------------*/

void pdrawgfxzoom_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask)
{
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE_PRIORITY, UINT8);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfxzoom_transpen - render a scaled gfx
    element with a single transparent pen,
    checking against the priority bitmap
-------------------------------------------------*/

void pdrawgfxzoom_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask,
		UINT32 transpen)
{
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, transpen);
		return;
	}

	/* special case invalid pens to opaque */
	if (transpen > 0xff)
	{
		pdrawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~(1 << transpen)) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & (1 << transpen)) == 0)
		{
			pdrawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
			return;
		}
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN_PRIORITY, UINT8);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfxzoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups, checking against the priority
    bitmap
-------------------------------------------------*/

void pdrawgfxzoom_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask,
		UINT32 transpen)
{
	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_transpen_raw(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, transpen);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfxzoom_transmask - render a scaled gfx
    element with a multiple transparent pens
    provided as a mask, checking against the
    priority bitmap
-------------------------------------------------*/

void pdrawgfxzoom_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask,
		UINT32 transmask)
{
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_transmask(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, transmask);
		return;
	}

	/* special case 0 mask to opaque */
	if (transmask == 0)
	{
		pdrawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* use pen usage to optimize */
	if (gfx->pen_usage != NULL && !gfx->dirty[code])
	{
		UINT32 usage = gfx->pen_usage[code];

		/* fully transparent; do nothing */
		if ((usage & ~transmask) == 0)
			return;

		/* fully opaque; draw as such */
		if ((usage & transmask) == 0)
		{
			pdrawgfxzoom_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
			return;
		}
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSMASK_PRIORITY, UINT8);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfxzoom_transtable - render a scaled gfx
    element using a table to look up which pens
    are transparent, opaque, or shadowing,
    checking against the priority bitmap
-------------------------------------------------*/

void pdrawgfxzoom_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask,
		const UINT8 *pentable, const pen_t *shadowtable)
{
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_transtable(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, pentable, shadowtable);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);
	assert(pentable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSTABLE16_PRIORITY, UINT8);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY, UINT8);
}


/*-------------------------------------------------
    pdrawgfxzoom_alpha - render a scaled gfx
    element with a single transparent pen, alpha
    blending the remaining pixels with a fixed
    alpha value, checking against the priority
    bitmap
-------------------------------------------------*/

void pdrawgfxzoom_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask,
		UINT32 transpen, UINT8 alpha)
{
	const pen_t *paldata;

	/* non-zoom case */
	if (scalex == 0x10000 && scaley == 0x10000)
	{
		pdrawgfx_alpha(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, priority, pmask, transpen, alpha);
		return;
	}

	/* special case alpha = 0xff */
	if (alpha == 0xff)
	{
		pdrawgfxzoom_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask, transpen);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 16 || dest->bpp == 32);
	assert(gfx != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && !gfx->dirty[code] && (gfx->pen_usage[code] & ~(1 << transpen)) == 0)
		return;

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	if (dest->bpp == 16)
		DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REMAP_TRANSPEN_ALPHA16_PRIORITY, UINT8);
	else
		DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY, UINT8);
}



/***************************************************************************
    DRAW_SCANLINE IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    draw_scanline8 - copy pixels from an 8bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline8(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT8 *srcptr, const pen_t *paldata)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(bitmap != NULL);
	assert(bitmap->bpp == 16 || bitmap->bpp == 32);

	/* palette lookup case */
	if (paldata != NULL)
	{
		/* 16bpp case */
		if (bitmap->bpp == 16)
			DRAWSCANLINE_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
		else
			DRAWSCANLINE_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
	}

	/* raw copy case */
	else
	{
		/* 16bpp case */
		if (bitmap->bpp == 16)
			DRAWSCANLINE_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
		else
			DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	}
}


/*-------------------------------------------------
    draw_scanline16 - copy pixels from a 16bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline16(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT16 *srcptr, const pen_t *paldata)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(bitmap != NULL);
	assert(bitmap->bpp == 16 || bitmap->bpp == 32);

	/* palette lookup case */
	if (paldata != NULL)
	{
		/* 16bpp case */
		if (bitmap->bpp == 16)
			DRAWSCANLINE_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
		else
			DRAWSCANLINE_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
	}

	/* raw copy case */
	else
	{
		/* 16bpp case */
		if (bitmap->bpp == 16)
			DRAWSCANLINE_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
		else
			DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	}
}


/*-------------------------------------------------
    draw_scanline32 - copy pixels from a 32bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline32(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr, const pen_t *paldata)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(bitmap != NULL);
	assert(bitmap->bpp == 16 || bitmap->bpp == 32);

	/* palette lookup case */
	if (paldata != NULL)
	{
		/* 16bpp case */
		if (bitmap->bpp == 16)
			DRAWSCANLINE_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
		else
			DRAWSCANLINE_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
	}

	/* raw copy case */
	else
	{
		/* 16bpp case */
		if (bitmap->bpp == 16)
			DRAWSCANLINE_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
		else
			DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	}
}



/***************************************************************************
    EXTRACT_SCANLINE IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    extract_scanline8 - copy pixels from a single
    scanline of a bitmap to an 8bpp buffer
-------------------------------------------------*/

void extract_scanline8(bitmap_t *bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT8 *destptr)
{
	assert(bitmap != NULL);
	assert(bitmap->bpp == 16 || bitmap->bpp == 32);

	/* 16bpp case */
	if (bitmap->bpp == 16)
		EXTRACTSCANLINE_CORE(UINT16);
	else
		EXTRACTSCANLINE_CORE(UINT32);
}


/*-------------------------------------------------
    extract_scanline16 - copy pixels from a single
    scanline of a bitmap to a 16bpp buffer
-------------------------------------------------*/

void extract_scanline16(bitmap_t *bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT16 *destptr)
{
	assert(bitmap != NULL);
	assert(bitmap->bpp == 16 || bitmap->bpp == 32);

	/* 16bpp case */
	if (bitmap->bpp == 16)
		EXTRACTSCANLINE_CORE(UINT16);
	else
		EXTRACTSCANLINE_CORE(UINT32);
}


/*-------------------------------------------------
    extract_scanline32 - copy pixels from a single
    scanline of a bitmap to a 32bpp buffer
-------------------------------------------------*/

void extract_scanline32(bitmap_t *bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT32 *destptr)
{
	assert(bitmap != NULL);
	assert(bitmap->bpp == 16 || bitmap->bpp == 32);

	/* 16bpp case */
	if (bitmap->bpp == 16)
		EXTRACTSCANLINE_CORE(UINT16);
	else
		EXTRACTSCANLINE_CORE(UINT32);
}



/***************************************************************************
    COPYBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copybitmap - copy from one bitmap to another,
    copying all unclipped pixels
-------------------------------------------------*/

void copybitmap(bitmap_t *dest, bitmap_t *src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle *cliprect)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(dest != NULL);
	assert(src != NULL);
	assert(dest->bpp == 8 || dest->bpp == 16 || dest->bpp == 32);
	assert(src->bpp == dest->bpp);

	if (dest->bpp == 8)
		COPYBITMAP_CORE(UINT8, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	else if (dest->bpp == 16)
		COPYBITMAP_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	else
		COPYBITMAP_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    copybitmap_trans - copy from one bitmap to
    another, copying all unclipped pixels except
    those that match transpen
-------------------------------------------------*/

void copybitmap_trans(bitmap_t *dest, bitmap_t *src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle *cliprect, UINT32 transpen)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(dest != NULL);
	assert(src != NULL);
	assert(dest->bpp == 8 || dest->bpp == 16 || dest->bpp == 32);
	assert(src->bpp == dest->bpp);

	if (dest->bpp == 8)
	{
		if (transpen > 0xff)
			copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
		else
			COPYBITMAP_CORE(UINT8, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
	}
	else if (dest->bpp == 16)
	{
		if (transpen > 0xffff)
			copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
		else
			COPYBITMAP_CORE(UINT16, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
	}
	else
	{
		if (transpen == 0xffffffff)
			copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
		else
			COPYBITMAP_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
	}
}



/***************************************************************************
    COPYSCROLLBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copyscrollbitmap - copy from one bitmap to
    another, copying all unclipped pixels, and
    applying scrolling to one or more rows/colums
-------------------------------------------------*/

void copyscrollbitmap(bitmap_t *dest, bitmap_t *src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle *cliprect)
{
	/* just call through to the transparent case as the underlying copybitmap will
       optimize for pen == 0xffffffff */
	copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff);
}


/*-------------------------------------------------
    copyscrollbitmap_trans - copy from one bitmap
    to another, copying all unclipped pixels
    except those that match transpen, and applying
    scrolling to one or more rows/colums
-------------------------------------------------*/

void copyscrollbitmap_trans(bitmap_t *dest, bitmap_t *src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle *cliprect, UINT32 transpen)
{
	/* no rowscroll and no colscroll means no scroll */
	if (numrows == 0 && numcols == 0)
	{
		copybitmap_trans(dest, src, 0, 0, 0, 0, cliprect, transpen);
		return;
	}

	assert(dest != NULL);
	assert(src != NULL);
	assert(dest->bpp == 8 || dest->bpp == 16 || dest->bpp == 32);
	assert(dest->bpp == src->bpp);
	assert(numrows != 0 || rowscroll == NULL);
	assert(numrows == 0 || rowscroll != NULL);
	assert(numcols != 0 || colscroll == NULL);
	assert(numcols == 0 || colscroll != NULL);

	/* NULL clip means use the full bitmap */
	if (cliprect == NULL)
		cliprect = &dest->cliprect;

	/* fully scrolling X,Y playfield */
	if (numrows <= 1 && numcols <= 1)
	{
		INT32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		INT32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);
		INT32 sx, sy;

		/* iterate over all portions of the scroll that overlap the destination */
		for (sx = xscroll - src->width; sx < dest->width; sx += src->width)
			for (sy = yscroll - src->height; sy < dest->height; sy += src->height)
				copybitmap_trans(dest, src, 0, 0, sx, sy, cliprect, transpen);
	}

	/* scrolling columns plus horizontal scroll */
	else if (numrows <= 1)
	{
		INT32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		rectangle subclip = *cliprect;
		int col, colwidth, groupcols;

		/* determine width of each column */
		colwidth = src->width / numcols;
		assert(src->width % colwidth == 0);

		/* iterate over each column */
		for (col = 0; col < numcols; col += groupcols)
		{
			INT32 yscroll = colscroll[col];
			INT32 sx, sy;

			/* count consecutive columns scrolled by the same amount */
			for (groupcols = 1; col + groupcols < numcols; groupcols++)
				 if (colscroll[col + groupcols] != yscroll)
					break;

			/* iterate over reps of the columns in question */
			yscroll = normalize_yscroll(src, yscroll);
			for (sx = xscroll - src->width; sx < dest->width; sx += src->width)
			{
				/* compute the cliprect for this group */
				subclip.min_x = col * colwidth + sx;
				subclip.max_x = (col + groupcols) * colwidth - 1 + sx;
				sect_rect(&subclip, cliprect);

				/* iterate over all portions of the scroll that overlap the destination */
				for (sy = yscroll - src->height; sy < dest->height; sy += src->height)
					copybitmap_trans(dest, src, 0, 0, sx, sy, &subclip, transpen);
			}
		}
	}

	/* scrolling rows plus vertical scroll */
	else if (numcols <= 1)
	{
		INT32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);
		rectangle subclip = *cliprect;
		int row, rowheight, grouprows;

		/* determine width of each rows */
		rowheight = src->height / numrows;
		assert(src->height % rowheight == 0);

		/* iterate over each row */
		for (row = 0; row < numrows; row += grouprows)
		{
			INT32 xscroll = rowscroll[row];
			INT32 sx, sy;

			/* count consecutive rows scrolled by the same amount */
			for (grouprows = 1; row + grouprows < numrows; grouprows++)
				 if (rowscroll[row + grouprows] != xscroll)
					break;

			/* iterate over reps of the rows in question */
			xscroll = normalize_xscroll(src, xscroll);
			for (sy = yscroll - src->height; sy < dest->height; sy += src->height)
			{
				/* compute the cliprect for this group */
				subclip.min_y = row * rowheight + sy;
				subclip.max_y = (row + grouprows) * rowheight - 1 + sy;
				sect_rect(&subclip, cliprect);

				/* iterate over all portions of the scroll that overlap the destination */
				for (sx = xscroll - src->width; sx < dest->width; sx += src->width)
					copybitmap_trans(dest, src, 0, 0, sx, sy, &subclip, transpen);
			}
		}
	}
}



/***************************************************************************
    COPYROZBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copyrozbitmap - copy from one bitmap to another,
    with zoom and rotation, copying all unclipped
    pixels
-------------------------------------------------*/

void copyrozbitmap(bitmap_t *dest, const rectangle *cliprect, bitmap_t *src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(dest != NULL);
	assert(src != NULL);
	assert(dest->bpp == 8 || dest->bpp == 16 || dest->bpp == 32);
	assert(src->bpp == dest->bpp);

	if (dest->bpp == 8)
		COPYROZBITMAP_CORE(UINT8, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	else if (dest->bpp == 16)
		COPYROZBITMAP_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
	else
		COPYROZBITMAP_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    copyrozbitmap_trans - copy from one bitmap to
    another, with zoom and rotation, copying all
    unclipped pixels whose values do not match
    transpen
-------------------------------------------------*/

void copyrozbitmap_trans(bitmap_t *dest, const rectangle *cliprect, bitmap_t *src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound, UINT32 transpen)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	assert(dest != NULL);
	assert(src != NULL);
	assert(dest->bpp == 8 || dest->bpp == 16 || dest->bpp == 32);
	assert(src->bpp == dest->bpp);

	if (dest->bpp == 8)
		COPYROZBITMAP_CORE(UINT8, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
	else if (dest->bpp == 16)
		COPYROZBITMAP_CORE(UINT16, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
	else
		COPYROZBITMAP_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
}
