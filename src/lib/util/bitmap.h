// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    bitmap.h

    Core bitmap routines.

***************************************************************************/

#ifndef MAME_UTIL_BITMAP_H
#define MAME_UTIL_BITMAP_H

#pragma once

#include "palette.h"

#include <algorithm>
#include <memory>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// bitmap_format describes the various bitmap formats we use
enum bitmap_format
{
	BITMAP_FORMAT_INVALID = 0,      // invalid forma
	BITMAP_FORMAT_IND8,             // 8bpp indexed
	BITMAP_FORMAT_IND16,            // 16bpp indexed
	BITMAP_FORMAT_IND32,            // 32bpp indexed
	BITMAP_FORMAT_IND64,            // 64bpp indexed
	BITMAP_FORMAT_RGB32,            // 32bpp 8-8-8 RGB
	BITMAP_FORMAT_ARGB32,           // 32bpp 8-8-8-8 ARGB
	BITMAP_FORMAT_YUY16             // 16bpp 8-8 Y/Cb, Y/Cr in sequence
};


// ======================> rectangle

// rectangles describe a bitmap portion
class rectangle
{
public:
	// construction/destruction
	constexpr rectangle() { }
	constexpr rectangle(int32_t minx, int32_t maxx, int32_t miny, int32_t maxy)
		: min_x(minx), max_x(maxx), min_y(miny), max_y(maxy)
	{ }

	// getters
	constexpr int32_t left() const { return min_x; }
	constexpr int32_t right() const { return max_x; }
	constexpr int32_t top() const { return min_y; }
	constexpr int32_t bottom() const { return max_y; }

	// compute intersection with another rect
	rectangle &operator&=(const rectangle &src)
	{
		if (src.min_x > min_x) min_x = src.min_x;
		if (src.max_x < max_x) max_x = src.max_x;
		if (src.min_y > min_y) min_y = src.min_y;
		if (src.max_y < max_y) max_y = src.max_y;
		return *this;
	}

	// compute union with another rect
	rectangle &operator|=(const rectangle &src)
	{
		if (src.min_x < min_x) min_x = src.min_x;
		if (src.max_x > max_x) max_x = src.max_x;
		if (src.min_y < min_y) min_y = src.min_y;
		if (src.max_y > max_y) max_y = src.max_y;
		return *this;
	}

	// comparisons
	constexpr bool operator==(const rectangle &rhs) const { return min_x == rhs.min_x && max_x == rhs.max_x && min_y == rhs.min_y && max_y == rhs.max_y; }
	constexpr bool operator!=(const rectangle &rhs) const { return min_x != rhs.min_x || max_x != rhs.max_x || min_y != rhs.min_y || max_y != rhs.max_y; }
	constexpr bool operator>(const rectangle &rhs) const { return min_x < rhs.min_x && min_y < rhs.min_y && max_x > rhs.max_x && max_y > rhs.max_y; }
	constexpr bool operator>=(const rectangle &rhs) const { return min_x <= rhs.min_x && min_y <= rhs.min_y && max_x >= rhs.max_x && max_y >= rhs.max_y; }
	constexpr bool operator<(const rectangle &rhs) const { return min_x >= rhs.min_x || min_y >= rhs.min_y || max_x <= rhs.max_x || max_y <= rhs.max_y; }
	constexpr bool operator<=(const rectangle &rhs) const { return min_x > rhs.min_x || min_y > rhs.min_y || max_x < rhs.max_x || max_y < rhs.max_y; }

	// other helpers
	constexpr bool empty() const { return (min_x > max_x) || (min_y > max_y); }
	constexpr bool contains(int32_t x, int32_t y) const { return (x >= min_x) && (x <= max_x) && (y >= min_y) && (y <= max_y); }
	constexpr bool contains(const rectangle &rect) const { return (min_x <= rect.min_x) && (max_x >= rect.max_x) && (min_y <= rect.min_y) && (max_y >= rect.max_y); }
	constexpr int32_t width() const { return max_x + 1 - min_x; }
	constexpr int32_t height() const { return max_y + 1 - min_y; }
	constexpr int32_t xcenter() const { return (min_x + max_x + 1) / 2; }
	constexpr int32_t ycenter() const { return (min_y + max_y + 1) / 2; }

