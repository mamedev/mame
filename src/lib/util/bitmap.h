/***************************************************************************

    bitmap.h

    Core bitmap routines.

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
	BITMAP_FORMAT_INVALID = 0,		// invalid forma
	BITMAP_FORMAT_INDEXED8,			// 8bpp indexed
	BITMAP_FORMAT_INDEXED16,		// 16bpp indexed
	BITMAP_FORMAT_INDEXED32,		// 32bpp indexed
	BITMAP_FORMAT_INDEXED64,		// 64bpp indexed
	BITMAP_FORMAT_RGB15,			// 15bpp 5-5-5 RGB
	BITMAP_FORMAT_RGB32,			// 32bpp 8-8-8 RGB
	BITMAP_FORMAT_ARGB32,			// 32bpp 8-8-8-8 ARGB
	BITMAP_FORMAT_YUY16,			// 16bpp 8-8 Y/Cb, Y/Cr in sequence
	BITMAP_FORMAT_LAST
};


// rectangles describe a bitmap portion
class rectangle
{
public:
	// construction/destruction
	rectangle() { }
	rectangle(INT32 minx, INT32 maxx, INT32 miny, INT32 maxy)
		: min_x(minx), max_x(maxx), min_y(miny), max_y(maxy) { }

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
	
	// other helpers
	bool empty() const { return (min_x > max_x || min_y > max_y); }
	bool contains(INT32 x, INT32 y) const { return (x >= min_x && x <= max_x && y >= min_y && y <= max_y); }
	INT32 width() const { return max_x + 1 - min_x; }
	INT32 height() const { return max_y + 1 - min_y; }
	void set(INT32 minx, INT32 maxx, INT32 miny, INT32 maxy) { min_x = minx; max_x = maxx; min_y = miny; max_y = maxy; }

	// internal state
	INT32			min_x;			// minimum X, or left coordinate
	INT32			max_x;			// maximum X, or right coordinate (inclusive)
	INT32			min_y;			// minimum Y, or top coordinate
	INT32			max_y;			// maximum Y, or bottom coordinate (inclusive)
};


// bitmaps describe a rectangular array of pixels
class bitmap_t
{
private:
	// prevent implicit copying
	bitmap_t(const bitmap_t &);
	bitmap_t &operator=(const bitmap_t &);

public:
	// construction/destruction
	bitmap_t();
	bitmap_t(int width, int height, bitmap_format format, int xslop = 0, int yslop = 0);
	bitmap_t(void *base, int width, int height, int rowpixels, bitmap_format format);
	~bitmap_t();
	
	// getters
	UINT32 width() const { return m_width; }
	UINT32 height() const { return m_height; }
	UINT32 rowpixels() const { return m_rowpixels; }
	UINT32 rowbytes() const { return m_rowpixels * m_bpp / 8; }
	UINT8 bpp() const { return m_bpp; }
	bitmap_format format() const { return m_format; }
	palette_t *palette() const { return m_palette; }
	const rectangle &cliprect() const { return m_cliprect; }
	
	// helpers
	void clone_existing(const bitmap_t &srcbitmap);
	void set_palette(palette_t *palette);
	void fill(rgb_t color) { fill(color, m_cliprect); }
	void fill(rgb_t color, const rectangle &cliprect);
	void plot_box(int x, int y, int width, int height, UINT32 color)
	{
		rectangle clip(x, x + width - 1, y, y + height - 1);
		fill(color, clip);
	}
	
	// pixel access
	template<typename _PixelType>
	_PixelType &pix(INT32 y, INT32 x = 0) const { return *(reinterpret_cast<_PixelType *>(m_base) + y * m_rowpixels + x); }
	UINT8 &pix8(INT32 y, INT32 x = 0) const { return *(reinterpret_cast<UINT8 *>(m_base) + y * m_rowpixels + x); }
	UINT16 &pix16(INT32 y, INT32 x = 0) const { return *(reinterpret_cast<UINT16 *>(m_base) + y * m_rowpixels + x); }
	UINT32 &pix32(INT32 y, INT32 x = 0) const { return *(reinterpret_cast<UINT32 *>(m_base) + y * m_rowpixels + x); }
	UINT64 &pix64(INT32 y, INT32 x = 0) const { return *(reinterpret_cast<UINT64 *>(m_base) + y * m_rowpixels + x); }
	void *raw_pixptr(INT32 y, INT32 x = 0) const { return reinterpret_cast<UINT8 *>(m_base) + (y * m_rowpixels + x) * m_bpp / 8; }
	
	// static helpers
	static UINT8 format_to_bpp(bitmap_format format);

private:
	// internal state
	UINT8 *			m_alloc;		// pointer to allocated pixel memory
	void *			m_base;			// pointer to pixel (0,0) (adjusted for padding)
	INT32			m_rowpixels;	// pixels per row (including padding)
	INT32 			m_width;		// width of the bitmap
	INT32			m_height;		// height of the bitmap
	bitmap_format	m_format;		// format of the bitmap
	UINT8			m_bpp;			// bits per pixel
	palette_t *		m_palette;		// optional palette
	rectangle		m_cliprect;		// a clipping rectangle covering the full bitmap
};


#endif	// __BITMAP_H__
