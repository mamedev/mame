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

#ifndef ROCKETCOREGEOMETRY_H
#define ROCKETCOREGEOMETRY_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Vertex.h>

namespace Rocket {
namespace Core {

class Context;
class Element;
class RenderInterface;
struct Texture;

/**
	A helper object for holding an array of vertices and indices, and compiling it as necessary when rendered.

	@author Peter Curry
 */

class ROCKETCORE_API Geometry
{
public:
	Geometry(Element* host_element = NULL);
	Geometry(Context* host_context);
	~Geometry();

	/// Set the host element for this geometry; this should be passed in the constructor if possible.
	/// @param[in] host_element The new host element for the geometry.
	void SetHostElement(Element* host_element);

	/// Attempts to compile the geometry if appropriate, then renders the geometry, compiled if it can.
	/// @param[in] translation The translation of the geometry.
	void Render(const Vector2f& translation);

	/// Returns the geometry's vertices. If these are written to, Release() should be called to force a recompile.
	/// @return The geometry's vertex array.
	std::vector< Vertex >& GetVertices();
	/// Returns the geometry's indices. If these are written to, Release() should be called to force a recompile.
	/// @return The geometry's index array.
	std::vector< int >& GetIndices();

	/// Gets the geometry's texture.
	/// @return The geometry's texture.
	const Texture* GetTexture() const;
	/// Sets the geometry's texture.
	void SetTexture(const Texture* texture);

	/// Releases any previously-compiled geometry, and forces any new geometry to have a compile attempted.
	/// @param[in] clear_buffers True to also clear the vertex and index buffers, false to leave intact.
	void Release(bool clear_buffers = false);

private:
	// Returns the host context's render interface.
	RenderInterface* GetRenderInterface();

	Context* host_context;
	Element* host_element;

	std::vector< Vertex > vertices;
	std::vector< int > indices;
	const Texture* texture;

	CompiledGeometryHandle compiled_geometry;
	bool compile_attempted;
	bool fixed_texcoords;
};

typedef std::vector< Geometry > GeometryList;

}
}

#endif
