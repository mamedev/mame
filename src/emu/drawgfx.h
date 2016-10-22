// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/*********************************************************************

    drawgfx.h

    Generic graphic functions.
**********************************************************************

    How to use priority-masked drawing (formerly pdrawgfx):

    There are two different standard ways to use the priority bitmap
    and the priority-masked draw methods, depending on how many layers
    of interest (tilemap or other layers that individual sprites can
    be either "behind" or "in front of") your driver has.

    In the more common scheme, which you can use when the number of
    layers of interest is four or fewer, the priority bitmap contains
    a bitmask indicating which layers are opaque at each location.
    To use this scheme, draw your tilemap layers this way, in order
    from back to front:

        screen.priority().fill(0, cliprect);
        m_tilemap1->draw(screen, bitmap, cliprect, tmap1flags, 1);
        m_tilemap2->draw(screen, bitmap, cliprect, tmap2flags, 2);
        m_tilemap3->draw(screen, bitmap, cliprect, tmap3flags, 4);
        m_tilemap4->draw(screen, bitmap, cliprect, tmap4flags, 8);

    Now, when drawing your sprites, the pmask parameter for each
    sprite should be the bitwise OR of all the GFX_PMASK_n constants
    corresponding to layers that the sprite should be masked by.
    For example, to draw a sprite that appears over tilemap1, but
    under opaque pixels of tilemap2, tilemap3, and tilemap4:

        uint32_t pmask = GFX_PMASK_2 | GFX_PMASK_4 | GFX_PMASK_8;
        gfx->prio_transpen(bitmap, cliprect,
                code, color,
                flipx, flipy,
                sx, sy,
                screen.priority(),
                pmask,
                trans_pen);

    This scheme does not require priority to be transitive: it is
    perfectly possible for a sprite to be "under" tilemap1 but "over"
    tilemap4, even though tilemap1 itself is "under" tilemap4.

    If you have more than four layers, you need to use a different
    scheme, in which the priority bitmap contains the index of the
    topmost opaque layer rather than a bitmask of all the opaque
    layers. To use this scheme, draw your tilemaps this way, again
    in order from back to front:

        screen.priority().fill(0, cliprect);
        m_tilemap1->draw(screen, bitmap, cliprect, tmap1flags, 1, 0);
        m_tilemap2->draw(screen, bitmap, cliprect, tmap2flags, 2, 0);
        m_tilemap3->draw(screen, bitmap, cliprect, tmap3flags, 3, 0);
        m_tilemap4->draw(screen, bitmap, cliprect, tmap4flags, 4, 0);
        m_tilemap5->draw(screen, bitmap, cliprect, tmap5flags, 5, 0);
        m_tilemap6->draw(screen, bitmap, cliprect, tmap6flags, 6, 0);
        m_tilemap7->draw(screen, bitmap, cliprect, tmap7flags, 7, 0);
        m_tilemap8->draw(screen, bitmap, cliprect, tmap8flags, 8, 0);

    Notice the additional 0 parameter to tilemap_t::draw(). This
    parameter causes the new layer's priority code to replace that of
    the underlying layer instead of being ORed with it (the parameter
    is a mask to be ANDed with the previous contents of the priority
    bitmap before the new code is ORed with it)

    You need to use a different pmask for your sprites with this
    scheme than with the previous scheme. The pmask should be set to
    ((~1) << n), where n is the index of the highest priority layer
    that the sprite should *not* be masked by. For example, to draw
    a sprite over the first four tilemaps but under the higher
    numbered ones:

        uint32_t pmask = (~1) << 4;
        gfx->prio_transpen(bitmap, cliprect,
                code, color,
                flipx, flipy,
                sx, sy,
                screen.priority(),
                pmask,
                trans_pen);

    Unlike the other scheme, this one does require priority to be
    transitive, because the priority bitmap only contains information
    about the topmost opaque pixel.

    These examples have used a different tilemap for each layer, but
    the layers could just as easily be different tile categories or
    pen layers from the same tilemap.

    If you have a layer that is behind all sprites, draw it with
    priority 0, and if you have a layer that is in front of all
    sprites, just draw it after the sprites. The bitmask scheme
    can handle up to 6 layers if you count "behind all sprites" and
    "in front of all sprites".

    An important thing to remember when using priority-masked drawing
    is that the sprites are drawn from front to back. Sprite pixels
    will not be drawn over already-drawn sprite pixels, even if the
    previously-drawn pixel was masked by a background layer.
    This reflects the fact that in most hardware, sprite-to-sprite
    priority is unrelated to sprite-to-background priority. Your
    sprites need to be pre-sorted by their sprite-to-sprite priority
    (whether that be a field in the sprite attributes or simply their
    order in sprite RAM) before drawing.


