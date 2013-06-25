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
#include <Rocket/Core/Box.h>

namespace Rocket {
namespace Core {

// Initialises a zero-sized box.
Box::Box() : content(0, 0), offset(0, 0)
{
	memset(area_edges, 0, sizeof(area_edges));
}

// Initialises a box with a default content area and no padding, borders and margins.
Box::Box(const Vector2f& content) : content(content), offset(0, 0)
{
	memset(area_edges, 0, sizeof(area_edges));
}

Box::~Box()
{
}

// Returns the offset of this box. This will usually be (0, 0).
const Vector2f& Box::GetOffset() const
{
	return offset;
}

// Returns the top-left position of one of the areas.
Vector2f Box::GetPosition(Area area) const
{
	Vector2f area_position(offset.x - area_edges[MARGIN][LEFT], offset.y - area_edges[MARGIN][TOP]);
	for (int i = 0; i < area; i++)
	{
		area_position.x += area_edges[i][LEFT];
		area_position.y += area_edges[i][TOP];
	}

	return area_position;
}

// Returns the size of one of the box's areas. This will include all inner areas.
Vector2f Box::GetSize(Area area) const
{
	Vector2f area_size(content);
	for (int i = PADDING; i >= area; i--)
	{
		area_size.x += (area_edges[i][LEFT] + area_edges[i][RIGHT]);
		area_size.y += (area_edges[i][TOP] + area_edges[i][BOTTOM]);
	}

	return area_size;
}

// Sets the offset of the box, relative usually to the owning element.
void Box::SetOffset(const Vector2f& _offset)
{
	offset = _offset;
}

// Sets the size of the content area.
void Box::SetContent(const Vector2f& _content)
{
	content = _content;
}

// Sets the size of one of the segments of one of the box's outer areas.
void Box::SetEdge(Area area, Edge edge, float size)
{
	area_edges[area][edge] = size;
}

// Returns the size of one of the area segments.
float Box::GetEdge(Area area, Edge edge) const
{
	return area_edges[area][edge];
}

// Returns the cumulative size of one edge up to one of the box's areas.
float Box::GetCumulativeEdge(Area area, Edge edge) const
{
	float size = 0;
	int max_area = Math::Min((int) area, 2);
	for (int i = 0; i <= max_area; i++)
		size += area_edges[i][edge];

	return size;
}

// Compares the size of the content area and the other area edges.
bool Box::operator==(const Box& rhs) const
{
	return content == rhs.content && memcmp(area_edges, rhs.area_edges, sizeof(area_edges)) == 0;
}

// Compares the size of the content area and the other area edges.
bool Box::operator!=(const Box& rhs) const
{
	return !(*this == rhs);
}

}
}
