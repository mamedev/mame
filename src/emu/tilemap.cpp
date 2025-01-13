// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tilemap.cpp

    Generic tilemap management system.

***************************************************************************/

#include "emu.h"
#include "tilemap.h"

#include "screen.h"


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  effective_rowscroll - return the effective
//  rowscroll value for a given index, taking into
//  account tilemap flip states
//-------------------------------------------------

inline s32 tilemap_t::effective_rowscroll(int index, u32 screen_width)
{
	// if we're flipping vertically, adjust the row number
	if (m_attributes & TILEMAP_FLIPY)
		index = m_scrollrows - 1 - index;

	// adjust final result based on the horizontal flip and dx values
	s32 value;
	if (!(m_attributes & TILEMAP_FLIPX))
		value = m_dx - m_rowscroll[index];
	else
		value = screen_width - m_width - (m_dx_flipped - m_rowscroll[index]);

	// clamp to 0..width
	if (value < 0)
		value = m_width - (-value) % m_width;
	else
		value %= m_width;
	return value;
}


//-------------------------------------------------
//  effective_colscroll - return the effective
//  colscroll value for a given index, taking into
//  account tilemap flip states
//-------------------------------------------------

inline s32 tilemap_t::effective_colscroll(int index, u32 screen_height)
{
	// if we're flipping horizontally, adjust the column number
	if (m_attributes & TILEMAP_FLIPX)
		index = m_scrollcols - 1 - index;

	// adjust final result based on the vertical flip and dx values
	s32 value;
	if (!(m_attributes & TILEMAP_FLIPY))
		value = m_dy - m_colscroll[index];
	else
		value = screen_height - m_height - (m_dy_flipped - m_colscroll[index]);

	// clamp to 0..height
	if (value < 0)
		value = m_height - (-value) % m_height;
	else
		value %= m_height;
	return value;
}


//-------------------------------------------------
//  gfx_tiles_changed - return true if any
//  gfx_elements used by this tilemap have
//  changed
//-------------------------------------------------

inline bool tilemap_t::gfx_elements_changed()
{
	u32 usedmask = m_gfx_used;
	bool isdirty = false;

	// iterate over all used gfx types and set the dirty flag if any of them have changed
	for (int gfxnum = 0; usedmask != 0; usedmask >>= 1, gfxnum++)
		if ((usedmask & 1) != 0)
			if (m_gfx_dirtyseq[gfxnum] != m_tileinfo.decoder->gfx(gfxnum)->dirtyseq())
			{
				m_gfx_dirtyseq[gfxnum] = m_tileinfo.decoder->gfx(gfxnum)->dirtyseq();
				isdirty = true;
			}

	return isdirty;
}


//**************************************************************************
//  SCANLINE RASTERIZERS
//**************************************************************************

//-------------------------------------------------
//  scanline_draw_opaque_null - draw to a nullptr
//  bitmap, setting priority only
//-------------------------------------------------

inline void tilemap_t::scanline_draw_opaque_null(int count, u8 *pri, u32 pcode)
{
	// skip entirely if not changing priority
	if (pcode == 0xff00)
		return;

	// update priority across the scanline
	for (int i = 0; i < count; i++)
		pri[i] = (pri[i] & (pcode >> 8)) | pcode;
}


//-------------------------------------------------
//  scanline_draw_masked_null - draw to a nullptr
//  bitmap using a mask, setting priority only
//-------------------------------------------------

inline void tilemap_t::scanline_draw_masked_null(const u8 *maskptr, int mask, int value, int count, u8 *pri, u32 pcode)
{
	// skip entirely if not changing priority
	if (pcode == 0xff00)
		return;

	// update priority across the scanline, checking the mask
	for (int i = 0; i < count; i++)
		if ((maskptr[i] & mask) == value)
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
}



//-------------------------------------------------
//  scanline_draw_opaque_ind16 - draw to a 16bpp
//  indexed bitmap
//-------------------------------------------------

