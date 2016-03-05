// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#pragma once

#ifndef __RECTPACKER_H__
#define __RECTPACKER_H__

#include "emu.h"

#include <vector>

class rectangle_packer
{
public:
	// The input and output are in terms of vectors of ints to avoid
	// dependencies (although I suppose a public member struct could have been
	// used). The parameters are:

	// packs : After packing, the outer array contains the packs (therefore
	// the number of packs is packs.size()). Each inner array contains a
	// sequence of sets of 3 ints. Each set represents a rectangle in the
	// pack. The elements in the set are 1) the rect ID, 2) the x position
	// of the rect with respect to the pack, and 3) the y position of the rect
	// with respect to the pack. The widths and heights of the rects are not
	// included, as it's assumed they are stored on the caller's side (they
	// were after all the input to the function).

	class packable_rectangle
	{
	public:
		packable_rectangle() : m_hash(0), m_width(-1), m_height(-1) { }
		packable_rectangle(UINT32 hash, UINT32 format, int width, int height, int rowpixels, const rgb_t *palette, void *base)
			: m_hash(hash)
			, m_format(format)
			, m_width(width)
			, m_height(height)
			, m_rowpixels(rowpixels)
			, m_palette(palette)
			, m_base(base)
		{
		}

		UINT32 hash() const { return m_hash; }
		UINT32 format() const { return m_format; }
		int width() const { return m_width; }
		int height() const { return m_height; }
		int rowpixels() const { return m_rowpixels; }
		const rgb_t* palette() const { return m_palette; }
		void* base() const { return m_base; }

	private:
		UINT32 m_hash;
		UINT32 m_format;
		int m_width;
		int m_height;
		int m_rowpixels;
		const rgb_t* m_palette;
		void* m_base;
	};

	class packed_rectangle
	{
	public:
		packed_rectangle() : m_hash(0), m_format(0), m_width(-1), m_height(-1), m_x(-1), m_y(-1), m_rowpixels(0), m_palette(nullptr), m_base(nullptr) { }
		packed_rectangle(const packed_rectangle& rect)
			: m_hash(rect.m_hash)
			, m_format(rect.m_format)
			, m_width(rect.m_width)
			, m_height(rect.m_height)
			, m_x(rect.m_x)
			, m_y(rect.m_y)
			, m_rowpixels(rect.m_rowpixels)
			, m_palette(rect.m_palette)
			, m_base(rect.m_base)
		{
		}
		packed_rectangle(UINT32 hash, UINT32 format, int width, int height, int x, int y, int rowpixels, const rgb_t *palette, void *base)
			: m_hash(hash)
			, m_format(format)
			, m_width(width)
			, m_height(height)
			, m_x(x)
			, m_y(y)
			, m_rowpixels(rowpixels)
			, m_palette(palette)
			, m_base(base)
		{
		}

		UINT32 hash() const { return m_hash; }
		UINT32 format() const { return m_format; }
		int width() const { return m_width; }
		int height() const { return m_height; }
		int x() const { return m_x; }
		int y() const { return m_y; }
		int rowpixels() const { return m_rowpixels; }
		const rgb_t* palette() const { return m_palette; }
		void* base() const { return m_base; }

	private:
		UINT32 m_hash;
		UINT32 m_format;
		int m_width;
		int m_height;
		int m_x;
		int m_y;
		int m_rowpixels;
		const rgb_t* m_palette;
		void* m_base;
	};

	bool pack(const std::vector<packable_rectangle>& rects, std::vector<std::vector<packed_rectangle>>& packs, int pack_size);

private:
	struct rectangle
	{
		rectangle(int size)
			: x(0)
			, y(0)
			, w(size)
			, h(size)
			, hash(-1)
			, format(0)
			, rowpixels(0)
			, palette(nullptr)
			, base(nullptr)
			, packed(false)
		{
			children[0] = -1;
			children[1] = -1;
		}

		rectangle(int x, int y, int w, int h, int hash, UINT32 format, int rowpixels, const rgb_t *palette, void *base)
			: x(x)
			, y(y)
			, w(w)
			, h(h)
			, hash(hash)
			, format(format)
			, rowpixels(rowpixels)
			, palette(palette)
			, base(base)
			, packed(false)
		{
			children[0] = -1;
			children[1] = -1;
		}

		int get_area() const
		{
			return w * h;
		}

		bool operator<(const rectangle& rect) const
		{
			return get_area() < rect.get_area();
		}

		int             x;
		int             y;
		int             w;
		int             h;
		int             hash;
		UINT32          format;
		int             rowpixels;
		const rgb_t*    palette;
		void*           base;
		int             children[2];
		bool            packed;
	};

	void clear();
	bool fill(int pack);
	void split(int pack, int rect);
	bool fits(rectangle& rect1, const rectangle& rect2);
	void add_pack_to_array(int pack, std::vector<packed_rectangle>& array) const;

	int                     m_pack_size;
	int                     m_num_packed;
	std::vector<rectangle>  m_rects;
	std::vector<rectangle>  m_packs;
	std::vector<int>        m_roots;
};

#endif // __RECTPACKER_H__
