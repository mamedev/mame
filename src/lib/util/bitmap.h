// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    bitmap.h

    Core bitmap routines.

***************************************************************************/

#pragma once

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "osdcore.h"
#include "palette.h"


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
	BITMAP_FORMAT_YUY16,            // 16bpp 8-8 Y/Cb, Y/Cr in sequence
	BITMAP_FORMAT_LAST
};


// ======================> rectangle

// rectangles describe a bitmap portion
class rectangle
{
public:
	// construction/destruction
	rectangle()
		: min_x(0), max_x(0), min_y(0), max_y(0) { }
	rectangle(INT32 minx, INT32 maxx, INT32 miny, INT32 maxy)
		: min_x(minx), max_x(maxx), min_y(miny), max_y(maxy) { }

	// getters
	INT32 left() const { return min_x; }
	INT32 right() const { return max_x; }
	INT32 top() const { return min_y; }
	INT32 bottom() const { return max_y; }

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
	bool operator==(const rectangle &rhs) const { return min_x == rhs.min_x && max_x == rhs.max_x && min_y == rhs.min_y && max_y == rhs.max_y; }
	bool operator!=(const rectangle &rhs) const { return min_x != rhs.min_x || max_x != rhs.max_x || min_y != rhs.min_y || max_y != rhs.max_y; }
	bool operator>(const rectangle &rhs) const { return min_x < rhs.min_x && min_y < rhs.min_y && max_x > rhs.max_x && max_y > rhs.max_y; }
	bool operator>=(const rectangle &rhs) const { return min_x <= rhs.min_x && min_y <= rhs.min_y && max_x >= rhs.max_x && max_y >= rhs.max_y; }
	bool operator<(const rectangle &rhs) const { return min_x >= rhs.min_x || min_y >= rhs.min_y || max_x <= rhs.max_x || max_y <= rhs.max_y; }
	bool operator<=(const rectangle &rhs) const { return min_x > rhs.min_x || min_y > rhs.min_y || max_x < rhs.max_x || max_y < rhs.max_y; }

	// other helpers
	bool empty() const { return (min_x > max_x || min_y > max_y); }
	bool contains(INT32 x, INT32 y) const { return (x >= min_x && x <= max_x && y >= min_y && y <= max_y); }
	bool contains(const rectangle &rect) const { return (min_x <= rect.min_x && max_x >= rect.max_x && min_y <= rect.min_y && max_y >= rect.max_y); }
	INT32 width() const { return max_x + 1 - min_x; }
	INT32 height() const { return max_y + 1 - min_y; }
	INT32 xcenter() const { return (min_x + max_x + 1) / 2; }
	INT32 ycenter() const { return (min_y + max_y + 1) / 2; }

	// setters
	void set(INT32 minx, INT32 maxx, INT32 miny, INT32 maxy) { min_x = minx; max_x = maxx; min_y = miny; max_y = maxy; }
	void setx(INT32 minx, INT32 maxx) { min_x = minx; max_x = maxx; }
	void sety(INT32 miny, INT32 maxy) { min_y = miny; max_y = maxy; }
	void set_width(INT32 width) { max_x = min_x + width - 1; }
	void set_height(INT32 height) { max_y = min_y + height - 1; }
	void set_origin(INT32 x, INT32 y) { max_x += x - min_x; max_y += y - min_y; min_x = x; min_y = y; }
	void set_size(INT32 width, INT32 height) { set_width(width); set_height(height); }

	// offset helpers
	void offset(INT32 xdelta, INT32 ydelta) { min_x += xdelta; max_x += xdelta; min_y += ydelta; max_y += ydelta; }
	void offsetx(INT32 delta) { min_x += delta; max_x += delta; }
	void offsety(INT32 delta) { min_y += delta; max_y += delta; }

	// internal state
	INT32           min_x;          // minimum X, or left coordinate
	INT32           max_x;          // maximum X, or right coordinate (inclusive)
	INT32           min_y;          // minimum Y, or top coordinate
	INT32           max_y;          // maximum Y, or bottom coordinate (inclusive)
};


// ======================> bitmap_t

// bitmaps describe a rectangular array of pixels
class bitmap_t
{
private:
	// prevent implicit copying
	bitmap_t(const bitmap_t &);
	bitmap_t &operator=(const bitmap_t &);

protected:
	// construction/destruction -- subclasses only to ensure type correctness
	bitmap_t(bitmap_format format, int bpp, int width = 0, int height = 0, int xslop = 0, int yslop = 0);
	bitmap_t(bitmap_format format, int bpp, void *base, int width, int height, int rowpixels);
	bitmap_t(bitmap_format format, int bpp, bitmap_t &source, const rectangle &subrect);
	virtual ~bitmap_t();

public:
	// allocation/deallocation
	void reset();

