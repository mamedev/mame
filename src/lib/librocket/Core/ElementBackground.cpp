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
#include "ElementBackground.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/GeometryUtilities.h>
#include <Rocket/Core/Property.h>

namespace Rocket {
namespace Core {

ElementBackground::ElementBackground(Element* _element) : geometry(_element)
{
	element = _element;
	background_dirty = true;
}

ElementBackground::~ElementBackground()
{
}

// Renders the element's background, if it has one.
void ElementBackground::RenderBackground()
{
	if (background_dirty)
	{
		background_dirty = false;
		GenerateBackground();
	}

	geometry.Render(element->GetAbsoluteOffset(Box::PADDING));
}

// Marks the background geometry as dirty.
void ElementBackground::DirtyBackground()
{
	background_dirty = true;
}

// Generates the background geometry for the element.
void ElementBackground::GenerateBackground()
{
	// Fetch the new colour for the background. If the colour is transparent, then we don't render any background.
	Colourb colour = element->GetProperty(BACKGROUND_COLOR)->value.Get< Colourb >();
	if (colour.alpha <= 0)
	{
		geometry.GetVertices().clear();
		geometry.GetIndices().clear();
		geometry.Release();

		return;
	}

	// Work out how many boxes we need to generate geometry for.
	int num_boxes = 0;

	for (int i = 0; i < element->GetNumBoxes(); ++i)
	{
		const Box& box = element->GetBox(i);
		Vector2f size = box.GetSize(Box::PADDING);
		if (size.x > 0 && size.y > 0)
			num_boxes++;
	}

	std::vector< Vertex >& vertices = geometry.GetVertices();
	std::vector< int >& indices = geometry.GetIndices();

	int index_offset = 0;
	vertices.resize(4 * num_boxes);
	indices.resize(6 * num_boxes);

	if (num_boxes > 0)
	{
		Vertex* raw_vertices = &vertices[0];
		int* raw_indices = &indices[0];

		for (int i = 0; i < element->GetNumBoxes(); ++i)
			GenerateBackground(raw_vertices, raw_indices, index_offset, element->GetBox(i), colour);
	}

	geometry.Release();
}

// Generates the background geometry for a single box.
void ElementBackground::GenerateBackground(Vertex*& vertices, int*& indices, int& index_offset, const Box& box, const Colourb& colour)
{
	Vector2f padded_size = box.GetSize(Box::PADDING);
	if (padded_size.x <= 0 ||
		padded_size.y <= 0)
		return;

	GeometryUtilities::GenerateQuad(vertices, indices, box.GetOffset(), padded_size, colour, index_offset);

	vertices += 4;
	indices += 6;
	index_offset += 4;
}

}
}
