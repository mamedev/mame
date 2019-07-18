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

        u32 pmask = GFX_PMASK_2 | GFX_PMASK_4 | GFX_PMASK_8;
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

        u32 pmask = (~1) << 4;
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

#ifndef MAME_EMU_DRAWGFX_H
#define MAME_EMU_DRAWGFX_H


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
	gfx_element(device_palette_interface *palette, const gfx_layout &gl, const u8 *srcdata, u32 xormask, u32 total_colors, u32 color_base);
	gfx_element(device_palette_interface *palette, const u8 *base, u32 total, u16 width, u16 height, u32 rowbytes, u32 total_colors, u32 color_base, u32 color_granularity);

	// getters
	device_palette_interface &palette() const { return *m_palette; }
	u16 width() const { return m_width; }
	u16 height() const { return m_height; }
	u32 elements() const { return m_total_elements; }
	u32 colorbase() const { return m_color_base; }
	u16 depth() const { return m_color_depth; }
	u16 granularity() const { return m_color_granularity; }
	u32 colors() const { return m_total_colors; }
	u32 rowbytes() const { return m_line_modulo; }
	bool has_pen_usage() const { return !m_pen_usage.empty(); }
	bool has_palette() const { return m_palette; }

	// used by tilemaps
	u32 dirtyseq() const { return m_dirtyseq; }

	// setters
	void set_layout(const gfx_layout &gl, const u8 *srcdata);
	void set_raw_layout(const u8 *srcdata, u32 width, u32 height, u32 total, u32 linemod, u32 charmod);
	void set_raw_layout(const u16 *srcdata, u32 width, u32 height, u32 total, u32 linemod, u32 charmod);
	void set_source(const u8 *source);
	void set_source_and_total(const u8 *source, u32 total);
	void set_xormask(u32 xormask) { m_layout_xormask = xormask; }
	void set_palette(device_palette_interface &palette) { m_palette = &palette; }
	void set_colors(u32 colors) { m_total_colors = colors; }
	void set_colorbase(u16 colorbase) { m_color_base = colorbase; }
	void set_granularity(u16 granularity) { m_color_granularity = granularity; }
	void set_source_clip(u32 xoffs, u32 width, u32 yoffs, u32 height);

	// operations
	void mark_dirty(u32 code) { if (code < elements()) { m_dirty[code] = 1; m_dirtyseq++; } }
	void mark_all_dirty() { memset(&m_dirty[0], 1, elements()); }

	const u16 *get_data(u32 code)
	{
		assert(code < elements());
		if (code < m_dirty.size() && m_dirty[code]) decode(code);
		return m_gfxdata + code * m_char_modulo + m_starty * m_line_modulo + m_startx;
	}

	u32 pen_usage(u32 code)
	{
		assert(code < m_pen_usage.size());
		if (m_dirty[code]) decode(code);
		return m_pen_usage[code];
	}

	// ----- core graphics drawing -----

	// specific drawgfx implementations for each transparency type
	void opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty);
	void opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty);
	void transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transpen);
	void transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transpen);
	void transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transpen);
	void transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transpen);
	void transmask(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transmask);
	void transmask(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transmask);
	void transtable(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, const u8 *pentable);
	void transtable(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, const u8 *pentable);
	void alpha(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 transpen, u8 alpha);

	// ----- zoomed graphics drawing -----

	// specific zoom implementations for each transparency type
	void zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley);
	void zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley);
	void zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transpen);
	void zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transpen);
	void zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transpen);
	void zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transpen);
	void zoom_transmask(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transmask);
	void zoom_transmask(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transmask);
	void zoom_transtable(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, const u8 *pentable);
	void zoom_transtable(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, const u8 *pentable);
	void zoom_alpha(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, u32 transpen, u8 alpha);

	// ----- priority masked graphics drawing -----

	// specific prio implementations for each transparency type
	void prio_opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask);
	void prio_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask);
	void prio_transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_transmask(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transmask);
	void prio_transmask(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transmask);
	void prio_transtable(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, const u8 *pentable);
	void prio_transtable(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, const u8 *pentable);
	void prio_alpha(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 transpen, u8 alpha);

	// ----- priority masked zoomed graphics drawing -----

	// specific prio_zoom implementations for each transparency type
	void prio_zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask);
	void prio_zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask);
	void prio_zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transpen);
	void prio_zoom_transmask(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transmask);
	void prio_zoom_transmask(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transmask);
	void prio_zoom_transtable(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, const u8 *pentable);
	void prio_zoom_transtable(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, const u8 *pentable);
	void prio_zoom_alpha(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 transpen, u8 alpha);

	// implementations moved here from specific drivers
	void prio_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_zoom_transpen_additive(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 scalex, u32 scaley, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void alphastore(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, int fixedalpha, u8 *alphatable);
	void alphatable(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, int fixedalpha, u8 *alphatable);
private:
	// internal helpers
	void decode(u32 code);

	// internal state
	device_palette_interface *m_palette;     // palette used for drawing (optional when used as a pure decoder)

	u16             m_width;                 // current pixel width of each element (changeable with source clipping)
	u16             m_height;                // current pixel height of each element (changeable with source clipping)
	u16             m_startx;                // current source clip X offset
	u16             m_starty;                // current source clip Y offset

	u16             m_origwidth;             // starting pixel width of each element
	u16             m_origheight;            // staring pixel height of each element
	u32             m_total_elements;        // total number of decoded elements

	u32             m_color_base;            // base color for rendering
	u16             m_color_depth;           // number of colors each pixel can represent
	u16             m_color_granularity;     // number of colors for each color code
	u32             m_total_colors;          // number of color codes

	u32             m_line_modulo;           // bytes between each row of data
	u32             m_char_modulo;           // bytes between each element
	const u8 *      m_srcdata;               // pointer to the source data for decoding
	u32             m_dirtyseq;              // sequence number; incremented each time a tile is dirtied

	u16 *            m_gfxdata;              // pointer to decoded pixel data, 16bpp
	std::vector<u16> m_gfxdata_allocated;    // allocated decoded pixel data, 8bpp
	std::vector<u8>  m_dirty;                // dirty array for detecting chars that need decoding
	std::vector<u32> m_pen_usage;            // bitmask of pens that are used (pens 0-31 only)

	bool             m_layout_is_raw;        // raw layout?
	bool             m_layout_is_16bpp;      // 16bpp raw layout?
	u8               m_layout_planes;        // bit planes in the layout
	u32              m_layout_xormask;       // xor mask applied to each bit offset
	u32              m_layout_charincrement; // per-character increment in source data
	std::vector<u32> m_layout_planeoffset;   // plane offsets
	std::vector<u32> m_layout_xoffset;       // X offsets
	std::vector<u32> m_layout_yoffset;       // Y offsets
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// ----- scanline copying -----

// copy pixels from an 8bpp buffer to a single scanline of a bitmap
void draw_scanline8(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata);
void draw_scanline8(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata);

void prio_draw_scanline8(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask);
void prio_draw_scanline8(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask);

void primask_draw_scanline8(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_draw_scanline8(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u8 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

// copy pixels from a 16bpp buffer to a single scanline of a bitmap
void draw_scanline16(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata);
void draw_scanline16(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata);

void prio_draw_scanline16(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask);
void prio_draw_scanline16(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask);

void primask_draw_scanline16(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_draw_scanline16(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u16 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

// copy pixels from a 32bpp buffer to a single scanline of a bitmap
void draw_scanline32(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata);
void draw_scanline32(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata);

void prio_draw_scanline32(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask);
void prio_draw_scanline32(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u32 pmask);

void primask_draw_scanline32(bitmap_ind16 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_draw_scanline32(bitmap_rgb32 &bitmap, s32 destx, s32 desty, s32 length, const u32 *srcptr, const pen_t *paldata, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);



// ----- scanline extraction -----

// copy pixels from a single scanline of a bitmap to an 8bpp buffer
void extract_scanline8(const bitmap_ind16 &bitmap, s32 srcx, s32 srcy, s32 length, u8 *destptr);
void extract_scanline8(const bitmap_rgb32 &bitmap, s32 srcx, s32 srcy, s32 length, u8 *destptr);

// copy pixels from a single scanline of a bitmap to a 16bpp buffer
void extract_scanline16(const bitmap_ind16 &bitmap, s32 srcx, s32 srcy, s32 length, u16 *destptr);
void extract_scanline16(const bitmap_rgb32 &bitmap, s32 srcx, s32 srcy, s32 length, u16 *destptr);

// copy pixels from a single scanline of a bitmap to a 32bpp buffer
void extract_scanline32(const bitmap_ind16 &bitmap, s32 srcx, s32 srcy, s32 length, u32 *destptr);
void extract_scanline32(const bitmap_rgb32 &bitmap, s32 srcx, s32 srcy, s32 length, u32 *destptr);



// ----- bitmap copying -----

// copy from one bitmap to another, copying all unclipped pixels
void copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect);
void copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect);

void prio_copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask);
void prio_copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask);

void primask_copybitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

// copy from one bitmap to another, copying all unclipped pixels except those that match transpen
void copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 transpen);
void copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 transpen);

