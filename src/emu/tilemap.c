/***************************************************************************

    tilemap.c

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

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  effective_rowscroll - return the effective
//  rowscroll value for a given index, taking into
//  account tilemap flip states
//-------------------------------------------------

inline INT32 tilemap_t::effective_rowscroll(int index, UINT32 screen_width)
{
	// if we're flipping vertically, adjust the row number
	if (m_attributes & TILEMAP_FLIPY)
		index = m_scrollrows - 1 - index;

	// adjust final result based on the horizontal flip and dx values
	INT32 value;
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

inline INT32 tilemap_t::effective_colscroll(int index, UINT32 screen_height)
{
	// if we're flipping horizontally, adjust the column number
	if (m_attributes & TILEMAP_FLIPX)
		index = m_scrollcols - 1 - index;

	// adjust final result based on the vertical flip and dx values
	INT32 value;
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
//  gfx_tiles_changed - return TRUE if any
//  gfx_elements used by this tilemap have
//  changed
//-------------------------------------------------

inline bool tilemap_t::gfx_elements_changed()
{
	UINT32 usedmask = m_gfx_used;
	bool isdirty = false;

	// iterate over all used gfx types and set the dirty flag if any of them have changed
	for (int gfxnum = 0; usedmask != 0; usedmask >>= 1, gfxnum++)
		if ((usedmask & 1) != 0)
			if (m_gfx_dirtyseq[gfxnum] != machine().gfx[gfxnum]->dirtyseq())
			{
				m_gfx_dirtyseq[gfxnum] = machine().gfx[gfxnum]->dirtyseq();
				isdirty = true;
			}

	return isdirty;
}


//**************************************************************************
//  SCANLINE RASTERIZERS
//**************************************************************************

//-------------------------------------------------
//  scanline_draw_opaque_null - draw to a NULL
//  bitmap, setting priority only
//-------------------------------------------------

inline void tilemap_t::scanline_draw_opaque_null(int count, UINT8 *pri, UINT32 pcode)
{
	// skip entirely if not changing priority
	if (pcode == 0xff00)
		return;

	// update priority across the scanline
	for (int i = 0; i < count; i++)
		pri[i] = (pri[i] & (pcode >> 8)) | pcode;
}


//-------------------------------------------------
//  scanline_draw_masked_null - draw to a NULL
//  bitmap using a mask, setting priority only
//-------------------------------------------------

inline void tilemap_t::scanline_draw_masked_null(const UINT8 *maskptr, int mask, int value, int count, UINT8 *pri, UINT32 pcode)
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

inline void tilemap_t::scanline_draw_opaque_ind16(UINT16 *dest, const UINT16 *source, int count, UINT8 *pri, UINT32 pcode)
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

inline void tilemap_t::scanline_draw_masked_ind16(UINT16 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, UINT8 *pri, UINT32 pcode)
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

inline void tilemap_t::scanline_draw_opaque_rgb32(UINT32 *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode)
{
	const pen_t *clut = &pens[pcode >> 16];

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

inline void tilemap_t::scanline_draw_masked_rgb32(UINT32 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode)
{
	const pen_t *clut = &pens[pcode >> 16];

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

inline void tilemap_t::scanline_draw_opaque_rgb32_alpha(UINT32 *dest, const UINT16 *source, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];

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

inline void tilemap_t::scanline_draw_masked_rgb32_alpha(UINT32 *dest, const UINT16 *source, const UINT8 *maskptr, int mask, int value, int count, const pen_t *pens, UINT8 *pri, UINT32 pcode, UINT8 alpha)
{
	const pen_t *clut = &pens[pcode >> 16];

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
//  tilemap_create_common - shared creation
//  function
//-------------------------------------------------

tilemap_t::tilemap_t(tilemap_manager &manager, tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, int tilewidth, int tileheight, int cols, int rows)
	: m_next(NULL),
		m_rows(rows),
		m_cols(cols),
		m_tilewidth(tilewidth),
		m_tileheight(tileheight),
		m_width(cols * tilewidth),
		m_height(rows * tileheight),
		m_mapper(mapper),
		m_memory_to_logical(NULL),
		m_max_logical_index(0),
		m_logical_to_memory(NULL),
		m_max_memory_index(0),
		m_tile_get_info(tile_get_info),
		m_user_data(NULL),
		m_enable(true),
		m_attributes(0),
		m_all_tiles_dirty(true),
		m_all_tiles_clean(false),
		m_palette_offset(0),
		m_pen_data_offset(0),
		m_gfx_used(0),
		m_scrollrows(1),
		m_scrollcols(1),
		m_rowscroll(auto_alloc_array_clear(manager.machine(), INT32, m_height)),
		m_colscroll(auto_alloc_array_clear(manager.machine(), INT32, m_width)),
		m_dx(0),
		m_dx_flipped(0),
		m_dy(0),
		m_dy_flipped(0),
		m_pixmap(m_width, m_height),
		m_flagsmap(m_width, m_height),
		m_tileflags(NULL),
		m_manager(manager)
{
	// reset internal arrays
	memset(m_gfx_dirtyseq, 0, sizeof(m_gfx_dirtyseq));
	memset(m_pen_to_flags, 0, sizeof(m_pen_to_flags));

	// create the initial mappings
	mappings_create();

	// set up the default pen mask
	memset(&m_tileinfo, 0, sizeof(m_tileinfo));
	m_tileinfo.pen_mask = 0xff;
	m_tileinfo.gfxnum = 0xff;

	// allocate transparency mapping data
	m_tileflags = auto_alloc_array(machine(), UINT8, m_max_logical_index);
	for (int group = 0; group < TILEMAP_NUM_GROUPS; group++)
		map_pens_to_layer(group, 0, 0, TILEMAP_PIXEL_LAYER0);

	// save relevant state
	int instance = manager.alloc_instance();
	machine().save().save_item("tilemap", NULL, instance, NAME(m_enable));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_attributes));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_palette_offset));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_pen_data_offset));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_scrollrows));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_scrollcols));
	machine().save().save_pointer("tilemap", NULL, instance, NAME(m_rowscroll), rows * tileheight);
	machine().save().save_pointer("tilemap", NULL, instance, NAME(m_colscroll), cols * tilewidth);
	machine().save().save_item("tilemap", NULL, instance, NAME(m_dx));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_dx_flipped));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_dy));
	machine().save().save_item("tilemap", NULL, instance, NAME(m_dy_flipped));

	// reset everything after a load
	machine().save().register_postload(save_prepost_delegate(FUNC(tilemap_t::postload), this));
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
	if (memindex < m_max_memory_index)
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

void tilemap_t::map_pens_to_layer(int group, pen_t pen, pen_t mask, UINT8 layermask)
{
	assert(group < TILEMAP_NUM_GROUPS);
	assert((layermask & TILEMAP_PIXEL_CATEGORY_MASK) == 0);

	// we start at the index where (pen & mask) == pen, and all other bits are 0
	pen_t start = pen & mask;

	// we stop at the index where (pen & mask) == pen, and all other bits are 1
	pen_t stop = start | ~mask;

	// clamp to the number of entries actually there
	stop = MIN(stop, MAX_PEN_TO_FLAGS - 1);

	// iterate and set
	UINT8 *array = m_pen_to_flags + group * MAX_PEN_TO_FLAGS;
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

void tilemap_t::set_transmask(int group, UINT32 fgmask, UINT32 bgmask)
{
	// iterate over all 32 pens specified
	for (pen_t pen = 0; pen < 32; pen++)
	{
		UINT8 fgbits = ((fgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER0;
		UINT8 bgbits = ((bgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER1;
		map_pen_to_layer(group, pen, fgbits | bgbits);
	}
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

tilemap_memory_index tilemap_t::scan_rows(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
{
	return row * num_cols + col;
}

tilemap_memory_index tilemap_t::scan_rows_flip_x(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
{
	return row * num_cols + (num_cols - 1 - col);
}

tilemap_memory_index tilemap_t::scan_rows_flip_y(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
{
	return (num_rows - 1 - row) * num_cols + col;
}

tilemap_memory_index tilemap_t::scan_rows_flip_xy(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
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

tilemap_memory_index tilemap_t::scan_cols(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
{
	return col * num_rows + row;
}

tilemap_memory_index tilemap_t::scan_cols_flip_x(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
{
	return (num_cols - 1 - col) * num_rows + row;
}

tilemap_memory_index tilemap_t::scan_cols_flip_y(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
{
	return col * num_rows + (num_rows - 1 - row);
}

tilemap_memory_index tilemap_t::scan_cols_flip_xy(running_machine &machine, UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows)
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
	m_max_logical_index = m_rows * m_cols;

	// compute the maximum memory index
	m_max_memory_index = 0;
	for (UINT32 row = 0; row < m_rows; row++)
		for (UINT32 col = 0; col < m_cols; col++)
		{
			tilemap_memory_index memindex = memory_index(col, row);
			m_max_memory_index = MAX(m_max_memory_index, memindex);
		}
	m_max_memory_index++;

	// allocate the necessary mappings
	m_memory_to_logical = auto_alloc_array(machine(), logical_index, m_max_memory_index);
	m_logical_to_memory = auto_alloc_array(machine(), tilemap_memory_index, m_max_logical_index);

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
	memset(m_memory_to_logical, 0xff, m_max_memory_index * sizeof(m_memory_to_logical[0]));

	// now iterate over all logical indexes and populate the memory index
	for (logical_index logindex = 0; logindex < m_max_logical_index; logindex++)
	{
		UINT32 logical_col = logindex % m_cols;
		UINT32 logical_row = logindex / m_cols;
		tilemap_memory_index memindex = memory_index(logical_col, logical_row);

		// apply tilemap flip to get the final location to store
		if (m_attributes & TILEMAP_FLIPX)
			logical_col = (m_cols - 1) - logical_col;
		if (m_attributes & TILEMAP_FLIPY)
			logical_row = (m_rows - 1) - logical_row;
		UINT32 flipped_logindex = logical_row * m_cols + logical_col;

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
		memset(m_tileflags, TILE_FLAG_DIRTY, m_max_logical_index);
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

g_profiler.start(PROFILER_TILEMAP_DRAW);

	// flush the dirty state to all tiles as appropriate
	realize_all_dirty_tiles();

	// iterate over rows and columns
	logical_index logindex = 0;
	for (int row = 0; row < m_rows; row++)
		for (int col = 0; col < m_cols; col++, logindex++)
			if (m_tileflags[logindex] == TILE_FLAG_DIRTY)
				tile_update(logindex, col, row);

	// mark it all clean
	m_all_tiles_clean = true;

g_profiler.stop();
}


//-------------------------------------------------
//  tile_update - update a single dirty tile
//-------------------------------------------------

void tilemap_t::tile_update(logical_index logindex, UINT32 col, UINT32 row)
{
g_profiler.start(PROFILER_TILEMAP_UPDATE);

	// call the get info callback for the associated memory index
	tilemap_memory_index memindex = m_logical_to_memory[logindex];
	m_tile_get_info(m_tileinfo, memindex, m_user_data);

	// apply the global tilemap flip to the returned flip flags
	UINT32 flags = m_tileinfo.flags ^ (m_attributes & 0x03);

	// draw the tile, using either direct or transparent
	UINT32 x0 = m_tilewidth * col;
	UINT32 y0 = m_tileheight * row;
	m_tileflags[logindex] = tile_draw(m_tileinfo.pen_data + m_pen_data_offset, x0, y0,
		m_tileinfo.palette_base, m_tileinfo.category, m_tileinfo.group, flags, m_tileinfo.pen_mask);

	// if mask data is specified, apply it
	if ((flags & (TILE_FORCE_LAYER0 | TILE_FORCE_LAYER1 | TILE_FORCE_LAYER2)) == 0 && m_tileinfo.mask_data != NULL)
		m_tileflags[logindex] = tile_apply_bitmask(m_tileinfo.mask_data, x0, y0, m_tileinfo.category, flags);

	// track which gfx have been used for this tilemap
	if (m_tileinfo.gfxnum != 0xff && (m_gfx_used & (1 << m_tileinfo.gfxnum)) == 0)
	{
		m_gfx_used |= 1 << m_tileinfo.gfxnum;
		m_gfx_dirtyseq[m_tileinfo.gfxnum] = machine().gfx[m_tileinfo.gfxnum]->dirtyseq();
	}

g_profiler.stop();
}


//-------------------------------------------------
//  tile_draw - draw a single tile to the
//  tilemap's internal pixmap, using the pen as
//  the pen_to_flags lookup value, and adding
//  the palette_base
//-------------------------------------------------

UINT8 tilemap_t::tile_draw(const UINT8 *pendata, UINT32 x0, UINT32 y0, UINT32 palette_base, UINT8 category, UINT8 group, UINT8 flags, UINT8 pen_mask)
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
	const UINT8 *penmap = m_pen_to_flags + group * MAX_PEN_TO_FLAGS;
	UINT8 andmask = ~0, ormask = 0;
	for (int ty = 0; ty < m_tileheight; ty++)
	{
		UINT16 *pixptr = &m_pixmap.pix16(y0, x0);
		UINT8 *flagsptr = &m_flagsmap.pix8(y0, x0);

		// pre-advance to the next row
		y0 += dy0;

		// 8bpp data
		for (int tx = 0, xoffs = 0; tx < m_tilewidth; tx++, xoffs += dx0)
		{
			UINT8 pen = (*pendata++) & pen_mask;
			UINT8 map = penmap[pen];
			pixptr[xoffs] = palette_base + pen;
			flagsptr[xoffs] = map | category;
			andmask &= map;
			ormask |= map;
		}
	}
	return andmask ^ ormask;
}


//-------------------------------------------------
//  tile_apply_bitmask - apply a bitmask to an
//  already-rendered tile by modifying the
//  flagsmap appropriately
//-------------------------------------------------

UINT8 tilemap_t::tile_apply_bitmask(const UINT8 *maskdata, UINT32 x0, UINT32 y0, UINT8 category, UINT8 flags)
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
	UINT8 andmask = ~0, ormask = 0;
	int bitoffs = 0;
	for (int ty = 0; ty < m_tileheight; ty++)
	{
		// pre-advance to the next row
		UINT8 *flagsptr = &m_flagsmap.pix8(y0, x0);
		y0 += dy0;

		// anywhere the bitmask is 0 should be transparent
		for (int tx = 0, xoffs = 0; tx < m_tilewidth; tx++, xoffs += dx0)
		{
			UINT8 map = flagsptr[xoffs];

			if ((maskdata[bitoffs / 8] & (0x80 >> (bitoffs & 7))) == 0)
				map = flagsptr[xoffs] = TILEMAP_PIXEL_TRANSPARENT | category;
			andmask &= map;
			ormask |= map;
			bitoffs++;
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

void tilemap_t::configure_blit_parameters(blit_parameters &blit, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{
	// set the target bitmap
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
void tilemap_t::draw_common(_BitmapClass &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{
	// skip if disabled
	if (!m_enable)
		return;

g_profiler.start(PROFILER_TILEMAP_DRAW);
	// configure the blit parameters based on the input parameters
	blit_parameters blit;
	configure_blit_parameters(blit, cliprect, flags, priority, priority_mask);

	// flush the dirty state to all tiles as appropriate
	realize_all_dirty_tiles();

	UINT32 width  = machine().primary_screen->width();
	UINT32 height = machine().primary_screen->height();

	// XY scrolling playfield
	if (m_scrollrows == 1 && m_scrollcols == 1)
	{
		// iterate to handle wraparound
		int scrollx = effective_rowscroll(0, width);
		int scrolly = effective_colscroll(0, height);
		for (int ypos = scrolly - m_height; ypos <= blit.cliprect.max_y; ypos += m_height)
			for (int xpos = scrollx - m_width; xpos <= blit.cliprect.max_x; xpos += m_width)
				draw_instance(dest, blit, xpos, ypos);
	}

	// scrolling rows + vertical scroll
	else if (m_scrollcols == 1)
	{
		const rectangle original_cliprect = blit.cliprect;

		// iterate over Y to handle wraparound
		int rowheight = m_height / m_scrollrows;
		int scrolly = effective_colscroll(0, height);
		for (int ypos = scrolly - m_height; ypos <= original_cliprect.max_y; ypos += m_height)
		{
			int const firstrow = MAX((original_cliprect.min_y - ypos) / rowheight, 0);
			int const lastrow =  MIN((original_cliprect.max_y - ypos) / rowheight, m_scrollrows - 1);

			// iterate over rows in the tilemap
			int nextrow;
			for (int currow = firstrow; currow <= lastrow; currow = nextrow)
			{
				// scan forward until we find a non-matching row
				int scrollx = effective_rowscroll(currow, width);
				for (nextrow = currow + 1; nextrow <= lastrow; nextrow++)
					if (effective_rowscroll(nextrow, width) != scrollx)
						break;

				// skip if disabled
				if (scrollx == TILE_LINE_DISABLED)
					continue;

				// update the cliprect just for this set of rows
				blit.cliprect.min_y = currow * rowheight + ypos;
				blit.cliprect.max_y = nextrow * rowheight - 1 + ypos;
				blit.cliprect &= original_cliprect;

				// iterate over X to handle wraparound
				for (int xpos = scrollx - m_width; xpos <= original_cliprect.max_x; xpos += m_width)
					draw_instance(dest, blit, xpos, ypos);
			}
		}
	}

	// scrolling columns + horizontal scroll
	else if (m_scrollrows == 1)
	{
		const rectangle original_cliprect = blit.cliprect;

		// iterate over columns in the tilemap
		int scrollx = effective_rowscroll(0, width);
		int colwidth = m_width / m_scrollcols;
		int nextcol;
		for (int curcol = 0; curcol < m_scrollcols; curcol = nextcol)
		{
			// scan forward until we find a non-matching column
			int scrolly = effective_colscroll(curcol, height);
			for (nextcol = curcol + 1; nextcol < m_scrollcols; nextcol++)
				if (effective_colscroll(nextcol, height) != scrolly)
					break;

			// skip if disabled
			if (scrolly == TILE_LINE_DISABLED)
				continue;

			// iterate over X to handle wraparound
			for (int xpos = scrollx - m_width; xpos <= original_cliprect.max_x; xpos += m_width)
			{
				// update the cliprect just for this set of columns
				blit.cliprect.min_x = curcol * colwidth + xpos;
				blit.cliprect.max_x = nextcol * colwidth - 1 + xpos;
				blit.cliprect &= original_cliprect;

				// iterate over Y to handle wraparound
				for (int ypos = scrolly - m_height; ypos <= original_cliprect.max_y; ypos += m_height)
					draw_instance(dest, blit, xpos, ypos);
			}
		}
	}
g_profiler.stop();
}

void tilemap_t::draw(bitmap_ind16 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{ draw_common(dest, cliprect, flags, priority, priority_mask); }

void tilemap_t::draw(bitmap_rgb32 &dest, const rectangle &cliprect, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{ draw_common(dest, cliprect, flags, priority, priority_mask); }


//-------------------------------------------------
//  draw_roz - draw a tilemap to the destination
//  with clipping and arbitrary rotate/zoom; pixels
//  apply priority/priority_mask to the priority
//  bitmap
//-------------------------------------------------

template<class _BitmapClass>
void tilemap_t::draw_roz_common(_BitmapClass &dest, const rectangle &cliprect,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{
// notes:
// - startx and starty MUST be UINT32 for calculations to work correctly
// - srcbim_width and height are assumed to be a power of 2 to speed up wraparound

	// skip if disabled
	if (!m_enable)
		return;

	// see if this is just a regular render and if so, do a regular render
	if (incxx == (1 << 16) && incxy == 0 && incyx == 0 && incyy == (1 << 16) && wraparound)
	{
		set_scrollx(0, startx >> 16);
		set_scrolly(0, starty >> 16);
		draw(dest, cliprect, flags, priority, priority_mask);
		return;
	}

g_profiler.start(PROFILER_TILEMAP_DRAW_ROZ);
	// configure the blit parameters
	blit_parameters blit;
	configure_blit_parameters(blit, cliprect, flags, priority, priority_mask);

	// get the full pixmap for the tilemap
	pixmap();

	// then do the roz copy
	draw_roz_core(dest, blit, startx, starty, incxx, incxy, incyx, incyy, wraparound);
g_profiler.stop();
}

void tilemap_t::draw_roz(bitmap_ind16 &dest, const rectangle &cliprect,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{ draw_roz_common(dest, cliprect, startx, starty, incxx, incxy, incyx, incyy, wraparound, flags, priority, priority_mask); }

void tilemap_t::draw_roz(bitmap_rgb32 &dest, const rectangle &cliprect,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		bool wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask)
{ draw_roz_common(dest, cliprect, startx, starty, incxx, incxy, incyx, incyy, wraparound, flags, priority, priority_mask); }


//-------------------------------------------------
//  draw_instance - draw a single instance of the
//  tilemap to the internal pixmap at the given
//  xpos,ypos
//-------------------------------------------------

template<class _BitmapClass>
void tilemap_t::draw_instance(_BitmapClass &dest, const blit_parameters &blit, int xpos, int ypos)
{
	// clip destination coordinates to the tilemap
	// note that x2/y2 are exclusive, not inclusive
	int x1 = MAX(xpos, blit.cliprect.min_x);
	int x2 = MIN(xpos + (int)m_width, blit.cliprect.max_x + 1);
	int y1 = MAX(ypos, blit.cliprect.min_y);
	int y2 = MIN(ypos + (int)m_height, blit.cliprect.max_y + 1);

	// if totally clipped, stop here
	if (x1 >= x2 || y1 >= y2)
		return;

	// look up priority and destination base addresses for y1
	bitmap_ind8 &priority_bitmap = machine().priority_bitmap;
	UINT8 *priority_baseaddr = &priority_bitmap.pix8(y1, xpos);
	typename _BitmapClass::pixel_t *dest_baseaddr = NULL;
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
	const UINT16 *source_baseaddr = &m_pixmap.pix16(y1);
	const UINT8 *mask_baseaddr = &m_flagsmap.pix8(y1);

	// get start/stop columns, rounding outward
	int mincol = x1 / m_tilewidth;
	int maxcol = (x2 + m_tilewidth - 1) / m_tilewidth;

	// set up row counter
	int y = y1;
	int nexty = m_tileheight * (y1 / m_tileheight) + m_tileheight;
	nexty = MIN(nexty, y2);

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
			x_end = MAX(x_end, x1);
			x_end = MIN(x_end, x2);

			// if we're rendering something, compute the pointers
			const rgb_t *clut = (dest.palette() != NULL) ? palette_entry_list_raw(dest.palette()) : machine().pens;
			if (prev_trans != WHOLLY_TRANSPARENT)
			{
				const UINT16 *source0 = source_baseaddr + x_start;
				typename _BitmapClass::pixel_t *dest0 = dest_baseaddr + x_start;
				UINT8 *pmap0 = priority_baseaddr + x_start;

				// if we were opaque, use the opaque renderer
				if (prev_trans == WHOLLY_OPAQUE)
				{
					for (int cury = y; cury < nexty; cury++)
					{
						if (dest_baseaddr == NULL)
							scanline_draw_opaque_null(x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 2)
							scanline_draw_opaque_ind16(reinterpret_cast<UINT16 *>(dest0), source0, x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4 && blit.alpha >= 0xff)
							scanline_draw_opaque_rgb32(reinterpret_cast<UINT32 *>(dest0), source0, x_end - x_start, clut, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4)
							scanline_draw_opaque_rgb32_alpha(reinterpret_cast<UINT32 *>(dest0), source0, x_end - x_start, clut, pmap0, blit.tilemap_priority_code, blit.alpha);

						dest0 += dest_rowpixels;
						source0 += m_pixmap.rowpixels();
						pmap0 += priority_bitmap.rowpixels();
					}
				}

				// otherwise use the masked renderer
				else
				{
					const UINT8 *mask0 = mask_baseaddr + x_start;
					for (int cury = y; cury < nexty; cury++)
					{
						if (dest_baseaddr == NULL)
							scanline_draw_masked_null(mask0, blit.mask, blit.value, x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 2)
							scanline_draw_masked_ind16(reinterpret_cast<UINT16 *>(dest0), source0, mask0, blit.mask, blit.value, x_end - x_start, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4 && blit.alpha >= 0xff)
							scanline_draw_masked_rgb32(reinterpret_cast<UINT32 *>(dest0), source0, mask0, blit.mask, blit.value, x_end - x_start, clut, pmap0, blit.tilemap_priority_code);
						else if (sizeof(*dest0) == 4)
							scanline_draw_masked_rgb32_alpha(reinterpret_cast<UINT32 *>(dest0), source0, mask0, blit.mask, blit.value, x_end - x_start, clut, pmap0, blit.tilemap_priority_code, blit.alpha);

						dest0 += dest_rowpixels;
						source0 += m_pixmap.rowpixels();
						mask0 += m_flagsmap.rowpixels();
						pmap0 += priority_bitmap.rowpixels();
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
		priority_baseaddr += priority_bitmap.rowpixels() * (nexty - y);
		source_baseaddr += m_pixmap.rowpixels() * (nexty - y);
		mask_baseaddr += m_flagsmap.rowpixels() * (nexty - y);
		dest_baseaddr += dest_rowpixels * (nexty - y);

		// increment the Y counter
		y = nexty;
		nexty += m_tileheight;
		nexty = MIN(nexty, y2);
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
void tilemap_t::draw_roz_core(_BitmapClass &destbitmap, const blit_parameters &blit,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound)
{
	// pre-cache all the inner loop values
	const rgb_t *clut = ((destbitmap.palette() != NULL) ? palette_entry_list_raw(destbitmap.palette()) : machine().pens) + (blit.tilemap_priority_code >> 16);
	bitmap_ind8 &priority_bitmap = machine().priority_bitmap;
	const int xmask = m_pixmap.width() - 1;
	const int ymask = m_pixmap.height() - 1;
	const int widthshifted = m_pixmap.width() << 16;
	const int heightshifted = m_pixmap.height() << 16;
	UINT32 priority = blit.tilemap_priority_code;
	UINT8 mask = blit.mask;
	UINT8 value = blit.value;
	UINT8 alpha = blit.alpha;

	// pre-advance based on the cliprect
	startx += blit.cliprect.min_x * incxx + blit.cliprect.min_y * incyx;
	starty += blit.cliprect.min_x * incxy + blit.cliprect.min_y * incyy;

	// extract start/end points
	int sx = blit.cliprect.min_x;
	int sy = blit.cliprect.min_y;
	int ex = blit.cliprect.max_x;
	int ey = blit.cliprect.max_y;

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
				UINT32 cx = startx;
				UINT32 cy = starty >> 16;

				// get source and priority pointers
				UINT8 *pri = &priority_bitmap.pix8(sy, sx);
				const UINT16 *src = &m_pixmap.pix16(cy);
				const UINT8 *maskptr = &m_flagsmap.pix8(cy);
				typename _BitmapClass::pixel_t *dest = &destbitmap.pix(sy, sx);

				// loop over columns
				while (x <= ex && cx < widthshifted)
				{
					// plot if we match the mask
					if ((maskptr[cx >> 16] & mask) == value)
					{
						ROZ_PLOT_PIXEL(src[cx >> 16]);
						*pri = (*pri & (priority >> 8)) | priority;
					}

					// advance in X
					cx += incxx;
					x++;
					dest++;
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
			UINT32 cx = startx;
			UINT32 cy = starty;

			// get dest and priority pointers
			typename _BitmapClass::pixel_t *dest = &destbitmap.pix(sy, sx);
			UINT8 *pri = &priority_bitmap.pix8(sy, sx);

			// loop over columns
			while (x <= ex)
			{
				// plot if we match the mask
				if ((m_flagsmap.pix8((cy >> 16) & ymask, (cx >> 16) & xmask) & mask) == value)
				{
					ROZ_PLOT_PIXEL(m_pixmap.pix16((cy >> 16) & ymask, (cx >> 16) & xmask));
					*pri = (*pri & (priority >> 8)) | priority;
				}

				// advance in X
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
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
			UINT32 cx = startx;
			UINT32 cy = starty;

			// get dest and priority pointers
			typename _BitmapClass::pixel_t *dest = &destbitmap.pix(sy, sx);
			UINT8 *pri = &priority_bitmap.pix8(sy, sx);

			// loop over columns
			while (x <= ex)
			{
				// plot if we're within the bitmap and we match the mask
				if (cx < widthshifted && cy < heightshifted)
					if ((m_flagsmap.pix8(cy >> 16, cx >> 16) & mask) == value)
					{
						ROZ_PLOT_PIXEL(m_pixmap.pix16(cy >> 16, cx >> 16));
						*pri = (*pri & (priority >> 8)) | priority;
					}

				// advance in X
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
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

void tilemap_t::draw_debug(bitmap_rgb32 &dest, UINT32 scrollx, UINT32 scrolly)
{
	// set up for the blit, using hard-coded parameters (no priority, etc)
	blit_parameters blit;
	configure_blit_parameters(blit, dest.cliprect(), TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0, 0xff);

	// compute the effective scroll positions
	scrollx = m_width  - scrollx % m_width;
	scrolly = m_height - scrolly % m_height;

	// flush the dirty state to all tiles as appropriate
	realize_all_dirty_tiles();

	// iterate to handle wraparound
	for (int ypos = scrolly - m_height; ypos <= blit.cliprect.max_y; ypos += m_height)
		for (int xpos = scrollx - m_width; xpos <= blit.cliprect.max_x; xpos += m_width)
			draw_instance(dest, blit, xpos, ypos);
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
	if (machine.primary_screen == NULL || machine.primary_screen->width() == 0)
		return;
	machine.primary_screen->register_screen_bitmap(machine.priority_bitmap);
}


//-------------------------------------------------
//  set_flip_all - set a global flip for all the
//  tilemaps
//-------------------------------------------------

static const struct
{
	tilemap_memory_index (*func)(running_machine &, UINT32, UINT32, UINT32, UINT32);
	const char *name;
} s_standard_mappers[TILEMAP_STANDARD_COUNT] =
{
	{ FUNC(tilemap_t::scan_rows) },
	{ FUNC(tilemap_t::scan_rows_flip_x) },
	{ FUNC(tilemap_t::scan_rows_flip_y) },
	{ FUNC(tilemap_t::scan_rows_flip_xy) },
	{ FUNC(tilemap_t::scan_cols) },
	{ FUNC(tilemap_t::scan_cols_flip_x) },
	{ FUNC(tilemap_t::scan_cols_flip_y) },
	{ FUNC(tilemap_t::scan_cols_flip_xy) }
};

tilemap_t &tilemap_manager::create(tilemap_get_info_delegate tile_get_info, tilemap_mapper_delegate mapper, int tilewidth, int tileheight, int cols, int rows)
{
	return m_tilemap_list.append(*auto_alloc(machine(), tilemap_t(*this, tile_get_info, mapper, tilewidth, tileheight, cols, rows)));
}

tilemap_t &tilemap_manager::create(tilemap_get_info_delegate tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows)
{
	return m_tilemap_list.append(*auto_alloc(machine(), tilemap_t(*this, tile_get_info, tilemap_mapper_delegate(s_standard_mappers[mapper].func, s_standard_mappers[mapper].name, &machine()), tilewidth, tileheight, cols, rows)));
}

tilemap_t &tilemap_manager::create(tile_get_info_func tile_get_info, tilemap_mapper_func mapper, int tilewidth, int tileheight, int cols, int rows)
{
	return m_tilemap_list.append(*auto_alloc(machine(), tilemap_t(*this, tilemap_get_info_delegate(tile_get_info, "", &machine()), tilemap_mapper_delegate(mapper, "", &machine()), tilewidth, tileheight, cols, rows)));
}

tilemap_t &tilemap_manager::create(tile_get_info_func tile_get_info, tilemap_standard_mapper mapper, int tilewidth, int tileheight, int cols, int rows)
{
	return m_tilemap_list.append(*auto_alloc(machine(), tilemap_t(*this, tilemap_get_info_delegate(tile_get_info, "", &machine()), tilemap_mapper_delegate(s_standard_mappers[mapper].func, s_standard_mappers[mapper].name, &machine()), tilewidth, tileheight, cols, rows)));
}


//-------------------------------------------------
//  set_flip_all - set a global flip for all the
//  tilemaps
//-------------------------------------------------

void tilemap_manager::set_flip_all(UINT32 attributes)
{
	for (tilemap_t *tmap = m_tilemap_list.first(); tmap != NULL; tmap = tmap->next())
		tmap->set_flip(attributes);
}


//-------------------------------------------------
//  mark_all_dirty - mark all the tiles in all the
//  tilemaps dirty
//-------------------------------------------------

void tilemap_manager::mark_all_dirty()
{
	for (tilemap_t *tmap = m_tilemap_list.first(); tmap != NULL; tmap = tmap->next())
		tmap->mark_all_dirty();
}
