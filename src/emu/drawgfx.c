/*********************************************************************

    drawgfx.c

    Generic graphic functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "drawgfxm.h"
#include "validity.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

// if this line errors during compile, the size of NO_PRIORITY is wrong and I need to use something else
UINT8 no_priority_size_is_wrong[2 * (sizeof(NO_PRIORITY) == 3) - 1];

bitmap_ind8 drawgfx_dummy_priority_bitmap;



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    readbit - read a single bit from a base
    offset
-------------------------------------------------*/

static inline int readbit(const UINT8 *src, unsigned int bitnum)
{
	return src[bitnum / 8] & (0x80 >> (bitnum % 8));
}


/*-------------------------------------------------
    normalize_xscroll - normalize an X scroll
    value for a bitmap to be positive and less
    than the width
-------------------------------------------------*/

static inline INT32 normalize_xscroll(bitmap_t &bitmap, INT32 xscroll)
{
	return (xscroll >= 0) ? xscroll % bitmap.width() : (bitmap.width() - (-xscroll) % bitmap.width());
}


/*-------------------------------------------------
    normalize_yscroll - normalize a Y scroll
    value for a bitmap to be positive and less
    than the height
-------------------------------------------------*/

static inline INT32 normalize_yscroll(bitmap_t &bitmap, INT32 yscroll)
{
	return (yscroll >= 0) ? yscroll % bitmap.height() : (bitmap.height() - (-yscroll) % bitmap.height());
}



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type GFXDECODE = &device_creator<gfxdecode_device>;

gfxdecode_device::gfxdecode_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GFXDECODE, "gfxdecode", tag, owner, clock, "gfxdecode", __FILE__),
		m_gfxdecodeinfo(NULL)
{
	memset(m_gfx, 0, sizeof(m_gfx));
}

//**************************************************************************
//  INITIALIZATION AND CONFIGURATION
//**************************************************************************

void gfxdecode_device::static_set_gfxdecodeinfo(device_t &device, const gfx_decode_entry *info)
{
	downcast<gfxdecode_device &>(device).m_gfxdecodeinfo = info;
}

//-------------------------------------------------
//  device_stop - final cleanup
//-------------------------------------------------

