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
#include "LayoutBlockBox.h"
#include "LayoutBlockBoxSpace.h"
#include "LayoutEngine.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/ElementScroll.h>
#include <Rocket/Core/Property.h>
#include <Rocket/Core/StyleSheetKeywords.h>

namespace Rocket {
namespace Core {

// Creates a new block box for rendering a block element.
LayoutBlockBox::LayoutBlockBox(LayoutEngine* _layout_engine, LayoutBlockBox* _parent, Element* _element) : position(0, 0)
{
	space = new LayoutBlockBoxSpace(this);

	layout_engine = _layout_engine;
	parent = _parent;

	context = BLOCK;
	element = _element;
	interrupted_chain = NULL;

	box_cursor = 0;
	vertical_overflow = false;

	// Get our offset root from our parent, if it has one; otherwise, our element is the offset parent.
	if (parent != NULL &&
		parent->offset_root->GetElement() != NULL)
		offset_root = parent->offset_root;
	else
		offset_root = this;

	// Determine the offset parent for this element.
	LayoutBlockBox* self_offset_parent;
	if (parent != NULL &&
		parent->offset_parent->GetElement() != NULL)
		self_offset_parent = parent->offset_parent;
	else
		self_offset_parent = this;

	// Determine the offset parent for our children.
	if (parent != NULL &&
		parent->offset_parent->GetElement() != NULL &&
		(element == NULL || element->GetProperty< int >(POSITION) == POSITION_STATIC))
		offset_parent = parent->offset_parent;
	else
		offset_parent = this;

	// Build the box for our element, and position it if we can.
	if (parent != NULL)
	{
		space->ImportSpace(*parent->space);

		// Build our box if possible; if not, it will have to be set up manually.
		layout_engine->BuildBox(box, min_height, max_height, parent, element);

		// Position ourselves within our containing block (if we have a valid offset parent).
		if (parent->GetElement() != NULL)
		{
			if (self_offset_parent != this)
			{
				// Get the next position within our offset parent's containing block.
				parent->PositionBlockBox(position, box, element->GetProperty< int >(CLEAR));
				element->SetOffset(position - (self_offset_parent->GetPosition() - offset_root->GetPosition()), self_offset_parent->GetElement());
			}
			else
				element->SetOffset(position, NULL);
		}
	}

	if (element != NULL)
	{
		wrap_content = element->GetProperty< int >(WHITE_SPACE) != WHITE_SPACE_NOWRAP;

		// Determine if this element should have scrollbars or not, and create them if so.
		overflow_x_property = element->GetProperty< int >(OVERFLOW_X);
		overflow_y_property = element->GetProperty< int >(OVERFLOW_Y);

		if (overflow_x_property == OVERFLOW_SCROLL)
			element->GetElementScroll()->EnableScrollbar(ElementScroll::HORIZONTAL, box.GetSize(Box::PADDING).x);
		else
			element->GetElementScroll()->DisableScrollbar(ElementScroll::HORIZONTAL);

		if (overflow_y_property == OVERFLOW_SCROLL)
			element->GetElementScroll()->EnableScrollbar(ElementScroll::VERTICAL, box.GetSize(Box::PADDING).x);
		else
			element->GetElementScroll()->DisableScrollbar(ElementScroll::VERTICAL);
	}
	else
	{
		wrap_content = true;
		overflow_x_property = OVERFLOW_VISIBLE;
		overflow_y_property = OVERFLOW_VISIBLE;
	}
}

// Creates a new block box in an inline context.
LayoutBlockBox::LayoutBlockBox(LayoutEngine* _layout_engine, LayoutBlockBox* _parent) : position(-1, -1)
{
	layout_engine = _layout_engine;
	parent = _parent;
	offset_parent = parent->offset_parent;
	offset_root = parent->offset_root;

	space = _parent->space;

	context = INLINE;
	line_boxes.push_back(new LayoutLineBox(this));
	wrap_content = parent->wrap_content;

	element = NULL;
	interrupted_chain = NULL;

	box_cursor = 0;
	vertical_overflow = false;

	layout_engine->BuildBox(box, min_height, max_height, parent, NULL);
	parent->PositionBlockBox(position, box, 0);
	box.SetContent(Vector2f(box.GetSize(Box::CONTENT).x, -1));

	// Reset the min and max heights; they're not valid for inline block boxes.
	min_height = 0;
	max_height = FLT_MAX;
}

// Releases the block box.
LayoutBlockBox::~LayoutBlockBox()
{
	for (size_t i = 0; i < block_boxes.size(); i++)
		delete block_boxes[i];

	for (size_t i = 0; i < line_boxes.size(); i++)
		delete line_boxes[i];

	if (context == BLOCK)
		delete space;
}

// Closes the box.
LayoutBlockBox::CloseResult LayoutBlockBox::Close()
{
	// If the last child of this block box is an inline box, then we haven't closed it; close it now!
	if (context == BLOCK)
	{
		CloseResult result = CloseInlineBlockBox();
		if (result != OK)
			return LAYOUT_SELF;
	}
	// Otherwise, we're an inline context box; so close our last line, which will still be open.
	else
	{
		line_boxes.back()->Close();

		// Expand our content area if any line boxes had to push themselves out.
		Vector2f content_area = box.GetSize();
		for (size_t i = 0; i < line_boxes.size(); i++)
			content_area.x = Math::Max(content_area.x, line_boxes[i]->GetDimensions().x);

		box.SetContent(content_area);
	}

	// Set this box's height, if necessary.
	if (box.GetSize(Box::CONTENT).y < 0)
	{
		Vector2f content_area = box.GetSize();
		content_area.y = Math::Clamp(box_cursor, min_height, max_height);

		if (element != NULL)
			content_area.y = Math::Max(content_area.y, space->GetDimensions().y);

		box.SetContent(content_area);
	}

	// Set the computed box on the element.
	if (element != NULL)
	{
		if (context == BLOCK)
		{
			// Calculate the dimensions of the box's *internal* content; this is the tightest-fitting box around all of the
			// internal elements, plus this element's padding.
			Vector2f content_box(0, 0);

			for (size_t i = 0; i < block_boxes.size(); i++)
				content_box.x = Math::Max(content_box.x, block_boxes[i]->GetBox().GetSize(Box::MARGIN).x);

			// Check how big our floated area is.
			Vector2f space_box = space->GetDimensions();
			content_box.x = Math::Max(content_box.x, space_box.x);

			// If our content is larger than our window, then we can either extend our box, if we're set to not wrap
			// our inline content, or enable the horizontal scrollbar, if we're set to auto-scrollbars. If we're set to
			// always use scrollbars, then the horiontal scrollbar will already have been enabled in the constructor.
			if (content_box.x > box.GetSize().x)
			{
				if (!wrap_content)
					box.SetContent(Vector2f(content_box.x, box.GetSize().y));
				else if (overflow_x_property == OVERFLOW_AUTO)
				{
					element->GetElementScroll()->EnableScrollbar(ElementScroll::HORIZONTAL, box.GetSize(Box::PADDING).x);

					if (!CatchVerticalOverflow())
						return LAYOUT_SELF;
				}
			}

			content_box.x += (box.GetEdge(Box::PADDING, Box::LEFT) + box.GetEdge(Box::PADDING, Box::RIGHT));

			content_box.y = box_cursor;
			content_box.y = Math::Max(content_box.y, space_box.y);
			if (!CatchVerticalOverflow(content_box.y))
				return LAYOUT_SELF;

			content_box.y += (box.GetEdge(Box::PADDING, Box::TOP) + box.GetEdge(Box::PADDING, Box::BOTTOM));

			element->SetBox(box);
			element->SetContentBox(space->GetOffset(), content_box);

			// Format any scrollbars which were enabled on this element.
			element->GetElementScroll()->FormatScrollbars();
		}
		else
			element->SetBox(box);
	}

	// Increment the parent's cursor.
	if (parent != NULL)
	{
		// If this close fails, it means this block box has caused our parent block box to generate an automatic
		// vertical scrollbar.
		if (!parent->CloseBlockBox(this))
			return LAYOUT_PARENT;
	}

	// If we represent a positioned element, then we can now (as we've been sized) act as the containing block for all
	// the absolutely-positioned elements of our descendants.
	if (context == BLOCK &&
		element != NULL)
	{
		if (element->GetProperty< int >(POSITION) != POSITION_STATIC)
			CloseAbsoluteElements();
	}

	return OK;
}

// Called by a closing block box child.
bool LayoutBlockBox::CloseBlockBox(LayoutBlockBox* child)
{
	ROCKET_ASSERT(context == BLOCK);
	box_cursor = (child->GetPosition().y - child->box.GetEdge(Box::MARGIN, Box::TOP) - (box.GetPosition().y + position.y)) + child->GetBox().GetSize(Box::MARGIN).y;

	return CatchVerticalOverflow();
}

// Called by a closing line box child.
LayoutInlineBox* LayoutBlockBox::CloseLineBox(LayoutLineBox* child, LayoutInlineBox* overflow, LayoutInlineBox* overflow_chain)
{
	ROCKET_ASSERT(context == INLINE);
	if (child->GetDimensions().x > 0)
		box_cursor = (child->GetPosition().y - (box.GetPosition().y + position.y)) + child->GetDimensions().y;

	// If we have any pending floating elements for our parent, then this would be an ideal time to position them.
	if (!float_elements.empty())
	{
		for (size_t i = 0; i < float_elements.size(); ++i)
			parent->PositionFloat(float_elements[i], box_cursor);

		float_elements.clear();
	}

	// Add a new line box.
	line_boxes.push_back(new LayoutLineBox(this));

	if (overflow_chain != NULL)
		line_boxes.back()->AddChainedBox(overflow_chain);

	if (overflow != NULL)
		return line_boxes.back()->AddBox(overflow);

	return NULL;
}

// Adds a new block element to this block box.
LayoutBlockBox* LayoutBlockBox::AddBlockElement(Element* element)
{
	ROCKET_ASSERT(context == BLOCK);

	// Check if our most previous block box is rendering in an inline context.
	if (!block_boxes.empty() &&
		block_boxes.back()->context == INLINE)
	{
		LayoutBlockBox* inline_block_box = block_boxes.back();
		LayoutInlineBox* open_inline_box = inline_block_box->line_boxes.back()->GetOpenInlineBox();
		if (open_inline_box != NULL)
		{
			// There's an open inline box chain, which means this block element is parented to it. The chain needs to
			// be positioned (if it hasn't already), closed and duplicated after this block box closes. Also, this
			// block needs to be aware of its parentage, so it can correctly compute its relative position. First of
			// all, we need to close the inline box; this will position the last line if necessary, but it will also
			// create a new line in the inline block box; we want this line to be in an inline box after our block
			// element.
			if (inline_block_box->Close() != OK)
				return NULL;

			interrupted_chain = open_inline_box;
		}
		else
		{
			// There are no open inline boxes, so this inline box just needs to be closed.
			if (CloseInlineBlockBox() != OK)
				return NULL;
		}
	}

	block_boxes.push_back(new LayoutBlockBox(layout_engine, this, element));
	return block_boxes.back();
}

// Adds a new inline element to this inline box.
LayoutInlineBox* LayoutBlockBox::AddInlineElement(Element* element, const Box& box)
{
	if (context == BLOCK)
	{
		LayoutInlineBox* inline_box;

		// If we have an open child rendering in an inline context, we can add this element into it.
		if (!block_boxes.empty() &&
			block_boxes.back()->context == INLINE)
			inline_box = block_boxes.back()->AddInlineElement(element, box);

		// No dice! Ah well, nothing for it but to open a new inline context block box.
		else
		{
			block_boxes.push_back(new LayoutBlockBox(layout_engine, this));

			if (interrupted_chain != NULL)
			{
				block_boxes.back()->line_boxes.back()->AddChainedBox(interrupted_chain);
				interrupted_chain = NULL;
			}

			inline_box = block_boxes.back()->AddInlineElement(element, box);
		}

		return inline_box;
	}
	else
	{
		// We're an inline context box, so we'll add this new inline element into our line boxes.
		return line_boxes.back()->AddElement(element, box);
	}
}

// Adds a line-break to this block box.
void LayoutBlockBox::AddBreak()
{
	int line_height = ElementUtilities::GetLineHeight(element);

	// Check for an inline box as our last child; if so, we can simply end its line and bail.
	if (!block_boxes.empty())
	{
		LayoutBlockBox* block_box = block_boxes.back();
		if (block_box->context == INLINE)
		{
			LayoutLineBox* last_line = block_box->line_boxes.back();
			if (last_line->GetDimensions().y < 0)
				block_box->box_cursor += line_height;
			else
				last_line->Close();

			return;
		}
	}

	// No inline box as our last child; no problem, just increment the cursor by the line height of this element.
	box_cursor += line_height;
}

// Adds an element to this block box to be handled as a floating element.
bool LayoutBlockBox::AddFloatElement(Element* element)
{
	// If we have an open inline block box, then we have to position the box a little differently.
	if (!block_boxes.empty() &&
		block_boxes.back()->context == INLINE)
		block_boxes.back()->float_elements.push_back(element);

	// Nope ... just place it!
	else
		PositionFloat(element);

	return true;
}

// Adds an element to this block box to be handled as an absolutely-positioned element.
void LayoutBlockBox::AddAbsoluteElement(Element* element)
{
	ROCKET_ASSERT(context == BLOCK);

	AbsoluteElement absolute_element;
	absolute_element.element = element;

	PositionBox(absolute_element.position, 0);

	// If we have an open inline-context block box as our last child, then the absolute element must appear after it,
	// but not actually close the box.
	if (!block_boxes.empty()
		&& block_boxes.back()->context == INLINE)
	{
		LayoutBlockBox* inline_context_box = block_boxes.back();
		float last_line_height = inline_context_box->line_boxes.back()->GetDimensions().y;

		absolute_element.position.y += (inline_context_box->box_cursor + Math::Max(0.0f, last_line_height));
	}

	// Find the positioned parent for this element.
	LayoutBlockBox* absolute_parent = this;
	while (absolute_parent != absolute_parent->offset_parent)
		absolute_parent = absolute_parent->parent;

	absolute_parent->absolute_elements.push_back(absolute_element);
}

// Lays out, sizes, and positions all absolute elements in this block relative to the containing block.
void LayoutBlockBox::CloseAbsoluteElements()
{
	if (!absolute_elements.empty())
	{
		// The size of the containing box, including the padding. This is used to resolve relative offsets.
		Vector2f containing_block = GetBox().GetSize(Box::PADDING);

		for (size_t i = 0; i < absolute_elements.size(); i++)
		{
			Element* absolute_element = absolute_elements[i].element;
			Vector2f absolute_position = absolute_elements[i].position;
			absolute_position -= position - offset_root->GetPosition();

			// Lay out the element.
			LayoutEngine layout_engine;
			layout_engine.FormatElement(absolute_element, containing_block);

			// Now that the element's box has been built, we can offset the position we determined was appropriate for
			// it by the element's margin. This is necessary because the coordinate system for the box begins at the
			// border, not the margin.
			absolute_position.x += absolute_element->GetBox().GetEdge(Box::MARGIN, Box::LEFT);
			absolute_position.y += absolute_element->GetBox().GetEdge(Box::MARGIN, Box::TOP);

			// Set the offset of the element; the element itself will take care of any RCSS-defined positional offsets.
			absolute_element->SetOffset(absolute_position, element);
		}

		absolute_elements.clear();
	}
}

// Returns the offset from the top-left corner of this box that the next child box will be positioned at.
void LayoutBlockBox::PositionBox(Vector2f& box_position, float top_margin, int clear_property) const
{
	// If our element is establishing a new offset hierarchy, then any children of ours don't inherit our offset.
	box_position = GetPosition();
	box_position += box.GetPosition();
	box_position.y += box_cursor;

	float clear_margin = space->ClearBoxes(box_position.y + top_margin, clear_property) - (box_position.y + top_margin);
	if (clear_margin > 0)
		box_position.y += clear_margin;
	else
	{
		// Check for a collapsing vertical margin.
		if (!block_boxes.empty() &&
			block_boxes.back()->context == BLOCK)
		{
			float bottom_margin = block_boxes.back()->GetBox().GetEdge(Box::MARGIN, Box::BOTTOM);
			box_position.y -= Math::Min(top_margin, bottom_margin);
		}
	}
}

// Returns the offset from the top-left corner of this box's offset element the next child block box, of the given
// dimensions, will be positioned at. This will include the margins on the new block box.
void LayoutBlockBox::PositionBlockBox(Vector2f& box_position, const Box& box, int clear_property) const
{
	PositionBox(box_position, box.GetEdge(Box::MARGIN, Box::TOP), clear_property);
	box_position.x += box.GetEdge(Box::MARGIN, Box::LEFT);
	box_position.y += box.GetEdge(Box::MARGIN, Box::TOP);

	LayoutEngine::Round(box_position);
}

// Returns the offset from the top-left corner of this box for the next line.
void LayoutBlockBox::PositionLineBox(Vector2f& box_position, float& box_width, bool& _wrap_content, const Vector2f& dimensions) const
{
	Vector2f cursor;
	PositionBox(cursor);

	space->PositionBox(box_position, box_width, cursor.y, dimensions);

	// Also, probably shouldn't check for widths when positioning the box?
	_wrap_content = wrap_content;
}

// Returns the block box's element.
Element* LayoutBlockBox::GetElement() const
{
	return element;
}

// Returns the block box's parent.
LayoutBlockBox* LayoutBlockBox::GetParent() const
{
	return parent;
}

// Returns the position of the block box, relative to its parent's content area.
const Vector2f& LayoutBlockBox::GetPosition() const
{
	return position;
}

// Returns the element against which all positions of boxes in the hierarchy are calculated relative to.
LayoutBlockBox* LayoutBlockBox::GetOffsetParent() const
{
	return offset_parent;
}

// Returns the block box against which all positions of boxes in the hierarchy are calculated relative to.
LayoutBlockBox* LayoutBlockBox::GetOffsetRoot() const
{
	return offset_root;
}

// Returns the block box's dimension box.
Box& LayoutBlockBox::GetBox()
{
	return box;
}

// Returns the block box's dimension box.
const Box& LayoutBlockBox::GetBox() const
{
	return box;
}

void* LayoutBlockBox::operator new(size_t size)
{
	void* memory = LayoutEngine::AllocateLayoutChunk(size);
	return memory;
}

void LayoutBlockBox::operator delete(void* chunk)
{
	LayoutEngine::DeallocateLayoutChunk(chunk);
}

// Closes our last block box, if it is an open inline block box.
LayoutBlockBox::CloseResult LayoutBlockBox::CloseInlineBlockBox()
{
	if (!block_boxes.empty() &&
		block_boxes.back()->context == INLINE)
		return block_boxes.back()->Close();

	return OK;
}

// Positions a floating element within this block box.
void LayoutBlockBox::PositionFloat(Element* element, float offset)
{
	Vector2f box_position;
	PositionBox(box_position);

	space->PositionBox(box_position.y + offset, element);
}

// Checks if we have a new vertical overflow on an auto-scrolling element.
bool LayoutBlockBox::CatchVerticalOverflow(float cursor)
{
	if (cursor == -1)
		cursor = box_cursor;

	float box_height = box.GetSize().y;
	if (box_height < 0)
		box_height = max_height;

	// If we're auto-scrolling and our height is fixed, we have to check if this box has exceeded our client height.
	if (!vertical_overflow &&
		box_height >= 0 &&
		overflow_y_property == OVERFLOW_AUTO)
	{
		if (cursor > box_height - element->GetElementScroll()->GetScrollbarSize(ElementScroll::HORIZONTAL))
		{
			vertical_overflow = true;
			element->GetElementScroll()->EnableScrollbar(ElementScroll::VERTICAL, box.GetSize(Box::PADDING).x);

			for (size_t i = 0; i < block_boxes.size(); i++)
				delete block_boxes[i];
			block_boxes.clear();

			delete space;
			space = new LayoutBlockBoxSpace(this);

			box_cursor = 0;
			interrupted_chain = NULL;

			return false;
		}
	}

	return true;
}

}
}
