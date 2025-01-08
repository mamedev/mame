// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/*********************************************************************

    drawgfx.cpp

    Generic graphic functions.

*********************************************************************/

#include "emu.h"
#include "drawgfxt.ipp"


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    readbit - read a single bit from a base
    offset
-------------------------------------------------*/

constexpr int readbit(const u8 *src, unsigned int bitnum)
{
	return src[bitnum / 8] & (0x80 >> (bitnum % 8));
}


/*-------------------------------------------------
    normalize_xscroll - normalize an X scroll
    value for a bitmap to be positive and less
    than the width
-------------------------------------------------*/

static inline s32 normalize_xscroll(const bitmap_t &bitmap, s32 xscroll)
{
	return (xscroll >= 0) ? xscroll % bitmap.width() : (bitmap.width() - (-xscroll) % bitmap.width());
}


/*-------------------------------------------------
    normalize_yscroll - normalize a Y scroll
    value for a bitmap to be positive and less
    than the height
-------------------------------------------------*/

static inline s32 normalize_yscroll(const bitmap_t &bitmap, s32 yscroll)
{
	return (yscroll >= 0) ? yscroll % bitmap.height() : (bitmap.height() - (-yscroll) % bitmap.height());
}



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GFXDECODE, gfxdecode_device, "gfxdecode", "gfxdecode")

gfxdecode_device::gfxdecode_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GFXDECODE, tag, owner, clock),
	device_gfx_interface(mconfig, *this)
{
}



/***************************************************************************
    GRAPHICS ELEMENTS
***************************************************************************/

//-------------------------------------------------
//  gfx_element - constructor
//-------------------------------------------------

gfx_element::gfx_element(device_palette_interface *palette, u8 *base, u16 width, u16 height, u32 rowbytes, u32 total_colors, u32 color_base, u32 color_granularity) :
	m_palette(palette),
	m_width(width),
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
	m_layout_xormask(0),
	m_layout_charincrement(0)
{
}

gfx_element::gfx_element(device_palette_interface *palette, const gfx_layout &gl, const u8 *srcdata, u32 xormask, u32 total_colors, u32 color_base) :
	m_palette(palette),
	m_width(0),
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
	m_srcdata(nullptr),
	m_dirtyseq(1),
	m_gfxdata(nullptr),
	m_layout_is_raw(false),
	m_layout_planes(0),
	m_layout_xormask(xormask),
	m_layout_charincrement(0)
{
	// set the layout
	set_layout(gl, srcdata);
}


//-------------------------------------------------
//  set_layout - set the layout for a gfx_element
//-------------------------------------------------

void gfx_element::set_layout(const gfx_layout &gl, const u8 *srcdata)
{
	m_srcdata = srcdata;

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

	// raw graphics case
	if (m_layout_is_raw)
	{
		// RAW layouts don't need these arrays
		m_layout_planeoffset.clear();
		m_layout_xoffset.clear();
		m_layout_yoffset.clear();
		m_gfxdata_allocated.clear();

		// modulos are determined for us by the layout
		m_line_modulo = gl.yoffs(0) / 8;
		m_char_modulo = gl.charincrement / 8;

		// RAW graphics must have a pointer up front
		assert(srcdata != nullptr);
		m_gfxdata = const_cast<u8 *>(srcdata);
	}

	// decoded graphics case
	else
	{
		// copy offsets
		m_layout_planeoffset.resize(m_layout_planes);
		m_layout_xoffset.resize(m_width);
		m_layout_yoffset.resize(m_height);

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
	memset(&m_dirty[0], 1, m_total_elements);

	// allocate a pen usage array for entries with 32 pens or less
	if (m_color_depth <= 32)
		m_pen_usage.resize(m_total_elements);
	else
		m_pen_usage.clear();
}


//-------------------------------------------------
//  set_raw_layout - set the layout for a gfx_element
//-------------------------------------------------

void gfx_element::set_raw_layout(const u8 *srcdata, u32 width, u32 height, u32 total, u32 linemod, u32 charmod)
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
// set_source - set the source data for a gfx_element
//-------------------------------------------------

void gfx_element::set_source(const u8 *source)
{
	m_srcdata = source;
	memset(&m_dirty[0], 1, elements());
	if (m_layout_is_raw) m_gfxdata = const_cast<u8 *>(source);
}


//-------------------------------------------------
// set_source_and_total - set the source data
// and total elements for a gfx_element
//-------------------------------------------------

void gfx_element::set_source_and_total(const u8 *source, u32 total)
{
	m_srcdata = source;
	m_total_elements = total;

	// mark everything dirty
	m_dirty.resize(m_total_elements);
	memset(&m_dirty[0], 1, m_total_elements);

	// allocate a pen usage array for entries with 32 pens or less
	if (m_color_depth <= 32)
		m_pen_usage.resize(m_total_elements);

	if (m_layout_is_raw)
	{
		m_gfxdata = const_cast<u8 *>(source);
	}
	else
	{
		// allocate memory for the data
		m_gfxdata_allocated.resize(m_total_elements * m_char_modulo);
		m_gfxdata = &m_gfxdata_allocated[0];
	}
}


//-------------------------------------------------
//  set_source_clip - set a source clipping rect
//-------------------------------------------------

void gfx_element::set_source_clip(u32 xoffs, u32 width, u32 yoffs, u32 height)
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

void gfx_element::decode(u32 code)
{
	// don't decode GFX_RAW
	if (!m_layout_is_raw)
	{
		// zap the data to 0
		u8 *decode_base = m_gfxdata + code * m_char_modulo;
		memset(decode_base, 0, m_char_modulo);

		// iterate over planes
		int plane, planebit;
		for (plane = 0, planebit = 1 << (m_layout_planes - 1);
				plane < m_layout_planes;
				plane++, planebit >>= 1)
		{
			int planeoffs = code * m_layout_charincrement + m_layout_planeoffset[plane];

			// iterate over rows
			for (int y = 0; y < m_origheight; y++)
			{
				int yoffs = planeoffs + m_layout_yoffset[y];
				u8 *dp = decode_base + y * m_line_modulo;

				// iterate over columns
				for (int x = 0; x < m_origwidth; x++)
					if (readbit(m_srcdata, (yoffs + m_layout_xoffset[x]) ^ m_layout_xormask))
						dp[x] |= planebit;
			}
		}
	}

	// (re)compute pen usage
	if (code < m_pen_usage.size())
	{
		// iterate over data, creating a bitmask of live pens
		const u8 *dp = m_gfxdata + code * m_char_modulo;
		u32 usage = 0;
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

void gfx_element::opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty)
{
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE(destp, srcp); });
}

