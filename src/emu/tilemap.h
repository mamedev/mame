// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tilemap.h

    Generic tilemap management system.

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
                rendered during a tilemap::draw() call

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
            specifically the category (specified in bits 0-3) and the
            layer (specified in bits 4-6).

****************************************************************************

    How to use a tilemap:

    1. First create a new tilemap by calling tilemap_manager::create().
        The parameters are as follows:

        decoder = reference to your device_gfx_interface; note that the
            graphics the tilemap will use do not have to be decoded
            first, but the decoder must be ready to provide a palette,
            which means device_missing_dependencies must be thrown if
            the decoder has not already started

        tile_get_info = callback function which accepts a memory index
            and in return fills in a tile_data structure that describes
            the characteristics of a tile; this function will be called
            whenever a dirty tile needs to be rendered

        mapper = callback function which maps the logical column and row
            to a memory index; several standard mappers are provided,
            with TILEMAP_SCAN_ROWS being the most common

        tilewidth = the width, in pixels, of each individual tile

        tileheight = the height, in pixels, of each individual tile

        cols = the number of columns in the tilemap

        rows = the number of rows in the tilemap

    2. Once you have created your tilemap, you need to configure it.
        Common configuration tasks include:

            * marking one of the pens as transparent via
                tilemap_t::set_transparent_pen()

            * performing more complex pen-to-layer mapping via
                tilemap_t::map_pen_to_layer() or
                tilemap_t::map_pens_to_layer()

            * configuring global scroll offsets via
                tilemap_t::set_scrolldx() and tilemap_t::set_scrolldy()

            * specifying a pointer that can be read back later (e.g. in
                your tile_get_info callback) via
                tilemap_t::set_user_data()

            * setting a global palette offset via
                tilemap_t::set_palette_offset()

    3. In your memory write handlers for the tile memory, anytime tile
        data is modified, you need to mark the tile dirty so that it is
        re-rendered with the new data the next time the tilemap is drawn.
        Use tilemap_t::mark_tile_dirty() and pass in the memory index.

    4. In your handlers for scrolling, update the scroll values for the
        tilemap via tilemap_t::set_scrollx() and tilemap_t::set_scrolly().

    5. If any other major characteristics of the tilemap change (generally
        any global state that is used by the tile_get_info callback but
        which is not reported via other calls to the tilemap code), you
        should invalidate the entire tilemap. You can do this by calling
        tilemap_t::mark_all_dirty().

    6. In your VIDEO_UPDATE callback, render the tiles by calling
        tilemap_t::draw() or tilemap_t::draw_roz(). If you need to do
        custom rendering and want access to the raw pixels, call
        tilemap_t::pixmap() to get a reference to the updated bitmap_ind16
        containing the tilemap graphics.