void gfxdecode_device::device_stop()
{
	for (int i = 0; i < MAX_GFX_ELEMENTS; i++)
		auto_free(machine(), m_gfx[i]);
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void gfxdecode_device::device_start()
{
	const gfx_decode_entry *gfxdecodeinfo = m_gfxdecodeinfo;
	int curgfx;

	// skip if nothing to do
	if (gfxdecodeinfo == NULL)
		return;

	// loop over all elements
	for (curgfx = 0; curgfx < MAX_GFX_ELEMENTS && gfxdecodeinfo[curgfx].gfxlayout != NULL; curgfx++)
	{
		const gfx_decode_entry *gfxdecode = &gfxdecodeinfo[curgfx];
		UINT32 region_length;
		const UINT8 *region_base;

		// resolve the region
		if (gfxdecode->memory_region != NULL)
		{
			astring gfxregion;
			owner()->subtag(gfxregion, gfxdecode->memory_region);
			memory_region *region = owner()->memregion(gfxregion);
			region_length = 8 * region->bytes();
			region_base = region->base();
		}
		else
		{
			region_length = 0;
			region_base = NULL;
		}

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

		// make a copy of the layout
		glcopy = *gfxdecode->gfxlayout;

		// copy the X and Y offsets into temporary arrays
		memcpy(extxoffs, glcopy.xoffset, sizeof(glcopy.xoffset));
		memcpy(extyoffs, glcopy.yoffset, sizeof(glcopy.yoffset));

		// if there are extended offsets, copy them over top
		if (glcopy.extxoffs != NULL)
			memcpy(extxoffs, glcopy.extxoffs, glcopy.width * sizeof(extxoffs[0]));
		if (glcopy.extyoffs != NULL)
			memcpy(extyoffs, glcopy.extyoffs, glcopy.height * sizeof(extyoffs[0]));

		// always use the extended offsets here
		glcopy.extxoffs = extxoffs;
		glcopy.extyoffs = extyoffs;

		extpoffs = glcopy.planeoffset;

		// expand X and Y by the scale factors
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

		// if the character count is a region fraction, compute the effective total
		if (IS_FRAC(total))
		{
			assert(region_length != 0);
			total = region_length / charincrement * FRAC_NUM(total) / FRAC_DEN(total);
		}

		// for non-raw graphics, decode the X and Y offsets
		if (!israw)
		{
			// loop over all the planes, converting fractions
			for (j = 0; j < planes; j++)
			{
				UINT32 value1 = extpoffs[j];
				if (IS_FRAC(value1))
				{
					assert(region_length != 0);
					extpoffs[j] = FRAC_OFFSET(value1) + region_length * FRAC_NUM(value1) / FRAC_DEN(value1);
				}
			}

			// loop over all the X/Y offsets, converting fractions
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

		// otherwise, just use the line modulo
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

		// update glcopy
		glcopy.width = width;
		glcopy.height = height;
		glcopy.total = total;

		// allocate the graphics
		m_gfx[curgfx] = auto_alloc(machine(), gfx_element(machine(), glcopy, (region_base != NULL) ? region_base + gfxdecode->start : NULL, gfxdecode->total_color_codes, gfxdecode->color_codes_start));
	}
}


//-------------------------------------------------
//  device_validity_check - validate graphics decoding
//  configuration
//-------------------------------------------------/

void gfxdecode_device::device_validity_check(validity_checker &valid) const
{

	// bail if no gfx
	if (!m_gfxdecodeinfo)
		return;

	// iterate over graphics decoding entries
	for (int gfxnum = 0; gfxnum < MAX_GFX_ELEMENTS && m_gfxdecodeinfo[gfxnum].gfxlayout != NULL; gfxnum++)
	{
		const gfx_decode_entry &gfx = m_gfxdecodeinfo[gfxnum];
		const gfx_layout &layout = *gfx.gfxlayout;

		// make sure the region exists
		const char *region = gfx.memory_region;
		if (region != NULL)
		{
			// resolve the region
			astring gfxregion;
			
			owner()->subtag(gfxregion, region);

			// loop over gfx regions
			UINT32 region_length = valid.region_length(gfxregion);
			if (region_length == 0)
				mame_printf_error("gfx[%d] references non-existent region '%s'\n", gfxnum, gfxregion.cstr());

			// if we have a valid region, and we're not using auto-sizing, check the decode against the region length
			else if (!IS_FRAC(layout.total))
			{
				// determine which plane is at the largest offset
				int start = 0;
				for (int plane = 0; plane < layout.planes; plane++)
					if (layout.planeoffset[plane] > start)
						start = layout.planeoffset[plane];
				start &= ~(layout.charincrement - 1);

				// determine the total length based on this info
				int len = layout.total * layout.charincrement;

				// do we have enough space in the region to cover the whole decode?
				int avail = region_length - (gfx.start & ~(layout.charincrement / 8 - 1));

				// if not, this is an error
				if ((start + len) / 8 > avail)
					mame_printf_error("gfx[%d] extends past allocated memory of region '%s'\n", gfxnum, region);
			}
		}

		int xscale = (m_gfxdecodeinfo[gfxnum].xscale == 0) ? 1 : m_gfxdecodeinfo[gfxnum].xscale;
		int yscale = (m_gfxdecodeinfo[gfxnum].yscale == 0) ? 1 : m_gfxdecodeinfo[gfxnum].yscale;

		// verify raw decode, which can only be full-region and have no scaling
		if (layout.planeoffset[0] == GFX_RAW)
		{
			if (layout.total != RGN_FRAC(1,1))
				mame_printf_error("gfx[%d] with unsupported layout total\n", gfxnum);
			if (xscale != 1 || yscale != 1)
				mame_printf_error("gfx[%d] with unsupported xscale/yscale\n", gfxnum);
		}

		// verify traditional decode doesn't have too many planes or is not too large
		else
		{
			if (layout.planes > MAX_GFX_PLANES)
				mame_printf_error("gfx[%d] with invalid planes\n", gfxnum);
			if (xscale * layout.width > MAX_ABS_GFX_SIZE || yscale * layout.height > MAX_ABS_GFX_SIZE)
				mame_printf_error("gfx[%d] with invalid xscale/yscale\n", gfxnum);
		}
	}
	
}


/***************************************************************************
    GRAPHICS ELEMENTS
***************************************************************************/


//-------------------------------------------------
//  gfx_element - constructor
//-------------------------------------------------

gfx_element::gfx_element(running_machine &machine)
	: m_width(0),
		m_height(0),
		m_startx(0),
		m_starty(0),
		m_origwidth(0),
		m_origheight(0),
		m_total_elements(0),
		m_color_base(0),
		m_color_depth(0),
		m_color_granularity(0),
		m_total_colors(0),
		m_line_modulo(0),
		m_char_modulo(0),
		m_srcdata(NULL),
		m_dirtyseq(1),
		m_gfxdata(NULL),
		m_layout_is_raw(false),
		m_layout_planes(0),
		m_layout_charincrement(0),
		m_machine(machine)
{
}

gfx_element::gfx_element(running_machine &machine, UINT8 *base, UINT32 width, UINT32 height, UINT32 rowbytes, UINT32 total_colors, UINT32 color_base, UINT32 color_granularity)
	: m_width(width),
		m_height(height),
		m_startx(0),
		m_starty(0),
		m_origwidth(width),
		m_origheight(height),
		m_total_elements(1),
		m_color_base(color_base),
		m_color_depth(color_granularity),
		m_color_granularity(color_granularity),
		m_total_colors((total_colors - color_base) / color_granularity),
		m_line_modulo(rowbytes),
		m_char_modulo(0),
		m_srcdata(base),
		m_dirtyseq(1),
		m_gfxdata(base),
		m_layout_is_raw(true),
		m_layout_planes(0),
		m_layout_charincrement(0),
		m_machine(machine)
{
}

gfx_element::gfx_element(running_machine &machine, const gfx_layout &gl, const UINT8 *srcdata, UINT32 total_colors, UINT32 color_base)
	: m_width(0),
		m_height(0),
		m_startx(0),
		m_starty(0),
		m_origwidth(0),
		m_origheight(0),
		m_total_elements(0),
		m_color_base(color_base),
		m_color_depth(0),
		m_color_granularity(0),
		m_total_colors(total_colors),
		m_line_modulo(0),
		m_char_modulo(0),
		m_srcdata(NULL),
		m_dirtyseq(1),
		m_gfxdata(NULL),
		m_layout_is_raw(false),
		m_layout_planes(0),
		m_layout_charincrement(0),
		m_machine(machine)
{
	// set the layout
	set_layout(gl, srcdata);
}


//-------------------------------------------------
//  set_layout - set the layout for a gfx_element
//-------------------------------------------------

void gfx_element::set_layout(const gfx_layout &gl, const UINT8 *srcdata)
{
	// configure ourselves
	m_width = m_origwidth = gl.width;
	m_height = m_origheight = gl.height;
	m_startx = m_starty = 0;
	m_total_elements = gl.total;
	m_color_depth = m_color_granularity = 1 << gl.planes;

	// copy data from the layout
	m_layout_is_raw = (gl.planeoffset[0] == GFX_RAW);
	m_layout_planes = gl.planes;
	m_layout_charincrement = gl.charincrement;
	m_layout_planeoffset.resize(m_layout_planes);
	m_layout_xoffset.resize(m_width);
	m_layout_yoffset.resize(m_height);

	// raw graphics case
	if (m_layout_is_raw)
	{
		// modulos are determined for us by the layout
		m_line_modulo = ((gl.extyoffs != NULL) ? gl.extyoffs[0] : gl.yoffset[0]) / 8;
		m_char_modulo = gl.charincrement / 8;

		// RAW graphics must have a pointer up front
		assert(srcdata != NULL);
		m_gfxdata_allocated.reset();
		m_gfxdata = const_cast<UINT8 *>(m_srcdata);
	}

	// decoded graphics case
	else
	{
		// copy offsets
		for (int p = 0; p < m_layout_planes; p++)
			m_layout_planeoffset[p] = gl.planeoffset[p];
		for (int y = 0; y < m_height; y++)
			m_layout_yoffset[y] = gl.yoffs(y);
		for (int x = 0; x < m_width; x++)
			m_layout_xoffset[x] = gl.xoffs(x);

		// we get to pick our own modulos
		m_line_modulo = m_origwidth;
		m_char_modulo = m_line_modulo * m_origheight;

		// allocate memory for the data
		m_gfxdata_allocated.resize(m_total_elements * m_char_modulo);
		m_gfxdata = &m_gfxdata_allocated[0];
	}

	// mark everything dirty
	m_dirty.resize(m_total_elements);
	memset(m_dirty, 1, m_total_elements);

	// allocate a pen usage array for entries with 32 pens or less
	if (m_color_depth <= 32)
		m_pen_usage.resize(m_total_elements);
	else
		m_pen_usage.reset();

	// set the source
	set_source(srcdata);
}


//-------------------------------------------------
//  set_raw_layout - set the layout for a gfx_element
//-------------------------------------------------

void gfx_element::set_raw_layout(const UINT8 *srcdata, UINT32 width, UINT32 height, UINT32 total, UINT32 linemod, UINT32 charmod)
{
	gfx_layout layout = { 0 };
	layout.width = width;
	layout.height = height;
	layout.total = total;
	layout.planes = 8;
	layout.planeoffset[0] = GFX_RAW;
	layout.yoffset[0] = linemod;
	layout.charincrement = charmod;
	set_layout(layout, srcdata);
}


//-------------------------------------------------
//  set_source_clip - set a source clipping rect
//-------------------------------------------------

void gfx_element::set_source_clip(UINT32 xoffs, UINT32 width, UINT32 yoffs, UINT32 height)
{
	assert(xoffs < m_origwidth);
	assert(yoffs < m_origheight);
	assert(xoffs + width <= m_origwidth);
	assert(yoffs + height <= m_origheight);

	m_width = width;
	m_height = height;
	m_startx = xoffs;
	m_starty = yoffs;
}


//-------------------------------------------------
//  decode - decode a single character
//-------------------------------------------------

void gfx_element::decode(UINT32 code)
{
	// don't decode GFX_RAW
	if (!m_layout_is_raw)
	{
		// zap the data to 0
		UINT8 *decode_base = m_gfxdata + code * m_char_modulo;
		memset(decode_base, 0, m_char_modulo);

		// iterate over planes
		for (int plane = 0; plane < m_layout_planes; plane++)
		{
			int planebit = 1 << (m_layout_planes - 1 - plane);
			int planeoffs = code * m_layout_charincrement + m_layout_planeoffset[plane];

			// iterate over rows
			for (int y = 0; y < m_origheight; y++)
			{
				int yoffs = planeoffs + m_layout_yoffset[y];
				UINT8 *dp = decode_base + y * rowbytes();

				// iterate over columns
				for (int x = 0; x < m_origwidth; x++)
					if (readbit(m_srcdata, yoffs + m_layout_xoffset[x]))
						dp[x] |= planebit;
			}
		}
	}

	// (re)compute pen usage
	if (code < m_pen_usage.count())
	{
		// iterate over data, creating a bitmask of live pens
		const UINT8 *dp = m_gfxdata + code * m_char_modulo;
		UINT32 usage = 0;
		for (int y = 0; y < m_origheight; y++)
		{
			for (int x = 0; x < m_origwidth; x++)
				usage |= 1 << dp[x];
			dp += m_line_modulo;
		}

		// store the final result
		m_pen_usage[code] = usage;
	}

	// no longer dirty
	m_dirty[code] = 0;
}



/***************************************************************************
    DRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    opaque - render a gfx element with
    no transparency
-------------------------------------------------*/

void gfx_element::opaque(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty)
{
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_OPAQUE, NO_PRIORITY);
}

void gfx_element::opaque(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty)
{
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    transpen - render a gfx element with
    a single transparent pen
-------------------------------------------------*/

void gfx_element::transpen(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}

void gfx_element::transpen(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    transpen_raw - render a gfx element
    with a single transparent pen and no color
    lookups
-------------------------------------------------*/

void gfx_element::transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}

void gfx_element::transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    transmask - render a gfx element
    with a multiple transparent pens provided as
    a mask
-------------------------------------------------*/

void gfx_element::transmask(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSMASK, NO_PRIORITY);
}

void gfx_element::transmask(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK, NO_PRIORITY);
}