	// getters
	INT32 width() const { return m_width; }
	INT32 height() const { return m_height; }
	INT32 rowpixels() const { return m_rowpixels; }
	INT32 rowbytes() const { return m_rowpixels * m_bpp / 8; }
	UINT8 bpp() const { return m_bpp; }
	bitmap_format format() const { return m_format; }
	bool valid() const { return (m_base != nullptr); }
	palette_t *palette() const { return m_palette; }
	const rectangle &cliprect() const { return m_cliprect; }

	// allocation/sizing
	void allocate(int width, int height, int xslop = 0, int yslop = 0);
	void resize(int width, int height, int xslop = 0, int yslop = 0);

	// operations
	void set_palette(palette_t *palette);
	void fill(UINT32 color) { fill(color, m_cliprect); }
	void fill(UINT32 color, const rectangle &cliprect);
	void plot_box(int x, int y, int width, int height, UINT32 color)
	{
		rectangle clip(x, x + width - 1, y, y + height - 1);
		fill(color, clip);
	}

	// pixel access
	template<typename _PixelType>
	_PixelType &pixt(INT32 y, INT32 x = 0) const { return *(reinterpret_cast<_PixelType *>(m_base) + y * m_rowpixels + x); }
	void *raw_pixptr(INT32 y, INT32 x = 0) const { return reinterpret_cast<UINT8 *>(m_base) + (y * m_rowpixels + x) * m_bpp / 8; }

protected:
	// for use by subclasses only to ensure type correctness
	void wrap(void *base, int width, int height, int rowpixels);
	void wrap(const bitmap_t &source, const rectangle &subrect);

private:
	// internal helpers
	INT32 compute_rowpixels(int width, int xslop);
	void compute_base(int xslop, int yslop);

	// internal state
	UINT8 *         m_alloc;        // pointer to allocated pixel memory
	UINT32          m_allocbytes;   // size of our allocation
	void *          m_base;         // pointer to pixel (0,0) (adjusted for padding)
	INT32           m_rowpixels;    // pixels per row (including padding)
	INT32           m_width;        // width of the bitmap
	INT32           m_height;       // height of the bitmap
	bitmap_format   m_format;       // format of the bitmap
	UINT8           m_bpp;          // bits per pixel
	palette_t *     m_palette;      // optional palette
	rectangle       m_cliprect;     // a clipping rectangle covering the full bitmap
};


// ======================> bitmap8_t, bitmap16_t, bitmap32_t, bitmap64_t