****************************************************************************

    The following example shows how to use the tilemap system to create
    a simple tilemap where pen 0 is transparent. Each tile is 8x8 pixels
    in size, and there are 32 rows and 64 columns, stored in row-major
    order. Based on bits in the tile memory, tiles can be drawn either
    behind or in front of sprites.

        tilemap_t *tmap;
        u16 *my_tmap_memory;
        required_device<gfxdecode_device> gfxdecode;

        TILE_GET_INFO_MEMBER( my_state::my_get_info )
        {
            u16 tiledata = my_tmap_memory[tile_index];
            u8 code = tiledata & 0xff;
            u8 color = (tiledata >> 8) & 0x1f;
            u8 flipx = (tiledata >> 13) & 1;
            u8 flipy = (tiledata >> 14) & 1;
            u8 category = (tiledata >> 15) & 1;

            // set the common info for the tile
            tileinfo.set(
                1,              // use gfxdecode->gfx(1) for tile graphics
                code,           // the index of the graphics for this tile
                color,          // the color to use for this tile
                (flipx ? TILE_FLIPX : 0) |  // flags for this tile; also
                (flipy ? TILE_FLIPY : 0)    // see the FLIP_YX macro
            );

            // set the category of each tile based on the high bit; this
            // allows us to draw each category independently
            tileinfo.category = category;
        }

        VIDEO_START_MEMBER( my_state, my_driver )
        {
            // first create the tilemap
            tmap = &machine().tilemap().create(
                    gfxdecode,
                    tilemap_get_info_delegate(FUNC(my_state::my_get_info), this),
                    TILEMAP_SCAN_ROWS,      // standard row-major mapper
                    8,8,                    // 8x8 tiles
                    64,32);                 // 64 columns, 32 rows

            // then set the transparent pen; all other pens will default
            // to being part of layer 0
            tmap.set_transparent_pen(0);
        }

        u32 my_state::screen_update_mydriver(
            screen_device &screen,
            bitmap_ind16 &bitmap,
            const rectangle &cliprect)
        {
            // draw the tilemap first, fully opaque since it needs to
            // erase all previous pixels
            tmap->draw(
                screen,                 // destination screen
                bitmap,                 // destination bitmap
                cliprect,               // clipping rectangle
                TILEMAP_DRAW_OPAQUE);   // flags

            // next draw the sprites
            my_draw_sprites();

            // then draw the tiles which have priority over sprites
            tmap->draw(
                screen,                 // destination screen
                bitmap,                 // destination bitmap
                cliprect,               // clipping rectangle
                TILEMAP_DRAW_CATEGORY(1));// flags: draw category 1

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
            tilemap_t::set_transparent_pen() to specify which pen is
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
            on a per-tile basis. You just call tilemap_t::set_transmask(),
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
        tilemap_t::draw() with the TILEMAP_DRAW_ALPHA flag.

    * To configure more complex pen-to-layer mapping, use the
        tilemap_t::map_pens_to_layer() call. This call takes a group
        number so that you can configure 1 of the 256 groups
        independently. It also takes a pen and a mask; the mapping is
        updated for all pens where ((pennum & mask) == pen). To set all
        the pens in a group to the same value, pass a mask of 0. To set
        a single pen in a group, pass a mask of ~0. The helper function
        tilemap_t::map_pen_to_layer() does this for you.

***************************************************************************/

#ifndef MAME_EMU_TILEMAP_H
#define MAME_EMU_TILEMAP_H

#pragma once

#include "memarray.h"
#include <type_traits>
#include <utility>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maximum number of groups
constexpr size_t TILEMAP_NUM_GROUPS = 256;


// these flags control tilemap_t::draw() behavior
constexpr u32 TILEMAP_DRAW_CATEGORY_MASK = 0x0f;     // specify the category to draw
constexpr u32 TILEMAP_DRAW_LAYER0 = 0x10;            // draw layer 0
constexpr u32 TILEMAP_DRAW_LAYER1 = 0x20;            // draw layer 1
constexpr u32 TILEMAP_DRAW_LAYER2 = 0x40;            // draw layer 2
constexpr u32 TILEMAP_DRAW_OPAQUE = 0x80;            // draw everything, even transparent stuff
constexpr u32 TILEMAP_DRAW_ALPHA_FLAG = 0x100;       // draw with alpha blending (in the upper 8 bits)
constexpr u32 TILEMAP_DRAW_ALL_CATEGORIES = 0x200;   // draw all categories

// per-pixel flags in the transparency_bitmap
constexpr u8 TILEMAP_PIXEL_CATEGORY_MASK = 0x0f;     // category is stored in the low 4 bits
constexpr u8 TILEMAP_PIXEL_TRANSPARENT = 0x00;       // transparent if in none of the layers below
constexpr u8 TILEMAP_PIXEL_LAYER0 = 0x10;            // pixel is opaque in layer 0
constexpr u8 TILEMAP_PIXEL_LAYER1 = 0x20;            // pixel is opaque in layer 1
constexpr u8 TILEMAP_PIXEL_LAYER2 = 0x40;            // pixel is opaque in layer 2

// per-tile flags, set by get_tile_info callback
constexpr u8 TILE_FLIPX = 0x01;                      // draw this tile horizontally flipped
constexpr u8 TILE_FLIPY = 0x02;                      // draw this tile vertically flipped
constexpr u8 TILE_FORCE_LAYER0 = TILEMAP_PIXEL_LAYER0; // force all pixels to be layer 0 (no transparency)
constexpr u8 TILE_FORCE_LAYER1 = TILEMAP_PIXEL_LAYER1; // force all pixels to be layer 1 (no transparency)
constexpr u8 TILE_FORCE_LAYER2 = TILEMAP_PIXEL_LAYER2; // force all pixels to be layer 2 (no transparency)

// tilemap global flags, used by tilemap_t::set_flip()
constexpr u32 TILEMAP_FLIPX = TILE_FLIPX;            // draw the tilemap horizontally flipped
constexpr u32 TILEMAP_FLIPY = TILE_FLIPY;            // draw the tilemap vertically flipped

// set this value for a scroll row/column to fully disable it
constexpr u32 TILE_LINE_DISABLED = 0x80000000;

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

// global types
typedef u32 tilemap_memory_index;


// tile_data is filled in by the get_tile_info callback
struct tile_data
{
	device_gfx_interface *decoder;  // set in tilemap_t::init()
	const u8 *      pen_data;       // required
	const u8 *      mask_data;      // required
	pen_t           palette_base;   // defaults to 0
	u8              category;       // defaults to 0; range from 0..15
	u8              group;          // defaults to 0; range from 0..TILEMAP_NUM_GROUPS
	u8              flags;          // defaults to 0; one or more of TILE_* flags above
	u8              pen_mask;       // defaults to 0xff; mask to apply to pen_data while rendering the tile
	u8              gfxnum;         // defaults to 0xff; specify index of gfx for auto-invalidation on dirty
	u32             code;

	void set(u8 _gfxnum, u32 rawcode, u32 rawcolor, u8 _flags)
	{
		gfx_element *gfx = decoder->gfx(_gfxnum);
		code = rawcode % gfx->elements();
		pen_data = gfx->get_data(code);
		palette_base = gfx->colorbase() + gfx->granularity() * (rawcolor % gfx->colors());
		flags = _flags;
		gfxnum = _gfxnum;
	}
};


// modern delegates
typedef device_delegate<void (tilemap_t &, tile_data &, tilemap_memory_index)> tilemap_get_info_delegate;
typedef device_delegate<tilemap_memory_index (u32, u32, u32, u32)> tilemap_mapper_delegate;


// ======================> tilemap_t

// core tilemap structure
class tilemap_t
{
	DISABLE_COPYING(tilemap_t);

	friend class tilemap_device;
	friend class tilemap_manager;
	friend class simple_list<tilemap_t>;

	// logical index
	typedef u32 logical_index;

	// internal usage to mark tiles dirty
	static const u8 TILE_FLAG_DIRTY = 0xff;

	// invalid logical index
	static const logical_index INVALID_LOGICAL_INDEX = (logical_index)~0;

	// maximum index in each array
	static const pen_t MAX_PEN_TO_FLAGS = 256;

	void init_common(tilemap_manager &manager, device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, u16 tilewidth, u16 tileheight, u32 cols, u32 rows);

protected:
	// tilemap_manager controls our allocations
	tilemap_t(device_t &owner);
	virtual ~tilemap_t();

	tilemap_t &init(tilemap_manager &manager, device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows);
	tilemap_t &init(tilemap_manager &manager, device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_standard_mapper mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows);

public:
	// getters
	running_machine &machine() const;
	device_palette_interface &palette() const { return *m_palette; }
	device_gfx_interface &decoder() const { return *m_tileinfo.decoder; }

	tilemap_t *next() const { return m_next; }
	void *user_data() const { return m_user_data; }
	u32 rows() const { return m_rows; }
	u32 cols() const { return m_cols; }
	u16 tilewidth() const { return m_tilewidth; }
	u16 tileheight() const { return m_tileheight; }
	u32 width() const { return m_width; }
	u32 height() const { return m_height; }
	bool enabled() const { return m_enable; }
	u32 palette_offset() const { return m_palette_offset; }
	int scrolldx() const { return (m_attributes & TILEMAP_FLIPX) ? m_dx_flipped : m_dx; }
	int scrolldy() const { return (m_attributes & TILEMAP_FLIPY) ? m_dy_flipped : m_dy; }
	int scrollx(int which = 0) const { return (which < m_scrollrows) ? m_rowscroll[which] : 0; }
	int scrolly(int which = 0) const { return (which < m_scrollcols) ? m_colscroll[which] : 0; }
	bitmap_ind16 &pixmap() { pixmap_update(); return m_pixmap; }
	bitmap_ind8 &flagsmap() { pixmap_update(); return m_flagsmap; }
	u8 *tile_flags() { pixmap_update(); return &m_tileflags[0]; }
	tilemap_memory_index memory_index(u32 col, u32 row) { return m_mapper(col, row, m_cols, m_rows); }
	void get_info_debug(u32 col, u32 row, u8 &gfxnum, u32 &code, u32 &color);

	// setters
	void enable(bool enable = true) { m_enable = enable; }
	void set_user_data(void *user_data) { m_user_data = user_data; }
	void set_palette(device_palette_interface &palette) { m_palette = &palette; }
	void set_palette_offset(u32 offset) { m_palette_offset = offset; }
	void set_scrolldx(int dx, int dx_flipped) { m_dx = dx; m_dx_flipped = dx_flipped; }
	void set_scrolldy(int dy, int dy_flipped) { m_dy = dy; m_dy_flipped = dy_flipped; }
	void set_scrollx(int which, int value) { if (which < m_scrollrows) m_rowscroll[which] = value; }
	void set_scrolly(int which, int value) { if (which < m_scrollcols) m_colscroll[which] = value; }
	void set_scrollx(int value) { set_scrollx(0, value); }
	void set_scrolly(int value) { set_scrolly(0, value); }
	void set_scroll_rows(u32 scroll_rows) { assert(scroll_rows <= m_height); m_scrollrows = scroll_rows; }
	void set_scroll_cols(u32 scroll_cols) { assert(scroll_cols <= m_width); m_scrollcols = scroll_cols; }
	void set_flip(u32 attributes) { if (m_attributes != attributes) { m_attributes = attributes; mappings_update(); } }

	// dirtying
	void mark_mapping_dirty() { mappings_update(); }
	void mark_tile_dirty(tilemap_memory_index memindex);
	void mark_all_dirty() { m_all_tiles_dirty = true; m_all_tiles_clean = false; }

	// pen mapping
	void map_pens_to_layer(int group, pen_t pen, pen_t mask, u8 layermask);
	void map_pen_to_layer(int group, pen_t pen, u8 layermask) { map_pens_to_layer(group, pen, ~0, layermask); }
	void set_transparent_pen(pen_t pen);
	void set_transmask(int group, u32 fgmask, u32 bgmask);
	void configure_groups(gfx_element &gfx, indirect_pen_t transcolor);

	// drawing
	void draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
	void draw(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
	void draw_roz(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
	void draw_roz(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
	void draw_debug(screen_device &screen, bitmap_rgb32 &dest, u32 scrollx, u32 scrolly, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES);

	// mappers
	// scan in row-major order with optional flipping
	tilemap_memory_index scan_rows(u32 col, u32 row, u32 num_cols, u32 num_rows);
	tilemap_memory_index scan_rows_flip_x(u32 col, u32 row, u32 num_cols, u32 num_rows);
	tilemap_memory_index scan_rows_flip_y(u32 col, u32 row, u32 num_cols, u32 num_rows);
	tilemap_memory_index scan_rows_flip_xy(u32 col, u32 row, u32 num_cols, u32 num_rows);

	// scan in column-major order with optional flipping
	tilemap_memory_index scan_cols(u32 col, u32 row, u32 num_cols, u32 num_rows);
	tilemap_memory_index scan_cols_flip_x(u32 col, u32 row, u32 num_cols, u32 num_rows);
	tilemap_memory_index scan_cols_flip_y(u32 col, u32 row, u32 num_cols, u32 num_rows);
	tilemap_memory_index scan_cols_flip_xy(u32 col, u32 row, u32 num_cols, u32 num_rows);

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
		bitmap_ind8 *       priority;
		rectangle           cliprect;
		u32                 tilemap_priority_code;
		u8                  mask;
		u8                  value;
		u8                  alpha;
	};

	// inline helpers
	s32 effective_rowscroll(int index, u32 screen_width);
	s32 effective_colscroll(int index, u32 screen_height);
	bool gfx_elements_changed();

	// inline scanline rasterizers
	void scanline_draw_opaque_null(int count, u8 *pri, u32 pcode);
	void scanline_draw_masked_null(const u8 *maskptr, int mask, int value, int count, u8 *pri, u32 pcode);
	void scanline_draw_opaque_ind16(u16 *dest, const u16 *source, int count, u8 *pri, u32 pcode);
	void scanline_draw_masked_ind16(u16 *dest, const u16 *source, const u8 *maskptr, int mask, int value, int count, u8 *pri, u32 pcode);
	void scanline_draw_opaque_rgb32(u32 *dest, const u16 *source, int count, const rgb_t *pens, u8 *pri, u32 pcode);
	void scanline_draw_masked_rgb32(u32 *dest, const u16 *source, const u8 *maskptr, int mask, int value, int count, const rgb_t *pens, u8 *pri, u32 pcode);
	void scanline_draw_opaque_rgb32_alpha(u32 *dest, const u16 *source, int count, const rgb_t *pens, u8 *pri, u32 pcode, u8 alpha);
	void scanline_draw_masked_rgb32_alpha(u32 *dest, const u16 *source, const u8 *maskptr, int mask, int value, int count, const rgb_t *pens, u8 *pri, u32 pcode, u8 alpha);

	// internal helpers
	void postload();
	void mappings_create();
	void mappings_update();
	void realize_all_dirty_tiles();

	// internal drawing
	void pixmap_update();
	void tile_update(logical_index logindex, u32 col, u32 row);
	u8 tile_draw(const u8 *pendata, u32 x0, u32 y0, u32 palette_base, u8 category, u8 group, u8 flags, u8 pen_mask);
	u8 tile_apply_bitmask(const u8 *maskdata, u32 x0, u32 y0, u8 category, u8 flags);
	void configure_blit_parameters(blit_parameters &blit, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask);
	template<class _BitmapClass> void draw_common(screen_device &screen, _BitmapClass &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask);
	template<class _BitmapClass> void draw_roz_common(screen_device &screen, _BitmapClass &dest, const rectangle &cliprect, u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, u32 flags, u8 priority, u8 priority_mask);
	template<class _BitmapClass> void draw_instance(screen_device &screen, _BitmapClass &dest, const blit_parameters &blit, int xpos, int ypos);
	template<class _BitmapClass> void draw_roz_core(screen_device &screen, _BitmapClass &destbitmap, const blit_parameters &blit, u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound);

	// managers and devices
	tilemap_manager *           m_manager;              // reference to the owning manager
	device_t *                  m_device;               // pointer to our owning device
	device_palette_interface *  m_palette;              // palette used for drawing
	tilemap_t *                 m_next;                 // pointer to next tilemap
	void *                      m_user_data;            // user data value

	// basic tilemap metrics
	u32                         m_rows;                 // number of tile rows
	u32                         m_cols;                 // number of tile columns
	u16                         m_tilewidth;            // width of a single tile in pixels
	u16                         m_tileheight;           // height of a single tile in pixels
	u32                         m_width;                // width of the full tilemap in pixels
	u32                         m_height;               // height of the full tilemap in pixels

	// logical <-> memory mappings
	tilemap_mapper_delegate     m_mapper;               // callback to map a row/column to a memory index
	std::vector<logical_index>  m_memory_to_logical;    // map from memory index to logical index
	std::vector<tilemap_memory_index> m_logical_to_memory; // map from logical index to memory index

	// callback to interpret video RAM for the tilemap
	tilemap_get_info_delegate   m_tile_get_info;        // callback to get information about a tile
	tile_data                   m_tileinfo;             // structure to hold the data for a tile

	// global tilemap states
	bool                        m_enable;               // true if we are enabled
	u8                          m_attributes;           // global attributes (flipx/y)
	bool                        m_all_tiles_dirty;      // true if all tiles are dirty
	bool                        m_all_tiles_clean;      // true if all tiles are clean
	u32                         m_palette_offset;       // palette offset
	u32                         m_gfx_used;             // bitmask of gfx items used
	u32                         m_gfx_dirtyseq[MAX_GFX_ELEMENTS]; // dirtyseq values from last check

	// scroll information
	u32                         m_scrollrows;           // number of independently scrolled rows
	u32                         m_scrollcols;           // number of independently scrolled columns
	std::vector<s32>            m_rowscroll;            // array of rowscroll values
	std::vector<s32>            m_colscroll;            // array of colscroll values
	s32                         m_dx;                   // global horizontal scroll offset
	s32                         m_dx_flipped;           // global horizontal scroll offset when flipped
	s32                         m_dy;                   // global vertical scroll offset
	s32                         m_dy_flipped;           // global vertical scroll offset when flipped

	// pixel data
	bitmap_ind16                m_pixmap;               // cached pixel data

	// transparency mapping
	bitmap_ind8                 m_flagsmap;             // per-pixel flags
	std::vector<u8>             m_tileflags;            // per-tile flags
	u8                          m_pen_to_flags[MAX_PEN_TO_FLAGS * TILEMAP_NUM_GROUPS]; // mapping of pens to flags
};


// ======================> tilemap_manager

// tilemap manager
class tilemap_manager
{
	friend class tilemap_t;

public:
	// construction/destruction
	tilemap_manager(running_machine &machine);
	~tilemap_manager();

	// getters
	running_machine &machine() const { return m_machine; }

	// tilemap creation
	template <typename T, typename U>
	tilemap_t &create(device_gfx_interface &decoder, T &&tile_get_info, U &&mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows)
	{ return create(decoder, std::forward<T>(tile_get_info), std::forward<U>(mapper), tilewidth, tileheight, cols, rows, nullptr); }
	template <typename T, typename U, class V>
	std::enable_if_t<std::is_base_of<device_t, V>::value, tilemap_t &> create(device_gfx_interface &decoder, T &&tile_get_info, U &&mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows, V &allocated)
	{ return create(decoder, std::forward<T>(tile_get_info), std::forward<U>(mapper), tilewidth, tileheight, cols, rows, &static_cast<tilemap_t &>(allocated)); }

	// tilemap list information
	tilemap_t *find(int index) { return m_tilemap_list.find(index); }
	int count() const { return m_tilemap_list.count(); }

	// global operations on all tilemaps
	void mark_all_dirty();
	void set_flip_all(u32 attributes);

private:
	// tilemap creation
	tilemap_t &create(device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows, tilemap_t *allocated);
	tilemap_t &create(device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_standard_mapper mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows, tilemap_t *allocated);

	// allocate an instance index
	int alloc_instance() { return ++m_instance; }

	// internal state
	running_machine &       m_machine;
	simple_list<tilemap_t>  m_tilemap_list;
	int                     m_instance;
};


// ======================> tilemap_device

// device type definition
DECLARE_DEVICE_TYPE(TILEMAP, tilemap_device)

class tilemap_device :  public device_t,
						public tilemap_t
{
public:
	template <typename T>
	tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gfxtag, int entrybytes
		, u16 tilewidth, u16 tileheight, tilemap_standard_mapper mapper, u32 columns, u32 rows, pen_t transpen)
		: tilemap_device(mconfig, tag, owner, (u32)0)
	{
		set_gfxdecode(std::forward<T>(gfxtag));
		set_bytes_per_entry(entrybytes);
		set_layout(mapper, columns, rows);
		set_tile_size(tilewidth, tileheight);
		set_configured_transparent_pen(transpen);
	}

	template <typename T>
	tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gfxtag, int entrybytes
		, u16 tilewidth, u16 tileheight, tilemap_standard_mapper mapper, u32 columns, u32 rows)
		: tilemap_device(mconfig, tag, owner, (u32)0)
	{
		set_gfxdecode(std::forward<T>(gfxtag));
		set_bytes_per_entry(entrybytes);
		set_layout(mapper, columns, rows);
		set_tile_size(tilewidth, tileheight);
	}

	template <typename T>
	tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gfxtag, int entrybytes, u16 tilewidth, u16 tileheight)
		: tilemap_device(mconfig, tag, owner, (u32)0)
	{
		set_gfxdecode(std::forward<T>(gfxtag));
		set_bytes_per_entry(entrybytes);
		set_tile_size(tilewidth, tileheight);
	}

	tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_gfxdecode(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_bytes_per_entry(int bpe) { m_bytes_per_entry = bpe; }

	template <typename... T> void set_info_callback(T &&... args) { m_get_info.set(std::forward<T>(args)...); }

	void set_layout(tilemap_standard_mapper mapper, u32 columns, u32 rows)
	{
		assert(TILEMAP_STANDARD_COUNT > mapper);
		m_standard_mapper = mapper;
		m_num_columns = columns;
		m_num_rows = rows;
	}
	template <typename F>
	void set_layout(F &&callback, const char *name, u32 columns, u32 rows)
	{
		m_standard_mapper = TILEMAP_STANDARD_COUNT;
		m_mapper.set(std::forward<F>(callback), name);
		m_num_columns = columns;
		m_num_rows = rows;
	}
	template <typename T, typename F>
	void set_layout(T &&target, F &&callback, const char *name, u32 columns, u32 rows)
	{
		m_standard_mapper = TILEMAP_STANDARD_COUNT;
		m_mapper.set(std::forward<T>(target), std::forward<F>(callback), name);
		m_num_columns = columns;
		m_num_rows = rows;
	}
	void set_tile_size(u16 width, u16 height) { m_tile_width = width; m_tile_height = height; }
	void set_configured_transparent_pen(pen_t pen) { m_transparent_pen_set = true; m_transparent_pen = pen; }

	// getters
	memory_array &basemem() { return m_basemem; }
	memory_array &extmem() { return m_extmem; }

	// write handlers
	void write8(offs_t offset, u8 data);
	void write16(offs_t offset, u16 data, u16 mem_mask = ~0);
	void write32(offs_t offset, u32 data, u32 mem_mask = ~0);
	void write8_ext(offs_t offset, u8 data);
	void write16_ext(offs_t offset, u16 data, u16 mem_mask = ~0);
	void write32_ext(offs_t offset, u32 data, u32 mem_mask = ~0);

	// optional memory accessors
	u32 basemem_read(offs_t offset) { return m_basemem.read(offset); }
	u32 extmem_read(offs_t offset) { return m_extmem.read(offset); }
	void basemem_write(offs_t offset, u32 data) { m_basemem.write(offset, data); mark_tile_dirty(offset); }
	void extmem_write(offs_t offset, u32 data) { m_extmem.write(offset, data); mark_tile_dirty(offset); }

	// pick one to use to avoid ambiguity errors
	using device_t::machine;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// devices
	required_device<gfxdecode_device> m_gfxdecode;

	// configuration state
	tilemap_get_info_delegate   m_get_info;
	tilemap_standard_mapper     m_standard_mapper;
	tilemap_mapper_delegate     m_mapper;
	int                         m_bytes_per_entry;
	u16                         m_tile_width;
	u16                         m_tile_height;
	u32                         m_num_columns;
	u32                         m_num_rows;
	bool                        m_transparent_pen_set;
	pen_t                       m_transparent_pen;

	// optional memory info
	memory_array                m_basemem;              // info about base memory
	memory_array                m_extmem;               // info about extension memory
};



//**************************************************************************
//  MACROS
//**************************************************************************

// macros to help form flags for tilemap_t::draw
#define TILEMAP_DRAW_CATEGORY(x)        (x)     // specify category to draw
#define TILEMAP_DRAW_ALPHA(x)           (TILEMAP_DRAW_ALPHA_FLAG | (rgb_t::clamp(x) << 24))

// function definition for a get info callback
#define TILE_GET_INFO_MEMBER(_name)     void _name(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)

// function definition for a logical-to-memory mapper
#define TILEMAP_MAPPER_MEMBER(_name)    tilemap_memory_index _name(u32 col, u32 row, u32 num_cols, u32 num_rows)

// Helpers for setting tile attributes in the TILE_GET_INFO callback:
//   TILE_FLIP_YX assumes that flipy is in bit 1 and flipx is in bit 0
//   TILE_FLIP_XY assumes that flipy is in bit 0 and flipx is in bit 1
template <typename T> constexpr u8 TILE_FLIPYX(T yx) { return u8(yx & 3); }
template <typename T> constexpr u8 TILE_FLIPXY(T xy) { return u8(((xy & 2) >> 1) | ((xy & 1) << 1)); }



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline running_machine &tilemap_t::machine() const
{
	return m_manager->machine();
}


#endif // MAME_EMU_TILEMAP_H