/*-------------------------------------------------
    transtable - render a gfx element
    using a table to look up which pens are
    transparent, opaque, or shadowing
-------------------------------------------------*/

void gfx_element::transtable(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSTABLE16, NO_PRIORITY);
}

void gfx_element::transtable(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32, NO_PRIORITY);
}


/*-------------------------------------------------
    alpha - render a gfx element with
    a single transparent pen, alpha blending the
    remaining pixels with a fixed alpha value
-------------------------------------------------*/

void gfx_element::alpha(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 trans_pen, UINT8 alpha_val)
{
	// special case alpha = 0xff
	if (alpha_val == 0xff)
		return transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// get final code and color, and grab lookup tables
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DECLARE_NO_PRIORITY;
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32, NO_PRIORITY);
}



/***************************************************************************
    DRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    zoom_opaque - render a scaled gfx
    element with no transparency
-------------------------------------------------*/

void gfx_element::zoom_opaque(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_OPAQUE, NO_PRIORITY);
}

void gfx_element::zoom_opaque(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty);

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    zoom_transpen - render a scaled gfx
    element with a single transparent pen
-------------------------------------------------*/

void gfx_element::zoom_transpen(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}

void gfx_element::zoom_transpen(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    zoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups
-------------------------------------------------*/

void gfx_element::zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}

