// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    General sprite handling helpers

***************************************************************************/

#include "emu.h"
#include "sprite.h"


//-------------------------------------------------
//  sparse_dirty_bitmap -- constructor
//-------------------------------------------------

sparse_dirty_bitmap::sparse_dirty_bitmap(int granularity) :
	m_width(0),
	m_height(0),
	m_granularity(granularity),
	m_rect_list_bounds(0, -1, 0, -1)
{
}

sparse_dirty_bitmap::sparse_dirty_bitmap(int width, int height, int granularity) :
	sparse_dirty_bitmap(granularity)
{
	// resize to the specified width/height
	resize(width, height);
}


//-------------------------------------------------
//  dirty -- dirty a region
//-------------------------------------------------

void sparse_dirty_bitmap::dirty(int32_t left, int32_t right, int32_t top, int32_t bottom)
{
	// compute a rectangle in dirty space, and fill it with 1
	m_bitmap.fill(
			1,
			rectangle(
				left >> m_granularity,
				right >> m_granularity,
				top >> m_granularity,
				bottom >> m_granularity));

	// invalidate existing rect list
	invalidate_rect_list();
}


//-------------------------------------------------
//  clean a region -- dirty a region
//-------------------------------------------------

void sparse_dirty_bitmap::clean(int32_t left, int32_t right, int32_t top, int32_t bottom)
{
	// if right or bottom intersect the edge of the bitmap, round up
	int round = (1 << m_granularity) - 1;
	if (right >= m_width - 1)
		right = m_width + round;
	if (bottom >= m_height - 1)
		bottom = m_height + round;

	// compute a rectangle in dirty space, and fill it with 0
	rectangle rect((left + round) >> m_granularity, (right - round) >> m_granularity, (top + round) >> m_granularity, (bottom - round) >> m_granularity);
	m_bitmap.fill(0, rect);

	// invalidate existing rect list
	invalidate_rect_list();
}


//-------------------------------------------------
//  resize -- resize the bitmap
//-------------------------------------------------

void sparse_dirty_bitmap::resize(int width, int height)
{
	// set new size
	m_width = width;
	m_height = height;

	// resize the bitmap
	int round = (1 << m_granularity) - 1;
	m_bitmap.resize((width + round) >> m_granularity, (height + round) >> m_granularity);

	// reset everything
	dirty_all();
}


//-------------------------------------------------
//  first_dirty_rect -- return the first dirty
//  rectangle in the list
//-------------------------------------------------

void sparse_dirty_bitmap::calculate_rect_list(const rectangle &cliprect)
{
	// if what we have is valid, just return it again
	if (m_rect_list_bounds == cliprect)
		return;

	// reclaim the dirty list and start over
	m_rect_list.clear();

	// compute dirty space rectangle coordinates
	int const sx = cliprect.min_x >> m_granularity;
	int const ex = cliprect.max_x >> m_granularity;
	int const sy = cliprect.min_y >> m_granularity;
	int const ey = cliprect.max_y >> m_granularity;
	int const tilesize = 1 << m_granularity;

	// loop over all grid rows that intersect our cliprect
	for (int y = sy; y <= ey; y++)
	{
		uint8_t const *const dirtybase = &m_bitmap.pix(y);
		rectangle *currect = nullptr;

		// loop over all grid columns that intersect our cliprect
		for (int x = sx; x <= ex; x++)
		{
			// if this tile is not dirty, end our current run and continue
			if (!dirtybase[x])
			{
				if (currect)
					*currect &= cliprect;
				currect = nullptr;
				continue;
			}

			// if we can't add to an existing rect, create a new one
			if (!currect)
			{
				// make a rect describing this grid square
				int32_t const min_x = x << m_granularity;
				int32_t const min_y = y << m_granularity;
				currect = &m_rect_list.emplace_back(
						min_x, min_x + tilesize - 1,
						min_y, min_y + tilesize - 1);
			}
			else
			{
				// if we can add to the previous rect, just expand its width
				currect->max_x += tilesize;
			}
		}

		// clip the last rect to the cliprect
		if (currect)
			*currect &= cliprect;
	}

	// mark the list as valid
	m_rect_list_bounds = cliprect;
}
