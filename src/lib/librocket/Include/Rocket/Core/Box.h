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

#ifndef ROCKETCOREBOX_H
#define ROCKETCOREBOX_H

//#include <Rocket/Core/Header.h>
//#include <Rocket/Core/Types.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

/**
	Stores a box with four sized areas; content, padding, a border and margin. See
	http://www.w3.org/TR/REC-CSS2/box.html#box-dimensions for a diagram.

	@author Peter Curry
 */

class ROCKETCORE_API Box
{
public:
	enum Area
	{
		MARGIN = 0,
		BORDER = 1,
		PADDING = 2,
		CONTENT = 3,
		NUM_AREAS = 3,		// ignores CONTENT
	};

	enum Edge
	{
		TOP = 0,
		RIGHT = 1,
		BOTTOM = 2,
		LEFT = 3,
		NUM_EDGES = 4
	};

	/// Initialises a zero-sized box.
	Box();
	/// Initialises a box with a default content area and no padding, borders and margins.
	Box(const Vector2f& content);
	~Box();

	/// Returns the offset of this box. This will usually be (0, 0).
	/// @return The box's offset.
	const Vector2f& GetOffset() const;
	/// Returns the top-left position of one of the box's areas, relative to the top-left of the border area. This
	/// means the position of the margin area is likely to be negative.
	/// @param area[in] The desired area.
	/// @return The position of the area.
	Vector2f GetPosition(Area area = Box::CONTENT) const;
	/// Returns the size of one of the box's areas. This will include all inner areas.
	/// @param area[in] The desired area.
	/// @return The size of the requested area.
	Vector2f GetSize(Area area = Box::CONTENT) const;

	/// Sets the offset of the box, relative usually to the owning element. This should only be set for auxiliary
	/// boxes of an element.
	/// @param offset[in] The offset of the box from the primary box.
	void SetOffset(const Vector2f& offset);
	/// Sets the size of the content area.
	/// @param content[in] The size of the new content area.
	void SetContent(const Vector2f& content);
	/// Sets the size of one of the edges of one of the box's outer areas.
	/// @param area[in] The area to change.
	/// @param edge[in] The area edge to change.
	/// @param size[in] The new size of the area segment.
	void SetEdge(Area area, Edge edge, float size);

	/// Returns the size of one of the area edges.
	/// @param area[in] The desired area.
	/// @param edge[in] The desired edge.
	/// @return The size of the requested area edge.
	float GetEdge(Area area, Edge edge) const;
	/// Returns the cumulative size of one edge up to one of the box's areas.
	/// @param area[in] The area to measure up to (and including). So, MARGIN will return the width of the margin, and PADDING will be the sum of the margin, border and padding.
	/// @param edge[in] The desired edge.
	/// @return The cumulative size of the edge.
	float GetCumulativeEdge(Area area, Edge edge) const;

	/// Compares the size of the content area and the other area edges.
	/// @return True if the boxes represent the same area.
	bool operator==(const Box& rhs) const;
	/// Compares the size of the content area and the other area edges.
	/// @return True if the boxes do not represent the same area.
	bool operator!=(const Box& rhs) const;

private:
	Vector2f content;
	float area_edges[NUM_AREAS][NUM_EDGES];

	Vector2f offset;
};

}
}

#endif
