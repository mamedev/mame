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

#ifndef ROCKETCORELAYOUTBLOCKBOX_H
#define ROCKETCORELAYOUTBLOCKBOX_H

#include "LayoutLineBox.h"
#include <Rocket/Core/Box.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

class LayoutBlockBoxSpace;
class LayoutEngine;

/**
	@author Peter Curry
 */

class LayoutBlockBox
{
public:
	enum FormattingContext
	{
		BLOCK,
		INLINE
	};

	enum CloseResult
	{
		OK,
		LAYOUT_SELF,
		LAYOUT_PARENT
	};

	/// Creates a new block box for rendering a block element.
	/// @param layout_engine[in] The layout engine that created this block box.
	/// @param parent[in] The parent of this block box. This will be NULL for the root element.
	/// @param element[in] The element this block box is laying out.
	LayoutBlockBox(LayoutEngine* layout_engine, LayoutBlockBox* parent, Element* element);
	/// Creates a new block box in an inline context.
	/// @param layout_engine[in] The layout engine that created this block box.
	/// @param parent[in] The parent of this block box.
	LayoutBlockBox(LayoutEngine* layout_engine, LayoutBlockBox* parent);
	/// Releases the block box.
	~LayoutBlockBox();

	/// Closes the box. This will determine the element's height (if it was unspecified).
	/// @return The result of the close; this may request a reformat of this element or our parent.
	CloseResult Close();

	/// Called by a closing block box child. Increments the cursor.
	/// @param child[in] The closing child block box.
	/// @return False if the block box caused an automatic vertical scrollbar to appear, forcing an entire reformat of the block box.
	bool CloseBlockBox(LayoutBlockBox* child);
	/// Called by a closing line box child. Increments the cursor, and creates a new line box to fit the overflow
	/// (if any).
	/// @param child[in] The closing child line box.
	/// @param overflow[in] The overflow from the closing line box. May be NULL if there was no overflow.
	/// @param overflow_chain[in] The end of the chained hierarchy to be spilled over to the new line, as the parent to the overflow box (if one exists).
	/// @return If the line box had overflow, this will be the last inline box created by the overflow.
	LayoutInlineBox* CloseLineBox(LayoutLineBox* child, LayoutInlineBox* overflow, LayoutInlineBox* overflow_chain);

	/// Adds a new block element to this block-context box.
	/// @param element[in] The new block element.
	/// @param placed[in] True if the element is to be placed, false otherwise.
	/// @return The block box representing the element. Once the element's children have been positioned, Close() must be called on it.
	LayoutBlockBox* AddBlockElement(Element* element);
	/// Adds a new inline element to this inline-context box.
	/// @param element[in] The new inline element.
	/// @param box[in] The box defining the element's bounds.
	/// @return The inline box representing the element. Once the element's children have been positioned, Close() must be called on it.
	LayoutInlineBox* AddInlineElement(Element* element, const Box& box);
	/// Adds a line-break to this block box.
	void AddBreak();

	/// Adds an element to this block box to be handled as a floating element.
	bool AddFloatElement(Element* element);

	/// Adds an element to this block box to be handled as an absolutely-positioned element. This element will be
	/// laid out, sized and positioned appropriately once this box is finished. This should only be called on boxes
	/// rendering in a block-context.
	/// @param element[in] The element to be positioned absolutely within this block box.
	void AddAbsoluteElement(Element* element);
	/// Formats, sizes, and positions all absolute elements in this block.
	void CloseAbsoluteElements();

	/// Returns the offset from the top-left corner of this box's offset element the next child box will be
	/// positioned at.
	/// @param[out] box_position The box cursor position.
	/// @param[in] top_margin The top margin of the box. This will be collapsed as appropriate against other block boxes.
	/// @param[in] clear_property The value of the underlying element's clear property.
	void PositionBox(Vector2f& box_position, float top_margin = 0, int clear_property = 0) const;
	/// Returns the offset from the top-left corner of this box's offset element the next child block box, of the
	/// given dimensions, will be positioned at. This will include the margins on the new block box.
	/// @param[out] box_position The block box cursor position.
	/// @param[in] box The dimensions of the new box.
	/// @param[in] clear_property The value of the underlying element's clear property.
	void PositionBlockBox(Vector2f& box_position, const Box& box, int clear_property) const;
	/// Returns the offset from the top-left corner of this box for the next line.
	/// @param box_position[out] The line box position.
	/// @param box_width[out] The available width for the line box.
	/// @param wrap_content[out] Set to true if the line box should grow to fit inline boxes, false if it should wrap them.
	/// @param dimensions[in] The minimum dimensions of the line.
	void PositionLineBox(Vector2f& box_position, float& box_width, bool& wrap_content, const Vector2f& dimensions) const;