*********************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DRAWGFX_H__
#define __DRAWGFX_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	DRAWMODE_NONE,
	DRAWMODE_SOURCE,
	DRAWMODE_SHADOW
};

enum
{
	GFX_PMASK_1  = 0xaaaa,
	GFX_PMASK_2  = 0xcccc,
	GFX_PMASK_4  = 0xf0f0,
	GFX_PMASK_8  = 0xff00
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class gfx_element
{
public:
	// construction/destruction
#ifdef UNUSED_FUNCTION
	gfx_element();
#endif
	gfx_element(palette_device &palette, const gfx_layout &gl, const uint8_t *srcdata, uint32_t xormask, uint32_t total_colors, uint32_t color_base);
	gfx_element(palette_device &palette, uint8_t *base, uint16_t width, uint16_t height, uint32_t rowbytes, uint32_t total_colors, uint32_t color_base, uint32_t color_granularity);

	// getters
	palette_device &palette() const { return *m_palette; }
	uint16_t width() const { return m_width; }
	uint16_t height() const { return m_height; }
	uint32_t elements() const { return m_total_elements; }
	uint32_t colorbase() const { return m_color_base; }
	uint16_t depth() const { return m_color_depth; }
	uint16_t granularity() const { return m_color_granularity; }
	uint32_t colors() const { return m_total_colors; }
	uint32_t rowbytes() const { return m_line_modulo; }
	bool has_pen_usage() const { return !m_pen_usage.empty(); }

	// used by tilemaps
	uint32_t dirtyseq() const { return m_dirtyseq; }

	// setters
	void set_layout(const gfx_layout &gl, const uint8_t *srcdata);
	void set_raw_layout(const uint8_t *srcdata, uint32_t width, uint32_t height, uint32_t total, uint32_t linemod, uint32_t charmod);
	void set_source(const uint8_t *source);
	void set_source_and_total(const uint8_t *source, uint32_t total);
	void set_xormask(uint32_t xormask) { m_layout_xormask = xormask; }
	void set_palette(palette_device &palette) { m_palette = &palette; }
	void set_colors(uint32_t colors) { m_total_colors = colors; }
	void set_colorbase(uint16_t colorbase) { m_color_base = colorbase; }
	void set_granularity(uint16_t granularity) { m_color_granularity = granularity; }
	void set_source_clip(uint32_t xoffs, uint32_t width, uint32_t yoffs, uint32_t height);

	// operations
	void mark_dirty(uint32_t code) { if (code < elements()) { m_dirty[code] = 1; m_dirtyseq++; } }
	void mark_all_dirty() { memset(&m_dirty[0], 1, elements()); }

	const uint8_t *get_data(uint32_t code)
	{
		assert(code < elements());
		if (code < m_dirty.size() && m_dirty[code]) decode(code);
		return m_gfxdata + code * m_char_modulo + m_starty * m_line_modulo + m_startx;
	}

	uint32_t pen_usage(uint32_t code)
	{
		assert(code < m_pen_usage.size());
		if (m_dirty[code]) decode(code);
		return m_pen_usage[code];
	}

	// ----- core graphics drawing -----

	// specific drawgfx implementations for each transparency type
	void opaque(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty);
	void opaque(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty);
	void transpen(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transpen);
	void transpen(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transpen);
	void transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transpen);
	void transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transpen);
	void transmask(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transmask);
	void transmask(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transmask);
	void transtable(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, const uint8_t *pentable);
	void transtable(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, const uint8_t *pentable);
	void alpha(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t transpen, uint8_t alpha);

	// ----- zoomed graphics drawing -----

	// specific zoom implementations for each transparency type
	void zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley);
	void zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley);
	void zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transpen);
	void zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transpen);
	void zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transpen);
	void zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transpen);
	void zoom_transmask(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transmask);
	void zoom_transmask(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transmask);
	void zoom_transtable(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, const uint8_t *pentable);
	void zoom_transtable(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, const uint8_t *pentable);
	void zoom_alpha(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, uint32_t transpen, uint8_t alpha);

	// ----- priority masked graphics drawing -----

	// specific prio implementations for each transparency type
	void prio_opaque(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask);
	void prio_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask);
	void prio_transpen(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_transmask(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transmask);
	void prio_transmask(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transmask);
	void prio_transtable(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, const uint8_t *pentable);
	void prio_transtable(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, const uint8_t *pentable);
	void prio_alpha(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen, uint8_t alpha);

	// ----- priority masked zoomed graphics drawing -----

	// specific prio_zoom implementations for each transparency type
	void prio_zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask);
	void prio_zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask);
	void prio_zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen);
	void prio_zoom_transmask(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transmask);
	void prio_zoom_transmask(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transmask);
	void prio_zoom_transtable(bitmap_ind16 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, const uint8_t *pentable);
	void prio_zoom_transtable(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, const uint8_t *pentable);
	void prio_zoom_alpha(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask, uint32_t transpen, uint8_t alpha);

	// implementations moved here from specific drivers
	void prio_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, bitmap_ind8 &priority, uint32_t pmask, uint32_t trans_pen);
	void prio_zoom_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect,uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty,uint32_t scalex, uint32_t scaley, bitmap_ind8 &priority, uint32_t pmask,uint32_t trans_pen);
	void alphastore(bitmap_rgb32 &dest, const rectangle &cliprect,uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty,int fixedalpha, uint8_t *alphatable);
	void alphatable(bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty, int fixedalpha ,uint8_t *alphatable);
