/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include "FontEffectOutline.h"

namespace Rocket {
namespace Core {

FontEffectOutline::FontEffectOutline()
{
	width = 0;

	// Default the z-index of an outline effect to be behind the main layer.
	SetZIndex(-1);
}

FontEffectOutline::~FontEffectOutline()
{
}

// Returns true.
bool FontEffectOutline::HasUniqueTexture() const
{
	return true;
}

// Initialise the outline effect.
bool FontEffectOutline::Initialise(int _width)
{
	if (_width <= 0)
		return false;

	width = _width;

	filter.Initialise(width, ConvolutionFilter::DILATION);
	for (int x = -width; x <= width; ++x)
	{
		for (int y = -width; y <= width; ++y)
		{
			float weight = 1;

			float distance = Math::SquareRoot(float(x * x + y * y));
			if (distance > width)
			{
				weight = (width + 1) - distance;
				weight = Math::Max(weight, 0.0f);
			}

			filter[x + width][y + width] = weight;
		}
	}

	return true;
}

// Resizes and repositions the glyph to fit the outline.
bool FontEffectOutline::GetGlyphMetrics(Vector2i& origin, Vector2i& dimensions, const FontGlyph& ROCKET_UNUSED(glyph)) const
{
	if (dimensions.x * dimensions.y > 0)
	{
		origin.x -= width;
		origin.y -= width;

		dimensions.x += width;
		dimensions.y += width;

		return true;
	}

	return false;
}

// Expands the original glyph texture for the outline.
void FontEffectOutline::GenerateGlyphTexture(byte* destination_data, const Vector2i& destination_dimensions, int destination_stride, const FontGlyph& glyph) const
{
	filter.Run(destination_data, destination_dimensions, destination_stride, glyph.bitmap_data, glyph.bitmap_dimensions, Vector2i(width, width));
}

}
}
