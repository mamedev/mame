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

        decoder = reference to your device_gfx_interface

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
        UINT16 *my_tmap_memory;
        required_device<gfxdecode_device> gfxdecode;

        TILE_GET_INFO_MEMBER( my_state::my_get_info )
        {
            UINT16 tiledata = my_tmap_memory[tile_index];
            UINT8 code = tiledata & 0xff;
            UINT8 color = (tiledata >> 8) & 0x1f;
            UINT8 flipx = (tiledata >> 13) & 1;
            UINT8 flipy = (tiledata >> 14) & 1;
            UINT8 category = (tiledata >> 15) & 1;

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

        UINT32 my_state::screen_update_mydriver(
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
#define TILEMAP_NUM_GROUPS              256


// these flags control tilemap_t::draw() behavior
const UINT32 TILEMAP_DRAW_CATEGORY_MASK = 0x0f;     // specify the category to draw
const UINT32 TILEMAP_DRAW_LAYER0 = 0x10;            // draw layer 0
const UINT32 TILEMAP_DRAW_LAYER1 = 0x20;            // draw layer 1
const UINT32 TILEMAP_DRAW_LAYER2 = 0x40;            // draw layer 2
const UINT32 TILEMAP_DRAW_OPAQUE = 0x80;            // draw everything, even transparent stuff
const UINT32 TILEMAP_DRAW_ALPHA_FLAG = 0x100;       // draw with alpha blending (in the upper 8 bits)
const UINT32 TILEMAP_DRAW_ALL_CATEGORIES = 0x200;   // draw all categories

// per-pixel flags in the transparency_bitmap
const UINT8 TILEMAP_PIXEL_CATEGORY_MASK = 0x0f;     // category is stored in the low 4 bits
const UINT8 TILEMAP_PIXEL_TRANSPARENT = 0x00;       // transparent if in none of the layers below
const UINT8 TILEMAP_PIXEL_LAYER0 = 0x10;            // pixel is opaque in layer 0
const UINT8 TILEMAP_PIXEL_LAYER1 = 0x20;            // pixel is opaque in layer 1
const UINT8 TILEMAP_PIXEL_LAYER2 = 0x40;            // pixel is opaque in layer 2

// per-tile flags, set by get_tile_info callback
const UINT8 TILE_FLIPX = 0x01;                      // draw this tile horizontally flipped
const UINT8 TILE_FLIPY = 0x02;                      // draw this tile vertically flipped
const UINT8 TILE_FORCE_LAYER0 = TILEMAP_PIXEL_LAYER0; // force all pixels to be layer 0 (no transparency)
const UINT8 TILE_FORCE_LAYER1 = TILEMAP_PIXEL_LAYER1; // force all pixels to be layer 1 (no transparency)
const UINT8 TILE_FORCE_LAYER2 = TILEMAP_PIXEL_LAYER2; // force all pixels to be layer 2 (no transparency)

// tilemap global flags, used by tilemap_t::set_flip()
const UINT32 TILEMAP_FLIPX = TILE_FLIPX;            // draw the tilemap horizontally flipped
const UINT32 TILEMAP_FLIPY = TILE_FLIPY;            // draw the tilemap vertically flipped

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
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// primitives
#define MCFG_TILEMAP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TILEMAP, 0)
#define MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	tilemap_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
#define MCFG_TILEMAP_BYTES_PER_ENTRY(_bpe) \
	tilemap_device::static_set_bytes_per_entry(*device, _bpe);
#define MCFG_TILEMAP_INFO_CB_DRIVER(_class, _method) \
	tilemap_device::static_set_info_callback(*device, tilemap_get_info_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_TILEMAP_INFO_CB_DEVICE(_device, _class, _method) \
	tilemap_device::static_set_info_callback(*device, tilemap_get_info_delegate(&_class::_method, #_class "::" #_method, _device, (_class *)0));
#define MCFG_TILEMAP_LAYOUT_STANDARD(_standard, _columns, _rows) \
	tilemap_device::static_set_layout(*device, TILEMAP_##_standard, _columns, _rows);
#define MCFG_TILEMAP_LAYOUT_CB_DRIVER(_class, _method, _columns, _rows) \
	tilemap_device::static_set_layout(*device, tilemap_mapper_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0), _columns, _rows);
#define MCFG_TILEMAP_LAYOUT_CB_DEVICE(_device, _class, _method, _columns, _rows) \
	tilemap_device::static_set_layout(*device, tilemap_mapper_delegate(&_class::_method, #_class "::" #_method, _device, (_class *)0), _columns, _rows);
#define MCFG_TILEMAP_TILE_SIZE(_width, _height) \
	tilemap_device::static_set_tile_size(*device, _width, _height);
#define MCFG_TILEMAP_TRANSPARENT_PEN(_pen) \
	tilemap_device::static_set_transparent_pen(*device, _pen);

// common cases
#define MCFG_TILEMAP_ADD_STANDARD(_tag, _gfxtag, _bytes_per_entry, _class, _method, _tilewidth, _tileheight, _mapper, _columns, _rows) \
	MCFG_TILEMAP_ADD(_tag) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(_bytes_per_entry) \
	MCFG_TILEMAP_INFO_CB_DRIVER(_class, _method) \
	MCFG_TILEMAP_LAYOUT_STANDARD(_mapper, _columns, _rows) \
	MCFG_TILEMAP_TILE_SIZE(_tilewidth, _tileheight)
#define MCFG_TILEMAP_ADD_CUSTOM(_tag, _gfxtag, _bytes_per_entry, _class, _method, _tilewidth, _tileheight, _mapper, _columns, _rows) \
	MCFG_TILEMAP_ADD(_tag) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(_bytes_per_entry) \
	MCFG_TILEMAP_INFO_CB_DRIVER(_class, _method) \
	MCFG_TILEMAP_LAYOUT_CB_DRIVER(_class, _mapper, _columns, _rows) \
	MCFG_TILEMAP_TILE_SIZE(_tilewidth, _tileheight)
#define MCFG_TILEMAP_ADD_STANDARD_TRANSPEN(_tag, _gfxtag, _bytes_per_entry, _class, _method, _tilewidth, _tileheight, _mapper, _columns, _rows, _transpen) \
	MCFG_TILEMAP_ADD(_tag) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(_bytes_per_entry) \
	MCFG_TILEMAP_INFO_CB_DRIVER(_class, _method) \
	MCFG_TILEMAP_LAYOUT_STANDARD(_mapper, _columns, _rows) \
	MCFG_TILEMAP_TILE_SIZE(_tilewidth, _tileheight) \
	MCFG_TILEMAP_TRANSPARENT_PEN(_transpen)
#define MCFG_TILEMAP_ADD_CUSTOM_TRANSPEN(_tag, _gfxtag, _bytes_per_entry, _class, _method, _tilewidth, _tileheight, _mapper, _columns, _rows, _transpen) \
	MCFG_TILEMAP_ADD(_tag) \
	MCFG_TILEMAP_GFXDECODE(_gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(_bytes_per_entry) \
	MCFG_TILEMAP_INFO_CB_DRIVER(_class, _method) \
	MCFG_TILEMAP_LAYOUT_CB_DRIVER(_columns, _mapper, _rows, _class) \
	MCFG_TILEMAP_TILE_SIZE(_tilewidth, _tileheight) \
	MCFG_TILEMAP_TRANSPARENT_PEN(_transpen)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class tilemap_t;
class tilemap_manager;
class tilemap_device;


// global types
typedef UINT32 tilemap_memory_index;


// tile_data is filled in by the get_tile_info callback
struct tile_data
{
	device_gfx_interface *decoder;  // set in tilemap_t::init()
	const UINT8 *   pen_data;       // required
	const UINT8 *   mask_data;      // required
	pen_t           palette_base;   // defaults to 0
	UINT8           category;       // defaults to 0; range from 0..15
	UINT8           group;          // defaults to 0; range from 0..TILEMAP_NUM_GROUPS
	UINT8           flags;          // defaults to 0; one or more of TILE_* flags above
	UINT8           pen_mask;       // defaults to 0xff; mask to apply to pen_data while rendering the tile
	UINT8           gfxnum;         // defaults to 0xff; specify index of gfx for auto-invalidation on dirty

	void set(int _gfxnum, int rawcode, int rawcolor, int _flags)
	{
		gfx_element *gfx = decoder->gfx(_gfxnum);
		int code = rawcode % gfx->elements();
		pen_data = gfx->get_data(code);
		palette_base = gfx->colorbase() + gfx->granularity() * (rawcolor % gfx->colors());
		flags = _flags;
		gfxnum = _gfxnum;
	}
};


// modern delegates
typedef device_delegate<void (tilemap_t &, tile_data &, tilemap_memory_index)> tilemap_get_info_delegate;
typedef device_delegate<tilemap_memory_index (UINT32, UINT32, UINT32, UINT32)> tilemap_mapper_delegate;


// ======================> tilemap_t

// core tilemap structure
class tilemap_t
{
	DISABLE_COPYING(tilemap_t);

	friend class tilemap_device;
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

protected:
	// tilemap_manager controlls our allocations
	tilemap_t();
	virtual ~tilemap_t();

	tilemap_t &init(tilemap_manager &manager, device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, int tilewidth, int tileheight, int cols, int rows);

public:
	// getters
	running_machine &machine() const;
	tilemap_device *device() const { return m_device; }
	palette_device &palette() const { return *m_palette; }
	tilemap_t *next() const { return m_next; }
	void *user_data() const { return m_user_data; }
	memory_array &basemem() { return m_basemem; }
	memory_array &extmem() { return m_extmem; }
	UINT32 width() const { return m_width; }
	UINT32 height() const { return m_height; }
	bool enabled() const { return m_enable; }
	int palette_offset() const { return m_palette_offset; }
	int scrolldx() const { return (m_attributes & TILEMAP_FLIPX) ? m_dx_flipped : m_dx; }
	int scrolldy() const { return (m_attributes & TILEMAP_FLIPY) ? m_dy_flipped : m_dy; }
	int scrollx(int which = 0) const { return (which < m_scrollrows) ? m_rowscroll[which] : 0; }
	int scrolly(int which = 0) const { return (which < m_scrollcols) ? m_colscroll[which] : 0; }
	bitmap_ind16 &pixmap() { pixmap_update(); return m_pixmap; }
	bitmap_ind8 &flagsmap() { pixmap_update(); return m_flagsmap; }
	UINT8 *tile_flags() { pixmap_update(); return &m_tileflags[0]; }
	tilemap_memory_index memory_index(UINT32 col, UINT32 row) { return m_mapper(col, row, m_cols, m_rows); }

	// setters
	void enable(bool enable = true) { m_enable = enable; }
	void set_user_data(void *user_data) { m_user_data = user_data; }
	void set_palette(palette_device &palette) { m_palette = &palette; }
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
	void configure_groups(gfx_element &gfx, int transcolor);

	// drawing
	void draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority = 0, UINT8 priority_mask = 0xff);
	void draw(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority = 0, UINT8 priority_mask = 0xff);
	void draw_roz(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, UINT32 flags, UINT8 priority = 0, UINT8 priority_mask = 0xff);
	void draw_roz(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, UINT32 flags, UINT8 priority = 0, UINT8 priority_mask = 0xff);
	void draw_debug(screen_device &screen, bitmap_rgb32 &dest, UINT32 scrollx, UINT32 scrolly);

	// mappers
	// scan in row-major order with optional flipping
	static tilemap_memory_index scan_rows(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_rows_flip_x(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_rows_flip_y(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_rows_flip_xy(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);

	// scan in column-major order with optional flipping
	static tilemap_memory_index scan_cols(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_cols_flip_x(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_cols_flip_y(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);
	static tilemap_memory_index scan_cols_flip_xy(driver_device &device, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows);

	// optional memory accessors
	UINT32 basemem_read(int index) { return m_basemem.read(index); }
	UINT32 extmem_read(int index) { return m_extmem.read(index); }
	void basemem_write(int index, UINT32 data) { m_basemem.write(index, data); mark_tile_dirty(index); }
	void extmem_write(int index, UINT32 data) { m_extmem.write(index, data); mark_tile_dirty(index); }

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
		UINT32              tilemap_priority_code;
		UINT8               mask;
		UINT8               value;
		UINT8               alpha;
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
	void scanline_draw_opaque_rgb32(UINT32 *dest, const UINT16 *source, int count, const rgb_t *pens, UINT8 *pri, UINT32 pcode);
	void scanline_draw_masked_rgb32(UINT32 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const rgb_t *pens, UINT8 *pri, UINT32 pcode);
	void scanline_draw_opaque_rgb32_alpha(UINT32 *dest, const UINT16 *source, int count, const rgb_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);
	void scanline_draw_masked_rgb32_alpha(UINT32 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const rgb_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha);

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
	void configure_blit_parameters(blit_parameters &blit, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask);
	template<class _BitmapClass> void draw_common(screen_device &screen, _BitmapClass &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask);
	template<class _BitmapClass> void draw_roz_common(screen_device &screen, _BitmapClass &dest, const rectangle &cliprect, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask);
	template<class _BitmapClass> void draw_instance(screen_device &screen, _BitmapClass &dest, const blit_parameters &blit, int xpos, int ypos);
	template<class _BitmapClass> void draw_roz_core(screen_device &screen, _BitmapClass &destbitmap, const blit_parameters &blit, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound);

	// managers and devices
	tilemap_manager *           m_manager;              // reference to the owning manager
	tilemap_device *            m_device;               // pointer to our owning device
	palette_device *            m_palette;              // palette used for drawing
	tilemap_t *                 m_next;                 // pointer to next tilemap
	void *                      m_user_data;            // user data value

	// optional memory info
	memory_array                m_basemem;              // info about base memory
	memory_array                m_extmem;               // info about extension memory

	// basic tilemap metrics
	UINT32                      m_rows;                 // number of tile rows
	UINT32                      m_cols;                 // number of tile columns
	UINT32                      m_tilewidth;            // width of a single tile in pixels
	UINT32                      m_tileheight;           // height of a single tile in pixels
	UINT32                      m_width;                // width of the full tilemap in pixels
	UINT32                      m_height;               // height of the full tilemap in pixels

	// logical <-> memory mappings
	tilemap_mapper_delegate      m_mapper;               // callback to map a row/column to a memory index
	std::vector<logical_index>        m_memory_to_logical;   // map from memory index to logical index
	std::vector<tilemap_memory_index> m_logical_to_memory; // map from logical index to memory index

	// callback to interpret video RAM for the tilemap
	tilemap_get_info_delegate   m_tile_get_info;        // callback to get information about a tile
	tile_data                   m_tileinfo;             // structure to hold the data for a tile

	// global tilemap states
	bool                        m_enable;               // true if we are enabled
	UINT8                       m_attributes;           // global attributes (flipx/y)
	bool                        m_all_tiles_dirty;      // true if all tiles are dirty
	bool                        m_all_tiles_clean;      // true if all tiles are clean
	UINT32                      m_palette_offset;       // palette offset
	UINT32                      m_gfx_used;             // bitmask of gfx items used
	UINT32                      m_gfx_dirtyseq[MAX_GFX_ELEMENTS]; // dirtyseq values from last check

	// scroll information
	UINT32                      m_scrollrows;           // number of independently scrolled rows
	UINT32                      m_scrollcols;           // number of independently scrolled columns
	std::vector<INT32>               m_rowscroll;            // array of rowscroll values
	std::vector<INT32>               m_colscroll;            // array of colscroll values
	INT32                       m_dx;                   // global horizontal scroll offset
	INT32                       m_dx_flipped;           // global horizontal scroll offset when flipped
	INT32                       m_dy;                   // global vertical scroll offset
	INT32                       m_dy_flipped;           // global vertical scroll offset when flipped

	// pixel data
	bitmap_ind16                m_pixmap;               // cached pixel data

	// transparency mapping
	bitmap_ind8                 m_flagsmap;             // per-pixel flags
	std::vector<UINT8>               m_tileflags;            // per-tile flags
	UINT8                       m_pen_to_flags[MAX_PEN_TO_FLAGS * TILEMAP_NUM_GROUPS]; // mapping of pens to flags
};


// ======================> tilemap_manager

// tilemap manager
class tilemap_manager
{
	friend class tilemap_t;

public:
	// construction/destuction
	tilemap_manager(running_machine &machine);
	~tilemap_manager();

	// getters
	running_machine &machine() const { return m_machine; }

	// tilemap creation
	tilemap_t &create(device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, int tilewidth, int tileheight, int cols, int rows, tilemap_t *allocated = nullptr);
	tilemap_t &create(device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows, tilemap_t *allocated = nullptr);

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
	running_machine &       m_machine;
	simple_list<tilemap_t>  m_tilemap_list;
	int                     m_instance;
};


// ======================> tilemap_device

// device type definition
extern const device_type TILEMAP;

class tilemap_device :  public device_t,
						public tilemap_t
{
public:
	// construction/destruction
	tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_bytes_per_entry(device_t &device, int bpe);
	static void static_set_info_callback(device_t &device, tilemap_get_info_delegate tile_get_info);
	static void static_set_layout(device_t &device, tilemap_standard_mapper mapper, int columns, int rows);
	static void static_set_layout(device_t &device, tilemap_mapper_delegate mapper, int columns, int rows);
	static void static_set_tile_size(device_t &device, int width, int height);
	static void static_set_transparent_pen(device_t &device, pen_t pen);

	// write handlers
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE32_MEMBER(write);
	DECLARE_WRITE8_MEMBER(write_ext);
	DECLARE_WRITE16_MEMBER(write_ext);
	DECLARE_WRITE32_MEMBER(write_ext);

	// pick one to use to avoid ambiguity errors
	using device_t::machine;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// devices
	required_device<gfxdecode_device> m_gfxdecode;

	// configuration state
	tilemap_get_info_delegate m_get_info;
	tilemap_standard_mapper m_standard_mapper;
	tilemap_mapper_delegate m_mapper;
	int             m_bytes_per_entry;
	int             m_tile_width;
	int             m_tile_height;
	int             m_num_columns;
	int             m_num_rows;
	bool            m_transparent_pen_set;
	pen_t           m_transparent_pen;
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
#define TILEMAP_MAPPER_MEMBER(_name)    tilemap_memory_index _name(UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)

// useful macro inside of a TILE_GET_INFO callback to set tile information
#define SET_TILE_INFO_MEMBER(GFX,CODE,COLOR,FLAGS)  tileinfo.set(GFX, CODE, COLOR, FLAGS)

// Macros for setting tile attributes in the TILE_GET_INFO callback:
//   TILE_FLIP_YX assumes that flipy is in bit 1 and flipx is in bit 0
//   TILE_FLIP_XY assumes that flipy is in bit 0 and flipx is in bit 1
#define TILE_FLIPYX(YX)                 ((YX) & 3)
#define TILE_FLIPXY(XY)                 ((((XY) & 2) >> 1) | (((XY) & 1) << 1))



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline running_machine &tilemap_t::machine() const
{
	return m_manager->machine();
}


#endif  // __TILEMAP_H__