private:
	// internal helpers
	void decode(uint32_t code);

	// internal state
	palette_device  *m_palette;             // palette used for drawing

	uint16_t          m_width;                // current pixel width of each element (changeable with source clipping)
	uint16_t          m_height;               // current pixel height of each element (changeable with source clipping)
	uint16_t          m_startx;               // current source clip X offset
	uint16_t          m_starty;               // current source clip Y offset

	uint16_t          m_origwidth;            // starting pixel width of each element
	uint16_t          m_origheight;           // staring pixel height of each element
	uint32_t          m_total_elements;       // total number of decoded elements

	uint32_t          m_color_base;           // base color for rendering
	uint16_t          m_color_depth;          // number of colors each pixel can represent
	uint16_t          m_color_granularity;    // number of colors for each color code
	uint32_t          m_total_colors;         // number of color codes

	uint32_t          m_line_modulo;          // bytes between each row of data
	uint32_t          m_char_modulo;          // bytes between each element
	const uint8_t *   m_srcdata;              // pointer to the source data for decoding
	uint32_t          m_dirtyseq;             // sequence number; incremented each time a tile is dirtied

	uint8_t *         m_gfxdata;              // pointer to decoded pixel data, 8bpp
	std::vector<uint8_t>  m_gfxdata_allocated;    // allocated decoded pixel data, 8bpp
	std::vector<uint8_t>  m_dirty;                // dirty array for detecting chars that need decoding
	std::vector<uint32_t>  m_pen_usage;      // bitmask of pens that are used (pens 0-31 only)

	bool            m_layout_is_raw;        // raw layout?
	uint8_t           m_layout_planes;        // bit planes in the layout
	uint32_t          m_layout_xormask;       // xor mask applied to each bit offset
	uint32_t          m_layout_charincrement; // per-character increment in source data
	std::vector<uint32_t>  m_layout_planeoffset;// plane offsets
	std::vector<uint32_t>  m_layout_xoffset; // X offsets
	std::vector<uint32_t>  m_layout_yoffset; // Y offsets
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// ----- scanline copying -----

