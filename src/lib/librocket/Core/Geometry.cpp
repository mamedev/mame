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
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core.h>
#include "GeometryDatabase.h"

namespace Rocket {
namespace Core {

static bool read_texel_offset = false;
static Vector2f texel_offset;

Geometry::Geometry(Element* _host_element)
{
	host_element = _host_element;
	host_context = NULL;

	GeometryDatabase::AddGeometry(this);

	texture = NULL;

	fixed_texcoords = false;
	compile_attempted = false;
	compiled_geometry = NULL;
}

Geometry::Geometry(Context* _host_context)
{
	host_element = NULL;
	host_context = _host_context;

	GeometryDatabase::AddGeometry(this);

	texture = NULL;

	fixed_texcoords = false;
	compile_attempted = false;
	compiled_geometry = NULL;
}

Geometry::~Geometry()
{
	GeometryDatabase::RemoveGeometry(this);

	Release();
}

// Set the host element for this geometry; this should be passed in the constructor if possible.
void Geometry::SetHostElement(Element* _host_element)
{
	if (host_element == _host_element)
		return;

	if (host_element != NULL)
	{
		Release();
		host_context = NULL;
	}

	host_element = _host_element;
}

void Geometry::Render(const Vector2f& translation)
{
	RenderInterface* render_interface = GetRenderInterface();
	if (render_interface == NULL)
		return;

	// Render our compiled geometry if possible.
	if (compiled_geometry)
	{
		render_interface->RenderCompiledGeometry(compiled_geometry, translation);
	}
	// Otherwise, if we actually have geometry, try to compile it if we haven't already done so, otherwise render it in
	// immediate mode.
	else
	{
		if (vertices.empty() ||
			indices.empty())
			return;

		if (!compile_attempted)
		{
			if (!fixed_texcoords)
			{
				fixed_texcoords = true;

				if (!read_texel_offset)
				{
					read_texel_offset = true;
					texel_offset.x = render_interface->GetHorizontalTexelOffset();
					texel_offset.y = render_interface->GetVerticalTexelOffset();
				}

				// Add a half-texel offset if required.
				if (texel_offset.x != 0 ||
					texel_offset.y != 0)
				{
					for (size_t i = 0; i < vertices.size(); ++i)
						vertices[i].position += texel_offset;
				}
			}

			compile_attempted = true;
			compiled_geometry = render_interface->CompileGeometry(&vertices[0], (int) vertices.size(), &indices[0], (int) indices.size(), texture != NULL ? texture->GetHandle(GetRenderInterface()) : NULL);

			// If we managed to compile the geometry, we can clear the local copy of vertices and indices and
			// immediately render the compiled version.
			if (compiled_geometry)
			{	
				render_interface->RenderCompiledGeometry(compiled_geometry, translation);
				return;
			}
		}

		// Either we've attempted to compile before (and failed), or the compile we just attempted failed; either way,
		// render the uncompiled version.
		render_interface->RenderGeometry(&vertices[0], (int) vertices.size(), &indices[0], (int) indices.size(), texture != NULL ? texture->GetHandle(GetRenderInterface()) : NULL, translation);
	}
}

// Returns the geometry's vertices. If these are written to, Release() should be called to force a recompile.
std::vector< Vertex >& Geometry::GetVertices()
{
	return vertices;
}

// Returns the geometry's indices. If these are written to, Release() should be called to force a recompile.
std::vector< int >& Geometry::GetIndices()
{
	return indices;
}

// Gets the geometry's texture.
const Texture* Geometry::GetTexture() const
{
	return texture;
}

// Sets the geometry's texture.
void Geometry::SetTexture(const Texture* _texture)
{
	texture = _texture;
	Release();
}

void Geometry::Release(bool clear_buffers)
{
	if (compiled_geometry)
	{
		GetRenderInterface()->ReleaseCompiledGeometry(compiled_geometry);
		compiled_geometry = NULL;
	}

	compile_attempted = false;

	if (clear_buffers)
	{
		vertices.clear();
		indices.clear();
		fixed_texcoords = false;
	}
}

// Returns the host context's render interface.
RenderInterface* Geometry::GetRenderInterface()
{
	if (host_context == NULL)
	{
		if (host_element != NULL)
			host_context = host_element->GetContext();
	}

	if (host_context == NULL)
		return Rocket::Core::GetRenderInterface();
	else
		return host_context->GetRenderInterface();
}

}
}