void gfx_element::opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty)
{
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });
}


/*-------------------------------------------------
    transpen - render a gfx element with
    a single transparent pen
-------------------------------------------------*/

void gfx_element::transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void gfx_element::transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_pen, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN(destp, srcp); });
}


/*-------------------------------------------------
    transpen_raw - render a gfx element
    with a single transparent pen and no color
    lookups
-------------------------------------------------*/

void gfx_element::transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void gfx_element::transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_pen, color](u32 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}


/*-------------------------------------------------
    transmask - render a gfx element
    with a multiple transparent pens provided as
    a mask
-------------------------------------------------*/

void gfx_element::transmask(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_mask, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSMASK(destp, srcp); });
}

void gfx_element::transmask(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);
	}

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_mask, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSMASK(destp, srcp); });
}


/*-------------------------------------------------
    transtable - render a gfx element
    using a table to look up which pens are
    transparent, opaque, or shadowing
-------------------------------------------------*/

void gfx_element::transtable(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		const u8 *pentable)
{
	assert(pentable != nullptr);

	// render
	color = colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [pentable, color, shadowtable](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSTABLE16(destp, srcp); });
}

void gfx_element::transtable(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		const u8 *pentable)
{
	assert(pentable != nullptr);

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [pentable, paldata, shadowtable](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSTABLE32(destp, srcp); });
}


/*-------------------------------------------------
    alpha - render a gfx element with
    a single transparent pen, alpha blending the
    remaining pixels with a fixed alpha value
-------------------------------------------------*/

void gfx_element::alpha(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 trans_pen, u8 alpha_val)
{
	// special case alpha = 0xff
	if (alpha_val == 0xff)
		return transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// get final code and color, and grab lookup tables
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [trans_pen, alpha_val, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_ALPHA32(destp, srcp); });
}



/***************************************************************************
    DRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    zoom_opaque - render a scaled gfx
    element with no transparency
-------------------------------------------------*/

void gfx_element::zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE(destp, srcp); });
}

void gfx_element::zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });
}


/*-------------------------------------------------
    zoom_transpen - render a scaled gfx
    element with a single transparent pen
-------------------------------------------------*/

void gfx_element::zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void gfx_element::zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_pen, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN(destp, srcp); });
}


/*-------------------------------------------------
    zoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups
-------------------------------------------------*/

void gfx_element::zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void gfx_element::zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_pen, color](u32 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}


/*-------------------------------------------------
    zoom_transmask - render a scaled gfx
    element with a multiple transparent pens
    provided as a mask
-------------------------------------------------*/

void gfx_element::zoom_transmask(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transmask(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_mask, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSMASK(destp, srcp); });
}

void gfx_element::zoom_transmask(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transmask(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley);
	}

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_mask, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSMASK(destp, srcp); });
}