	// setters
	void set(int32_t minx, int32_t maxx, int32_t miny, int32_t maxy) { min_x = minx; max_x = maxx; min_y = miny; max_y = maxy; }
	void setx(int32_t minx, int32_t maxx) { min_x = minx; max_x = maxx; }
	void sety(int32_t miny, int32_t maxy) { min_y = miny; max_y = maxy; }
	void set_width(int32_t width) { max_x = min_x + width - 1; }
	void set_height(int32_t height) { max_y = min_y + height - 1; }
	void set_origin(int32_t x, int32_t y) { max_x += x - min_x; max_y += y - min_y; min_x = x; min_y = y; }
	void set_size(int32_t width, int32_t height) { set_width(width); set_height(height); }

	// offset helpers
	void offset(int32_t xdelta, int32_t ydelta) { min_x += xdelta; max_x += xdelta; min_y += ydelta; max_y += ydelta; }
	void offsetx(int32_t delta) { min_x += delta; max_x += delta; }
	void offsety(int32_t delta) { min_y += delta; max_y += delta; }

	// internal state
	int32_t min_x = 0;  // minimum X, or left coordinate
	int32_t max_x = 0;  // maximum X, or right coordinate (inclusive)
	int32_t min_y = 0;  // minimum Y, or top coordinate
	int32_t max_y = 0;  // maximum Y, or bottom coordinate (inclusive)
};


// ======================> bitmap_t

// bitmaps describe a rectangular array of pixels
class bitmap_t
{
protected:
	// construction/destruction -- subclasses only to ensure type correctness
	bitmap_t(const bitmap_t &) = delete;
	bitmap_t(bitmap_t &&that);
	bitmap_t(bitmap_format format, uint8_t bpp, int width = 0, int height = 0, int xslop = 0, int yslop = 0);
	bitmap_t(bitmap_format format, uint8_t bpp, void *base, int width, int height, int rowpixels);
	bitmap_t(bitmap_format format, uint8_t bpp, bitmap_t &source, const rectangle &subrect);
	virtual ~bitmap_t();

	// prevent implicit copying
	bitmap_t &operator=(const bitmap_t &) = delete;
	bitmap_t &operator=(bitmap_t &&that);

public:
	// allocation/deallocation
	void reset();

	// getters
	int32_t width() const { return m_width; }
	int32_t height() const { return m_height; }
	int32_t rowpixels() const { return m_rowpixels; }
	int32_t rowbytes() const { return m_rowpixels * m_bpp / 8; }
	uint8_t bpp() const { return m_bpp; }
	bitmap_format format() const { return m_format; }
	bool valid() const { return (m_base != nullptr); }
	palette_t *palette() const { return m_palette; }
	const rectangle &cliprect() const { return m_cliprect; }

	// allocation/sizing
	void allocate(int width, int height, int xslop = 0, int yslop = 0);
	void resize(int width, int height, int xslop = 0, int yslop = 0);

	// operations
	void set_palette(palette_t *palette);
	void fill(uint64_t color) { fill(color, m_cliprect); }
	void fill(uint64_t color, const rectangle &bounds);
	void plot_box(int32_t x, int32_t y, int32_t width, int32_t height, uint64_t color)
	{
		fill(color, rectangle(x, x + width - 1, y, y + height - 1));
	}

	// pixel access
	void *raw_pixptr(int32_t y, int32_t x = 0) { return reinterpret_cast<uint8_t *>(m_base) + (y * m_rowpixels + x) * m_bpp / 8; }
	void const *raw_pixptr(int32_t y, int32_t x = 0) const { return reinterpret_cast<uint8_t *>(m_base) + (y * m_rowpixels + x) * m_bpp / 8; }

protected:
	// for use by subclasses only to ensure type correctness
	template <typename PixelType> PixelType &pixt(int32_t y, int32_t x = 0) { return *(reinterpret_cast<PixelType *>(m_base) + y * m_rowpixels + x); }
	template <typename PixelType> PixelType const &pixt(int32_t y, int32_t x = 0) const { return *(reinterpret_cast<PixelType *>(m_base) + y * m_rowpixels + x); }
	void wrap(void *base, int width, int height, int rowpixels);
	void wrap(bitmap_t &source, const rectangle &subrect);

private:
	// internal helpers
	int32_t compute_rowpixels(int width, int xslop);
	void compute_base(int xslop, int yslop);
	bool valid_format() const;