inline void tilemap_t::scanline_draw_opaque_ind16(u16 *dest, const u16 *source, int count, u8 *pri, u32 pcode)
{
	// special case for no palette offset
	int pal = pcode >> 16;
	if (pal == 0)
	{
		// use memcpy which should be well-optimized for the platform
		memcpy(dest, source, count * 2);

		// skip the rest if not changing priority
		if (pcode == 0xff00)
			return;

		// update priority across the scanline
		for (int i = 0; i < count; i++)
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
	}

	// priority case
	else if ((pcode & 0xffff) != 0xff00)
	{
		for (int i = 0; i < count; i++)
		{
			dest[i] = source[i] + pal;
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	// no priority case
	else
	{
		for (int i = 0; i < count; i++)
			dest[i] = source[i] + pal;
	}
}


//-------------------------------------------------
//  scanline_draw_masked_ind16 - draw to a 16bpp
//  indexed bitmap using a mask
//-------------------------------------------------

inline void tilemap_t::scanline_draw_masked_ind16(u16 *dest, const u16 *source, const u8 *maskptr, int mask, int value, int count, u8 *pri, u32 pcode)
{
	int pal = pcode >> 16;

	// priority case
	if ((pcode & 0xffff) != 0xff00)
	{
		for (int i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = source[i] + pal;
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	// no priority case
	else
	{
		for (int i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = source[i] + pal;
	}
}



//-------------------------------------------------
//  scanline_draw_opaque_rgb32 - draw to a 32bpp
//  RGB bitmap
//-------------------------------------------------

inline void tilemap_t::scanline_draw_opaque_rgb32(u32 *dest, const u16 *source, int count, const rgb_t *pens, u8 *pri, u32 pcode)
{
	const rgb_t *clut = &pens[pcode >> 16];

	// priority case
	if ((pcode & 0xffff) != 0xff00)
	{
		for (int i = 0; i < count; i++)
		{
			dest[i] = clut[source[i]];
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	// no priority case
	else
	{
		for (int i = 0; i < count; i++)
			dest[i] = clut[source[i]];
	}
}


//-------------------------------------------------
//  scanline_draw_masked_rgb32 - draw to a 32bpp
//  RGB bitmap using a mask
//-------------------------------------------------

inline void tilemap_t::scanline_draw_masked_rgb32(u32 *dest, const u16 *source, const u8 *maskptr, int mask, int value, int count, const rgb_t *pens, u8 *pri, u32 pcode)
{
	const rgb_t *clut = &pens[pcode >> 16];

	// priority case
	if ((pcode & 0xffff) != 0xff00)
	{
		for (int i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = clut[source[i]];
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	// no priority case
	else
	{
		for (int i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = clut[source[i]];
	}
}


//-------------------------------------------------
//  scanline_draw_opaque_rgb32_alpha - draw to a
//  32bpp RGB bitmap with alpha blending
//-------------------------------------------------

inline void tilemap_t::scanline_draw_opaque_rgb32_alpha(u32 *dest, const u16 *source, int count, const rgb_t *pens, u8 *pri, u32 pcode, u8 alpha)
{
	const rgb_t *clut = &pens[pcode >> 16];

	// priority case
	if ((pcode & 0xffff) != 0xff00)
	{
		for (int i = 0; i < count; i++)
		{
			dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}

	// no priority case
	else
	{
		for (int i = 0; i < count; i++)
			dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
	}
}


//-------------------------------------------------
//  scanline_draw_masked_rgb32_alpha - draw to a
//  32bpp RGB bitmap using a mask and alpha
//  blending
//-------------------------------------------------

inline void tilemap_t::scanline_draw_masked_rgb32_alpha(u32 *dest, const u16 *source, const u8 *maskptr, int mask, int value, int count, const rgb_t *pens, u8 *pri, u32 pcode, u8 alpha)
{
	const rgb_t *clut = &pens[pcode >> 16];

	// priority case
	if ((pcode & 0xffff) != 0xff00)
	{
		for (int i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}

	// no priority case
	else
	{
		for (int i = 0; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = alpha_blend_r32(dest[i], clut[source[i]], alpha);
	}
}


//**************************************************************************
//  TILEMAP CREATION AND CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  tilemap_t - constructor
//-------------------------------------------------

tilemap_t::tilemap_t(device_t &owner)
	: m_mapper(owner)
	, m_tile_get_info(owner)
{
	// until init() is called, data is floating; this is deliberate
}


//-------------------------------------------------
//  init - initialize the tilemap
//-------------------------------------------------

void tilemap_t::init_common(
		tilemap_manager &manager,
		device_gfx_interface &decoder,
		tilemap_get_info_delegate tile_get_info,
		u16 tilewidth,
		u16 tileheight,
		u32 cols,
		u32 rows)
{
	// populate managers and devices
	m_manager = &manager;
	m_device = dynamic_cast<device_t *>(this);
	m_palette = &decoder.palette();
	m_next = nullptr;
	m_user_data = nullptr;

	// populate tilemap metrics
	m_rows = rows;
	m_cols = cols;
	m_tilewidth = tilewidth;
	m_tileheight = tileheight;
	m_width = cols * tilewidth;
	m_height = rows * tileheight;

	// initialize tile information geters
	m_tile_get_info = tile_get_info;

	// reset global states
	m_enable = true;
	m_attributes = 0;
	m_all_tiles_dirty = true;
	m_all_tiles_clean = false;
	m_palette_offset = 0;
	m_gfx_used = 0;
	memset(m_gfx_dirtyseq, 0, sizeof(m_gfx_dirtyseq));

	// reset scroll information
	m_scrollrows = 1;
	m_scrollcols = 1;
	m_rowscroll.resize(m_height);
	memset(&m_rowscroll[0], 0, m_height*sizeof(m_rowscroll[0]));
	m_colscroll.resize(m_width);
	memset(&m_colscroll[0], 0, m_width*sizeof(m_colscroll[0]));
	m_dx = 0;
	m_dx_flipped = 0;
	m_dy = 0;
	m_dy_flipped = 0;

	// allocate pixmap
	m_pixmap.allocate(m_width, m_height);

	// allocate transparency mapping
	m_flagsmap.allocate(m_width, m_height);
	memset(m_pen_to_flags, 0, sizeof(m_pen_to_flags));

	// create the initial mappings
	mappings_create();

	// set up the default tile data
	memset(&m_tileinfo, 0, sizeof(m_tileinfo));
	m_tileinfo.decoder = &decoder;
	m_tileinfo.pen_mask = 0xff;
	m_tileinfo.gfxnum = 0xff;

	// allocate transparency mapping data
	for (int group = 0; group < TILEMAP_NUM_GROUPS; group++)
		map_pens_to_layer(group, 0, 0, TILEMAP_PIXEL_LAYER0);

	// save relevant state
	int instance = manager.alloc_instance();
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_enable));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_attributes));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_palette_offset));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_scrollrows));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_scrollcols));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_rowscroll));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_colscroll));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_dx));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_dx_flipped));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_dy));
	machine().save().save_item(m_device, "tilemap", nullptr, instance, NAME(m_dy_flipped));

	// reset everything after a load
	machine().save().register_postload(save_prepost_delegate(FUNC(tilemap_t::postload), this));
}

tilemap_t &tilemap_t::init(
		tilemap_manager &manager,
		device_gfx_interface &decoder,
		tilemap_get_info_delegate tile_get_info,
		tilemap_mapper_delegate mapper,
		u16 tilewidth,
		u16 tileheight,
		u32 cols,
		u32 rows)
{
	// populate logical <-> memory mappings
	m_mapper = mapper;

	init_common(manager, decoder, tile_get_info, tilewidth, tileheight, cols, rows);

	return *this;
}

tilemap_t &tilemap_t::init(
		tilemap_manager &manager,
		device_gfx_interface &decoder,
		tilemap_get_info_delegate tile_get_info,
		tilemap_standard_mapper mapper,
		u16 tilewidth,
		u16 tileheight,
		u32 cols,
		u32 rows)
{
	// populate logical <-> memory mappings
	switch (mapper)
	{
		case TILEMAP_SCAN_ROWS:         m_mapper.set(*this, FUNC(tilemap_t::scan_rows));         break;
		case TILEMAP_SCAN_ROWS_FLIP_X:  m_mapper.set(*this, FUNC(tilemap_t::scan_rows_flip_x));  break;
		case TILEMAP_SCAN_ROWS_FLIP_Y:  m_mapper.set(*this, FUNC(tilemap_t::scan_rows_flip_y));  break;
		case TILEMAP_SCAN_ROWS_FLIP_XY: m_mapper.set(*this, FUNC(tilemap_t::scan_rows_flip_xy)); break;
		case TILEMAP_SCAN_COLS:         m_mapper.set(*this, FUNC(tilemap_t::scan_cols));         break;
		case TILEMAP_SCAN_COLS_FLIP_X:  m_mapper.set(*this, FUNC(tilemap_t::scan_cols_flip_x));  break;
		case TILEMAP_SCAN_COLS_FLIP_Y:  m_mapper.set(*this, FUNC(tilemap_t::scan_cols_flip_y));  break;
		case TILEMAP_SCAN_COLS_FLIP_XY: m_mapper.set(*this, FUNC(tilemap_t::scan_cols_flip_xy)); break;
		default: throw emu_fatalerror("Tilemap init unknown mapper %d", mapper);
	}

	init_common(manager, decoder, tile_get_info, tilewidth, tileheight, cols, rows);

	return *this;
}


//-------------------------------------------------
//  ~tilemap_t - destructor
//-------------------------------------------------

tilemap_t::~tilemap_t()
{
}


//-------------------------------------------------
//  mark_tile_dirty - mark a single tile dirty
//  based on its memory index
//-------------------------------------------------

void tilemap_t::mark_tile_dirty(tilemap_memory_index memindex)
{
	// only mark if within range
	if (memindex < m_memory_to_logical.size())
	{
		// there may be no logical index for a given memory index
		logical_index logindex = m_memory_to_logical[memindex];
		if (logindex != INVALID_LOGICAL_INDEX)
		{
			m_tileflags[logindex] = TILE_FLAG_DIRTY;
			m_all_tiles_clean = false;
		}
	}
}


//-------------------------------------------------
//  map_pens_to_layer - specify the mapping of one
//  or more pens (where (<pen> & mask) == pen) to
//  a layer
//-------------------------------------------------