void prio_copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 transpen);
void prio_copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 transpen);

void primask_copybitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 transpen, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_copybitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, u32 transpen, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

void copybitmap_transalpha(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect);

void prio_copybitmap_transalpha(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask);

void primask_copybitmap_transalpha(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, s32 destx, s32 desty, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

/*
  Copy a bitmap onto another with scroll and wraparound.
  These functions support multiple independently scrolling rows/columns.
  "rows" is the number of independently scrolling rows. "rowscroll" is an
  array of integers telling how much to scroll each row. Same thing for
  "numcols" and "colscroll".
  If the bitmap cannot scroll in one direction, set numrows or columns to 0.
  If the bitmap scrolls as a whole, set numrows and/or numcols to 1.
  Bidirectional scrolling is, of course, supported only if the bitmap
  scrolls as a whole in at least one direction.
*/

// copy from one bitmap to another, copying all unclipped pixels, and applying scrolling to one or more rows/columns
void copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect);
void copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect);

void prio_copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask);
void prio_copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask);

void primask_copyscrollbitmap(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_copyscrollbitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

// copy from one bitmap to another, copying all unclipped pixels except those that match transpen, and applying scrolling to one or more rows/columns
void copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 transpen);
void copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 transpen);

void prio_copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 transpen);
void prio_copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, bitmap_ind8 &priority, u32 pmask, u32 transpen);