void gfx_element::zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN, NO_PRIORITY);
}


/*-------------------------------------------------
    zoom_transmask - render a scaled gfx
    element with a multiple transparent pens
    provided as a mask
-------------------------------------------------*/

void gfx_element::zoom_transmask(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transmask(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSMASK, NO_PRIORITY);
}

void gfx_element::zoom_transmask(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transmask(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK, NO_PRIORITY);
}


/*-------------------------------------------------
    zoom_transtable - render a scaled gfx
    element using a table to look up which pens
    are transparent, opaque, or shadowing
-------------------------------------------------*/

void gfx_element::zoom_transtable(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transtable(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, pentable, shadowtable);

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSTABLE16, NO_PRIORITY);
}

void gfx_element::zoom_transtable(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transtable(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, pentable, shadowtable);

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32, NO_PRIORITY);
}


/*-------------------------------------------------
    zoom_alpha - render a scaled gfx element
    with a single transparent pen, alpha blending
    the remaining pixels with a fixed alpha value
-------------------------------------------------*/

void gfx_element::zoom_alpha(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, UINT32 trans_pen, UINT8 alpha_val)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return alpha(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen, alpha_val);

	// special case alpha_val = 0xff
	if (alpha_val == 0xff)
		return zoom_transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DECLARE_NO_PRIORITY;
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32, NO_PRIORITY);
}



