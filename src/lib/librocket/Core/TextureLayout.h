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

#ifndef TEXTURELAYOUT_H
#define TEXTURELAYOUT_H

#include "TextureLayoutRectangle.h"
#include "TextureLayoutTexture.h"

namespace Rocket {
namespace Core {

/**
	A texture layout generates and stores a layout of rectangles within a series of textures. It is
	used primarily by the font system for generating font textures.

	@author Peter
 */

class TextureLayout
{
public:
	TextureLayout();
	~TextureLayout();

	/// Adds a rectangle to the list of rectangles to be laid out. All rectangles must be added to
	/// the layout before the layout is generated.
	/// @param[in] id The id of the rectangle; used to identify the rectangle after it has been positioned.
	/// @param[in] dimensions The dimensions of the rectangle.
	void AddRectangle(int id, const Vector2i& dimensions);

	/// Returns one of the layout's rectangles.
	/// @param[in] index The index of the desired rectangle.
	/// @return The desired rectangle.
	TextureLayoutRectangle& GetRectangle(int index);
	/// Returns the number of rectangles in the layout.
	/// @return The layout's rectangle count.
	int GetNumRectangles() const;

	/// Returns one of the layout's textures.
	/// @param[in] index The index of the desired texture.
	/// @return The desired texture.
	TextureLayoutTexture& GetTexture(int index);
	/// Returns the number of textures in the layout.
	/// @return The layout's texture count.
	int GetNumTextures() const;

	/// Attempts to generate an efficient texture layout for the rectangles.
	/// @param[in] max_texture_dimensions The maximum dimensions allowed for any single texture.
	/// @return True if the layout was generated successfully, false if not.
	bool GenerateLayout(int max_texture_dimensions);

private:
	typedef std::vector< TextureLayoutRectangle > RectangleList;
	typedef std::vector< TextureLayoutTexture > TextureList;

	TextureList textures;
	RectangleList rectangles;
};

}
}

#endif