// 8bpp bitmaps
class bitmap8_t : public bitmap_t
{
private:
	// private helpers
	bool valid_format(bitmap_format format) const { return (format == BITMAP_FORMAT_IND8); }

protected:
	// construction/destruction -- subclasses only
	bitmap8_t(bitmap_format format, int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap_t(format, 8, width, height, xslop, yslop) { }
	bitmap8_t(bitmap_format format, UINT8 *base, int width, int height, int rowpixels) : bitmap_t(format, 8, base, width, height, rowpixels) { assert(valid_format(format)); }
	bitmap8_t(bitmap_format format, bitmap8_t &source, const rectangle &subrect) : bitmap_t(format, 8, source, subrect) { }

public:
	// getters
	UINT8 bpp() const { return 8; }

	// pixel accessors
	typedef UINT8 pixel_t;
	pixel_t &pix(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
	pixel_t &pix8(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
};

// 16bpp bitmaps
class bitmap16_t : public bitmap_t
{
private:
	// private helpers
	bool valid_format(bitmap_format format) const { return (format == BITMAP_FORMAT_IND16 || format == BITMAP_FORMAT_YUY16); }

protected:
	// construction/destruction -- subclasses only
	bitmap16_t(bitmap_format format, int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap_t(format, 16, width, height, xslop, yslop) { assert(valid_format(format)); }
	bitmap16_t(bitmap_format format, UINT16 *base, int width, int height, int rowpixels) : bitmap_t(format, 16, base, width, height, rowpixels) { assert(valid_format(format)); }
	bitmap16_t(bitmap_format format, bitmap16_t &source, const rectangle &subrect) : bitmap_t(format, 16, source, subrect) { }

public:
	// getters
	UINT8 bpp() const { return 16; }

	// pixel accessors
	typedef UINT16 pixel_t;
	pixel_t &pix(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
	pixel_t &pix16(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
};

// 32bpp bitmaps
class bitmap32_t : public bitmap_t
{
private:
	// private helpers
	bool valid_format(bitmap_format format) const { return (format == BITMAP_FORMAT_IND32 || format == BITMAP_FORMAT_RGB32 || format == BITMAP_FORMAT_ARGB32); }

protected:
	// construction/destruction -- subclasses only
	bitmap32_t(bitmap_format format, int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap_t(format, 32, width, height, xslop, yslop) { assert(valid_format(format)); }
	bitmap32_t(bitmap_format format, UINT32 *base, int width, int height, int rowpixels) : bitmap_t(format, 32, base, width, height, rowpixels) { assert(valid_format(format)); }
	bitmap32_t(bitmap_format format, bitmap32_t &source, const rectangle &subrect) : bitmap_t(format, 32, source, subrect) { }

public:
	// getters
	UINT8 bpp() const { return 32; }

	// pixel accessors
	typedef UINT32 pixel_t;
	pixel_t &pix(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
	pixel_t &pix32(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
};

// 64bpp bitmaps
class bitmap64_t : public bitmap_t
{
private:
	// private helpers
	bool valid_format(bitmap_format format) const { return (format == BITMAP_FORMAT_IND64); }

protected:
	// construction/destruction -- subclasses only
	bitmap64_t(bitmap_format format, int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap_t(format, 64, width, height, xslop, yslop) { assert(valid_format(format)); }
	bitmap64_t(bitmap_format format, UINT64 *base, int width, int height, int rowpixels) : bitmap_t(format, 64, base, width, height, rowpixels) { assert(valid_format(format)); }
	bitmap64_t(bitmap_format format, bitmap64_t &source, const rectangle &subrect) : bitmap_t(format, 64, source, subrect) { }

public:
	// getters
	UINT8 bpp() const { return 64; }

	// pixel accessors
	typedef UINT64 pixel_t;
	pixel_t &pix(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
	pixel_t &pix64(INT32 y, INT32 x = 0) const { return pixt<pixel_t>(y, x); }
};


// ======================> bitmap_ind8, bitmap_ind16, bitmap_ind32, bitmap_ind64

// BITMAP_FORMAT_IND8 bitmaps
class bitmap_ind8 : public bitmap8_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND8;

public:
	// construction/destruction
	bitmap_ind8(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap8_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind8(UINT8 *base, int width, int height, int rowpixels) : bitmap8_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind8(bitmap_ind8 &source, const rectangle &subrect) : bitmap8_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT8 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};

// BITMAP_FORMAT_IND16 bitmaps
class bitmap_ind16 : public bitmap16_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND16;

public:
	// construction/destruction
	bitmap_ind16(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap16_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind16(UINT16 *base, int width, int height, int rowpixels) : bitmap16_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind16(bitmap_ind16 &source, const rectangle &subrect) : bitmap16_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT16 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};

// BITMAP_FORMAT_IND32 bitmaps
class bitmap_ind32 : public bitmap32_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND32;

public:
	// construction/destruction
	bitmap_ind32(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap32_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind32(UINT32 *base, int width, int height, int rowpixels) : bitmap32_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind32(bitmap_ind32 &source, const rectangle &subrect) : bitmap32_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT32 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};

// BITMAP_FORMAT_IND64 bitmaps
class bitmap_ind64 : public bitmap64_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_IND64;

public:
	// construction/destruction
	bitmap_ind64(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap64_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_ind64(UINT64 *base, int width, int height, int rowpixels) : bitmap64_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_ind64(bitmap_ind64 &source, const rectangle &subrect) : bitmap64_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT64 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_ind8 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};


// ======================> bitmap_yuy16, bitmap_rgb32, bitmap_argb32

// BITMAP_FORMAT_YUY16 bitmaps
class bitmap_yuy16 : public bitmap16_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_YUY16;

public:
	// construction/destruction
	bitmap_yuy16(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap16_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_yuy16(UINT16 *base, int width, int height, int rowpixels) : bitmap16_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_yuy16(bitmap_yuy16 &source, const rectangle &subrect) : bitmap16_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT16 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_yuy16 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};

// BITMAP_FORMAT_RGB32 bitmaps
class bitmap_rgb32 : public bitmap32_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_RGB32;

public:
	// construction/destruction
	bitmap_rgb32(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap32_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_rgb32(UINT32 *base, int width, int height, int rowpixels) : bitmap32_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_rgb32(bitmap_rgb32 &source, const rectangle &subrect) : bitmap32_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT32 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_rgb32 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};

// BITMAP_FORMAT_ARGB32 bitmaps
class bitmap_argb32 : public bitmap32_t
{
	static const bitmap_format k_bitmap_format = BITMAP_FORMAT_ARGB32;

public:
	// construction/destruction
	bitmap_argb32(int width = 0, int height = 0, int xslop = 0, int yslop = 0) : bitmap32_t(k_bitmap_format, width, height, xslop, yslop) { }
	bitmap_argb32(UINT32 *base, int width, int height, int rowpixels) : bitmap32_t(k_bitmap_format, base, width, height, rowpixels) { }
	bitmap_argb32(bitmap_argb32 &source, const rectangle &subrect) : bitmap32_t(k_bitmap_format, source, subrect) { }
	void wrap(UINT32 *base, int width, int height, int rowpixels) { bitmap_t::wrap(base, width, height, rowpixels); }
	void wrap(const bitmap_argb32 &source, const rectangle &subrect) { bitmap_t::wrap(static_cast<const bitmap_t &>(source), subrect); }

	// getters
	bitmap_format format() const { return k_bitmap_format; }
};


#endif  // __BITMAP_H__