/***************************************************************************
    PDRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    prio_opaque - render a gfx element with
    no transparency, checking against the priority
    bitmap
-------------------------------------------------*/

void gfx_element::prio_opaque(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_OPAQUE_PRIORITY, UINT8);
}

void gfx_element::prio_opaque(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_transpen - render a gfx element with
    a single transparent pen, checking against the
    priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transpen(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}

void gfx_element::prio_transpen(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    priotranspen_raw - render a gfx element
    with a single transparent pen and no color
    lookups, checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}

void gfx_element::prio_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	DRAWGFX_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_transmask - render a gfx element
    with a multiple transparent pens provided as
    a mask, checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transmask(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSMASK_PRIORITY, UINT8);
}

void gfx_element::prio_transmask(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_transtable - render a gfx element
    using a table to look up which pens are
    transparent, opaque, or shadowing, checking
    against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transtable(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DRAWGFX_CORE(UINT16, PIXEL_OP_REBASE_TRANSTABLE16_PRIORITY, UINT8);
}

void gfx_element::prio_transtable(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_alpha - render a gfx element with
    a single transparent pen, alpha blending the
    remaining pixels with a fixed alpha value,
    checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_alpha(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_pen, UINT8 alpha_val)
{
	// special case alpha = 0xff
	if (alpha_val == 0xff)
		return prio_transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY, UINT8);
}



/***************************************************************************
    PDRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    prio_zoom_opaque - render a scaled gfx
    element with no transparency, checking against
    the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_opaque(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_OPAQUE_PRIORITY, UINT8);
}

void gfx_element::prio_zoom_opaque(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_zoom_transpen - render a scaled gfx
    element with a single transparent pen,
    checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transpen(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}

void gfx_element::prio_zoom_transpen(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_zoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups, checking against the priority
    bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}

void gfx_element::prio_zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REBASE_TRANSPEN_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_zoom_transmask - render a scaled gfx
    element with a multiple transparent pens
    provided as a mask, checking against the
    priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transmask(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transmask(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSMASK_PRIORITY, UINT8);
}

void gfx_element::prio_zoom_transmask(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transmask(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		UINT32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_zoom_opaque(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSMASK_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_zoom_transtable - render a scaled gfx
    element using a table to look up which pens
    are transparent, opaque, or shadowing,
    checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transtable(palette_device &palette, bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transtable(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, pentable, shadowtable);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	DRAWGFXZOOM_CORE(UINT16, PIXEL_OP_REBASE_TRANSTABLE16_PRIORITY, UINT8);
}

void gfx_element::prio_zoom_transtable(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		const UINT8 *pentable, const pen_t *shadowtable)
{
	assert(pentable != NULL);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transtable(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, pentable, shadowtable);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	code %= elements();
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY, UINT8);
}


/*-------------------------------------------------
    prio_zoom_alpha - render a scaled gfx
    element with a single transparent pen, alpha
    blending the remaining pixels with a fixed
    alpha value, checking against the priority
    bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_alpha(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_pen, UINT8 alpha_val)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_alpha(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen, alpha_val);

	// special case alpha_val = 0xff
	if (alpha_val == 0xff)
		return prio_zoom_transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = &palette.pen(colorbase() + granularity() * (color % colors()));
	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY, UINT8);
}


#define PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32(DEST, PRIORITY, SOURCE)                  \
do                                                                                  \
{                                                                                   \
	UINT32 srcdata = (SOURCE);                                                      \
	if (srcdata != trans_pen)                                                        \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
		{                                                                           \
			UINT32 srcdata2 = paldata[srcdata];                                     \
																					\
			UINT32 add;                                                             \
			add = (srcdata2 & 0x00ff0000) + (DEST & 0x00ff0000);                    \
			if (add & 0x01000000) DEST = (DEST & 0xff00ffff) | (0x00ff0000);        \
			else DEST = (DEST & 0xff00ffff) | (add & 0x00ff0000);                   \
			add = (srcdata2 & 0x000000ff) + (DEST & 0x000000ff);                    \
			if (add & 0x00000100) DEST = (DEST & 0xffffff00) | (0x000000ff);        \
			else DEST = (DEST & 0xffffff00) | (add & 0x000000ff);                   \
			add = (srcdata2 & 0x0000ff00) + (DEST & 0x0000ff00);                    \
			if (add & 0x00010000) DEST = (DEST & 0xffff00ff) | (0x0000ff00);        \
			else DEST = (DEST & 0xffff00ff) | (add & 0x0000ff00);                   \
		}                                                                           \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)

void gfx_element::prio_transpen_additive(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		bitmap_ind8 &priority, UINT32 pmask, UINT32 trans_pen)
{
	const pen_t *paldata;

	assert(dest.valid());
	assert(dest.bpp() == 32);

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = &palette.pen(colorbase() + granularity() * color);

	/* use pen usage to optimize */
	if (has_pen_usage())
	{
		UINT32 usage = pen_usage(code);

		/* fully transparent; do nothing */
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32, UINT8);
}