/*-------------------------------------------------
    zoom_transtable - render a scaled gfx
    element using a table to look up which pens
    are transparent, opaque, or shadowing
-------------------------------------------------*/

void gfx_element::zoom_transtable(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, const u8 *pentable)
{
	assert(pentable != nullptr);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transtable(dest, cliprect, code, color, flipx, flipy, destx, desty, pentable);

	// render
	color = colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [pentable, color, shadowtable](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSTABLE16(destp, srcp); });
}

void gfx_element::zoom_transtable(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, const u8 *pentable)
{
	assert(pentable != nullptr);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return transtable(dest, cliprect, code, color, flipx, flipy, destx, desty, pentable);

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [pentable, paldata, shadowtable](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSTABLE32(destp, srcp); });
}


/*-------------------------------------------------
    zoom_alpha - render a scaled gfx element
    with a single transparent pen, alpha blending
    the remaining pixels with a fixed alpha value
-------------------------------------------------*/

void gfx_element::zoom_alpha(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, u32 trans_pen, u8 alpha_val)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return alpha(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen, alpha_val);

	// special case alpha_val = 0xff
	if (alpha_val == 0xff)
		return zoom_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, [trans_pen, alpha_val, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_ALPHA32(destp, srcp); });
}



/***************************************************************************
    PDRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    prio_opaque - render a gfx element with
    no transparency, checking against the priority
    bitmap
-------------------------------------------------*/

void gfx_element::prio_opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_transpen - render a gfx element with
    a single transparent pen, checking against the
    priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_pen, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    priotranspen_raw - render a gfx element
    with a single transparent pen and no color
    lookups, checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_pen, color](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_transmask - render a gfx element
    with a multiple transparent pens provided as
    a mask, checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transmask(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_mask, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSMASK_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_transmask(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_mask)
{
	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_mask, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSMASK_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_transtable - render a gfx element
    using a table to look up which pens are
    transparent, opaque, or shadowing, checking
    against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_transtable(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, const u8 *pentable)
{
	assert(pentable != nullptr);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, pentable, color, shadowtable](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSTABLE16_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_transtable(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, const u8 *pentable)
{
	assert(pentable != nullptr);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, pentable, paldata, shadowtable](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_alpha - render a gfx element with
    a single transparent pen, alpha blending the
    remaining pixels with a fixed alpha value,
    checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_alpha(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen, u8 alpha_val)
{
	// special case alpha = 0xff
	if (alpha_val == 0xff)
		return prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_pen, alpha_val, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY(destp, pri, srcp); });
}



/***************************************************************************
    PDRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    prio_zoom_opaque - render a scaled gfx
    element with no transparency, checking against
    the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_transpen - render a scaled gfx
    element with a single transparent pen,
    checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_pen, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups, checking against the priority
    bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
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
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
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
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_pen, color](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_transmask - render a scaled gfx
    element with a multiple transparent pens
    provided as a mask, checking against the
    priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transmask(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transmask(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_mask, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSMASK_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_zoom_transmask(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_mask)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transmask(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_mask);

	// special case 0 mask to opaque
	if (trans_mask == 0)
		return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);

	// use pen usage to optimize
	code %= elements();
	if (has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = pen_usage(code);
		if ((usage & ~trans_mask) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & trans_mask) == 0)
			return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_mask, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSMASK_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_transtable - render a scaled gfx
    element using a table to look up which pens
    are transparent, opaque, or shadowing,
    checking against the priority bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_transtable(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		const u8 *pentable)
{
	assert(pentable != nullptr);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transtable(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, pentable);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, pentable, color, shadowtable](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSTABLE16_PRIORITY(destp, pri, srcp); });
}

void gfx_element::prio_zoom_transtable(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		const u8 *pentable)
{
	assert(pentable != nullptr);

	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_transtable(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, pentable);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	const pen_t *shadowtable = m_palette->shadow_table();
	code %= elements();
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, pentable, paldata, shadowtable](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSTABLE32_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_alpha - render a scaled gfx
    element with a single transparent pen, alpha
    blending the remaining pixels with a fixed
    alpha value, checking against the priority
    bitmap
-------------------------------------------------*/

void gfx_element::prio_zoom_alpha(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen, u8 alpha_val)
{
	// non-zoom case
	if (scalex == 0x10000 && scaley == 0x10000)
		return prio_alpha(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen, alpha_val);

	// special case alpha_val = 0xff
	if (alpha_val == 0xff)
		return prio_zoom_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, scalex, scaley, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= elements();
	if (has_pen_usage() && (pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = m_palette->pens() + colorbase() + granularity() * (color % colors());
	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_pen, alpha_val, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_ALPHA32_PRIORITY(destp, pri, srcp); });
}


#define PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32(DEST, PRIORITY, SOURCE)          \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != trans_pen)                                                       \
	{                                                                               \
		if (((1 << ((PRIORITY) & 0x1f)) & pmask) == 0)                              \
			(DEST) = add_blend_r32((DEST), paldata[srcdata]);                       \
		(PRIORITY) = 31;                                                            \
	}                                                                               \
}                                                                                   \
while (0)

void gfx_element::prio_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	const pen_t *paldata;

	assert(dest.valid());
	assert(dest.bpp() == 32);

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = m_palette->pens() + colorbase() + granularity() * color;

	/* use pen usage to optimize */
	if (has_pen_usage())
	{
		u32 usage = pen_usage(code);

		/* fully transparent; do nothing */
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	/* render based on dest bitmap depth */
	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, priority, [pmask, trans_pen, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32(destp, pri, srcp); });
}


void gfx_element::prio_zoom_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	const pen_t *paldata;

	/* non-zoom case */

	if (scalex == 0x10000 && scaley == 0x10000)
	{
		prio_transpen_additive(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);
		return;
	}

	assert(dest.valid());
	assert(dest.bpp() == 32);

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = m_palette->pens() + colorbase() + granularity() * color;

	/* use pen usage to optimize */
	if (has_pen_usage())
	{
		u32 usage = pen_usage(code);

		/* fully transparent; do nothing */
		if ((usage & ~(1 << trans_pen)) == 0)
			return;
	}

	/* high bit of the mask is implicitly on */
	pmask |= 1 << 31;

	drawgfxzoom_core(dest, cliprect, code, flipx, flipy, destx, desty, scalex, scaley, priority, [pmask, trans_pen, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_PRIORITY_ADDIIVE32(destp, pri, srcp); });
}