	// internal state
	std::unique_ptr<uint8_t []> m_alloc;        // pointer to allocated pixel memory
	uint32_t                    m_allocbytes;   // size of our allocation
	void *                      m_base;         // pointer to pixel (0,0) (adjusted for padding)
	int32_t                     m_rowpixels;    // pixels per row (including padding)
	int32_t                     m_width;        // width of the bitmap
	int32_t                     m_height;       // height of the bitmap
	bitmap_format               m_format;       // format of the bitmap
	uint8_t                     m_bpp;          // bits per pixel
	palette_t *                 m_palette;      // optional palette
	rectangle                   m_cliprect;     // a clipping rectangle covering the full bitmap
};


// ======================> bitmap_specific, bitmap8_t, bitmap16_t, bitmap32_t, bitmap64_t

template <typename PixelType>
class bitmap_specific : public bitmap_t
{
	static constexpr int PIXEL_BITS = 8 * sizeof(PixelType);

protected:
	// construction/destruction -- subclasses only
	bitmap_specific(bitmap_specific<PixelType> &&) = default;
	bitmap_specific(bitmap_format format, int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap_t(format, PIXEL_BITS, width, height, xslop, yslop) { }
	bitmap_specific(bitmap_format format, PixelType *base, int width, int height, int rowpixels) : bitmap_t(format, PIXEL_BITS, base, width, height, rowpixels) { }
	bitmap_specific(bitmap_format format, bitmap_specific<PixelType> &source, const rectangle &subrect) : bitmap_t(format, PIXEL_BITS, source, subrect) { }

	bitmap_specific<PixelType> &operator=(bitmap_specific<PixelType> &&) = default;

public:
	using pixel_t = PixelType;

	// getters
	uint8_t bpp() const { return PIXEL_BITS; }

	// pixel accessors
	PixelType &pix(int32_t y, int32_t x = 0) { return pixt<PixelType>(y, x); }
	PixelType const &pix(int32_t y, int32_t x = 0) const { return pixt<PixelType>(y, x); }

	// operations
	void fill(PixelType color) { fill(color, cliprect()); }
	void fill(PixelType color, const rectangle &bounds)
	{
		// if we have a cliprect, intersect with that
		rectangle fill(bounds);
		fill &= cliprect();
		if (!fill.empty())
		{
			for (int32_t y = fill.top(); y <= fill.bottom(); y++)
				std::fill_n(&pix(y, fill.left()), fill.width(), color);
		}
	}
	void plot_box(int32_t x, int32_t y, int32_t width, int32_t height, PixelType color)
	{
		fill(color, rectangle(x, x + width - 1, y, y + height - 1));
	}
};

// 8bpp bitmaps
using bitmap8_t = bitmap_specific<uint8_t>;
extern template class bitmap_specific<uint8_t>;

// 16bpp bitmaps
using bitmap16_t = bitmap_specific<uint16_t>;
extern template class bitmap_specific<uint16_t>;

// 32bpp bitmaps
using bitmap32_t = bitmap_specific<uint32_t>;
extern template class bitmap_specific<uint32_t>;

// 64bpp bitmaps
using bitmap64_t = bitmap_specific<uint64_t>;
extern template class bitmap_specific<uint64_t>;


// ======================> bitmap_ind8, bitmap_ind16, bitmap_ind32, bitmap_ind64

// BITMAP_FORMAT_IND8 bitmaps
class bitmap_ind8 : public bitmap8_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND8;

public:
	// construction/destruction
	bitmap_ind8(bitmap_ind8 &&) = default;
	bitmap_ind8(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap8_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind8(uint8_t *base, int width, int height, int rowpixels) : bitmap8_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind8(bitmap_ind8 &source, const rectangle &subrect) : bitmap8_t(k_bitmap_format, source, subrect) { }
	void wrap(uint8_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_ind8 &operator=(bitmap_ind8 &&) = default;
};

// BITMAP_FORMAT_IND16 bitmaps
class bitmap_ind16 : public bitmap16_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND16;

public:
	// construction/destruction
	bitmap_ind16(bitmap_ind16 &&) = default;
	bitmap_ind16(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap16_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind16(uint16_t *base, int width, int height, int rowpixels) : bitmap16_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind16(bitmap_ind16 &source, const rectangle &subrect) : bitmap16_t(k_bitmap_format, source, subrect) { }
	void wrap(uint16_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_ind16 &operator=(bitmap_ind16 &&) = default;
};

// BITMAP_FORMAT_IND32 bitmaps
class bitmap_ind32 : public bitmap32_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND32;

public:
	// construction/destruction
	bitmap_ind32(bitmap_ind32 &&) = default;
	bitmap_ind32(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap32_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind32(uint32_t *base, int width, int height, int rowpixels) : bitmap32_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind32(bitmap_ind32 &source, const rectangle &subrect) : bitmap32_t(k_bitmap_format, source, subrect) { }
	void wrap(uint32_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_ind32 &operator=(bitmap_ind32 &&) = default;
};

// BITMAP_FORMAT_IND64 bitmaps
class bitmap_ind64 : public bitmap64_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND64;

public:
	// construction/destruction
	bitmap_ind64(bitmap_ind64 &&) = default;
	bitmap_ind64(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap64_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind64(uint64_t *base, int width, int height, int rowpixels) : bitmap64_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind64(bitmap_ind64 &source, const rectangle &subrect) : bitmap64_t(k_bitmap_format, source, subrect) { }
	void wrap(uint64_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_ind64 &operator=(bitmap_ind64 &&) = default;
};


// ======================> bitmap_yuy16, bitmap_rgb32, bitmap_argb32

// BITMAP_FORMAT_YUY16 bitmaps
class bitmap_yuy16 : public bitmap16_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_YUY16;

public:
	// construction/destruction
	bitmap_yuy16(bitmap_yuy16 &&) = default;
	bitmap_yuy16(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap16_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_yuy16(uint16_t *base, int width, int height, int rowpixels) : bitmap16_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_yuy16(bitmap_yuy16 &source, const rectangle &subrect) : bitmap16_t(k_bitmap_format, source, subrect) { }
	void wrap(uint16_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_yuy16 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_yuy16 &operator=(bitmap_yuy16 &&) = default;
};

// BITMAP_FORMAT_RGB32 bitmaps
class bitmap_rgb32 : public bitmap32_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_RGB32;

public:
	// construction/destruction
	bitmap_rgb32(bitmap_rgb32 &&) = default;
	bitmap_rgb32(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap32_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_rgb32(uint32_t *base, int width, int height, int rowpixels) : bitmap32_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_rgb32(bitmap_rgb32 &source, const rectangle &subrect) : bitmap32_t(k_bitmap_format, source, subrect) { }
	void wrap(uint32_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_rgb32 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_rgb32 &operator=(bitmap_rgb32 &&) = default;
};

// BITMAP_FORMAT_ARGB32 bitmaps
class bitmap_argb32 : public bitmap32_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_ARGB32;

public:
	// construction/destruction
	bitmap_argb32(bitmap_argb32 &&) = default;
	bitmap_argb32(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap32_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_argb32(uint32_t *base, int width, int height, int rowpixels) : bitmap32_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_argb32(bitmap_argb32 &source, const rectangle &subrect) : bitmap32_t(k_bitmap_format, source, subrect) { }
	void wrap(uint32_t *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(bitmap_argb32 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }

	bitmap_argb32 &operator=(bitmap_argb32 &&) = default;
};

#endif // MAME_UTIL_BITMAP_H
