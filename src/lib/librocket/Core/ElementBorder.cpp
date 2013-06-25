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
#include "ElementBorder.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/Property.h>

namespace Rocket {
namespace Core {

ElementBorder::ElementBorder(Element* _element) : geometry(_element)
{
	element = _element;
	border_dirty = true;
}

ElementBorder::~ElementBorder()
{
}

// Renders the element's border, if it has one.
void ElementBorder::RenderBorder()
{
	if (border_dirty)
	{
		border_dirty = false;
		GenerateBorder();
	}

	geometry.Render(element->GetAbsoluteOffset(Box::BORDER));
}

// Marks the border geometry as dirty.
void ElementBorder::DirtyBorder()
{
	border_dirty = true;
}

// Generates the border geometry for the element.
void ElementBorder::GenerateBorder()
{
	int num_edges = 0;

	for (int i = 0; i < element->GetNumBoxes(); ++i)
	{
		const Box& box = element->GetBox(i);
		for (int j = 0; j < 4; j++)
		{
			if (box.GetEdge(Box::BORDER, (Box::Edge) j) > 0)
				num_edges++;
		}
	}

	std::vector< Vertex >& vertices = geometry.GetVertices();
	std::vector< int >& indices = geometry.GetIndices();

	int index_offset = 0;
	vertices.resize(4 * num_edges);
	indices.resize(6 * num_edges);

	if (num_edges > 0)
	{
		Vertex* raw_vertices = &vertices[0];
		int* raw_indices = &indices[0];

		Colourb border_colours[4];
		border_colours[0] = element->GetProperty(BORDER_TOP_COLOR)->value.Get< Colourb >();
		border_colours[1] = element->GetProperty(BORDER_RIGHT_COLOR)->value.Get< Colourb >();
		border_colours[2] = element->GetProperty(BORDER_BOTTOM_COLOR)->value.Get< Colourb >();
		border_colours[3] = element->GetProperty(BORDER_LEFT_COLOR)->value.Get< Colourb >();

		for (int i = 0; i < element->GetNumBoxes(); ++i)
			GenerateBorder(raw_vertices, raw_indices, index_offset, element->GetBox(i), border_colours);
	}

	geometry.Release();
}

// Generates the border geometry for a single box.
void ElementBorder::GenerateBorder(Vertex*& vertices, int*& indices, int& index_offset, const Box& box, const Colourb* colours)
{
	// The axis of extrusion for each of the edges.
	Vector2f box_extrusions[4] =
	{
		Vector2f(0, -1 * box.GetEdge(Box::BORDER, Box::TOP)),
		Vector2f(box.GetEdge(Box::BORDER, Box::RIGHT), 0),
		Vector2f(0, box.GetEdge(Box::BORDER, Box::BOTTOM)),
		Vector2f(-1 * box.GetEdge(Box::BORDER, Box::LEFT), 0)
	};

	// The position of each of the corners of the inner border.
	Vector2f box_corners[4];
	box_corners[0] = box.GetPosition(Box::PADDING);
	box_corners[2] = box_corners[0] + box.GetSize(Box::PADDING);
	box_corners[1] = Vector2f(box_corners[2].x, box_corners[0].y);
	box_corners[3] = Vector2f(box_corners[0].x, box_corners[2].y);

	for (int i = 0; i < 4; i++)
	{
		float border_width = box.GetEdge(Box::BORDER, (Box::Edge) i);
		if (border_width <= 0)
			continue;

		vertices[0].position = box_corners[i];
		vertices[1].position = box_corners[i] + box_extrusions[i] + box_extrusions[i == 0 ? 3 : i - 1];
		vertices[2].position = box_corners[i == 3 ? 0 : i + 1];
		vertices[3].position = vertices[2].position + box_extrusions[i] + box_extrusions[i == 3 ? 0 : i + 1];

		vertices[0].colour = colours[i];
		vertices[1].colour = colours[i];
		vertices[2].colour = colours[i];
		vertices[3].colour = colours[i];

		indices[0] = index_offset;
		indices[1] = index_offset + 3;
		indices[2] = index_offset + 1;
		indices[3] = index_offset;
		indices[4] = index_offset + 2;
		indices[5] = index_offset + 3;

		vertices += 4;
		indices += 6;
		index_offset += 4;
	}
}

}
}