//#define MAKE_ARGB_RGB(a, rgb) rgb_t(a, rgb.r(), rgb.g(), rgb.b())
#define MAKE_ARGB_RGB(a, rgb)   rgb_t(rgb).set_a(a)

// combine in 'alpha' when copying to store in ARGB
#define PIXEL_OP_REMAP_TRANS0_ALPHASTORE32(DEST, SOURCE)                                  \
do                                                                                                  \
{                                                                                                   \
	u32 srcdata = (SOURCE);                                                                         \
	if (srcdata != 0)                                                                               \
		(DEST) = MAKE_ARGB_RGB(alpha,paldata[srcdata]);                                             \
}                                                                                                   \
while (0)
// combine in 'alphatable' value to store in ARGB
#define PIXEL_OP_REMAP_TRANS0_ALPHATABLESTORE32(DEST, SOURCE)                             \
do                                                                                                  \
{                                                                                                   \
	u32 srcdata = (SOURCE);                                                                         \
	if (srcdata != 0)                                                                               \
		(DEST) = MAKE_ARGB_RGB(alphatable[srcdata], paldata[srcdata]);                              \
}                                                                                                   \
while (0)
// drawgfxm.h macro to render alpha into 32-bit buffer
#define PIXEL_OP_REMAP_TRANS0_ALPHATABLE32(DEST, SOURCE)                                  \
do                                                                                                  \
{                                                                                                   \
	u32 srcdata = (SOURCE);                                                                         \
	if (srcdata != 0)                                                                               \
		(DEST) = alpha_blend_r32((DEST), paldata[srcdata], alphatable[srcdata]);                    \
}                                                                                                   \
while (0)

/*-------------------------------------------------
    alphastore - render a gfx element with
    a single transparent pen, storing the alpha value
    in alpha field of ARGB32, negative alpha implies alphatable
-------------------------------------------------*/
void gfx_element::alphastore(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		int fixedalpha, u8 *alphatable)
{
	const pen_t *paldata;

	assert(alphatable != nullptr);

	/* if we have a fixed alpha, call the standard drawgfx_transpen */
	if (fixedalpha == 0xff)
	{
		transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, 0);
		return;
	}

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = m_palette->pens() + colorbase() + granularity() * color;

	/* early out if completely transparent */
	if (has_pen_usage() && (pen_usage(code) & ~(1 << 0)) == 0)
		return;

	if (fixedalpha >= 0)
		drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [paldata, alpha = fixedalpha](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANS0_ALPHASTORE32(destp, srcp); });
	else
		drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [paldata, alphatable](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANS0_ALPHATABLESTORE32(destp, srcp); });
}

/*-------------------------------------------------
    alphatable - render a sprite with either
    a fixed alpha value, or if alpha==-1 then uses
    the per-pen alphatable[] array
 -------------------------------------------------*/
