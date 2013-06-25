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
#include <Rocket/Core/Texture.h>
#include "TextureDatabase.h"
#include "TextureResource.h"

namespace Rocket {
namespace Core {

// Constructs an unloaded texture with no resource.
Texture::Texture()
{
	resource = NULL;
}

// Constructs a texture sharing the resource of another.
Texture::Texture(const Texture& copy)
{
	resource = NULL;
	*this = copy;
}

Texture::~Texture()
{
	if (resource)
		resource->RemoveReference();
}

// Attempts to load a texture.
bool Texture::Load(const String& source, const String& source_path)
{
	if (resource != NULL)
		resource->RemoveReference();

	resource = TextureDatabase::Fetch(source, source_path);
	return resource != NULL;
}

// Returns the texture's source name. This is usually the name of the file the texture was loaded from.
String Texture::GetSource() const
{
	if (resource == NULL)
		return NULL;

	return resource->GetSource();
}

// Returns the texture's handle. 
TextureHandle Texture::GetHandle(RenderInterface* render_interface) const
{
	if (resource == NULL)
		return NULL;

	return resource->GetHandle(render_interface);
}

// Returns the texture's dimensions.
Vector2i Texture::GetDimensions(RenderInterface* render_interface) const
{
	if (resource == NULL)
		return Vector2i(0, 0);

	return resource->GetDimensions(render_interface);
}

// Releases this texture's resource (if any), and sets it to another texture's resource.
const Texture& Texture::operator=(const Texture& copy)
{
	if (resource != NULL)
		resource->RemoveReference();

	resource = copy.resource;
	if (resource != NULL)
		resource->AddReference();

	return *this;
}

}
}
