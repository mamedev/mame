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
#include "LayoutBlockBoxSpace.h"
#include "LayoutBlockBox.h"
#include "LayoutEngine.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementScroll.h>
#include <Rocket/Core/StyleSheetKeywords.h>

namespace Rocket {
namespace Core {

LayoutBlockBoxSpace::LayoutBlockBoxSpace(LayoutBlockBox* _parent) : offset(0, 0), dimensions(0, 0)
{
	parent = _parent;
}

LayoutBlockBoxSpace::~LayoutBlockBoxSpace()
{
}

// Imports boxes from another block into this space.
void LayoutBlockBoxSpace::ImportSpace(const LayoutBlockBoxSpace& space)
{
	// Copy all the boxes from the parent into this space. Could do some optimisation here!
	for (int i = 0; i < NUM_ANCHOR_EDGES; ++i)
	{
		for (size_t j = 0; j < space.boxes[i].size(); ++j)
			boxes[i].push_back(space.boxes[i][j]);
	}
}

// Generates the position for a box of a given size within a containing block box.
void LayoutBlockBoxSpace::PositionBox(Vector2f& box_position, float& box_width, float cursor, const Vector2f& dimensions) const
{
	box_width = PositionBox(box_position, cursor, dimensions);
}

// Generates and sets the position for a floating box of a given size within our block box.
float LayoutBlockBoxSpace::PositionBox(float cursor, Element* element)
{
	Vector2f element_size = element->GetBox().GetSize(Box::MARGIN);
	int float_property = element->GetProperty< int >(FLOAT);

	// Shift the cursor down (if necessary) so it isn't placed any higher than a previously-floated box.
	for (int i = 0; i < NUM_ANCHOR_EDGES; ++i)
	{
		if (!boxes[i].empty())
			cursor = Math::Max(cursor, boxes[i].back().offset.y);
	}

	// Shift the cursor down past to clear boxes, if necessary.
	cursor = ClearBoxes(cursor, element->GetProperty< int >(CLEAR));

	// Find a place to put this box.
	Vector2f element_offset;
	PositionBox(element_offset, cursor, element_size, float_property);

	// It's been placed, so we can now add it to our list of floating boxes.
	boxes[float_property == FLOAT_LEFT ? LEFT : RIGHT].push_back(SpaceBox(element_offset, element_size));

	// Set our offset and dimensions (if necessary) so they enclose the new box.
	Vector2f normalised_offset = element_offset - (parent->GetPosition() + parent->GetBox().GetPosition());
	offset.x = Math::Min(offset.x, normalised_offset.x);
	offset.y = Math::Min(offset.y, normalised_offset.y);
	dimensions.x = Math::Max(dimensions.x, normalised_offset.x + element_size.x);
	dimensions.y = Math::Max(dimensions.y, normalised_offset.y + element_size.y);

	// Shift the offset into the correct space relative to the element's offset parent.
	element_offset += Vector2f(element->GetBox().GetEdge(Box::MARGIN, Box::LEFT), element->GetBox().GetEdge(Box::MARGIN, Box::TOP));
	element->SetOffset(element_offset - parent->GetOffsetParent()->GetPosition(), parent->GetOffsetParent()->GetElement());

	return element_offset.y + element_size.y;
}

// Determines the appropriate vertical position for an object that is choosing to clear floating elements to the left
// or right (or both).
float LayoutBlockBoxSpace::ClearBoxes(float cursor, int clear_property)
{
	// Clear left boxes.
	if (clear_property == CLEAR_LEFT ||
		clear_property == CLEAR_BOTH)
	{
		for (size_t i = 0; i < boxes[LEFT].size(); ++i)
			cursor = Math::Max(cursor, boxes[LEFT][i].offset.y + boxes[LEFT][i].dimensions.y);
	}

	// Clear right boxes.
	if (clear_property == CLEAR_RIGHT ||
		clear_property == CLEAR_BOTH)
	{
		for (size_t i = 0; i < boxes[RIGHT].size(); ++i)
			cursor = Math::Max(cursor, boxes[RIGHT][i].offset.y + boxes[RIGHT][i].dimensions.y);
	}

	return cursor;
}

// Generates the position for an arbitrary box within our space layout, floated against either the left or right edge.
float LayoutBlockBoxSpace::PositionBox(Vector2f& box_position, float cursor, const Vector2f& dimensions, int float_property) const
{
	float parent_scrollbar_width = parent->GetElement()->GetElementScroll()->GetScrollbarSize(ElementScroll::VERTICAL);
	float parent_origin = parent->GetPosition().x + parent->GetBox().GetPosition(Box::CONTENT).x;
	float parent_edge = parent->GetBox().GetSize().x + parent_origin - parent_scrollbar_width;

	AnchorEdge box_edge = float_property == FLOAT_RIGHT ? RIGHT : LEFT;

	box_position.y = cursor;
	box_position.x = box_edge == LEFT ? 0 : (parent->GetBox().GetSize().x - dimensions.x) - parent_scrollbar_width;
	box_position.x += parent_origin;

	float next_cursor = FLT_MAX;

	// First up; we iterate through all boxes that share our edge, pushing ourself to the side of them if we intersect
	// them. We record the height of the lowest box that gets in our way; in the event we can't be positioned at this
	// height, we'll reposition ourselves at that height for the next iteration.
	for (size_t i = 0; i < boxes[box_edge].size(); ++i)
	{
		const SpaceBox& fixed_box = boxes[box_edge][i];

		// If the fixed box's bottom edge is above our top edge, then we can safely skip it.
		if (fixed_box.offset.y + fixed_box.dimensions.y <= box_position.y)
			continue;

		// If the fixed box's top edge is below our bottom edge, then we can safely skip it.
		if (fixed_box.offset.y >= box_position.y + dimensions.y)
			continue;

		// We're intersecting this box vertically, so the box is pushed to the side if necessary.
		bool collision = false;
		if (box_edge == LEFT)
		{
			float right_edge = fixed_box.offset.x + fixed_box.dimensions.x;
			collision = box_position.x < right_edge;
			if (collision)
				box_position.x = right_edge;
		}
		else
		{
			collision = box_position.x + dimensions.x > fixed_box.offset.x;
			if (collision)
				box_position.x = fixed_box.offset.x - dimensions.x;
		}

		// If there was a collision, then we *might* want to remember the height of this box if it is the earliest-
		// terminating box we've collided with so far.
		if (collision)
		{
			next_cursor = Math::Min(next_cursor, fixed_box.offset.y + fixed_box.dimensions.y);

			// Were we pushed out of our containing box? If so, try again at the next cursor position.
			float normalised_position = box_position.x - parent_origin;
			if (normalised_position < 0 ||
				normalised_position + dimensions.x > parent->GetBox().GetSize().x)
				return PositionBox(box_position, next_cursor + 0.01f, dimensions, float_property);
		}
	}

	// Second; we go through all of the boxes on the other edge, checking for horizontal collisions and determining the
	// maximum width the box can stretch to, if it is placed at this location.
	float maximum_box_width = box_edge == LEFT ? parent_edge - box_position.x : box_position.x + dimensions.x;

	for (size_t i = 0; i < boxes[1 - box_edge].size(); ++i)
	{
		const SpaceBox& fixed_box = boxes[1 - box_edge][i];

		// If the fixed box's bottom edge is above our top edge, then we can safely skip it.
		if (fixed_box.offset.y + fixed_box.dimensions.y <= box_position.y)
			continue;

		// If the fixed box's top edge is below our bottom edge, then we can safely skip it.
		if (fixed_box.offset.y >= box_position.y + dimensions.y)
			continue;

		// We intersect this box vertically, so check if it intersects horizontally.
		bool collision = false;
		if (box_edge == LEFT)
		{
			maximum_box_width = Math::Min(maximum_box_width, fixed_box.offset.x - box_position.x);
			collision = box_position.x + dimensions.x > fixed_box.offset.x;
		}
		else
		{
			maximum_box_width = Math::Min(maximum_box_width, (box_position.x + dimensions.x) - (fixed_box.offset.x + fixed_box.dimensions.x));
			collision = box_position.x < fixed_box.offset.x + fixed_box.dimensions.x;
		}

		// If we collided with this box ... d'oh! We'll try again lower down the page, at the highest bottom-edge of
		// any of the boxes we've been pushed around by so far.
		if (collision)
		{
			next_cursor = Math::Min(next_cursor, fixed_box.offset.y + fixed_box.dimensions.y);
			return PositionBox(box_position, next_cursor + 0.00001f, dimensions, float_property);
		}
	}

	// Third; we go through all of the boxes (on both sides), checking for vertical collisions.
	for (int i = 0; i < 2; ++i)
	{
		for (size_t j = 0; j < boxes[i].size(); ++j)
		{
			const SpaceBox& fixed_box = boxes[i][j];

			// If the fixed box's bottom edge is above our top edge, then we can safely skip it.
			if (fixed_box.offset.y + fixed_box.dimensions.y <= box_position.y)
				continue;

			// If the fixed box's top edge is below our bottom edge, then we can safely skip it.
			if (fixed_box.offset.y >= box_position.y + dimensions.y)
				continue;

			// We collide vertically; if we also collide horizontally, then we have to try again further down the
			// layout. If the fixed box's left edge is to right of our right edge, then we can safely skip it.
			if (fixed_box.offset.x >= box_position.x + dimensions.x)
				continue;

			// If the fixed box's right edge is to the left of our left edge, then we can safely skip it.
			if (fixed_box.offset.x + fixed_box.dimensions.x <= box_position.x)
				continue;

			// D'oh! We hit this box. Ah well; we'll try again lower down the page, at the highest bottom-edge of any
			// of the boxes we've been pushed around by so far.
			next_cursor = Math::Min(next_cursor, fixed_box.offset.y + fixed_box.dimensions.y);
			return PositionBox(box_position, next_cursor + 0.00001f, dimensions, float_property);
		}
	}

	// Looks like we've found a winner!
	return maximum_box_width;
}

// Returns the top-left offset of the boxes within the space.
const Vector2f& LayoutBlockBoxSpace::GetOffset() const
{
	return offset;
}

// Returns the dimensions of the boxes within the space.
Vector2f LayoutBlockBoxSpace::GetDimensions() const
{
	return dimensions - offset;
}

void* LayoutBlockBoxSpace::operator new(size_t size)
{
	return LayoutEngine::AllocateLayoutChunk(size);
}

void LayoutBlockBoxSpace::operator delete(void* chunk)
{
	LayoutEngine::DeallocateLayoutChunk(chunk);
}

LayoutBlockBoxSpace::SpaceBox::SpaceBox() : offset(0, 0), dimensions(0, 0)
{
}

LayoutBlockBoxSpace::SpaceBox::SpaceBox(const Vector2f& offset, const Vector2f& dimensions) : offset(offset), dimensions(dimensions)
{
}

}
}