void gfx_element::alphatable(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty,
		int fixedalpha, u8 *alphatable)
{
	const pen_t *paldata;

	/* if we have a fixed alpha, call the standard drawgfx_alpha */
	if (fixedalpha >= 0)
	{
		alpha(dest, cliprect, code, color, flipx, flipy, destx, desty, 0, fixedalpha);
		return;
	}

	assert(dest.bpp() == 32);
	assert(alphatable != nullptr);

	/* get final code and color, and grab lookup tables */
	code %= elements();
	color %= colors();
	paldata = m_palette->pens() + colorbase() + granularity() * color;

	/* early out if completely transparent */
	if (has_pen_usage() && (pen_usage(code) & ~(1 << 0)) == 0)
		return;

	drawgfx_core(dest, cliprect, code, flipx, flipy, destx, desty, [paldata, alphatable](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANS0_ALPHATABLE32(destp, srcp); });
}


/***************************************************************************
    DRAW_SCANLINE IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    draw_scanline8 - copy pixels from an 8bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline8(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata)
{
	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, [paldata](u16 &destp, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, [](u16 &destp, const u8 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void draw_scanline8(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata)
{
	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, [paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, [](u32 &destp, const u8 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void prio_draw_scanline8(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pmask](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pmask](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void prio_draw_scanline8(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pmask](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pmask](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void primask_draw_scanline8(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		return draw_scanline8(bitmap, destx, desty, length, srcptr, paldata);

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pcode, pmask](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIMASK(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pcode, pmask](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}

void primask_draw_scanline8(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		return draw_scanline8(bitmap, destx, desty, length, srcptr, paldata);

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pcode, pmask](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIMASK(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pcode, pmask](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}


/*-------------------------------------------------
    draw_scanline16 - copy pixels from a 16bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline16(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata)
{
	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, [paldata](u16 &destp, const u16 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, [](u16 &destp, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void draw_scanline16(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata)
{
	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, [paldata](u32 &destp, const u16 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, [](u32 &destp, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void prio_draw_scanline16(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void prio_draw_scanline16(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pmask](u32 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pmask](u32 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void primask_draw_scanline16(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		return draw_scanline16(bitmap, destx, desty, length, srcptr, paldata);

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pcode, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIMASK(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pcode, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}

void primask_draw_scanline16(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		return draw_scanline16(bitmap, destx, desty, length, srcptr, paldata);

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pcode, pmask](u32 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIMASK(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pcode, pmask](u32 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}


/*-------------------------------------------------
    draw_scanline32 - copy pixels from a 32bpp
    buffer to a single scanline of a bitmap
-------------------------------------------------*/

void draw_scanline32(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata)
{
	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, [paldata](u16 &destp, const u32 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, [](u16 &destp, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void draw_scanline32(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata)
{
	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, [paldata](u32 &destp, const u32 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, [](u32 &destp, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void prio_draw_scanline32(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pmask](u16 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pmask](u16 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void prio_draw_scanline32(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void primask_draw_scanline32(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		return draw_scanline32(bitmap, destx, desty, length, srcptr, paldata);

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pcode, pmask](u16 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIMASK(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pcode, pmask](u16 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}

void primask_draw_scanline32(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		return draw_scanline32(bitmap, destx, desty, length, srcptr, paldata);

	// palette lookup case
	if (paldata != nullptr)
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [paldata, pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIMASK(destp, pri, srcp); });

	// raw copy case
	else
		drawscanline_core(bitmap, destx, desty, length, srcptr, priority, [pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}



/***************************************************************************
    COPYBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copybitmap - copy from one bitmap to another,
    copying all unclipped pixels
-------------------------------------------------*/

void copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect)
{
	copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, [](u16 &destp, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect)
{
	copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, [](u32 &destp, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void prio_copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void prio_copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void primask_copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pcode, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}

void primask_copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}


/*-------------------------------------------------
    copybitmap_trans - copy from one bitmap to
    another, copying all unclipped pixels except
    those that match transpen
-------------------------------------------------*/

void copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 trans_pen)
{
	if (trans_pen > 0xffff)
		copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, [trans_pen](u16 &destp, const u16 &srcp) { PIXEL_OP_COPY_TRANSPEN(destp, srcp); });
}

void copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 trans_pen)
{
	if (trans_pen == 0xffffffff)
		copybitmap(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, [trans_pen](u32 &destp, const u32 &srcp) { PIXEL_OP_COPY_TRANSPEN(destp, srcp); });
}

void prio_copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	if (trans_pen > 0xffff)
		prio_copybitmap(dest, src, flipx, flipy, destx, desty, cliprect, priority, pmask);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pmask, trans_pen](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void prio_copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	if (trans_pen == 0xffffffff)
		prio_copybitmap(dest, src, flipx, flipy, destx, desty, cliprect, priority, pmask);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pmask, trans_pen](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void primask_copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 trans_pen, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copybitmap_trans(dest, src, flipx, flipy, destx, desty, cliprect, trans_pen);
	else if (trans_pen > 0xffff)
		primask_copybitmap(dest, src, flipx, flipy, destx, desty, cliprect, priority, pcode, pmask);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [trans_pen, pcode, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIMASK(destp, pri, srcp); });
}

void primask_copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 trans_pen, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copybitmap_trans(dest, src, flipx, flipy, destx, desty, cliprect, trans_pen);
	else if (trans_pen == 0xffffffff)
		primask_copybitmap(dest, src, flipx, flipy, destx, desty, cliprect, priority, pcode, pmask);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [trans_pen, pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIMASK(destp, pri, srcp); });
}


/*-------------------------------------------------
    copybitmap_transalphpa - copy from one bitmap
    to another, copying all unclipped pixels except
    those with an alpha value of zero
-------------------------------------------------*/

void copybitmap_transalpha(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect)
{
	copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, [](u32 &destp, const u32 &srcp) { PIXEL_OP_COPY_TRANSALPHA(destp, srcp); });
}

void prio_copybitmap_transalpha(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_TRANSALPHA_PRIORITY(destp, pri, srcp); });
}

