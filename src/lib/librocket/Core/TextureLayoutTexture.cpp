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
#include "TextureLayoutTexture.h"
#include <Rocket/Core/Core.h>
#include "TextureDatabase.h"
#include "TextureLayout.h"

namespace Rocket {
namespace Core {

TextureLayoutTexture::TextureLayoutTexture() : dimensions(0, 0)
{
	texture_data = NULL;
}

TextureLayoutTexture::~TextureLayoutTexture()
{
	// Don't free texture data; freed in the texture loader.
}

// Returns the texture's dimensions. This is only valid after the texture has been generated.
const Vector2i& TextureLayoutTexture::GetDimensions() const
{
	return dimensions;
}

// Attempts to position unplaced rectangles from the layout into this texture.
int TextureLayoutTexture::Generate(TextureLayout& layout, int maximum_dimensions)
{
	// Come up with an estimate for how big a texture we need. Calculate the total square pixels
	// required by the remaining rectangles to place, square-root it to get the dimensions of the
	// smallest texture necessary (under optimal circumstances) and round it up to the nearest
	// power of two.
	int square_pixels = 0;
	int unplaced_rectangles = 0;
	for (int i = 0; i < layout.GetNumRectangles(); ++i)
	{
		const TextureLayoutRectangle& rectangle = layout.GetRectangle(i);

		if (!rectangle.IsPlaced())
		{
			square_pixels += (rectangle.GetDimensions().x + 1) * (rectangle.GetDimensions().y + 1);
			++unplaced_rectangles;
		}
	}

	int texture_width = Math::RealToInteger(Math::SquareRoot((float) square_pixels));
	dimensions.y = Math::ToPowerOfTwo(texture_width);
	dimensions.x = dimensions.y >> 1;

	dimensions.x = Math::Min(dimensions.x, maximum_dimensions);
	dimensions.y = Math::Min(dimensions.y, maximum_dimensions);

	// Now we're layout out the rectangles in the texture. If we don't fit all the rectangles on
	// and have room to grow (ie, haven't hit the maximum texture size in both dimensions) then
	// we'll have another go with a bigger texture.
	int num_placed_rectangles = 0;
	for (;;)
	{
		bool success = true;
		int height = 1;

		while (num_placed_rectangles != unplaced_rectangles)
		{
			TextureLayoutRow row;
			int row_size = row.Generate(layout, dimensions.x, height);
			if (row_size == 0)
			{
				success = false;
				break;
			}

			height += row.GetHeight() + 1;
			if (height > dimensions.y)
			{
				// D'oh! We've exceeded our height boundaries. This row should be unplaced.
				row.Unplace();
				success = false;
				break;
			}

			rows.push_back(row);
			num_placed_rectangles += row_size;
		}

		// If the rectangles were successfully laid out within the texture limits, we're done.
		if (success)
			return num_placed_rectangles;

		// Couldn't do it! Increase the texture size, clear the rectangles and try again - unless
		// we've hit the maximum texture size, in which case return true if we've placed any
		// rectangles (ie, the layout isn't empty).
		if (dimensions.y > dimensions.x)
			dimensions.x = dimensions.y;
		else
		{
			if (dimensions.y << 1 > maximum_dimensions)
				return num_placed_rectangles;

			dimensions.y <<= 1;
		}

		// Unplace all of the glyphs we tried to place and have an other crack.
		for (size_t i = 0; i < rows.size(); i++)
			rows[i].Unplace();

		rows.clear();
		num_placed_rectangles = 0;
	}
}

// Allocates the texture.
byte* TextureLayoutTexture::AllocateTexture()
{
	// Note: this object does not free this texture data. It is freed in the font texture loader.

	if (dimensions.x > 0 &&
		dimensions.y > 0)
	{
		texture_data = new byte[dimensions.x * dimensions.y * 4];

		// Set the texture to transparent white.
		for (int i = 0; i < dimensions.x * dimensions.y; i++)
			((unsigned int*)(texture_data))[i] = 0x00ffffff;

		for (size_t i = 0; i < rows.size(); ++i)
			rows[i].Allocate(texture_data, dimensions.x * 4);
	}

	return texture_data;
}

}
}