void gfx_element::prio_zoom_transpen_additive(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		UINT32 scalex, UINT32 scaley, bitmap_ind8 &priority, UINT32 pmask,
		UINT32 trans_pen)
{
	const pen_t *paldata;

	/* non-zoom case */

	if (scalex == 0x10000 && scaley == 0x10000)
	{
		prio_transpen_additive(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);
		return;
	}

	assert(dest.valid());
	assert(dest.bpp() == 32);

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = &palette.pen(colorbase() + granularity() * color);

	/* use pen usage to optimize */
	if (has_pen_usage())
	{
		UINT32 usage = pen_usage(code);

		/* fully transparent; do nothing */
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	DRAWGFXZOOM_CORE(UINT32, PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32, UINT8);
}

//#define MAKE_ARGB_RGB(a, rgb) rgb_t(a, rgb.r(), rgb.g(), rgb.b())
#define MAKE_ARGB_RGB(a, rgb)   rgb_t(rgb).set_a(a)

// combine in 'alpha' when copying to store in ARGB
#define PIXEL_OP_REMAP_TRANS0_ALPHASTORE32(DEST, PRIORITY, SOURCE)                                  \
do                                                                                                  \
{                                                                                                   \
	UINT32 srcdata = (SOURCE);                                                                      \
	if (srcdata != 0)                                                                               \
		(DEST) = MAKE_ARGB_RGB(alpha,paldata[srcdata]);                                             \
}                                                                                                   \
while (0)
// combine in 'alphatable' value to store in ARGB
#define PIXEL_OP_REMAP_TRANS0_ALPHATABLESTORE32(DEST, PRIORITY, SOURCE)                             \
do                                                                                                  \
{                                                                                                   \
	UINT32 srcdata = (SOURCE);                                                                      \
	if (srcdata != 0)                                                                               \
		(DEST) = MAKE_ARGB_RGB(alphatable[srcdata], paldata[srcdata]);                              \
}                                                                                                   \
while (0)
// drawgfxm.h macro to render alpha into 32-bit buffer
#define PIXEL_OP_REMAP_TRANS0_ALPHATABLE32(DEST, PRIORITY, SOURCE)                                  \
do                                                                                                  \
{                                                                                                   \
	UINT32 srcdata = (SOURCE);                                                                      \
	if (srcdata != 0)                                                                               \
		(DEST) = alpha_blend_r32((DEST), paldata[srcdata], alphatable[srcdata]);                    \
}                                                                                                   \
while (0)

/*-------------------------------------------------
    alphastore - render a gfx element with
    a single transparent pen, storing the alpha value
    in alpha field of ARGB32, negative alpha implies alphatable
-------------------------------------------------*/
void gfx_element::alphastore(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		int fixedalpha, UINT8 *alphatable)
{
	DECLARE_NO_PRIORITY;
	const pen_t *paldata;

	assert(dest.bpp() == 32);
	assert(dest.format() == BITMAP_FORMAT_ARGB32);
	assert(alphatable != NULL);

	/* if we have a fixed alpha, call the standard drawgfx_transpen */
	if (fixedalpha == 0xff)
	{
		transpen(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, 0);
		return;
	}

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = &palette.pen(colorbase() + granularity() * color);

	/* early out if completely transparent */
	if (has_pen_usage() && (pen_usage(code) & ~(1 << 0)) == 0)
		return;

	if (fixedalpha >= 0)
	{
		UINT8 alpha = fixedalpha;
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHASTORE32, NO_PRIORITY);
	}
	else
	{
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHATABLESTORE32, NO_PRIORITY);
	}
}

/*-------------------------------------------------
    alphatable - render a sprite with either
    a fixed alpha value, or if alpha==-1 then uses
    the per-pen alphatable[] array
 -------------------------------------------------*/
void gfx_element::alphatable(palette_device &palette, bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		int fixedalpha ,UINT8 *alphatable)
{
	DECLARE_NO_PRIORITY;

	const pen_t *paldata;

	/* if we have a fixed alpha, call the standard drawgfx_alpha */
	if (fixedalpha >= 0)
	{
		alpha(palette, dest, cliprect, code, color, flipx, flipy, destx, desty, 0, fixedalpha);
		return;
	}

	assert(dest.bpp() == 32);
	assert(alphatable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = &palette.pen(colorbase() + granularity() * color);

	/* early out if completely transparent */
	if (has_pen_usage() && (pen_usage(code) & ~(1 << 0)) == 0)
		return;

	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHATABLE32, NO_PRIORITY);
}