void primask_copybitmap_transalpha(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copybitmap_transalpha(dest, src, flipx, flipy, destx, desty, cliprect);
	else
		copybitmap_core(dest, src, flipx, flipy, destx, desty, cliprect, priority, [pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_TRANSALPHA_PRIMASK(destp, pri, srcp); });
}


/***************************************************************************
    COPYSCROLLBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copyscrollbitmap - copy from one bitmap to
    another, copying all unclipped pixels, and
    applying scrolling to one or more rows/columns
-------------------------------------------------*/

void copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff);
}

void copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff);
}

void prio_copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	prio_copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, priority, pmask, 0xffffffff);
}

void prio_copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	prio_copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, priority, pmask, 0xffffffff);
}

void primask_copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	if (pcode == 0 && pmask == 0xff)
		copyscrollbitmap(dest, src, numrows, rowscroll, numcols, colscroll, cliprect);
	else
		primask_copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff, priority, pcode, pmask);
}

void primask_copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	// just call through to the transparent case as the underlying copybitmap will
	// optimize for pen == 0xffffffff
	if (pcode == 0 && pmask == 0xff)
		copyscrollbitmap(dest, src, numrows, rowscroll, numcols, colscroll, cliprect);
	else
		primask_copyscrollbitmap_trans(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, 0xffffffff, priority, pcode, pmask);
}


/*-------------------------------------------------
    copyscrollbitmap_trans - copy from one bitmap
    to another, copying all unclipped pixels
    except those that match transpen, and applying
    scrolling to one or more rows/columns
-------------------------------------------------*/

template<class BitmapClass>
static inline void copyscrollbitmap_trans_common(BitmapClass &dest, const BitmapClass &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 trans_pen)
{
	// no rowscroll and no colscroll means no scroll
	if (numrows == 0 && numcols == 0)
		return copybitmap_trans(dest, src, 0, 0, 0, 0, cliprect, trans_pen);

	assert(numrows != 0 || rowscroll == nullptr);
	assert(numrows == 0 || rowscroll != nullptr);
	assert(numcols != 0 || colscroll == nullptr);
	assert(numcols == 0 || colscroll != nullptr);

	// fully scrolling X,Y playfield
	if (numrows <= 1 && numcols <= 1)
	{
		s32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		s32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);

		// iterate over all portions of the scroll that overlap the destination
		for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
				copybitmap_trans(dest, src, 0, 0, sx, sy, cliprect, trans_pen);
	}

	// scrolling columns plus horizontal scroll
	else if (numrows <= 1)
	{
		s32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each column
		int colwidth = src.width() / numcols;
		assert(src.width() % colwidth == 0);

		// iterate over each column
		int groupcols;
		for (int col = 0; col < numcols; col += groupcols)
		{
			s32 yscroll = colscroll[col];

			// count consecutive columns scrolled by the same amount
			for (groupcols = 1; col + groupcols < numcols; groupcols++)
					if (colscroll[col + groupcols] != yscroll)
					break;

			// iterate over reps of the columns in question
			yscroll = normalize_yscroll(src, yscroll);
			for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			{
				// compute the cliprect for this group
				subclip.setx(col * colwidth + sx, (col + groupcols) * colwidth - 1 + sx);
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
					copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, trans_pen);
			}
		}
	}

	// scrolling rows plus vertical scroll
	else if (numcols <= 1)
	{
		s32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each rows
		int rowheight = src.height() / numrows;
		assert(src.height() % rowheight == 0);

		// iterate over each row
		int grouprows;
		for (int row = 0; row < numrows; row += grouprows)
		{
			s32 xscroll = rowscroll[row];

			// count consecutive rows scrolled by the same amount
			for (grouprows = 1; row + grouprows < numrows; grouprows++)
					if (rowscroll[row + grouprows] != xscroll)
					break;

			// iterate over reps of the rows in question
			xscroll = normalize_xscroll(src, xscroll);
			for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
			{
				// compute the cliprect for this group
				subclip.sety(row * rowheight + sy, (row + grouprows) * rowheight - 1 + sy);
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
					copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, trans_pen);
			}
		}
	}
}

void copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 trans_pen)
{ copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen); }

void copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 trans_pen)
{ copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen); }

template<class BitmapClass>
static inline void prio_copyscrollbitmap_trans_common(BitmapClass &dest, const BitmapClass &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// no rowscroll and no colscroll means no scroll
	if (numrows == 0 && numcols == 0)
		return prio_copybitmap_trans(dest, src, 0, 0, 0, 0, cliprect, priority, pmask, trans_pen);

	assert(numrows != 0 || rowscroll == nullptr);
	assert(numrows == 0 || rowscroll != nullptr);
	assert(numcols != 0 || colscroll == nullptr);
	assert(numcols == 0 || colscroll != nullptr);

	// fully scrolling X,Y playfield
	if (numrows <= 1 && numcols <= 1)
	{
		s32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		s32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);

		// iterate over all portions of the scroll that overlap the destination
		for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
				prio_copybitmap_trans(dest, src, 0, 0, sx, sy, cliprect, priority, pmask, trans_pen);
	}

	// scrolling columns plus horizontal scroll
	else if (numrows <= 1)
	{
		s32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each column
		int colwidth = src.width() / numcols;
		assert(src.width() % colwidth == 0);

		// iterate over each column
		int groupcols;
		for (int col = 0; col < numcols; col += groupcols)
		{
			s32 yscroll = colscroll[col];

			// count consecutive columns scrolled by the same amount
			for (groupcols = 1; col + groupcols < numcols; groupcols++)
					if (colscroll[col + groupcols] != yscroll)
					break;

			// iterate over reps of the columns in question
			yscroll = normalize_yscroll(src, yscroll);
			for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			{
				// compute the cliprect for this group
				subclip.setx(col * colwidth + sx, (col + groupcols) * colwidth - 1 + sx);
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
					prio_copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, priority, pmask, trans_pen);
			}
		}
	}

	// scrolling rows plus vertical scroll
	else if (numcols <= 1)
	{
		s32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each rows
		int rowheight = src.height() / numrows;
		assert(src.height() % rowheight == 0);

		// iterate over each row
		int grouprows;
		for (int row = 0; row < numrows; row += grouprows)
		{
			s32 xscroll = rowscroll[row];

			// count consecutive rows scrolled by the same amount
			for (grouprows = 1; row + grouprows < numrows; grouprows++)
					if (rowscroll[row + grouprows] != xscroll)
					break;

			// iterate over reps of the rows in question
			xscroll = normalize_xscroll(src, xscroll);
			for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
			{
				// compute the cliprect for this group
				subclip.sety(row * rowheight + sy, (row + grouprows) * rowheight - 1 + sy);
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
					prio_copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, priority, pmask, trans_pen);
			}
		}
	}
}

void prio_copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{ prio_copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, priority, pmask, trans_pen); }

void prio_copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{ prio_copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, priority, pmask, trans_pen); }

