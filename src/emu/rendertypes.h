// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    rendertypes.h

    Core renderer constants and structures for MAME.

***************************************************************************/
#ifndef MAME_EMU_RENDERTYPES_H
#define MAME_EMU_RENDERTYPES_H

#pragma once

#include <algorithm>


//**************************************************************************
//  CONSTANTS
//**************************************************************************


// blending modes
enum
{
	BLENDMODE_NONE = 0,                                 // no blending
	BLENDMODE_ALPHA,                                    // standard alpha blend
	BLENDMODE_RGB_MULTIPLY,                             // apply source alpha to source pix, then multiply RGB values
	BLENDMODE_ADD,                                      // apply source alpha to source pix, then add to destination

	BLENDMODE_COUNT
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// render_bounds - floating point bounding rectangle
struct render_bounds
{
	float               x0;                 // leftmost X coordinate
	float               y0;                 // topmost Y coordinate
	float               x1;                 // rightmost X coordinate
	float               y1;                 // bottommost Y coordinate

	constexpr float width() const { return x1 - x0; }
	constexpr float height() const { return y1 - y0; }
	constexpr float aspect() const { return width() / height(); }
	constexpr bool includes(float x, float y) const { return (x >= x0) && (x <= x1) && (y >= y0) && (y <= y1); }

	// intersection
	constexpr render_bounds operator&(render_bounds const &b) const
	{
		return render_bounds{ (std::max)(x0, b.x0), (std::max)(y0, b.y0), (std::min)(x1, b.x1), (std::min)(y1, b.y1) };
	}

	render_bounds &operator&=(render_bounds const &b)
	{
		x0 = (std::max)(x0, b.x0);
		y0 = (std::max)(y0, b.y0);
		x1 = (std::min)(x1, b.x1);
		y1 = (std::min)(y1, b.y1);
		return *this;
	}

	// union
	constexpr render_bounds operator|(render_bounds const &b) const
	{
		return render_bounds{ (std::min)(x0, b.x0), (std::min)(y0, b.y0), (std::max)(x1, b.x1), (std::max)(y1, b.y1) };
	}

	render_bounds &operator|=(render_bounds const &b)
	{
		x0 = (std::min)(x0, b.x0);
		y0 = (std::min)(y0, b.y0);
		x1 = (std::max)(x1, b.x1);
		y1 = (std::max)(y1, b.y1);
		return *this;
	}

	render_bounds &set_xy(float left, float top, float right, float bottom)
	{
		x0 = left;
		y0 = top;
		x1 = right;
		y1 = bottom;
		return *this;
	}

	render_bounds &set_wh(float left, float top, float width, float height)
	{
		x0 = left;
		y0 = top;
		x1 = left + width;
		y1 = top + height;
		return *this;
	}
};


// render_color - floating point set of ARGB values
struct render_color
{
	float               a;                  // alpha component (0.0 = transparent, 1.0 = opaque)
	float               r;                  // red component (0.0 = none, 1.0 = max)
	float               g;                  // green component (0.0 = none, 1.0 = max)
	float               b;                  // blue component (0.0 = none, 1.0 = max)

	constexpr render_color operator*(render_color const &c) const
	{
		return render_color{ a * c.a, r * c.r, g * c.g, b * c.b };
	}

	render_color &operator*=(render_color const &c)
	{
		a *= c.a;
		r *= c.r;
		g *= c.g;
		b *= c.b;
		return *this;
	}

	render_color &set(float alpha, float red, float green, float blue)
	{
		a = alpha;
		r = red;
		g = green;
		b = blue;
		return *this;
	}
};


// render_texuv - floating point set of UV texture coordinates
struct render_texuv
{
	float               u;                  // U coordinate (0.0-1.0)
	float               v;                  // V coordinate (0.0-1.0)
};


// render_quad_texuv - floating point set of UV texture coordinates
struct render_quad_texuv
{
	render_texuv        tl;                 // top-left UV coordinate
	render_texuv        tr;                 // top-right UV coordinate
	render_texuv        bl;                 // bottom-left UV coordinate
	render_texuv        br;                 // bottom-right UV coordinate
};

#endif // MAME_EMU_RENDERTYPES_H