void primask_copyscrollbitmap_trans(bitmap_ind16 &dest, const bitmap_ind16 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 transpen, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_copyscrollbitmap_trans(bitmap_rgb32 &dest, const bitmap_rgb32 &src, u32 numrows, const s32 *rowscroll, u32 numcols, const s32 *colscroll, const rectangle &cliprect, u32 transpen, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

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
void copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound);
void copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound);

void prio_copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, bitmap_ind8 &priority, u32 pmask);
void prio_copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, bitmap_ind8 &priority, u32 pmask);

void primask_copyrozbitmap(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_copyrozbitmap(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);

// copy from one bitmap to another, with zoom and rotation, copying all unclipped pixels whose values do not match transpen
void copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, u32 transparent_color);
void copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, u32 transparent_color);

void prio_copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, bitmap_ind8 &priority, u32 pmask, u32 transparent_color);
void prio_copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, bitmap_ind8 &priority, u32 pmask, u32 transparent_color);

void primask_copyrozbitmap_trans(bitmap_ind16 &dest, const rectangle &cliprect, const bitmap_ind16 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, u32 transparent_color, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);
void primask_copyrozbitmap_trans(bitmap_rgb32 &dest, const rectangle &cliprect, const bitmap_rgb32 &src, s32 startx, s32 starty, s32 incxx, s32 incxy, s32 incyx, s32 incyy, int wraparound, u32 transparent_color, bitmap_ind8 &priority, u8 pcode = 0, u8 pmask = 0xff);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  alpha_blend_r16 - alpha blend two 16-bit
//  5-5-5 RGB pixels
//-------------------------------------------------

constexpr u32 alpha_blend_r16(u32 d, u32 s, u8 level)
{
	return ((((s & 0x001f) * level + (d & 0x001f) * int(256 - level)) >> 8)) |
			((((s & 0x03e0) * level + (d & 0x03e0) * int(256 - level)) >> 8) & 0x03e0) |
			((((s & 0x7c00) * level + (d & 0x7c00) * int(256 - level)) >> 8) & 0x7c00);
}


//-------------------------------------------------
//  alpha_blend_r32 - alpha blend two 32-bit
//  8-8-8 RGB pixels
//-------------------------------------------------

constexpr u32 alpha_blend_r32(u32 d, u32 s, u8 level)
{
	return ((((s & 0x0000ff) * level + (d & 0x0000ff) * int(256 - level)) >> 8)) |
			((((s & 0x00ff00) * level + (d & 0x00ff00) * int(256 - level)) >> 8) & 0x00ff00) |
			((((s & 0xff0000) * level + (d & 0xff0000) * int(256 - level)) >> 8) & 0xff0000);
}


//-------------------------------------------------
//  add_blend_r16 - additive blend two 16-bit
//  5-5-5 RGB pixels
//-------------------------------------------------

constexpr u32 add_blend_r16(u32 d, u32 s)
{
	return std::min(u32((s & 0x001f) + (d & 0x001f)), u32(0x001f)) |
			std::min(u32((s & 0x03e0) + (d & 0x03e0)), u32(0x03e0)) |
			std::min(u32((s & 0x7c00) + (d & 0x7c00)), u32(0x7c00));
}


//-------------------------------------------------
//  add_blend_r32 - additive blend two 32-bit
//  8-8-8 RGB pixels
//-------------------------------------------------

constexpr u32 add_blend_r32(u32 d, u32 s)
{
	return std::min(u32((s & 0x0000ff) + (d & 0x0000ff)), u32(0x0000ff)) |
			std::min(u32((s & 0x00ff00) + (d & 0x00ff00)), u32(0x00ff00)) |
			std::min(u32((s & 0xff0000) + (d & 0xff0000)), u32(0xff0000));
}

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gfxdecode_device

// device type definition
DECLARE_DEVICE_TYPE(GFXDECODE, gfxdecode_device)

class gfxdecode_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	template <typename T>
	gfxdecode_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: gfxdecode_device(mconfig, tag, owner, 0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_info(gfxinfo);
	}
	gfxdecode_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override {}
};

#endif  // MAME_EMU_DRAWGFX_H