template<class BitmapClass>
static inline void primask_copyscrollbitmap_trans_common(BitmapClass &dest, const BitmapClass &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 trans_pen, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff)
{
	// no rowscroll and no colscroll means no scroll
	if (pcode == 0 && pmask == 0xff)
		return copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen);

	if (numrows == 0 && numcols == 0)
		return primask_copybitmap_trans(dest, src, 0, 0, 0, 0, cliprect, trans_pen, priority, pcode, pmask);

	assert(numrows != 0 || rowscroll == nullptr);
	assert(numrows == 0 || rowscroll != nullptr);
	assert(numcols != 0 || colscroll == nullptr);
	assert(numcols == 0 || colscroll != nullptr);

	// fully scrolling X,Y playfield
	if (numrows <= 1 && numcols <= 1)
	{
		s32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		s32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);

		// iterate over all portions of the scroll that overlap the destination
		for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
				primask_copybitmap_trans(dest, src, 0, 0, sx, sy, cliprect, trans_pen, priority, pcode, pmask);
	}

	// scrolling columns plus horizontal scroll
	else if (numrows <= 1)
	{
		s32 xscroll = normalize_xscroll(src, (numrows == 0) ? 0 : rowscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each column
		int colwidth = src.width() / numcols;
		assert(src.width() % colwidth == 0);

		// iterate over each column
		int groupcols;
		for (int col = 0; col < numcols; col += groupcols)
		{
			s32 yscroll = colscroll[col];

			// count consecutive columns scrolled by the same amount
			for (groupcols = 1; col + groupcols < numcols; groupcols++)
					if (colscroll[col + groupcols] != yscroll)
					break;

			// iterate over reps of the columns in question
			yscroll = normalize_yscroll(src, yscroll);
			for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
			{
				// compute the cliprect for this group
				subclip.setx(col * colwidth + sx, (col + groupcols) * colwidth - 1 + sx);
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
					primask_copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, trans_pen, priority, pcode, pmask);
			}
		}
	}

	// scrolling rows plus vertical scroll
	else if (numcols <= 1)
	{
		s32 yscroll = normalize_yscroll(src, (numcols == 0) ? 0 : colscroll[0]);
		rectangle subclip = cliprect;

		// determine width of each rows
		int rowheight = src.height() / numrows;
		assert(src.height() % rowheight == 0);

		// iterate over each row
		int grouprows;
		for (int row = 0; row < numrows; row += grouprows)
		{
			s32 xscroll = rowscroll[row];

			// count consecutive rows scrolled by the same amount
			for (grouprows = 1; row + grouprows < numrows; grouprows++)
					if (rowscroll[row + grouprows] != xscroll)
					break;

			// iterate over reps of the rows in question
			xscroll = normalize_xscroll(src, xscroll);
			for (s32 sy = yscroll - src.height(); sy < dest.height(); sy += src.height())
			{
				// compute the cliprect for this group
				subclip.sety(row * rowheight + sy, (row + grouprows) * rowheight - 1 + sy);
				subclip &= cliprect;

				// iterate over all portions of the scroll that overlap the destination
				for (s32 sx = xscroll - src.width(); sx < dest.width(); sx += src.width())
					primask_copybitmap_trans(dest, src, 0, 0, sx, sy, subclip, trans_pen, priority, pcode, pmask);
			}
		}
	}
}

void primask_copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 trans_pen, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{ primask_copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen, priority, pcode, pmask); }

void primask_copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 trans_pen, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{ primask_copyscrollbitmap_trans_common(dest, src, numrows, rowscroll, numcols, colscroll, cliprect, trans_pen, priority, pcode, pmask); }


/***************************************************************************
    COPYROZBITMAP IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    copyrozbitmap - copy from one bitmap to another,
    with zoom and rotation, copying all unclipped
    pixels
-------------------------------------------------*/

void copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound)
{
	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, [](u16 &destp, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound)
{
	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, [](u32 &destp, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE(destp, srcp); });
}

void prio_copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void prio_copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, bitmap_ind8 &priority, u32 pmask)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void primask_copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copyrozbitmap(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound);
	else
		copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [pcode, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}

void primask_copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copyrozbitmap(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound);
	else
		copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_OPAQUE_PRIMASK(destp, pri, srcp); });
}


/*-------------------------------------------------
    copyrozbitmap_trans - copy from one bitmap to
    another, with zoom and rotation, copying all
    unclipped pixels whose values do not match
    transpen
-------------------------------------------------*/

void copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, u32 trans_pen)
{
	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, [trans_pen](u16 &destp, const u16 &srcp) { PIXEL_OP_COPY_TRANSPEN(destp, srcp); });
}

void copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, u32 trans_pen)
{
	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, [trans_pen](u32 &destp, const u32 &srcp) { PIXEL_OP_COPY_TRANSPEN(destp, srcp); });
}

void prio_copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [pmask, trans_pen](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void prio_copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [pmask, trans_pen](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void primask_copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, u32 trans_pen, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copyrozbitmap_trans(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, trans_pen);
	else
		copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [trans_pen, pcode, pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIMASK(destp, pri, srcp); });
}

void primask_copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, bool wraparound, u32 trans_pen, bitmap_ind8 &priority, u8 pcode, u8 pmask)
{
	if (pcode == 0 && pmask == 0xff)
		copyrozbitmap_trans(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, trans_pen);
	else
		copyrozbitmap_core(dest, cliprect, src, startx, starty, incxx, incxy, incyx, incyy, wraparound, priority, [trans_pen, pcode, pmask](u32 &destp, u8 &pri, const u32 &srcp) { PIXEL_OP_COPY_TRANSPEN_PRIMASK(destp, pri, srcp); });
}
