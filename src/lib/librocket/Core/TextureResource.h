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

#ifndef ROCKETCORETEXTURERESOURCE_H
#define ROCKETCORETEXTURERESOURCE_H

#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/Texture.h>

namespace Rocket {
namespace Core {

/**
	A texture resource stores application-generated texture data (handle and dimensions) for each
	unique render interface that needs to render the data. It is used through a Texture object.

	@author Peter Curry
 */

class TextureResource : public ReferenceCountable
{
friend class TextureDatabase;

public:
	virtual ~TextureResource();

	/// Attempts to load a texture from the application into the resource. Note that this always
	/// succeeds now; as texture loading is now delayed until the texture is accessed by a specific
	/// render interface, all this does is store the source.
	bool Load(const String& source);

	/// Returns the resource's underlying texture handle.
	TextureHandle GetHandle(RenderInterface* render_interface) const;
	/// Returns the dimensions of the resource's texture.
	const Vector2i& GetDimensions(RenderInterface* render_interface) const;

	/// Returns the resource's source.
	const String& GetSource() const;

	/// Releases the texture's handle.
	void Release(RenderInterface* render_interface = NULL);

protected:
	/// Attempts to load the texture from the source.
	bool Load(RenderInterface* render_interface) const;

	/// Releases the texture and destroys the resource.
	virtual void OnReferenceDeactivate();

private:
	TextureResource();

	String source;

	typedef std::pair< TextureHandle, Vector2i > TextureData;
	typedef std::map< RenderInterface*, TextureData > TextureDataMap;
	mutable TextureDataMap texture_data;
};

}
}

#endif
