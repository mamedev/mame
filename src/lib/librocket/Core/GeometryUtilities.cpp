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
#include <Rocket/Core/GeometryUtilities.h>

namespace Rocket {
namespace Core {

GeometryUtilities::GeometryUtilities()
{
}

GeometryUtilities::~GeometryUtilities()
{
}

// Generates a quad from a position, size and colour.
void GeometryUtilities::GenerateQuad(Vertex* vertices, int* indices, const Vector2f& origin, const Vector2f& dimensions, const Colourb& colour, int index_offset)
{
	GenerateQuad(vertices, indices, origin, dimensions, colour, Vector2f(0, 0), Vector2f(1, 1), index_offset);
}

// Generates a quad from a position, size, colour and texture coordinates.
void GeometryUtilities::GenerateQuad(Vertex* vertices, int* indices, const Vector2f& origin, const Vector2f& dimensions, const Colourb& colour, const Vector2f& top_left_texcoord, const Vector2f& bottom_right_texcoord, int index_offset)
{
	vertices[0].position = origin;
	vertices[0].colour = colour;
	vertices[0].tex_coord = top_left_texcoord;

	vertices[1].position = Vector2f(origin.x + dimensions.x, origin.y);
	vertices[1].colour = colour;
	vertices[1].tex_coord = Vector2f(bottom_right_texcoord.x, top_left_texcoord.y);

	vertices[2].position = origin + dimensions;
	vertices[2].colour = colour;
	vertices[2].tex_coord = bottom_right_texcoord;

	vertices[3].position = Vector2f(origin.x, origin.y + dimensions.y);
	vertices[3].colour = colour;
	vertices[3].tex_coord = Vector2f(top_left_texcoord.x, bottom_right_texcoord.y);

	indices[0] = index_offset + 0;
	indices[1] = index_offset + 3;
	indices[2] = index_offset + 1;

	indices[3] = index_offset + 1;
	indices[4] = index_offset + 3;
	indices[5] = index_offset + 2;
}

}
}
