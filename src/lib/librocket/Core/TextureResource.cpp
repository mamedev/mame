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
#include "TextureResource.h"
#include "FontFaceHandle.h"
#include "TextureDatabase.h"
#include <Rocket/Core.h>

namespace Rocket {
namespace Core {

TextureResource::TextureResource()
{
}

TextureResource::~TextureResource()
{
	TextureDatabase::RemoveTexture(this);
}

// Attempts to load a texture from the application into the resource.
bool TextureResource::Load(const String& _source)
{
	Release();
	source = _source;

	return true;
}

// Returns the resource's underlying texture.
TextureHandle TextureResource::GetHandle(RenderInterface* render_interface) const
{
	TextureDataMap::iterator texture_iterator = texture_data.find(render_interface);
	if (texture_iterator == texture_data.end())
	{
		Load(render_interface);
		texture_iterator = texture_data.find(render_interface);
	}

	return texture_iterator->second.first;
}

// Returns the dimensions of the resource's texture.
const Vector2i& TextureResource::GetDimensions(RenderInterface* render_interface) const
{
	TextureDataMap::iterator texture_iterator = texture_data.find(render_interface);
	if (texture_iterator == texture_data.end())
	{
		Load(render_interface);
		texture_iterator = texture_data.find(render_interface);
	}

	return texture_iterator->second.second;
}

// Returns the resource's source.
const String& TextureResource::GetSource() const
{
	return source;
}

// Releases the texture's handle.
void TextureResource::Release(RenderInterface* render_interface)
{
	if (render_interface == NULL)
	{
		for (TextureDataMap::iterator texture_iterator = texture_data.begin(); texture_iterator != texture_data.end(); ++texture_iterator)
		{
			TextureHandle handle = texture_iterator->second.first;
			if (handle)
				texture_iterator->first->ReleaseTexture(handle);
		}

		texture_data.clear();
	}
	else
	{
		TextureDataMap::iterator texture_iterator = texture_data.find(render_interface);
		if (texture_iterator == texture_data.end())
			return;

		TextureHandle handle = texture_iterator->second.first;
		if (handle)
			texture_iterator->first->ReleaseTexture(handle);

		texture_data.erase(render_interface);
	}
}

// Attempts to load the texture from the source.
bool TextureResource::Load(RenderInterface* render_interface) const
{
	// Check for special loader tokens.
	if (!source.Empty() &&
		source[0] == '?')
	{
		Vector2i dimensions;

		bool delete_data = false;
		const byte* data = NULL;

		// Find the generation protocol and generate the data accordingly.
		String protocol = source.Substring(1, source.Find("::") - 1);
		if (protocol == "font")
		{
			// The requested texture is a font layer.
			delete_data = true;
			
			FontFaceHandle* handle;
			FontEffect* layer_id;
			int texture_id;
			
			if (sscanf(source.CString(), "?font::%p/%p/%d", &handle, &layer_id, &texture_id) == 3)
			{
				handle->GenerateLayerTexture(data,
											 dimensions,
											 layer_id,
											 texture_id);
			}
		}

		// If texture data was generated, great! Otherwise, fallback to the LoadTexture() code and
		// hope the client knows what the hell to do with the question mark in their file name.
		if (data != NULL)
		{
			TextureHandle handle;
			bool success = render_interface->GenerateTexture(handle, data, dimensions);

			if (delete_data)
				delete[] data;

			if (success)
			{
				texture_data[render_interface] = TextureData(handle, dimensions);
				return true;
			}
			else
			{
				Log::Message(Log::LT_WARNING, "Failed to generate internal texture %s.", source.CString());
				//texture_data[render_interface] = TextureData(NULL, Vector2i(0, 0));

				return false;
			}
		}
	}

	TextureHandle handle;
	Vector2i dimensions;
	if (!render_interface->LoadTexture(handle, dimensions, source))
	{
		Log::Message(Log::LT_WARNING, "Failed to load texture from %s.", source.CString());
		//texture_data[render_interface] = TextureData(NULL, Vector2i(0, 0));

		return false;
	}

	texture_data[render_interface] = TextureData(handle, dimensions);
	return true;
}

void TextureResource::OnReferenceDeactivate()
{
	Release();
	delete this;
}

}
}
