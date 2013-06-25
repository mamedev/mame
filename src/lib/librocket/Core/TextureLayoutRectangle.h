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

#ifndef TEXTURELAYOUTRECTANGLE_H
#define TEXTURELAYOUTRECTANGLE_H

namespace Rocket {
namespace Core {

/**
	A texture layout rectangle is an area positioned with a texture layout.

	@author Peter
 */

class TextureLayoutRectangle
{
public:
	TextureLayoutRectangle(int id, const Vector2i& dimensions);
	~TextureLayoutRectangle();

	/// Returns the rectangle's id.
	/// @return The rectangle's id.
	int GetId() const;
	/// Returns the rectangle's position; this is only valid if it has been placed.
	/// @return The rectangle's position within its texture.
	const Vector2i& GetPosition() const;
	/// Returns the rectangle's dimensions.
	/// @return The rectangle's dimensions.
	const Vector2i& GetDimensions() const;

	/// Places the rectangle within a texture.
	/// @param[in] texture_index The index of the texture this rectangle is placed on.
	/// @param[in] position The position within the texture of this rectangle's top-left corner.
	void Place(int texture_index, const Vector2i& position);
	/// Unplaces the rectangle.
	void Unplace();
	/// Returns the rectangle's placed state.
	/// @return True if the rectangle has been placed, false if not.
	bool IsPlaced() const;

	/// Sets the rectangle's texture data and stride.
	/// @param[in] texture_data The pointer to the top-left corner of the texture's data.
	/// @param[in] texture_stride The stride of the texture data, in bytes.
	void Allocate(byte* texture_data, int texture_stride);

	/// Returns the index of the texture this rectangle is placed on.
	/// @return The texture index.
	int GetTextureIndex();
	/// Returns the rectangle's allocated texture data.
	/// @return The texture data.
	byte* GetTextureData();
	/// Returns the stride of the rectangle's texture data.
	/// @return The texture data stride.
	int GetTextureStride() const;

private:
	int id;
	Vector2i dimensions;

	int texture_index;
	Vector2i texture_position;

	byte* texture_data;
	int texture_stride;
};

}
}

#endif
