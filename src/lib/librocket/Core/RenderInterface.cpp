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
#include <Rocket/Core/RenderInterface.h>
#include "TextureDatabase.h"

namespace Rocket {
namespace Core {

RenderInterface::RenderInterface() : ReferenceCountable(0)
{
	context = NULL;
}

RenderInterface::~RenderInterface()
{
}

// Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
CompiledGeometryHandle RenderInterface::CompileGeometry(Vertex* ROCKET_UNUSED(vertices), int ROCKET_UNUSED(num_vertices), int* ROCKET_UNUSED(indices), int ROCKET_UNUSED(num_indices), TextureHandle ROCKET_UNUSED(texture))
{
	return NULL;
}

// Called by Rocket when it wants to render application-compiled geometry.
void RenderInterface::RenderCompiledGeometry(CompiledGeometryHandle ROCKET_UNUSED(geometry), const Vector2f& ROCKET_UNUSED(translation))
{
}

// Called by Rocket when it wants to release application-compiled geometry.
void RenderInterface::ReleaseCompiledGeometry(CompiledGeometryHandle ROCKET_UNUSED(geometry))
{
}

// Called by Rocket when a texture is required by the library.
bool RenderInterface::LoadTexture(TextureHandle& ROCKET_UNUSED(texture_handle), Vector2i& ROCKET_UNUSED(texture_dimensions), const String& ROCKET_UNUSED(source))
{
	return false;
}

// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
bool RenderInterface::GenerateTexture(TextureHandle& ROCKET_UNUSED(texture_handle), const byte* ROCKET_UNUSED(source), const Vector2i& ROCKET_UNUSED(source_dimensions))
{
	return false;
}

// Called by Rocket when a loaded texture is no longer required.
void RenderInterface::ReleaseTexture(TextureHandle ROCKET_UNUSED(texture))
{
}

// Returns the native horizontal texel offset for the renderer.
float RenderInterface::GetHorizontalTexelOffset()
{
	return 0;
}

// Returns the native vertical texel offset for the renderer.
float RenderInterface::GetVerticalTexelOffset()
{
	return 0;
}

// Called when this render interface is released.
void RenderInterface::Release()
{
}

void RenderInterface::OnReferenceDeactivate()
{
	TextureDatabase::ReleaseTextures(this);
	Release();
}

// Get the context currently being rendered.
Context* RenderInterface::GetContext() const
{
	return context;
}

}
}