/***************************************************************************
    DRAW_SCANLINE IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    draw_scanline8 - copy pixels from an 8bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline8(bitmap_ind16 &bitmap, INT32 destx, INT32 desty, INT32 length, const UINT8 *srcptr, const pen_t *paldata)
{
	DECLARE_NO_PRIORITY;

	// palette lookup case
	if (paldata != NULL)
		DRAWSCANLINE_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);

	// raw copy case
	else
		DRAWSCANLINE_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}

void draw_scanline8(bitmap_rgb32 &bitmap, INT32 destx, INT32 desty, INT32 length, const UINT8 *srcptr, const pen_t *paldata)
{
	DECLARE_NO_PRIORITY;

	// palette lookup case
	if (paldata != NULL)
		DRAWSCANLINE_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);

	// raw copy case
	else
		DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    draw_scanline16 - copy pixels from a 16bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline16(bitmap_ind16 &bitmap, INT32 destx, INT32 desty, INT32 length, const UINT16 *srcptr, const pen_t *paldata)
{
	DECLARE_NO_PRIORITY;

	// palette lookup case
	if (paldata != NULL)
		DRAWSCANLINE_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);

	// raw copy case
	else
		DRAWSCANLINE_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}

void draw_scanline16(bitmap_rgb32 &bitmap, INT32 destx, INT32 desty, INT32 length, const UINT16 *srcptr, const pen_t *paldata)
{
	DECLARE_NO_PRIORITY;

	// palette lookup case
	if (paldata != NULL)
		DRAWSCANLINE_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);

	// raw copy case
	else
		DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    draw_scanline32 - copy pixels from a 32bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline32(bitmap_ind16 &bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr, const pen_t *paldata)
{
	DECLARE_NO_PRIORITY;

	// palette lookup case
	if (paldata != NULL)
		DRAWSCANLINE_CORE(UINT16, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);

	// raw copy case
	else
		DRAWSCANLINE_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}

void draw_scanline32(bitmap_rgb32 &bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr, const pen_t *paldata)
{
	DECLARE_NO_PRIORITY;

	// palette lookup case
	if (paldata != NULL)
		DRAWSCANLINE_CORE(UINT32, PIXEL_OP_REMAP_OPAQUE, NO_PRIORITY);

	// raw copy case
	else
		DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}



/***************************************************************************
    EXTRACT_SCANLINE IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    extract_scanline8 - copy pixels from a single
    scanline of a bitmap to an 8bpp buffer
-------------------------------------------------*/

void extract_scanline8(bitmap_ind16 &bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT8 *destptr)
{
	EXTRACTSCANLINE_CORE(UINT16);
}

void extract_scanline8(bitmap_rgb32 &bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT8 *destptr)
{
	EXTRACTSCANLINE_CORE(UINT32);
}


/*-------------------------------------------------
    extract_scanline16 - copy pixels from a single
    scanline of a bitmap to a 16bpp buffer
-------------------------------------------------*/

void extract_scanline16(bitmap_ind16 &bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT16 *destptr)
{
	EXTRACTSCANLINE_CORE(UINT16);
}

void extract_scanline16(bitmap_rgb32 &bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT16 *destptr)
{
	EXTRACTSCANLINE_CORE(UINT32);
}


/*-------------------------------------------------
    extract_scanline32 - copy pixels from a single
    scanline of a bitmap to a 32bpp buffer
-------------------------------------------------*/

void extract_scanline32(bitmap_ind16 &bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT32 *destptr)
{
	EXTRACTSCANLINE_CORE(UINT16);
}

void extract_scanline32(bitmap_rgb32 &bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT32 *destptr)
{
	EXTRACTSCANLINE_CORE(UINT32);
}



/***************************************************************************
    COPYBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copybitmap - copy from one bitmap to another,
    copying all unclipped pixels
-------------------------------------------------*/

void copybitmap(bitmap_ind16 &dest, bitmap_ind16 &src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle &cliprect)
{
	DECLARE_NO_PRIORITY;
	COPYBITMAP_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}

void copybitmap(bitmap_rgb32 &dest, bitmap_rgb32 &src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle &cliprect)
{
	DECLARE_NO_PRIORITY;
	COPYBITMAP_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    copybitmap_trans - copy from one bitmap to
    another, copying all unclipped pixels except
    those that match transpen
-------------------------------------------------*/

void copybitmap_trans(bitmap_ind16 &dest, bitmap_ind16 &src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle &cliprect, UINT32 trans_pen)
{
	DECLARE_NO_PRIORITY;
	if (trans_pen > 0xffff)
		copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		COPYBITMAP_CORE(UINT16, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
}

void copybitmap_trans(bitmap_rgb32 &dest, bitmap_rgb32 &src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle &cliprect, UINT32 trans_pen)
{
	DECLARE_NO_PRIORITY;
	if (trans_pen == 0xffffffff)
		copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		COPYBITMAP_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
}



/***************************************************************************
    COPYSCROLLBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copyscrollbitmap - copy from one bitmap to
    another, copying all unclipped pixels, and
    applying scrolling to one or more rows/columns
-------------------------------------------------*/

void copyscrollbitmap(bitmap_ind16 &dest, bitmap_ind16 &src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle &cliprect)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff);
}

