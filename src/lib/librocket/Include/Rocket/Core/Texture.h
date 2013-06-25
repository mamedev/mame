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

#ifndef ROCKETCORETEXTURE_H
#define ROCKETCORETEXTURE_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

class TextureResource;
class RenderInterface;

/**
	Abstraction of a two-dimensional texture image, with an application-specific texture handle.

	@author Peter Curry
 */

struct ROCKETCORE_API Texture
{
public:
	/// Constructs an unloaded texture with no resource.
	Texture();
	/// Constructs a texture sharing the resource of another.
	Texture(const Texture&);
	~Texture();

	/// Attempts to load a texture.
	/// @param[in] source The name of the texture.
	/// @param[in] source_path The path of the resource that is requesting the texture (ie, the RCSS file in which it was specified, etc).
	/// @return True if the texture loaded successfully, false if not.
	bool Load(const String& source, const String& source_path = "");

	/// Returns the texture's source name. This is usually the name of the file the texture was loaded from.
	/// @return The name of the this texture's source. This will be the empty string if this texture is not loaded.
	String GetSource() const;
	/// Returns the texture's handle.
	/// @param[in] The render interface that is requesting the handle.
	/// @return The texture's handle. This will be NULL if the texture isn't loaded.
	TextureHandle GetHandle(RenderInterface* render_interface) const;
	/// Returns the texture's dimensions.
	/// @param[in] The render interface that is requesting the dimensions.
	/// @return The texture's dimensions. This will be (0, 0) if the texture isn't loaded.
	Vector2i GetDimensions(RenderInterface* render_interface) const;

	/// Releases this texture's resource (if any), and sets it to another texture's resource.
	const Texture& operator=(const Texture&);

private:
	TextureResource* resource;
};

}
}

#endif