	/// Returns the block box's element.
	/// @return The block box's element.
	Element* GetElement() const;

	/// Returns the block box's parent.
	/// @return The block box's parent.
	LayoutBlockBox* GetParent() const;

	/// Returns the position of the block box, relative to its parent's content area.
	/// @return The relative position of the block box.
	const Vector2f& GetPosition() const;

	/// Returns the block box against which all positions of boxes in the hierarchy are set relative to.
	/// @return This box's offset parent.
	LayoutBlockBox* GetOffsetParent() const;
	/// Returns the block box against which all positions of boxes in the hierarchy are calculated relative to.
	/// @return This box's offset root.
	LayoutBlockBox* GetOffsetRoot() const;

	/// Returns the block box's dimension box.
	/// @return The block box's dimension box.
	Box& GetBox();
	/// Returns the block box's dimension box.
	/// @return The block box's dimension box.
	const Box& GetBox() const;

	void* operator new(size_t size);
	void operator delete(void* chunk);

private:
	struct AbsoluteElement
	{
		Element* element;
		Vector2f position;
	};

	// Closes our last block box, if it is an open inline block box.
	CloseResult CloseInlineBlockBox();

	// Positions a floating element within this block box.
	void PositionFloat(Element* element, float offset = 0);

	// Checks if we have a new vertical overflow on an auto-scrolling element. If so, our vertical scrollbar will
	// be enabled and our block boxes will be destroyed. All content will need to re-formatted. Returns true if no
	// overflow occured, false if it did.
	bool CatchVerticalOverflow(float cursor = -1);

	typedef std::vector< AbsoluteElement > AbsoluteElementList;
	typedef std::vector< LayoutBlockBox* > BlockBoxList;
	typedef std::vector< LayoutLineBox* > LineBoxList;

	// The object managing our space, as occupied by floating elements of this box and our ancestors.
	LayoutBlockBoxSpace* space;

	// The box's layout engine.
	LayoutEngine* layout_engine;
	// The element this box represents. This will be NULL for boxes rendering in an inline context.
	Element* element;

	// The element we'll be computing our offset relative to during layout.
	LayoutBlockBox* offset_root;
	// The element this block box's children are to be offset from.
	LayoutBlockBox* offset_parent;

	// The box's block parent. This will be NULL for the root of the box tree.
	LayoutBlockBox* parent;

	// The context of the box's context; either block or inline.
	FormattingContext context;

	// The block box's position.
	Vector2f position;
	// The block box's size.
	Box box;
	float min_height;
	float max_height;
	// Used by inline contexts only; set to true if the block box's line boxes should stretch to fit their inline content instead of wrapping.
	bool wrap_content;

	// The vertical position of the next block box to be added to this box, relative to the top of this box.
	float box_cursor;

	// Used by block contexts only; stores the list of block boxes under this box.
	BlockBoxList block_boxes;
	// Used by block contexts only; stores any elements that are to be absolutely positioned within this block box.
	AbsoluteElementList absolute_elements;
	// Used by block contexts only; stores an inline element hierarchy that was interrupted by a child block box.
	// The hierarchy will be resumed in an inline-context box once the intervening block box is completed.
	LayoutInlineBox* interrupted_chain;
	// Used by block contexts only; stores the value of the overflow property for the element.
	int overflow_x_property;
	int overflow_y_property;
	// Used by block contexts only; if true, we've enabled our vertical scrollbar.
	bool vertical_overflow;

	// Used by inline contexts only; stores the list of line boxes flowing inline content.
	LineBoxList line_boxes;
	// Used by inline contexts only; stores any floating elements that are waiting for a line break to be positioned.
	ElementList float_elements;
};

}
}

#endif