// copy pixels from an 8bpp buffer to a single scanline of a bitmap
void draw_scanline8(bitmap_ind16 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint8_t *srcptr, const pen_t *paldata);
void draw_scanline8(bitmap_rgb32 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint8_t *srcptr, const pen_t *paldata);

// copy pixels from a 16bpp buffer to a single scanline of a bitmap
void draw_scanline16(bitmap_ind16 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint16_t *srcptr, const pen_t *paldata);
void draw_scanline16(bitmap_rgb32 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint16_t *srcptr, const pen_t *paldata);

// copy pixels from a 32bpp buffer to a single scanline of a bitmap
void draw_scanline32(bitmap_ind16 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint32_t *srcptr, const pen_t *paldata);
void draw_scanline32(bitmap_rgb32 &bitmap, int32_t destx, int32_t desty, int32_t length, const uint32_t *srcptr, const pen_t *paldata);



// ----- scanline extraction -----

// copy pixels from a single scanline of a bitmap to an 8bpp buffer
void extract_scanline8(const bitmap_ind16 &bitmap, int32_t srcx, int32_t srcy, int32_t length, uint8_t *destptr);
void extract_scanline8(const bitmap_rgb32 &bitmap, int32_t srcx, int32_t srcy, int32_t length, uint8_t *destptr);

// copy pixels from a single scanline of a bitmap to a 16bpp buffer
void extract_scanline16(const bitmap_ind16 &bitmap, int32_t srcx, int32_t srcy, int32_t length, uint16_t *destptr);
void extract_scanline16(const bitmap_rgb32 &bitmap, int32_t srcx, int32_t srcy, int32_t length, uint16_t *destptr);

// copy pixels from a single scanline of a bitmap to a 32bpp buffer
void extract_scanline32(const bitmap_ind16 &bitmap, int32_t srcx, int32_t srcy, int32_t length, uint32_t *destptr);
void extract_scanline32(const bitmap_rgb32 &bitmap, int32_t srcx, int32_t srcy, int32_t length, uint32_t *destptr);



// ----- bitmap copying -----

// copy from one bitmap to another, copying all unclipped pixels
void copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, int32_t destx, int32_t desty, const rectangle &cliprect);
void copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, int32_t destx, int32_t desty, const rectangle &cliprect);

// copy from one bitmap to another, copying all unclipped pixels except those that match transpen
void copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, int32_t destx, int32_t desty, const rectangle &cliprect, uint32_t transpen);
void copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, int32_t destx, int32_t desty, const rectangle &cliprect, uint32_t transpen);

/*
  Copy a bitmap onto another with scroll and wraparound.
  These functions support multiple independently scrolling rows/columns.
  "rows" is the number of indepentently scrolling rows. "rowscroll" is an
  array of integers telling how much to scroll each row. Same thing for
  "numcols" and "colscroll".
  If the bitmap cannot scroll in one direction, set numrows or columns to 0.
  If the bitmap scrolls as a whole, set numrows and/or numcols to 1.
  Bidirectional scrolling is, of course, supported only if the bitmap
  scrolls as a whole in at least one direction.
*/

// copy from one bitmap to another, copying all unclipped pixels, and applying scrolling to one or more rows/columns
void copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, uint32_t numrows, const int32_t *rowscroll, uint32_t numcols, const int32_t *colscroll, const rectangle &cliprect);
void copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, uint32_t numrows, const int32_t *rowscroll, uint32_t numcols, const int32_t *colscroll, const rectangle &cliprect);