void copyscrollbitmap(bitmap_rgb32 &dest, bitmap_rgb32 &src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle &cliprect)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff);
}


/*-------------------------------------------------
    copyscrollbitmap_trans - copy from one bitmap
    to another, copying all unclipped pixels
    except those that match transpen, and applying
    scrolling to one or more rows/columns
-------------------------------------------------*/

template<class _BitmapClass>
static inline void copyscrollbitmap_trans_common(_BitmapClass &dest, _BitmapClass &src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle &cliprect, UINT32 trans_pen)
{
	// no rowscroll and no colscroll means no scroll
	if (numrows == 0 && numcols == 0)
		return copybitmap_trans(dest, src, 0, 0, 0, 0, cliprect, trans_pen);

	assert(numrows != 0 || rowscroll == NULL);
	assert(numrows == 0 || rowscroll != NULL);
	assert(numcols != 0 || colscroll == NULL);
	assert(numcols == 0 || colscroll != NULL);

	// fully scrolling X,Y playfield
	if (numrows <= 1 && numcols <= 1)
	{
		INT32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		INT32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);

		// iterate over all portions of the scroll that overlap the destination
		for (INT32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			for (INT32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
				copybitmap_trans(dest, src, 0, 0, sx, sy, cliprect, trans_pen);
	}

	// scrolling columns plus horizontal scroll
	else if (numrows <= 1)
	{
		INT32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each column
		int colwidth = src.width() / numcols;
		assert(src.width() % colwidth == 0);

		// iterate over each column
		int groupcols;
		for (int col = 0; col < numcols; col += groupcols)
		{
			INT32 yscroll = colscroll[col];

			// count consecutive columns scrolled by the same amount
			for (groupcols = 1; col + groupcols < numcols; groupcols++)
					if (colscroll[col + groupcols] != yscroll)
					break;

			// iterate over reps of the columns in question
			yscroll = normalize_yscroll(src, yscroll);
			for (INT32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			{
				// compute the cliprect for this group
				subclip.min_x = col * colwidth + sx;
				subclip.max_x = (col + groupcols) * colwidth - 1 + sx;
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (INT32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
					copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, trans_pen);
			}
		}
	}

	// scrolling rows plus vertical scroll
	else if (numcols <= 1)
	{
		INT32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each rows
		int rowheight = src.height() / numrows;
		assert(src.height() % rowheight == 0);

		// iterate over each row
		int grouprows;
		for (int row = 0; row < numrows; row += grouprows)
		{
			INT32 xscroll = rowscroll[row];

			// count consecutive rows scrolled by the same amount
			for (grouprows = 1; row + grouprows < numrows; grouprows++)
					if (rowscroll[row + grouprows] != xscroll)
					break;

			// iterate over reps of the rows in question
			xscroll = normalize_xscroll(src, xscroll);
			for (INT32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
			{
				// compute the cliprect for this group
				subclip.min_y = row * rowheight + sy;
				subclip.max_y = (row + grouprows) * rowheight - 1 + sy;
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (INT32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
					copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, trans_pen);
			}
		}
	}
}

void copyscrollbitmap_trans(bitmap_ind16 &dest, bitmap_ind16 &src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle &cliprect, UINT32 trans_pen)
{ copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen); }

void copyscrollbitmap_trans(bitmap_rgb32 &dest, bitmap_rgb32 &src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle &cliprect, UINT32 trans_pen)
{ copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen); }



/***************************************************************************
    COPYROZBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copyrozbitmap - copy from one bitmap to another,
    with zoom and rotation, copying all unclipped
    pixels
-------------------------------------------------*/

void copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, bitmap_ind16 &src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound)
{
	DECLARE_NO_PRIORITY;
	COPYROZBITMAP_CORE(UINT16, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}

void copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, bitmap_rgb32 &src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound)
{
	DECLARE_NO_PRIORITY;
	COPYROZBITMAP_CORE(UINT32, PIXEL_OP_COPY_OPAQUE, NO_PRIORITY);
}


/*-------------------------------------------------
    copyrozbitmap_trans - copy from one bitmap to
    another, with zoom and rotation, copying all
    unclipped pixels whose values do not match
    transpen
-------------------------------------------------*/

void copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, bitmap_ind16 &src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound, UINT32 trans_pen)
{
	DECLARE_NO_PRIORITY;
	COPYROZBITMAP_CORE(UINT16, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
}

void copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, bitmap_rgb32 &src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound, UINT32 trans_pen)
{
	DECLARE_NO_PRIORITY;
	COPYROZBITMAP_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN, NO_PRIORITY);
}

GFXDECODE_START( empty )
GFXDECODE_END