void tilemap_t::map_pens_to_layer(int group, pen_t pen, pen_t mask, u8 layermask)
{
	assert(group < TILEMAP_NUM_GROUPS);
	assert((layermask & TILEMAP_PIXEL_CATEGORY_MASK) == 0);

	// we start at the index where (pen & mask) == pen, and all other bits are 0
	pen_t start = pen & mask;

	// we stop at the index where (pen & mask) == pen, and all other bits are 1
	pen_t stop = start | ~mask;

	// clamp to the number of entries actually there
	stop = std::min(stop, MAX_PEN_TO_FLAGS - 1);

	// iterate and set
	u8 *array = m_pen_to_flags + group * MAX_PEN_TO_FLAGS;
	bool changed = false;
	for (pen_t cur = start; cur <= stop; cur++)
		if ((cur & mask) == pen && array[cur] != layermask)
		{
			changed = true;
			array[cur] = layermask;
		}

	// everything gets dirty if anything changed
	if (changed)
		mark_all_dirty();
}


//-------------------------------------------------
//  set_transparent_pen - set a single transparent
//  pen into the tilemap, mapping all other pens
//  to layer 0
//-------------------------------------------------

void tilemap_t::set_transparent_pen(pen_t pen)
{
	// reset the whole pen map to opaque
	map_pens_to_layer(0, 0, 0, TILEMAP_PIXEL_LAYER0);

	// set the single pen to transparent
	map_pen_to_layer(0, pen, TILEMAP_PIXEL_TRANSPARENT);
}


//-------------------------------------------------
//  set_transmask - set up the first 32 pens using
//  a foreground mask (mapping to layer 0) and a
//  background mask (mapping to layer 1)
//-------------------------------------------------

