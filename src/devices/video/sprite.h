// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    General sprite handling helpers

***************************************************************************/

#ifndef MAME_VIDEO_SPRITE_H
#define MAME_VIDEO_SPRITE_H

#pragma once


// ======================> sparse_dirty_rect

// class representing a single dirty region
class sparse_dirty_rect : public rectangle
{
	friend class simple_list<sparse_dirty_rect>;

public:
	sparse_dirty_rect(): m_next(nullptr) { }
	// getters
	const sparse_dirty_rect *next() const { return m_next; }

private:
	// internal state
	sparse_dirty_rect * m_next;
};


// ======================> sparse_dirty_bitmap

class sparse_dirty_bitmap
{
public:
	// construction/destruction
	sparse_dirty_bitmap(int granularity = 3);
	sparse_dirty_bitmap(int width, int height, int granularity = 3);

	// dirtying operations - partially intersecting tiles are dirtied
	void dirty(const rectangle &rect) { dirty(rect.left(), rect.right(), rect.top(), rect.bottom()); }
	void dirty(int32_t left, int32_t right, int32_t top, int32_t bottom);
	void dirty_all() { dirty(0, m_width - 1, 0, m_height - 1); }

	// cleaning operations - partially intersecting tiles are NOT cleaned
	void clean(const rectangle &rect) { clean(rect.left(), rect.right(), rect.top(), rect.bottom()); }
	void clean(int32_t left, int32_t right, int32_t top, int32_t bottom);
	void clean_all() { clean(0, m_width - 1, 0, m_height - 1); }

	// convert to rect list
	sparse_dirty_rect *first_dirty_rect() { rectangle fullrect(0, m_width - 1, 0, m_height - 1); return first_dirty_rect(fullrect); }
	sparse_dirty_rect *first_dirty_rect(const rectangle &cliprect);

	// dynamic resizing
	void resize(int width, int height);

private:
	// invalidate cached rect list
	void invalidate_rect_list() { m_rect_list_bounds.set(0, -1, 0, -1); }

	// internal state
	int                     m_width;
	int                     m_height;
	int                     m_granularity;
	bitmap_ind8             m_bitmap;
	rectangle               m_rect_list_bounds;
	fixed_allocator<sparse_dirty_rect>  m_rect_allocator;
	simple_list<sparse_dirty_rect> m_rect_list;
};


// ======================> sprite_device

template<typename _SpriteRAMType, class _BitmapType>
class sprite_device : public device_t
{
	// constants
	static const int BITMAP_SLOP = 16;

protected:
	// construction/destruction - only for subclasses
	sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int dirty_granularity = 3)
		: device_t(mconfig, type, tag, owner, 0)
		, m_xorigin(0)
		, m_yorigin(0)
		, m_spriteram(nullptr)
		, m_spriteram_bytes(0)
		, m_dirty(dirty_granularity)
	{
		force_clear();
	}