// copy from one bitmap to another, copying all unclipped pixels except those that match transpen, and applying scrolling to one or more rows/columns
void copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, uint32_t numrows, const int32_t *rowscroll, uint32_t numcols, const int32_t *colscroll, const rectangle &cliprect, uint32_t transpen);
void copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, uint32_t numrows, const int32_t *rowscroll, uint32_t numcols, const int32_t *colscroll, const rectangle &cliprect, uint32_t transpen);

/*
    Copy a bitmap applying rotation, zooming, and arbitrary distortion.
    This function works in a way that mimics some real hardware like the Konami
    051316, so it requires little or no further processing on the caller side.

    Two 16.16 fixed point counters are used to keep track of the position on
    the source bitmap. startx and starty are the initial values of those counters,
    indicating the source pixel that will be drawn at coordinates (0,0) in the
    destination bitmap. The destination bitmap is scanned left to right, top to
    bottom; every time the cursor moves one pixel to the right, incxx is added
    to startx and incxy is added to starty. Every time the cursor moves to the
    next line, incyx is added to startx and incyy is added to startyy.

    What this means is that if incxy and incyx are both 0, the bitmap will be
    copied with only zoom and no rotation. If e.g. incxx and incyy are both 0x8000,
    the source bitmap will be doubled.

    Rotation is performed this way:
    incxx = 0x10000 * cos(theta)
    incxy = 0x10000 * -sin(theta)
    incyx = 0x10000 * sin(theta)
    incyy = 0x10000 * cos(theta)
    this will perform a rotation around (0,0), you'll have to adjust startx and
    starty to move the center of rotation elsewhere.

    Optionally the bitmap can be tiled across the screen instead of doing a single
    copy. This is obtained by setting the wraparound parameter to true.
*/

// copy from one bitmap to another, with zoom and rotation, copying all unclipped pixels
void copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, int32_t startx, int32_t starty, int32_t incxx, int32_t incxy, int32_t incyx, int32_t incyy, int wraparound);
void copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, int32_t startx, int32_t starty, int32_t incxx, int32_t incxy, int32_t incyx, int32_t incyy, int wraparound);

// copy from one bitmap to another, with zoom and rotation, copying all unclipped pixels whose values do not match transpen
void copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, int32_t startx, int32_t starty, int32_t incxx, int32_t incxy, int32_t incyx, int32_t incyy, int wraparound, uint32_t transparent_color);
void copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, int32_t startx, int32_t starty, int32_t incxx, int32_t incxy, int32_t incyx, int32_t incyy, int wraparound, uint32_t transparent_color);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  alpha_blend_r16 - alpha blend two 16-bit
//  5-5-5 RGB pixels
//-------------------------------------------------

inline uint32_t alpha_blend_r16(uint32_t d, uint32_t s, uint8_t level)
{
	int alphad = 256 - level;
	return ((((s & 0x001f) * level + (d & 0x001f) * alphad) >> 8)) |
			((((s & 0x03e0) * level + (d & 0x03e0) * alphad) >> 8) & 0x03e0) |
			((((s & 0x7c00) * level + (d & 0x7c00) * alphad) >> 8) & 0x7c00);
}


//-------------------------------------------------
//  alpha_blend_r16 - alpha blend two 32-bit
//  8-8-8 RGB pixels
//-------------------------------------------------

inline uint32_t alpha_blend_r32(uint32_t d, uint32_t s, uint8_t level)
{
	int alphad = 256 - level;
	return ((((s & 0x0000ff) * level + (d & 0x0000ff) * alphad) >> 8)) |
			((((s & 0x00ff00) * level + (d & 0x00ff00) * alphad) >> 8) & 0x00ff00) |
			((((s & 0xff0000) * level + (d & 0xff0000) * alphad) >> 8) & 0xff0000);
}

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gfxdecode_device

// device type definition
extern const device_type GFXDECODE;

class gfxdecode_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	gfxdecode_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override {};
};

GFXDECODE_EXTERN(empty);

#endif  // __DRAWGFX_H__
