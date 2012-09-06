/***************************************************************************

    tilemap.h

    Generic tilemap management system.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    Core concepts:

        Tilemap = a 2-dimensional array of tiles; each tile has its own
            independent characteristics which describe how that tile is
            rendered. A tilemap is described by the number of rows and
            columns in the map. Each tile in the tilemap is a fixed size
            specified as width and height in pixels.

        Tile = a single rectangular element in a tilemap; tiles can be
            any size, but all tiles in a tilemap are the same size. Each
            tile is described by the following parameters:

            pen_data (required): pointer to an array of 8bpp pen data
                describing the raw pixels to be rendered for a tile;
                for example, an 8x8 tile requires 64 bytes of pen_data

            palette_base (required): the base index in the global palette;
                each pixel fetched from pen_data will have this base added
                to it before it is stored in the pixmap

            mask_data (optional): pointer to an array of 1bpp mask data
                which controls which pixels are to be considered visible
                or transparent (part of no layer); data is packed MSB
                first with no padding, so an 8x8 tile requires 64 bits or
                8 bytes data

            category (optional): specifies one of 16 categories for the
                pixels in the tile; the category controls which tiles are
                rendered during a tilemap_draw() call

            group (optional): specifies one of 256 groups for pen mapping;
                each pen in the tile is looked up in a table to determine
                which layer(s) it belongs to, and the group selects one of
                256 different tables to use for this mapping

            flags (optional): specifies one or more of the following bits:

                TILE_FLIPX means render the tile flipped horizontally

                TILE_FLIPY means render the tile flipped vertically

                TILE_FORCE_LAYERn means all pixels are forced to be in
                    layer n, where n = 0,1,2

        Pen = another name for pixel value; tilemap graphics are either
            4 or 8 bits per pixel, meaning that pen values range from
            0-15 (4bpp) or 0-255 (8bpp).

        Logical index = index of a tile in a tilemap, assuming perfect
            packing in row-major order; this is always equal to
            (rownum * tilemap_columns + colnum).

        Memory index = index of a tile in memory; this often does not
            map 1:1 to the logical index (though it can). A mapper
            function is provided when creating a new tilemap that can
            map from (column,row) to memory index.

        Layer = one of 3 categories each pixel can belong to, based on
            the pens in a tile; traditionally, layer 0 is the only
            one used. If a pixel does not belong to any layers, it is
            considered transparent.

        Group = one of 256 per-tile selectable means of mapping from
            pens to layers; traditionally only group 0 is used.

        Category = a 4-bit value specified per-tile which allows for
            separating tiles into categories which can be rendered
            independently.

        Pixmap = a 16bpp bitmap containing the full tilemap with
            all tiles rendered.

        Flagsmap = an 8bpp bitmap containing per-pixel flags,
            specifically the category (specified in bits 0-3) and
            the layer (specified in bits 4-6).

****************************************************************************

    How to use a tilemap:

    1. First create a new tilemap by calling tilemap_create(). The
        parameters are as follows:

        tile_get_info = pointer to a callback function which accepts a
            memory index and in return fills in a tile_info structure
            that describes the characteristics of a tile; this function
            will be called whenever a dirty tile needs to be rendered

        mapper = pointer to a callback function which maps the logical
            column and row to a memory index; several standard mappers
            are provided, with tilemap_scan_rows being the most common

        tilewidth = the width, in pixels, of each individual tile

        tileheight = the height, in pixels, of each individual tile

        cols = the number of columns in the tilemap

        rows = the number of rows in the tilemap

    2. Once you have created your tilemap, you need to configure it.
        Common configuration tasks include:

            * marking one of the pens as transparent via
                tilemap_set_transparent_pen()

            * performing more complex pen-to-layer mapping via
                tilemap_map_pen_to_layer() or
                tilemap_map_pens_to_layer()

            * configuring global scroll offsets via
                tilemap_set_scrolldx() and tilemap_set_scrolldy()

            * specifying a pointer that is passed to your tile_get_info
                callback via tilemap_set_user_data()

            * setting a global palette offset via
                tilemap_set_palette_offset()

    3. In your memory write handlers for the tile memory, anytime tile
        data is modified, you need to mark the tile dirty so that it is
        re-rendered with the new data the next time the tilemap is drawn.
        Use tilemap_mark_tile_dirty() and pass in the memory index.

    4. In your handlers for scrolling, update the scroll values for the
        tilemap via tilemap_set_scrollx() and tilemap_set_scrolly().

    5. If any other major characteristics of the tilemap change (generally
        any global state that is used by the tile_get_info callback but
        which is not reported via other calls to the tilemap code), you
        should invalidate the entire tilemap. You can do this by calling
        tilemap_mark_all_tiles_dirty().

    6. In your VIDEO_UPDATE callback, render the tiles by calling
        tilemap_draw() or tilemap_draw_roz(). If you need to do custom
        rendering and want access to the raw pixels, call
        tilemap_get_pixmap() to get a pointer to the updated bitmap_ind16
        containing the tilemap graphics.

****************************************************************************

    The following example shows how to use the tilemap system to create
    a simple tilemap where pen 0 is transparent. Each tile is 8x8 pixels
    in size, and there are 32 rows and 64 columns, stored in row-major
    order. Based on bits in the tile memory, tiles can be drawn either
    behind or in front of sprites.

        tilemap_t *tmap;
        UINT16 *my_tmap_memory;

        TILE_GET_INFO( my_get_info )
        {
            UINT8 tiledata = my_tmap_memory[tile_index];
            UINT8 code = tiledata & 0xff;
            UINT8 color = (tiledata >> 8) & 0x1f;
            UINT8 flipx = (tiledata >> 13) & 1;
            UINT8 flipy = (tiledata >> 14) & 1;
            UINT8 category = (tiledata >> 15) & 1;

            // set the common info for the tile
            SET_TILE_INFO(
                1,              // use machine.gfx[1] for tile graphics
                code,           // the index of the graphics for this tile
                color,          // the color to use for this tile
                (flipx ? TILE_FLIPX : 0) |  // flags for this tile; also
                (flipy ? TILE_FLIPY : 0);   // see the FLIP_YX macro
            );

            // set the category of each tile based on the high bit; this
            // allows us to draw each category independently
            tileinfo.category = category;
        }

        VIDEO_START( mydriver )
        {
            // first create the tilemap
            tmap = tilemap_create(machine,
                    my_get_info,            // pointer to your get_info
                    tilemap_scan_rows,      // standard row-major mapper
                    8,8,                    // 8x8 tiles
                    64,32);                 // 64 columns, 32 rows

            // then set the transparent pen; all other pens will default
            // to being part of layer 0
            tilemap_set_transparent_pen(tmap, 0);
        }

        SCREEN_UPDATE( mydriver )
        {
            // draw the tilemap first, fully opaque since it needs to
            // erase all previous pixels
            tilemap_draw(
                bitmap,                 // destination bitmap
                cliprect,               // clipping rectangle
                tmap,                   // tilemap to draw
                TILEMAP_DRAW_OPAQUE,    // flags
                0);                     // don't use priority_bitmap

            // next draw the sprites
            my_draw_sprites();

            // then draw the tiles which have priority over sprites
            tilemap_draw(
                bitmap,                 // destination bitmap
                cliprect,               // clipping rectangle
                tmap,                   // tilemap to draw
                TILEMAP_DRAW_CATEGORY(1),// flags: draw category 1
                0);                     // don't use priority_bitmap

            return 0;
        }

****************************************************************************

    Tilemap techniques:

    * The previous tilemap code specified a number of different tilemap
        types that were fundamentally similar. This has been collapsed
        into just two types now. Here is how to replicate that
        functionality with the new system:

        TILEMAP_OPAQUE: If you want a tilemap with no transparency,
            you don't need to do anything; by default all pens map to
            layer 0 and are thus non-transparent. Note that a lot of
            code used to create OPAQUE tilemaps and then set the
            transparent pen for them; this no longer works.

        TILEMAP_TRANSPARENT: This described a tilemap with a single
            transparent pen. To create the same effect, call
            tilemap_set_transparent_pen() to specify which pen is
            transparent; all other pens will map to layer 0.

        TILEMAP_BITMASK: This type is no longer special; with the new
            code, any tile_get_info callback can specify a bitmask which
            will be processed after rendering to make some pixels
            transparent.

        TILEMAP_SPLIT: This type used to let you map pens into two
            layers (called "front" and "back") based on their pens. It
            also allowed for you to choose one of 4 mappings on a per-tile
            basis. All of this functionality is now expanded: you can
            specify one of 3 layers and can choose from one of 256 mappings
            on a per-tile basis. You just call tilemap_set_transmask(),
            which still exists but maps onto the new behavior. The "front"
            layer is now "layer 0" and the "back" layer is now "layer 1".

        TILEMAP_SPLIT_PENBIT: This type was only used in one driver
            and is not worth describing in detail how the new mapping
            works. :)

    * By far the most common usage of layers is the simplest: layer 0 is
        the only one used and rendered. If you want a particular tile to
        render opaque, you can set the TILE_FORCE_LAYER0 file in your
        tile_get_info callback.

    * When drawing, you can specify in the flags which classes of pixels
        to draw. In the low 4 bits, you specify the category; this must
        match exactly the category specified for the tile in order to
        render. In addition, the flags TILEMAP_DRAW_LAYERn allow you to
        control which layer pattern to draw; note that combining these
        flags may produce interesting results. Specifying both layer 0
        and layer 1 will render only pixels that are in *both* layers.
        If you want to render everything, regardless of layer, specify
        the TILEMAP_DRAW_OPAQUE flag. If you don't specify any layers,
        TILEMAP_DRAW_LAYER0 is assumed.

    * If you want to render with alpha blending, you can call
        tilemap_draw() with the TILEMAP_DRAW_ALPHA flag.

    * To configure more complex pen-to-layer mapping, use the
        tilemap_map_pens_to_layer() call. This call takes a group number
        so that you can configure 1 of the 256 groups independently.
        It also takes a pen and a mask; the mapping is updated for all
        pens where ((pennum & mask) == pen). To set all the pens in a
        group to the same value, pass a mask of 0. To set a single pen in
        a group, pass a mask of ~0. The helper function
        tilemap_map_pen_to_layer() does this for you.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __TILEMAP_H__
#define __TILEMAP_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maximum number of groups
#define TILEMAP_NUM_GROUPS				256


// these flags control tilemap_draw() behavior
const UINT32 TILEMAP_DRAW_CATEGORY_MASK = 0x0f;		// specify the category to draw
const UINT32 TILEMAP_DRAW_LAYER0 = 0x10;			// draw layer 0
const UINT32 TILEMAP_DRAW_LAYER1 = 0x20;			// draw layer 1
const UINT32 TILEMAP_DRAW_LAYER2 = 0x40;			// draw layer 2
const UINT32 TILEMAP_DRAW_OPAQUE = 0x80;			// draw everything, even transparent stuff
const UINT32 TILEMAP_DRAW_ALPHA_FLAG = 0x100;		// draw with alpha blending (in the upper 8 bits)
const UINT32 TILEMAP_DRAW_ALL_CATEGORIES = 0x200;	// draw all categories

// per-pixel flags in the transparency_bitmap
const UINT8 TILEMAP_PIXEL_CATEGORY_MASK = 0x0f;		// category is stored in the low 4 bits
const UINT8 TILEMAP_PIXEL_TRANSPARENT = 0x00;		// transparent if in none of the layers below
const UINT8 TILEMAP_PIXEL_LAYER0 = 0x10;			// pixel is opaque in layer 0
const UINT8 TILEMAP_PIXEL_LAYER1 = 0x20;			// pixel is opaque in layer 1
const UINT8 TILEMAP_PIXEL_LAYER2 = 0x40;			// pixel is opaque in layer 2

// per-tile flags, set by get_tile_info callback
const UINT8 TILE_FLIPX = 0x01;						// draw this tile horizontally flipped
const UINT8 TILE_FLIPY = 0x02;						// draw this tile vertically flipped
const UINT8 TILE_FORCE_LAYER0 = TILEMAP_PIXEL_LAYER0; // force all pixels to be layer 0 (no transparency)
const UINT8 TILE_FORCE_LAYER1 = TILEMAP_PIXEL_LAYER1; // force all pixels to be layer 1 (no transparency)
const UINT8 TILE_FORCE_LAYER2 = TILEMAP_PIXEL_LAYER2; // force all pixels to be layer 2 (no transparency)

// tilemap global flags, used by tilemap_set_flip()
const UINT32 TILEMAP_FLIPX = TILE_FLIPX;			// draw the tilemap horizontally flipped
const UINT32 TILEMAP_FLIPY = TILE_FLIPY;			// draw the tilemap vertically flipped

// set this value for a scroll row/column to fully disable it
const UINT32 TILE_LINE_DISABLED = 0x80000000;

// standard mappers
enum tilemap_standard_mapper
{
	TILEMAP_SCAN_ROWS = 0,
	TILEMAP_SCAN_ROWS_FLIP_X,
	TILEMAP_SCAN_ROWS_FLIP_Y,
	TILEMAP_SCAN_ROWS_FLIP_XY,
	TILEMAP_SCAN_COLS,
	TILEMAP_SCAN_COLS_FLIP_X,
	TILEMAP_SCAN_COLS_FLIP_Y,
	TILEMAP_SCAN_COLS_FLIP_XY,
	TILEMAP_STANDARD_COUNT
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class tilemap_t;
class tilemap_manager;


// global types
typedef UINT32 tilemap_memory_index;


// tile_data is filled in by the get_tile_info callback
struct tile_data
{
	const UINT8 *	pen_data;		// required
	const UINT8 *	mask_data;		// required
	pen_t			palette_base;	// defaults to 0
	UINT8			category;		// defaults to 0; range from 0..15
	UINT8			group;			// defaults to 0; range from 0..TILEMAP_NUM_GROUPS
	UINT8			flags;			// defaults to 0; one or more of TILE_* flags above
	UINT8			pen_mask;		// defaults to 0xff; mask to apply to pen_data while rendering the tile
	UINT8			gfxnum;			// defaults to 0xff; specify index of machine.gfx for auto-invalidation on dirty

	void set(running_machine &machine, int _gfxnum, int rawcode, int rawcolor, int _flags)
	{
		gfx_element *gfx = machine.gfx[_gfxnum];
		int code = rawcode % gfx->elements();
		pen_data = gfx->get_data(code);
		palette_base = gfx->colorbase() + gfx->granularity() * rawcolor;
		flags = _flags;
		gfxnum = _gfxnum;
	}

	void set(running_machine &machine, gfx_element &gfx, int rawcode, int rawcolor, int _flags)
	{
		int code = rawcode % gfx.elements();
		pen_data = gfx.get_data(code);
		palette_base = gfx.colorbase() + gfx.granularity() * rawcolor;
		flags = _flags;
	}
};


// modern delegates
typedef delegate<void (tile_data &, tilemap_memory_index, void *)> tilemap_get_info_delegate;
typedef delegate<tilemap_memory_index (UINT32, UINT32, UINT32, UINT32)> tilemap_mapper_delegate;


// legacy callbacks
typedef void (*tile_get_info_func)(running_machine &machine, tile_data &tileinfo, tilemap_memory_index tile_index, void *param);
typedef void (*tile_get_info_device_func)(device_t *device, tile_data &tileinfo, tilemap_memory_index tile_index, void *param);
typedef tilemap_memory_index (*tilemap_mapper_func)(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);


// core tilemap structure
class tilemap_t
{
	DISABLE_COPYING(tilemap_t);

	friend class tilemap_manager;
	friend class simple_list<tilemap_t>;
    friend resource_pool_object<tilemap_t>::~resource_pool_object();

	// logical index
	typedef UINT32 logical_index;

	// internal usage to mark tiles dirty
	static const UINT8 TILE_FLAG_DIRTY = 0xff;

	// invalid logical index
	static const logical_index INVALID_LOGICAL_INDEX = (logical_index)~0;

	// maximum index in each array
	static const int MAX_PEN_TO_FLAGS = 256;

	// tilemap_manager controlls our allocations
	tilemap_t(tilemap_manager &manager, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, int tilewidth, int tileheight, int cols, int rows);
	~tilemap_t();

public:
	// getters
	running_machine &machine() const;
	tilemap_t *next() const { return m_next; }
	UINT32 width() const { return m_width; }
	UINT32 height() const { return m_height; }
	bool enabled() const { return m_enable; }
	int scrolldx() const { return (m_attributes & TILEMAP_FLIPX) ? m_dx_flipped : m_dx; }
	int scrolldy() const { return (m_attributes & TILEMAP_FLIPY) ? m_dy_flipped : m_dy; }
	int scrollx(int which = 0) const { return (which < m_scrollrows) ? m_rowscroll[which] : 0; }
	int scrolly(int which = 0) const { return (which < m_scrollcols) ? m_colscroll[which] : 0; }
	bitmap_ind16 &pixmap() { pixmap_update(); return m_pixmap; }
	bitmap_ind8 &flagsmap() { pixmap_update(); return m_flagsmap; }
	UINT8 *tile_flags() { pixmap_update(); return m_tileflags; }
	tilemap_memory_index memory_index(UINT32 col, UINT32 row) { return m_mapper(col, row, m_cols, m_rows); }

	// setters
	void enable(bool enable = true) { m_enable = enable; }
	void set_user_data(void *user_data) { m_user_data = user_data; }
	void set_palette_offset(UINT32 offset) { m_palette_offset = offset; }
	void set_scrolldx(int dx, int dx_flipped) { m_dx = dx; m_dx_flipped = dx_flipped; }
	void set_scrolldy(int dy, int dy_flipped) { m_dy = dy; m_dy_flipped = dy_flipped; }
	void set_scrollx(int which, int value) { if (which < m_scrollrows) m_rowscroll[which] = value; }
	void set_scrolly(int which, int value) { if (which < m_scrollcols) m_colscroll[which] = value; }
	void set_scrollx(int value) { set_scrollx(0, value); }
	void set_scrolly(int value) { set_scrolly(0, value); }
	void set_scroll_rows(UINT32 scroll_rows) { assert(scroll_rows <= m_height); m_scrollrows = scroll_rows; }
	void set_scroll_cols(UINT32 scroll_cols) { assert(scroll_cols <= m_width); m_scrollcols = scroll_cols; }
	void set_flip(UINT32 attributes) { if (m_attributes != attributes) { m_attributes = attributes; mappings_update(); } }

	// dirtying
	void mark_tile_dirty(tilemap_memory_index memindex);
	void mark_all_dirty() { m_all_tiles_dirty = true; m_all_tiles_clean = false; }

	// pen mapping
	void map_pens_to_layer(int group, pen_t pen, pen_t mask, UINT8 layermask);
	void map_pen_to_layer(int group, pen_t pen, UINT8 layermask) { map_pens_to_layer(group, pen, ~0, layermask); }
	void set_transparent_pen(pen_t pen);
	void set_transmask(int group, UINT32 fgmask, UINT32 bgmask);

	// drawing
	void draw(bitmap_ind16 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask = 0xff);
	void draw(bitmap_rgb32 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask = 0xff);
	void draw_roz(bitmap_ind16 &dest, const rectangle &cliprect, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask = 0xff);
	void draw_roz(bitmap_rgb32 &dest, const rectangle &cliprect, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask = 0xff);
	void draw_debug(bitmap_rgb32 &dest, UINT32 scrollx, UINT32 scrolly);

	// mappers
	// scan in row-major order with optional flipping
	static tilemap_memory_index scan_rows(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_rows_flip_x(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_rows_flip_y(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_rows_flip_xy(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);

	// scan in column-major order with optional flipping
	static tilemap_memory_index scan_cols(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_cols_flip_x(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_cols_flip_y(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_cols_flip_xy(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);

private:
	// internal set of transparency states for rendering
	enum trans_t
	{
		WHOLLY_TRANSPARENT,
		WHOLLY_OPAQUE,
		MASKED
	};

	// blitting parameters for rendering
	struct blit_parameters
	{
		rectangle			cliprect;
		UINT32				tilemap_priority_code;
		UINT8				mask;
		UINT8				value;
		UINT8				alpha;
	};

	// inline helpers
	INT32 effective_rowscroll(int index, UINT32 screen_width);
	INT32 effective_colscroll(int index, UINT32 screen_height);
	bool gfx_elements_changed();

	// inline scanline rasterizers
	void scanline_draw_opaque_null(int count, UINT8 *pri, UINT32 pcode);
	void scanline_draw_masked_null(const UINT8 *maskptr, int mask, int value, int count, UINT8 *pri, UINT32 pcode);
	void scanline_draw_opaque_ind16(UINT16 *dest, const UINT16 *source, int count, UINT8 *pri, UINT32 pcode);
	void scanline_draw_masked_ind16(UINT16 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, UINT8 *pri, UINT32 pcode);
	void scanline_draw_opaque_rgb32(UINT32 *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode);
	void scanline_draw_masked_rgb32(UINT32 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode);
	void scanline_draw_opaque_rgb32_alpha(UINT32 *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
	void scanline_draw_masked_rgb32_alpha(UINT32 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);

	// internal helpers
	void postload();
	void mappings_create();
	void mappings_update();
	void realize_all_dirty_tiles();

	// internal drawing
	void pixmap_update();
	void tile_update(logical_index logindex, UINT32 col, UINT32 row);
	UINT8 tile_draw(const UINT8 *pendata, UINT32 x0, UINT32 y0, UINT32 palette_base, UINT8 category, UINT8 group, UINT8 flags, UINT8 pen_mask);
	UINT8 tile_apply_bitmask(const UINT8 *maskdata, UINT32 x0, UINT32 y0, UINT8 category, UINT8 flags);
	void configure_blit_parameters(blit_parameters &blit, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask);
	template<class _BitmapClass> void draw_common(_BitmapClass &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask);
	template<class _BitmapClass> void draw_roz_common(_BitmapClass &dest, const rectangle &cliprect, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask);
	template<class _BitmapClass> void draw_instance(_BitmapClass &dest, const blit_parameters &blit, int xpos, int ypos);
	template<class _BitmapClass> void draw_roz_core(_BitmapClass &destbitmap, const blit_parameters &blit, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound);

	// basic tilemap metrics
	tilemap_t *					m_next;					// pointer to next tilemap
	UINT32						m_rows;					// number of tile rows
	UINT32						m_cols;					// number of tile columns
	UINT32						m_tilewidth;			// width of a single tile in pixels
	UINT32						m_tileheight;			// height of a single tile in pixels
	UINT32						m_width;				// width of the full tilemap in pixels
	UINT32						m_height;				// height of the full tilemap in pixels

	// logical <-> memory mappings
	tilemap_mapper_delegate		m_mapper;				// callback to map a row/column to a memory index
	logical_index *				m_memory_to_logical;	// map from memory index to logical index
	logical_index				m_max_logical_index;	// maximum valid logical index
	tilemap_memory_index *		m_logical_to_memory;	// map from logical index to memory index
	tilemap_memory_index		m_max_memory_index;		// maximum valid memory index

	// callback to interpret video RAM for the tilemap
	tilemap_get_info_delegate	m_tile_get_info;		// callback to get information about a tile
	tile_data					m_tileinfo;				// structure to hold the data for a tile
	void *						m_user_data;			// user data value passed to the callback

	// global tilemap states
	bool						m_enable;				// true if we are enabled
	UINT8						m_attributes;			// global attributes (flipx/y)
	bool						m_all_tiles_dirty;		// true if all tiles are dirty
	bool						m_all_tiles_clean;		// true if all tiles are clean
	UINT32						m_palette_offset;		// palette offset
	UINT32						m_pen_data_offset;		// pen data offset
	UINT32						m_gfx_used;				// bitmask of gfx items used
	UINT32						m_gfx_dirtyseq[MAX_GFX_ELEMENTS]; // dirtyseq values from last check

	// scroll information
	UINT32						m_scrollrows;			// number of independently scrolled rows
	UINT32						m_scrollcols;			// number of independently scrolled columns
	INT32 *						m_rowscroll;			// array of rowscroll values
	INT32 *						m_colscroll;			// array of colscroll values
	INT32						m_dx;					// global horizontal scroll offset
	INT32						m_dx_flipped;			// global horizontal scroll offset when flipped
	INT32						m_dy;					// global vertical scroll offset
	INT32						m_dy_flipped;			// global vertical scroll offset when flipped

	// pixel data
	bitmap_ind16				m_pixmap;				// cached pixel data

	// transparency mapping
	bitmap_ind8					m_flagsmap;				// per-pixel flags
	UINT8 *						m_tileflags;			// per-tile flags
	UINT8						m_pen_to_flags[MAX_PEN_TO_FLAGS * TILEMAP_NUM_GROUPS];		// mapping of pens to flags

private:
	tilemap_manager &			m_manager;				// reference to the owning manager
};


// tilemap manager
class tilemap_manager
{
	friend class tilemap_t;

public:
	// construction/destuction
	tilemap_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }

	// tilemap creation
	tilemap_t &create(tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, int tilewidth, int tileheight, int cols, int rows);
	tilemap_t &create(tilemap_get_info_delegate tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows);
	tilemap_t &create(tile_get_info_func tile_get_info, tilemap_mapper_func mapper, int tilewidth, int tileheight, int cols, int rows);
	tilemap_t &create(tile_get_info_func tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows);

	// tilemap list information
	tilemap_t *find(int index) { return m_tilemap_list.find(index); }
	int count() const { return m_tilemap_list.count(); }

	// global operations on all tilemaps
	void mark_all_dirty();
	void set_flip_all(UINT32 attributes);

private:
	// allocate an instance index
	int alloc_instance() { return ++m_instance; }

	// internal state
	running_machine &		m_machine;
	simple_list<tilemap_t>	m_tilemap_list;
	int						m_instance;
};



//**************************************************************************
//  MACROS
//**************************************************************************

// macros to help form flags for tilemap_draw
#define TILEMAP_DRAW_CATEGORY(x)		(x)		// specify category to draw
#define TILEMAP_DRAW_ALPHA(x)			(TILEMAP_DRAW_ALPHA_FLAG | (rgb_clamp(x) << 24))

// function definition for a get info callback
#define TILE_GET_INFO(_name)			void _name(running_machine &machine, tile_data &tileinfo, tilemap_memory_index tile_index, void *param)
#define TILE_GET_INFO_MEMBER(_name)		void _name(tile_data &tileinfo, tilemap_memory_index tile_index, void *param)
#define TILE_GET_INFO_DEVICE(_name)		void _name(device_t *device, tile_data &tileinfo, tilemap_memory_index tile_index, void *param)

// function definition for a logical-to-memory mapper
#define TILEMAP_MAPPER(_name)			tilemap_memory_index _name(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
#define TILEMAP_MAPPER_MEMBER(_name)	tilemap_memory_index _name(UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)

// useful macro inside of a TILE_GET_INFO callback to set tile information
#define SET_TILE_INFO(GFX,CODE,COLOR,FLAGS)         tileinfo.set(machine, GFX, CODE, COLOR, FLAGS)
#define SET_TILE_INFO_MEMBER(GFX,CODE,COLOR,FLAGS)  tileinfo.set(machine(), GFX, CODE, COLOR, FLAGS)
#define SET_TILE_INFO_DEVICE(GFX,CODE,COLOR,FLAGS)  tileinfo.set(device->machine(), GFX, CODE, COLOR, FLAGS)

// Macros for setting tile attributes in the TILE_GET_INFO callback:
//   TILE_FLIP_YX assumes that flipy is in bit 1 and flipx is in bit 0
//   TILE_FLIP_XY assumes that flipy is in bit 0 and flipx is in bit 1
#define TILE_FLIPYX(YX)					((YX) & 3)
#define TILE_FLIPXY(XY)					((((XY) & 2) >> 1) | (((XY) & 1) << 1))



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************


// ----- tilemap creation and configuration -----

// create a new tilemap; note that tilemaps are tracked by the core so there is no dispose
inline tilemap_t *tilemap_create(running_machine &machine, tile_get_info_func tile_get_info, tilemap_mapper_func mapper, int tilewidth, int tileheight, int cols, int rows)
{ return &machine.tilemap().create(tilemap_get_info_delegate(tile_get_info, "", &machine), tilemap_mapper_delegate(mapper, "", &machine), tilewidth, tileheight, cols, rows); }

inline tilemap_t *tilemap_create(running_machine &machine, tile_get_info_func tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows)
{ return &machine.tilemap().create(tilemap_get_info_delegate(tile_get_info, "", &machine), mapper, tilewidth, tileheight, cols, rows); }

// create a new tilemap that is owned by a device
inline tilemap_t *tilemap_create_device(device_t *device, tile_get_info_device_func tile_get_info, tilemap_mapper_func mapper, int tilewidth, int tileheight, int cols, int rows)
{ return &device->machine().tilemap().create(tilemap_get_info_delegate(tile_get_info, "", device), tilemap_mapper_delegate(mapper, "", &device->machine()), tilewidth, tileheight, cols, rows); }

inline tilemap_t *tilemap_create_device(device_t *device, tile_get_info_device_func tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows)
{ return &device->machine().tilemap().create(tilemap_get_info_delegate(tile_get_info, "", device), mapper, tilewidth, tileheight, cols, rows); }



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline running_machine &tilemap_t::machine() const
{
	return m_manager.machine();
}


#endif	// __TILEMAP_H__