public:
	// getters
	int32_t xorigin() const { return m_xorigin; }
	int32_t yorigin() const { return m_yorigin; }
	_BitmapType &bitmap() { return m_bitmap; }
	sparse_dirty_rect *first_dirty_rect() { return m_dirty.first_dirty_rect(); }
	sparse_dirty_rect *first_dirty_rect(const rectangle &cliprect) { return m_dirty.first_dirty_rect(cliprect); }
	_SpriteRAMType *spriteram() const { return m_spriteram; }
	uint32_t spriteram_bytes() const { return m_spriteram_bytes; }
	uint32_t spriteram_elements() const { return m_spriteram_bytes / sizeof(_SpriteRAMType); }
	_SpriteRAMType *buffer() { return &m_buffer[0]; }

	// configuration
	void set_spriteram(_SpriteRAMType *base, uint32_t bytes) { m_spriteram = base; m_spriteram_bytes = bytes; m_buffer.resize(m_spriteram_bytes / sizeof(_SpriteRAMType)); }
	void set_origin(int32_t xorigin = 0, int32_t yorigin = 0) { m_xorigin = xorigin; m_yorigin = yorigin; }
	void set_xorigin(int32_t xorigin) { m_xorigin = xorigin; }
	void set_yorigin(int32_t yorigin) { m_yorigin = yorigin; }

	// buffering
	void copy_to_buffer() { memcpy(m_buffer, m_spriteram, m_spriteram_bytes); }

	// clearing
	void clear() { clear(m_bitmap.cliprect()); }
	void clear(const rectangle &cliprect)
	{
		for (const sparse_dirty_rect *rect = m_dirty.first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
			m_bitmap.fill(~0, *rect);
		m_dirty.clean(cliprect);
	}

	// force clear (don't use dirty rects)
	void force_clear()
	{
		m_bitmap.fill(~0);
		m_dirty.clean_all();
	}

	// drawing
	void draw_async(const rectangle &cliprect, bool clearit = true)
	{
		// if the cliprect exceeds our current bitmap dimensions, expand
		if (cliprect.right() >= m_bitmap.width() || cliprect.bottom() >= m_bitmap.height())
		{
			int new_width = std::max(cliprect.right() + 1, m_bitmap.width());
			int new_height = std::max(cliprect.bottom() + 1, m_bitmap.height());
			m_bitmap.resize(new_width, new_height, BITMAP_SLOP, BITMAP_SLOP);
			m_dirty.resize(new_width, new_height);
		}

		// clear out the region
		if (clearit)
			clear(cliprect);

		// wrap the bitmap, adjusting for x/y origins
		_BitmapType wrapped(&m_bitmap.pix(0) - m_xorigin - m_yorigin * m_bitmap.rowpixels(), m_xorigin + cliprect.right() + 1, m_yorigin + cliprect.bottom() + 1, m_bitmap.rowpixels());

		// compute adjusted cliprect in source space
		rectangle adjusted = cliprect;
		adjusted.offset(m_xorigin, m_yorigin);

		// render
		draw(wrapped, adjusted);
	}

protected:
	// device-level overrides
	virtual void device_start() override
	{
		// find spriteram
		memory_share *spriteram = owner()->memshare(tag());
		if (spriteram != nullptr)
		{
			set_spriteram(reinterpret_cast<_SpriteRAMType *>(spriteram->ptr()), spriteram->bytes());

			// save states
			save_item(NAME(m_buffer));
		}
	}

	// subclass overrides
	virtual void draw(_BitmapType &bitmap, const rectangle &cliprect) = 0;

	// subclass helpers
	void mark_dirty(const rectangle &rect) { mark_dirty(rect.left(), rect.right(), rect.top(), rect.bottom()); }
	void mark_dirty(int32_t left, int32_t right, int32_t top, int32_t bottom) { m_dirty.dirty(left - m_xorigin, right - m_xorigin, top - m_yorigin, bottom - m_yorigin); }

private:
	// configuration
	int32_t                           m_xorigin;              // X origin for drawing
	int32_t                           m_yorigin;              // Y origin for drawing

	// memory pointers and buffers
	_SpriteRAMType *                m_spriteram;            // pointer to spriteram pointer
	int32_t                           m_spriteram_bytes;      // size of sprite RAM in bytes
	std::vector<_SpriteRAMType>          m_buffer;               // buffered spriteram for those that use it

	// bitmaps
	_BitmapType                     m_bitmap;               // live bitmap
	sparse_dirty_bitmap             m_dirty;                // dirty bitmap
};

typedef sprite_device<uint8_t, bitmap_ind16> sprite8_device_ind16;
typedef sprite_device<uint16_t, bitmap_ind16> sprite16_device_ind16;
typedef sprite_device<uint32_t, bitmap_ind16> sprite32_device_ind16;

typedef sprite_device<uint8_t, bitmap_ind32> sprite8_device_ind32;
typedef sprite_device<uint16_t, bitmap_ind32> sprite16_device_ind32;
typedef sprite_device<uint32_t, bitmap_ind32> sprite32_device_ind32;


#endif // MAME_VIDEO_SPRITE_H