void tilemap_t::set_transmask(int group, u32 fgmask, u32 bgmask)
{
	// iterate over all 32 pens specified
	for (pen_t pen = 0; pen < 32; pen++)
	{
		u8 fgbits = ((fgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER0;
		u8 bgbits = ((bgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER1;
		map_pen_to_layer(group, pen, fgbits | bgbits);
	}
}


//-------------------------------------------------
//  configure_groups - configure groups so that
//  when group == color, pens whose indirect value
//  matches the given transcolor are transparent
//-------------------------------------------------

void tilemap_t::configure_groups(gfx_element &gfx, indirect_pen_t transcolor)
{
	assert(gfx.colors() <= TILEMAP_NUM_GROUPS);

	// iterate over all colors in the tilemap
	for (u32 color = 0; color < gfx.colors(); color++)
		set_transmask(color, m_palette->transpen_mask(gfx, color, transcolor), 0);
}



//**************************************************************************
//  COMMON LOGICAL-TO-MEMORY MAPPERS
//**************************************************************************

//-------------------------------------------------
// scan_rows
// scan_rows_flip_x
// scan_rows_flip_y
// scan_rows_flip_xy - scan in row-major
//  order with optional flipping
//-------------------------------------------------

tilemap_memory_index tilemap_t::scan_rows(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return row * num_cols + col;
}

tilemap_memory_index tilemap_t::scan_rows_flip_x(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return row * num_cols + (num_cols - 1 - col);
}

tilemap_memory_index tilemap_t::scan_rows_flip_y(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return (num_rows - 1 - row) * num_cols + col;
}

tilemap_memory_index tilemap_t::scan_rows_flip_xy(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return (num_rows - 1 - row) * num_cols + (num_cols - 1 - col);
}


//-------------------------------------------------
//  scan_cols
//  scan_cols_flip_x
//  scan_cols_flip_y
//  scan_cols_flip_xy - scan in column-
//  major order with optional flipping
//-------------------------------------------------

tilemap_memory_index tilemap_t::scan_cols(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return col * num_rows + row;
}

tilemap_memory_index tilemap_t::scan_cols_flip_x(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return (num_cols - 1 - col) * num_rows + row;
}

tilemap_memory_index tilemap_t::scan_cols_flip_y(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return col * num_rows + (num_rows - 1 - row);
}

tilemap_memory_index tilemap_t::scan_cols_flip_xy(u32 col, u32 row, u32 num_cols, u32 num_rows)
{
	return (num_cols - 1 - col) * num_rows + (num_rows - 1 - row);
}


//-------------------------------------------------
//  postload - after loading a save state
//  invalidate everything
//-------------------------------------------------

void tilemap_t::postload()
{
	mappings_update();
}



//**************************************************************************
//  LOGICAL <-> MEMORY INDEX MAPPING
//**************************************************************************

//-------------------------------------------------
//  mappings_create - allocate memory for the
//  mapping tables and compute their extents
//-------------------------------------------------

void tilemap_t::mappings_create()
{
	// compute the maximum logical index
	const logical_index max_logical_index = m_rows * m_cols;

	// compute the maximum memory index
	tilemap_memory_index max_memory_index = 0;
	for (u32 row = 0; row < m_rows; row++)
		for (u32 col = 0; col < m_cols; col++)
		{
			tilemap_memory_index memindex = memory_index(col, row);
			max_memory_index = std::max(max_memory_index, memindex);
		}
	max_memory_index++;

	// allocate the necessary mappings
	m_memory_to_logical.resize(max_memory_index);
	m_logical_to_memory.resize(max_logical_index);
	m_tileflags.resize(max_logical_index);

	// update the mappings
	mappings_update();
}


//-------------------------------------------------
//  mappings_update - update the mappings after
//  a major change (flip x/y changes)
//-------------------------------------------------

void tilemap_t::mappings_update()
{
	// initialize all the mappings to invalid values
	memset(&m_memory_to_logical[0], 0xff, m_memory_to_logical.size() * sizeof(m_memory_to_logical[0]));

	// now iterate over all logical indexes and populate the memory index
	for (logical_index logindex = 0; logindex < m_logical_to_memory.size(); logindex++)
	{
		u32 logical_col = logindex % m_cols;
		u32 logical_row = logindex / m_cols;
		tilemap_memory_index memindex = memory_index(logical_col, logical_row);

		// apply tilemap flip to get the final location to store
		if (m_attributes & TILEMAP_FLIPX)
			logical_col = (m_cols - 1) - logical_col;
		if (m_attributes & TILEMAP_FLIPY)
			logical_row = (m_rows - 1) - logical_row;
		u32 flipped_logindex = logical_row * m_cols + logical_col;

		// fill in entries in both arrays
		m_memory_to_logical[memindex] = flipped_logindex;
		m_logical_to_memory[flipped_logindex] = memindex;
	}

	// mark the whole tilemap dirty
	mark_all_dirty();
}



//**************************************************************************
//  TILE RENDERING
//**************************************************************************

inline void tilemap_t::realize_all_dirty_tiles()
{
	// if all the tiles are marked dirty, or something in the gfx has changed,
	// flush the dirty status to all tiles
	if (m_all_tiles_dirty || gfx_elements_changed())
	{
		memset(&m_tileflags[0], TILE_FLAG_DIRTY, m_tileflags.size());
		m_all_tiles_dirty = false;
		m_gfx_used = 0;
	}
}

//-------------------------------------------------
//  pixmap_update - update the entire pixmap
//-------------------------------------------------

void tilemap_t::pixmap_update()
{
	// if the graphics changed, we need to mark everything dirty
	if (gfx_elements_changed())
		mark_all_dirty();

	// if everything is clean, do nothing
	if (m_all_tiles_clean)
		return;

	auto profile = g_profiler.start(PROFILER_TILEMAP_DRAW);

	// flush the dirty state to all tiles as appropriate
	realize_all_dirty_tiles();

	// iterate over rows and columns
	logical_index logindex = 0;
	for (u32 row = 0; row < m_rows; row++)
		for (u32 col = 0; col < m_cols; col++, logindex++)
			if (m_tileflags[logindex] == TILE_FLAG_DIRTY)
				tile_update(logindex, col, row);

	// mark it all clean
	m_all_tiles_clean = true;
}


//-------------------------------------------------
//  tile_update - update a single dirty tile
//-------------------------------------------------

void tilemap_t::tile_update(logical_index logindex, u32 col, u32 row)
{
	auto profile = g_profiler.start(PROFILER_TILEMAP_UPDATE);

	// call the get info callback for the associated memory index
	tilemap_memory_index memindex = m_logical_to_memory[logindex];
	m_tile_get_info(*this, m_tileinfo, memindex);

	// apply the global tilemap flip to the returned flip flags
	u32 flags = m_tileinfo.flags ^ (m_attributes & 0x03);

	// draw the tile, using either direct or transparent
	u32 x0 = m_tilewidth * col;
	u32 y0 = m_tileheight * row;
	m_tileflags[logindex] = tile_draw(m_tileinfo.pen_data, x0, y0,
		m_tileinfo.palette_base, m_tileinfo.category, m_tileinfo.group, flags, m_tileinfo.pen_mask);

	// if mask data is specified, apply it
	if ((flags & (TILE_FORCE_LAYER0 | TILE_FORCE_LAYER1 | TILE_FORCE_LAYER2)) == 0 && m_tileinfo.mask_data != nullptr)
		m_tileflags[logindex] = tile_apply_bitmask(m_tileinfo.mask_data, x0, y0, m_tileinfo.category, flags);

	// track which gfx have been used for this tilemap
	if (m_tileinfo.gfxnum != 0xff && (m_gfx_used & (1 << m_tileinfo.gfxnum)) == 0)
	{
		m_gfx_used |= 1 << m_tileinfo.gfxnum;
		m_gfx_dirtyseq[m_tileinfo.gfxnum] = m_tileinfo.decoder->gfx(m_tileinfo.gfxnum)->dirtyseq();
	}
}


//-------------------------------------------------
//  tile_draw - draw a single tile to the
//  tilemap's internal pixmap, using the pen as
//  the pen_to_flags lookup value, and adding
//  the palette_base
//-------------------------------------------------

u8 tilemap_t::tile_draw(const u8 *pendata, u32 x0, u32 y0, u32 palette_base, u8 category, u8 group, u8 flags, u8 pen_mask)
{
	// OR in the force layer flags
	category |= flags & (TILE_FORCE_LAYER0 | TILE_FORCE_LAYER1 | TILE_FORCE_LAYER2);

	// if we're vertically flipped, point to the bottom row and work backwards
	int dy0 = 1;
	if (flags & TILE_FLIPY)
	{
		y0 += m_tileheight - 1;
		dy0 = -1;
	}

	// if we're horizontally flipped, point to the rightmost column and work backwards
	int dx0 = 1;
	if (flags & TILE_FLIPX)
	{
		x0 += m_tilewidth - 1;
		dx0 = -1;
	}

	// iterate over rows
	const u8 *penmap = m_pen_to_flags + group * MAX_PEN_TO_FLAGS;
	u8 andmask = ~0, ormask = 0;
	for (u16 ty = 0; ty < m_tileheight; ty++)
	{
		u16 *pixptr = &m_pixmap.pix(y0, x0);
		u8 *flagsptr = &m_flagsmap.pix(y0, x0);

		// pre-advance to the next row
		y0 += dy0;

		// 8bpp data
		int xoffs = 0;
		for (u16 tx = 0; tx < m_tilewidth; tx++)
		{
			u8 pen = (*pendata++) & pen_mask;
			u8 map = penmap[pen];
			pixptr[xoffs] = palette_base + pen;
			flagsptr[xoffs] = map | category;
			andmask &= map;
			ormask |= map;
			xoffs += dx0;
		}
	}
	return andmask ^ ormask;
}


//-------------------------------------------------
//  tile_apply_bitmask - apply a bitmask to an
//  already-rendered tile by modifying the
//  flagsmap appropriately
//-------------------------------------------------

u8 tilemap_t::tile_apply_bitmask(const u8 *maskdata, u32 x0, u32 y0, u8 category, u8 flags)
{
	// if we're vertically flipped, point to the bottom row and work backwards
	int dy0 = 1;
	if (flags & TILE_FLIPY)
	{
		y0 += m_tileheight - 1;
		dy0 = -1;
	}

	// if we're horizontally flipped, point to the rightmost column and work backwards
	int dx0 = 1;
	if (flags & TILE_FLIPX)
	{
		x0 += m_tilewidth - 1;
		dx0 = -1;
	}

	// iterate over rows
	u8 andmask = ~0, ormask = 0;
	int bitoffs = 0;
	for (u16 ty = 0; ty < m_tileheight; ty++)
	{
		// pre-advance to the next row
		u8 *flagsptr = &m_flagsmap.pix(y0, x0);
		y0 += dy0;

		// anywhere the bitmask is 0 should be transparent
		int xoffs = 0;
		for (u16 tx = 0; tx < m_tilewidth; tx++)
		{
			u8 map = flagsptr[xoffs];

			if ((maskdata[bitoffs / 8] & (0x80 >> (bitoffs & 7))) == 0)
				map = flagsptr[xoffs] = TILEMAP_PIXEL_TRANSPARENT | category;
			andmask &= map;
			ormask |= map;
			bitoffs++;
			xoffs += dx0;
		}
	}
	return andmask ^ ormask;
}



//**************************************************************************
//  DRAWING HELPERS
//**************************************************************************

//-------------------------------------------------
//  configure_blit_parameters - fill in the
//  standard blit parameters based on the input
//  data; this code is shared by normal, roz,
//  and indexed drawing code
//-------------------------------------------------

void tilemap_t::configure_blit_parameters(blit_parameters &blit, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	// set the target bitmap
	blit.priority = &priority_bitmap;
	blit.cliprect = cliprect;

	// set the priority code and alpha
	blit.tilemap_priority_code = priority | (priority_mask << 8) | (m_palette_offset << 16);
	blit.alpha = (flags & TILEMAP_DRAW_ALPHA_FLAG) ? (flags >> 24) : 0xff;

	// tile priority; unless otherwise specified, draw anything in layer 0
	blit.mask = TILEMAP_PIXEL_CATEGORY_MASK;
	blit.value = flags & TILEMAP_PIXEL_CATEGORY_MASK;

	// if no layers specified, draw layer 0
	if ((flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2)) == 0)
		flags |= TILEMAP_DRAW_LAYER0;

	// OR in the bits from the draw masks
	blit.mask |= flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2);
	blit.value |= flags & (TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_LAYER2);

	// for all-opaque rendering, don't check any of the layer bits
	if (flags & TILEMAP_DRAW_OPAQUE)
	{
		blit.mask &= ~(TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 | TILEMAP_PIXEL_LAYER2);
		blit.value &= ~(TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1 | TILEMAP_PIXEL_LAYER2);
	}

	// don't check category if requested
	if (flags & TILEMAP_DRAW_ALL_CATEGORIES)
	{
		blit.mask &= ~TILEMAP_PIXEL_CATEGORY_MASK;
		blit.value &= ~TILEMAP_PIXEL_CATEGORY_MASK;
	}
}


//-------------------------------------------------
//  draw_common - draw a tilemap to the
//  destination with clipping; pixels apply
//  priority/priority_mask to the priority bitmap
//-------------------------------------------------

template<class _BitmapClass>
void tilemap_t::draw_common(screen_device &screen, _BitmapClass &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	// skip if disabled
	if (!m_enable)
		return;

	auto profile = g_profiler.start(PROFILER_TILEMAP_DRAW);

	// configure the blit parameters based on the input parameters
	blit_parameters blit;
	configure_blit_parameters(blit, screen.priority(), cliprect, flags, priority, priority_mask);
	assert(dest.cliprect().contains(cliprect));
	assert(screen.cliprect().contains(cliprect) || blit.tilemap_priority_code == 0xff00);

	// flush the dirty state to all tiles as appropriate
	realize_all_dirty_tiles();

	// flip the tilemap around the center of the visible area
	rectangle const visarea = screen.visible_area();
	u32 const xextent = visarea.right() + visarea.left() + 1; // x0 + x1 + 1 for calculating horizontal centre as (x0 + x1 + 1) >> 1
	u32 const yextent = visarea.bottom() + visarea.top() + 1; // y0 + y1 + 1 for calculating vertical centre as (y0 + y1 + 1) >> 1

	// XY scrolling playfield
	if (m_scrollrows == 1 && m_scrollcols == 1)
	{
		// iterate to handle wraparound
		int scrollx = effective_rowscroll(0, xextent);
		int scrolly = effective_colscroll(0, yextent);
		for (int ypos = scrolly - m_height; ypos <= blit.cliprect.bottom(); ypos += m_height)
			for (int xpos = scrollx - m_width; xpos <= blit.cliprect.right(); xpos += m_width)
				draw_instance(screen, dest, blit, xpos, ypos);
	}

	// scrolling rows + vertical scroll
	else if (m_scrollcols == 1)
	{
		const rectangle original_cliprect = blit.cliprect;

		// iterate over Y to handle wraparound
		int rowheight = m_height / m_scrollrows;
		int scrolly = effective_colscroll(0, yextent);
		for (int ypos = scrolly - m_height; ypos <= original_cliprect.bottom(); ypos += m_height)
		{
			int const firstrow = std::max((original_cliprect.top() - ypos) / rowheight, 0);
			int const lastrow = std::min((original_cliprect.bottom() - ypos) / rowheight, s32(m_scrollrows) - 1);

			// iterate over rows in the tilemap
			int nextrow;
			for (int currow = firstrow; currow <= lastrow; currow = nextrow)
			{
				// scan forward until we find a non-matching row
				int scrollx = effective_rowscroll(currow, xextent);
				for (nextrow = currow + 1; nextrow <= lastrow; nextrow++)
					if (effective_rowscroll(nextrow, xextent) != scrollx)
						break;

				// skip if disabled
				if (scrollx == TILE_LINE_DISABLED)
					continue;

				// update the cliprect just for this set of rows
				blit.cliprect.sety(currow * rowheight + ypos, nextrow * rowheight - 1 + ypos);
				blit.cliprect &= original_cliprect;

				// iterate over X to handle wraparound
				for (int xpos = scrollx - m_width; xpos <= original_cliprect.right(); xpos += m_width)
					draw_instance(screen, dest, blit, xpos, ypos);
			}
		}
	}

	// scrolling columns + horizontal scroll
	else if (m_scrollrows == 1)
	{
		const rectangle original_cliprect = blit.cliprect;

		// iterate over columns in the tilemap
		int scrollx = effective_rowscroll(0, xextent);
		int colwidth = m_width / m_scrollcols;
		int nextcol;
		for (int curcol = 0; curcol < m_scrollcols; curcol = nextcol)
		{
			// scan forward until we find a non-matching column
			int scrolly = effective_colscroll(curcol, yextent);
			for (nextcol = curcol + 1; nextcol < m_scrollcols; nextcol++)
				if (effective_colscroll(nextcol, yextent) != scrolly)
					break;

			// skip if disabled
			if (scrolly == TILE_LINE_DISABLED)
				continue;

			// iterate over X to handle wraparound
			for (int xpos = scrollx - m_width; xpos <= original_cliprect.right(); xpos += m_width)
			{
				// update the cliprect just for this set of columns
				blit.cliprect.setx(curcol * colwidth + xpos, nextcol * colwidth - 1 + xpos);
				blit.cliprect &= original_cliprect;

				// iterate over Y to handle wraparound
				for (int ypos = scrolly - m_height; ypos <= original_cliprect.bottom(); ypos += m_height)
					draw_instance(screen, dest, blit, xpos, ypos);
			}
		}
	}
}

void tilemap_t::draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{ draw_common(screen, dest, cliprect, flags, priority, priority_mask); }

void tilemap_t::draw(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{ draw_common(screen, dest, cliprect, flags, priority, priority_mask); }


//-------------------------------------------------
//  draw_roz - draw a tilemap to the destination
//  with clipping and arbitrary rotate/zoom; pixels
//  apply priority/priority_mask to the priority
//  bitmap
//-------------------------------------------------

template<class _BitmapClass>
void tilemap_t::draw_roz_common(screen_device &screen, _BitmapClass &dest, const rectangle &cliprect,
		u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy,
		bool wraparound, u32 flags, u8 priority, u8 priority_mask)
{
// notes:
// - startx and starty MUST be u32 for calculations to work correctly
// - srcbim_width and height are assumed to be a power of 2 to speed up wraparound

	// skip if disabled
	if (!m_enable)
		return;

	// see if this is just a regular render and if so, do a regular render
	if (incxx == (1 << 16) && incxy == 0 && incyx == 0 && incyy == (1 << 16) && wraparound)
	{
		set_scrollx(0, startx >> 16);
		set_scrolly(0, starty >> 16);
		draw(screen, dest, cliprect, flags, priority, priority_mask);
		return;
	}

	auto profile = g_profiler.start(PROFILER_TILEMAP_DRAW_ROZ);

	// configure the blit parameters
	blit_parameters blit;
	configure_blit_parameters(blit, screen.priority(), cliprect, flags, priority, priority_mask);
	assert(dest.cliprect().contains(cliprect));
	assert(screen.cliprect().contains(cliprect) || blit.tilemap_priority_code == 0xff00);

	// get the full pixmap for the tilemap
	pixmap();

	// then do the roz copy
	draw_roz_core(screen, dest, blit, startx, starty, incxx, incxy, incyx, incyy, wraparound);
}

void tilemap_t::draw_roz(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect,
		u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy,
		bool wraparound, u32 flags, u8 priority, u8 priority_mask)
{ draw_roz_common(screen, dest, cliprect, startx, starty, incxx, incxy, incyx, incyy, wraparound, flags, priority, priority_mask); }

void tilemap_t::draw_roz(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy,
		bool wraparound, u32 flags, u8 priority, u8 priority_mask)
{ draw_roz_common(screen, dest, cliprect, startx, starty, incxx, incxy, incyx, incyy, wraparound, flags, priority, priority_mask); }


//-------------------------------------------------
//  draw_instance - draw a single instance of the
//  tilemap to the internal pixmap at the given
//  xpos,ypos
//-------------------------------------------------

template<class _BitmapClass>
void tilemap_t::draw_instance(screen_device &screen, _BitmapClass &dest, const blit_parameters &blit, int xpos, int ypos)
{
	// clip destination coordinates to the tilemap
	// note that x2/y2 are exclusive, not inclusive
	int x1 = (std::max)(xpos, blit.cliprect.left());
	int x2 = (std::min)(xpos + int(m_width), blit.cliprect.right() + 1);
	int y1 = (std::max)(ypos, blit.cliprect.top());
	int y2 = (std::min)(ypos + int(m_height), blit.cliprect.bottom() + 1);

	// if totally clipped, stop here
	if (x1 >= x2 || y1 >= y2)
		return;

	// look up priority and destination base addresses for y1
	bitmap_ind8 &priority_bitmap = *blit.priority;
	u8 *priority_baseaddr = nullptr;
	int prio_rowpixels = 0;
	if (priority_bitmap.valid())
	{
		prio_rowpixels = priority_bitmap.rowpixels();
		priority_baseaddr = &priority_bitmap.pix(y1, xpos);
	}
	else
		assert((blit.tilemap_priority_code & 0xffff) == 0xff00);

	typename _BitmapClass::pixel_t *dest_baseaddr = nullptr;
	int dest_rowpixels = 0;
	if (dest.valid())
	{
		dest_rowpixels = dest.rowpixels();
		dest_baseaddr = &dest.pix(y1, xpos);
	}

	// convert screen coordinates to source tilemap coordinates
	x1 -= xpos;
	y1 -= ypos;
	x2 -= xpos;
	y2 -= ypos;

	// get tilemap pixels
	const u16 *source_baseaddr = &m_pixmap.pix(y1);
	const u8 *mask_baseaddr = &m_flagsmap.pix(y1);

	// get start/stop columns, rounding outward
	int mincol = x1 / m_tilewidth;
	int maxcol = (x2 + m_tilewidth - 1) / m_tilewidth;

	// set up row counter
	int y = y1;
	int nexty = m_tileheight * (y1 / m_tileheight) + m_tileheight;
	nexty = std::min(nexty, y2);

	// loop over tilemap rows
	for (;;)
	{
		int row = y / m_tileheight;
		int x_start = x1;

		// iterate across the applicable tilemap columns
		trans_t prev_trans = WHOLLY_TRANSPARENT;
		trans_t cur_trans;
		for (int column = mincol; column <= maxcol; column++)
		{
			int x_end;

			// we don't actually render the last column; it is always just used for flushing
			if (column == maxcol)
				cur_trans = WHOLLY_TRANSPARENT;

			// for other columns we look up the transparency information
			else
			{
				logical_index logindex = row * m_cols + column;

				// if the current tile is dirty, fix it
				if (m_tileflags[logindex] == TILE_FLAG_DIRTY)
					tile_update(logindex, column, row);

				// if the current summary data is non-zero, we must draw masked
				if ((m_tileflags[logindex] & blit.mask) != 0)
					cur_trans = MASKED;

				// otherwise, our transparency state is constant across the tile; fetch it
				else
					cur_trans = ((mask_baseaddr[column * m_tilewidth] & blit.mask) == blit.value) ? WHOLLY_OPAQUE : WHOLLY_TRANSPARENT;
			}

			// if the transparency state is the same as last time, don't render yet
			if (cur_trans == prev_trans)
				continue;

			// compute the end of this run, in pixels
			x_end = column * m_tilewidth;
			x_end = std::max(x_end, x1);
			x_end = std::min(x_end, x2);

			// if we're rendering something, compute the pointers
			const rgb_t *clut = m_palette->palette()->entry_list_adjusted();
			if (prev_trans != WHOLLY_TRANSPARENT)
			{
				const u16 *source0 = source_baseaddr + x_start;
				typename _BitmapClass::pixel_t *dest0 = dest_baseaddr + x_start;
				u8 *pmap0 = priority_baseaddr ? (priority_baseaddr + x_start) : nullptr;

				// if we were opaque, use the opaque renderer
				if (prev_trans == WHOLLY_OPAQUE)
				{
					for (int cury = y; cury < nexty; cury++)
					{
						if (dest_baseaddr == nullptr)
							scanline_draw_opaque_null(x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 2)
							scanline_draw_opaque_ind16(reinterpret_cast<u16 *>(dest0), source0, x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4 && blit.alpha >= 0xff)
							scanline_draw_opaque_rgb32(reinterpret_cast<u32 *>(dest0), source0, x_end - x_start, clut, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4)
							scanline_draw_opaque_rgb32_alpha(reinterpret_cast<u32 *>(dest0), source0, x_end - x_start, clut, pmap0, blit.tilemap_priority_code, blit.alpha);

						dest0 += dest_rowpixels;
						source0 += m_pixmap.rowpixels();
						pmap0 += prio_rowpixels;
					}
				}

				// otherwise use the masked renderer
				else
				{
					const u8 *mask0 = mask_baseaddr + x_start;
					for (int cury = y; cury < nexty; cury++)
					{
						if (dest_baseaddr == nullptr)
							scanline_draw_masked_null(mask0, blit.mask, blit.value, x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 2)
							scanline_draw_masked_ind16(reinterpret_cast<u16 *>(dest0), source0, mask0, blit.mask, blit.value, x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4 && blit.alpha >= 0xff)
							scanline_draw_masked_rgb32(reinterpret_cast<u32 *>(dest0), source0, mask0, blit.mask, blit.value, x_end - x_start, clut, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4)
							scanline_draw_masked_rgb32_alpha(reinterpret_cast<u32 *>(dest0), source0, mask0, blit.mask, blit.value, x_end - x_start, clut, pmap0, blit.tilemap_priority_code, blit.alpha);

						dest0 += dest_rowpixels;
						source0 += m_pixmap.rowpixels();
						mask0 += m_flagsmap.rowpixels();
						pmap0 += prio_rowpixels;
					}
				}
			}

			// the new start is the end
			x_start = x_end;
			prev_trans = cur_trans;
		}

		// if this was the last row, stop
		if (nexty == y2)
			break;

		// advance to the next row on all our bitmaps
		priority_baseaddr += prio_rowpixels * (nexty - y);
		source_baseaddr += m_pixmap.rowpixels() * (nexty - y);
		mask_baseaddr += m_flagsmap.rowpixels() * (nexty - y);
		dest_baseaddr += dest_rowpixels * (nexty - y);

		// increment the Y counter
		y = nexty;
		nexty += m_tileheight;
		nexty = std::min(nexty, y2);
	}
}


//-------------------------------------------------
//  tilemap_draw_roz_core - render the tilemap's
//  pixmap to the destination with rotation
//  and zoom
//-------------------------------------------------

#define ROZ_PLOT_PIXEL(INPUT_VAL)                                           \
do {                                                                        \
	if (sizeof(*dest) == 2)                                                 \
		*dest = (INPUT_VAL) + (priority >> 16);                             \
	else if (sizeof(*dest) == 4 && alpha >= 0xff)                           \
		*dest = clut[INPUT_VAL];                                            \
	else if (sizeof(*dest) == 4)                                            \
		*dest = alpha_blend_r32(*dest, clut[INPUT_VAL], alpha);             \
} while (0)

template<class _BitmapClass>
void tilemap_t::draw_roz_core(screen_device &screen, _BitmapClass &destbitmap, const blit_parameters &blit,
		u32 startx, u32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound)
{
	// pre-cache all the inner loop values
	const rgb_t *clut = m_palette->palette()->entry_list_adjusted() + (blit.tilemap_priority_code >> 16);
	bitmap_ind8 &priority_bitmap = *blit.priority;
	const int xmask = m_pixmap.width() - 1;
	const int ymask = m_pixmap.height() - 1;
	const int widthshifted = m_pixmap.width() << 16;
	const int heightshifted = m_pixmap.height() << 16;
	const u32 priority = blit.tilemap_priority_code;
	u8 mask = blit.mask;
	u8 value = blit.value;
	u8 alpha = blit.alpha;

	// pre-advance based on the cliprect
	startx += blit.cliprect.left() * incxx + blit.cliprect.top() * incyx;
	starty += blit.cliprect.left() * incxy + blit.cliprect.top() * incyy;

	// extract start/end points
	int sx = blit.cliprect.left();
	int sy = blit.cliprect.top();
	int ex = blit.cliprect.right();
	int ey = blit.cliprect.bottom();

	// optimized loop for the not rotated case
	if (incxy == 0 && incyx == 0 && !wraparound)
	{
		// skip without drawing until we are within the bitmap
		while (startx >= widthshifted && sx <= ex)
		{
			startx += incxx;
			sx++;
		}

		// early exit if we're done already
		if (sx > ex)
			return;

		// loop over rows
		while (sy <= ey)
		{
			// only draw if Y is within range
			if (starty < heightshifted)
			{
				// initialize X counters
				int x = sx;
				u32 cx = startx;
				u32 cy = starty >> 16;

				// get source and priority pointers
				u8 *pri = (priority != 0xff00) ? &priority_bitmap.pix(sy, sx) : nullptr;
				const u16 *src = &m_pixmap.pix(cy);
				const u8 *maskptr = &m_flagsmap.pix(cy);
				typename _BitmapClass::pixel_t *dest = &destbitmap.pix(sy, sx);

				// loop over columns
				while (x <= ex && cx < widthshifted)
				{
					// plot if we match the mask
					if ((maskptr[cx >> 16] & mask) == value)
					{
						ROZ_PLOT_PIXEL(src[cx >> 16]);
						if (priority != 0xff00)
							*pri = (*pri & (priority >> 8)) | priority;
					}

					// advance in X
					cx += incxx;
					x++;
					++dest;
					if (priority != 0xff00)
						pri++;
				}
			}

			// advance in Y
			starty += incyy;
			sy++;
		}
	}

	// wraparound case
	else if (wraparound)
	{
		// loop over rows
		while (sy <= ey)
		{
			// initialize X counters
			int x = sx;
			u32 cx = startx;
			u32 cy = starty;

			// get dest and priority pointers
			typename _BitmapClass::pixel_t *dest = &destbitmap.pix(sy, sx);
			u8 *pri = (priority != 0xff00) ? &priority_bitmap.pix(sy, sx) : nullptr;

			// loop over columns
			while (x <= ex)
			{
				// plot if we match the mask
				if ((m_flagsmap.pix((cy >> 16) & ymask, (cx >> 16) & xmask) & mask) == value)
				{
					ROZ_PLOT_PIXEL(m_pixmap.pix((cy >> 16) & ymask, (cx >> 16) & xmask));
					if (priority != 0xff00)
						*pri = (*pri & (priority >> 8)) | priority;
				}

				// advance in X
				cx += incxx;
				cy += incxy;
				x++;
				++dest;
				if (priority != 0xff00)
					pri++;
			}

			// advance in Y
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}

	// non-wraparound case
	else
	{
		// loop over rows
		while (sy <= ey)
		{
			// initialize X counters
			int x = sx;
			u32 cx = startx;
			u32 cy = starty;

			// get dest and priority pointers
			typename _BitmapClass::pixel_t *dest = &destbitmap.pix(sy, sx);
			u8 *pri = (priority != 0xff00) ? &priority_bitmap.pix(sy, sx) : nullptr;

			// loop over columns
			while (x <= ex)
			{
				// plot if we're within the bitmap and we match the mask
				if (cx < widthshifted && cy < heightshifted)
					if ((m_flagsmap.pix(cy >> 16, cx >> 16) & mask) == value)
					{
						ROZ_PLOT_PIXEL(m_pixmap.pix(cy >> 16, cx >> 16));
						if (priority != 0xff00)
							*pri = (*pri & (priority >> 8)) | priority;
					}

				// advance in X
				cx += incxx;
				cy += incxy;
				x++;
				++dest;
				if (priority != 0xff00)
					pri++;
			}

			// advance in Y
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}


//-------------------------------------------------
//  draw_debug - draw a debug version without any
//  rowscroll and with fixed parameters
//-------------------------------------------------

void tilemap_t::draw_debug(screen_device &screen, bitmap_rgb32 &dest, u32 scrollx, u32 scrolly, u32 flags)
{
	// set up for the blit, using hard-coded parameters (no priority, etc)
	blit_parameters blit;
	bitmap_ind8 dummy_priority;

	// draw everything
	flags |= TILEMAP_DRAW_OPAQUE;

	configure_blit_parameters(blit, dummy_priority, dest.cliprect(), flags, 0, 0xff);

	// compute the effective scroll positions
	scrollx = m_width  - scrollx % m_width;
	scrolly = m_height - scrolly % m_height;

	// flush the dirty state to all tiles as appropriate
	realize_all_dirty_tiles();

	// iterate to handle wraparound
	for (int ypos = scrolly - m_height; ypos <= blit.cliprect.bottom(); ypos += m_height)
		for (int xpos = scrollx - m_width; xpos <= blit.cliprect.right(); xpos += m_width)
			draw_instance(screen, dest, blit, xpos, ypos);
}


//-------------------------------------------------
//  get_info_debug - extract info for one tile
//-------------------------------------------------

void tilemap_t::get_info_debug(u32 col, u32 row, u8 &gfxnum, u32 &code, u32 &color)
{
	// first map to the memory index
	if (m_attributes & TILEMAP_FLIPX)
		col = (m_cols - 1) - col;
	if (m_attributes & TILEMAP_FLIPY)
		row = (m_rows - 1) - row;
	tilemap_memory_index memindex = memory_index(col, row);

	// next invoke the get info callback
	m_tile_get_info(*this, m_tileinfo, memindex);

	// get the GFX number and code
	gfxnum = m_tileinfo.gfxnum;
	code = m_tileinfo.code;
	color = m_tileinfo.palette_base;

	if (gfxnum != 0xff)
	{
		// work back from the palette base to get the color
		const gfx_element &gfx = *m_tileinfo.decoder->gfx(gfxnum);
		color = (color - gfx.colorbase()) / gfx.granularity();
	}
}


//**************************************************************************
//  TILEMAP MANAGER
//**************************************************************************

//-------------------------------------------------
//  tilemap_manager - constructor
//-------------------------------------------------

tilemap_manager::tilemap_manager(running_machine &machine)
	: m_machine(machine),
		m_instance(0)
{
}


//-------------------------------------------------
//  ~tilemap_manager - destructor
//-------------------------------------------------

tilemap_manager::~tilemap_manager()
{
	// detach all device tilemaps since they will be destroyed as subdevices elsewhere
	bool found = true;
	while (found)
	{
		found = false;
		for (tilemap_t &tmap : m_tilemap_list)
			if (tmap.m_device)
			{
				found = true;
				m_tilemap_list.detach(tmap);
				break;
			}
	}
}


//-------------------------------------------------
//  create - allocate a tilemap
//-------------------------------------------------

tilemap_t &tilemap_manager::create(device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows, tilemap_t *allocated)
{
	if (!allocated)
		allocated = new tilemap_t(machine().root_device());
	return m_tilemap_list.append(allocated->init(*this, decoder, tile_get_info, mapper, tilewidth, tileheight, cols, rows));
}

tilemap_t &tilemap_manager::create(device_gfx_interface &decoder, tilemap_get_info_delegate tile_get_info, tilemap_standard_mapper mapper, u16 tilewidth, u16 tileheight, u32 cols, u32 rows, tilemap_t *allocated)
{
	if (!allocated)
		allocated = new tilemap_t(machine().root_device());
	return m_tilemap_list.append(allocated->init(*this, decoder, tile_get_info, mapper, tilewidth, tileheight, cols, rows));
}


//-------------------------------------------------
//  set_flip_all - set a global flip for all the
//  tilemaps
//-------------------------------------------------

void tilemap_manager::set_flip_all(u32 attributes)
{
	for (tilemap_t &tmap : m_tilemap_list)
		tmap.set_flip(attributes);
}


//-------------------------------------------------
//  mark_all_dirty - mark all the tiles in all the
//  tilemaps dirty
//-------------------------------------------------

void tilemap_manager::mark_all_dirty()
{
	for (tilemap_t &tmap : m_tilemap_list)
		tmap.mark_all_dirty();
}



//**************************************************************************
//  TILEMAP DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TILEMAP, tilemap_device, "tilemap", "Tilemap")

//-------------------------------------------------
//  tilemap_device - constructor
//-------------------------------------------------

tilemap_device::tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TILEMAP, tag, owner, clock)
	, tilemap_t(static_cast<device_t &>(*this))
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_get_info(*this)
	, m_standard_mapper(TILEMAP_STANDARD_COUNT)
	, m_mapper(*this)
	, m_bytes_per_entry(0)
	, m_tile_width(8)
	, m_tile_height(8)
	, m_num_columns(64)
	, m_num_rows(64)
	, m_transparent_pen_set(false)
	, m_transparent_pen(0)
{
}


//-------------------------------------------------
//  write: Main memory writes
//-------------------------------------------------

void tilemap_device::write8(offs_t offset, u8 data)
{
	m_basemem.write8(offset, data);
	offset /= m_bytes_per_entry;
	mark_tile_dirty(offset);
}

void tilemap_device::write16(offs_t offset, u16 data, u16 mem_mask)
{
	m_basemem.write16(offset, data, mem_mask);
	offset = offset * 2 / m_bytes_per_entry;
	mark_tile_dirty(offset);
	if (m_bytes_per_entry < 2)
		mark_tile_dirty(offset + 1);
}

void tilemap_device::write32(offs_t offset, u32 data, u32 mem_mask)
{
	m_basemem.write32(offset, data, mem_mask);
	offset = offset * 4 / m_bytes_per_entry;
	mark_tile_dirty(offset);
	if (m_bytes_per_entry < 4)
	{
		mark_tile_dirty(offset + 1);
		if (m_bytes_per_entry < 2)
		{
			mark_tile_dirty(offset + 2);
			mark_tile_dirty(offset + 3);
		}
	}
}


//-------------------------------------------------
//  write_entry_ext: Extension memory writes
//-------------------------------------------------

void tilemap_device::write8_ext(offs_t offset, u8 data)
{
	m_extmem.write8(offset, data);
	offset /= m_bytes_per_entry;
	mark_tile_dirty(offset);
}

void tilemap_device::write16_ext(offs_t offset, u16 data, u16 mem_mask)
{
	m_extmem.write16(offset, data, mem_mask);
	offset = offset * 2 / m_bytes_per_entry;
	mark_tile_dirty(offset);
	if (m_bytes_per_entry < 2)
		mark_tile_dirty(offset + 1);
}

void tilemap_device::write32_ext(offs_t offset, u32 data, u32 mem_mask)
{
	m_extmem.write32(offset, data, mem_mask);
	offset = offset * 4 / m_bytes_per_entry;
	mark_tile_dirty(offset);
	if (m_bytes_per_entry < 4)
	{
		mark_tile_dirty(offset + 1);
		if (m_bytes_per_entry < 2)
		{
			mark_tile_dirty(offset + 2);
			mark_tile_dirty(offset + 3);
		}
	}
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void tilemap_device::device_start()
{
	// check configuration
	if (m_get_info.isnull())
		throw emu_fatalerror("Tilemap device '%s' has no get info callback!", tag());
	if (m_standard_mapper == TILEMAP_STANDARD_COUNT && m_mapper.isnull())
		throw emu_fatalerror("Tilemap device '%s' has no mapper callback!", tag());

	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	// bind our callbacks
	m_get_info.resolve();
	m_mapper.resolve();

	// allocate the tilemap
	if (m_standard_mapper == TILEMAP_STANDARD_COUNT)
		machine().tilemap().create(*m_gfxdecode, m_get_info, m_mapper, m_tile_width, m_tile_height, m_num_columns, m_num_rows, *this);
	else
		machine().tilemap().create(*m_gfxdecode, m_get_info, m_standard_mapper, m_tile_width, m_tile_height, m_num_columns, m_num_rows, *this);

	// find the memory, if present
	const memory_share *share = memshare(tag());
	if (share != nullptr)
	{
		m_basemem.set(*share, m_bytes_per_entry);

		// look for an extension entry
		std::string tag_ext = std::string(tag()).append("_ext");
		share = memshare(tag_ext);
		if (share != nullptr)
			m_extmem.set(*share, m_bytes_per_entry);
	}

	// configure the device and set the pen
	if (m_transparent_pen_set)
		set_transparent_pen(m_transparent_pen);
}
