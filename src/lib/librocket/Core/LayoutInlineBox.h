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

#ifndef ROCKETCORELAYOUTINLINEBOX_H
#define ROCKETCORELAYOUTINLINEBOX_H

#include <Rocket/Core/Box.h>

namespace Rocket {
namespace Core {

class Element;
class ElementText;
class FontFaceHandle;
class LayoutBlockBox;
class LayoutLineBox;

/**
	@author Peter Curry
 */

class LayoutInlineBox
{
friend class LayoutInlineBoxText;

public:
	/// Constructs a new inline box for an element.
	/// @param element[in] The element this inline box is flowing.
	/// @param box[in] The extents of the inline box's element.
	LayoutInlineBox(Element* element, const Box& box);
	/// Constructs a new inline box for a split box.
	/// @param chain[in] The box that has overflowed into us.
	LayoutInlineBox(LayoutInlineBox* chain);
	virtual ~LayoutInlineBox();

	/// Sets the inline box's line.
	/// @param line[in] The line this inline box resides in.
	void SetLine(LayoutLineBox* line);
	/// Sets the inline box's parent.
	/// @param parent[in] The parent this inline box resides in.
	void SetParent(LayoutInlineBox* parent);

	/// Closes the box.
	void Close();

	/// Flows the inline box's content into its parent line.
	/// @param[in] first_box True if this box is the first box containing content to be flowed into this line.
	/// @param[in] available_width The width available for flowing this box's content. This is measured from the left side of this box's content area.
	/// @param[in] right_spacing_width The width of the spacing that must be left on the right of the element if no overflow occurs. If overflow occurs, then the entire width can be used.
	/// @return The overflow box containing any content that spilled over from the flow. This must be NULL if no overflow occured.
	virtual LayoutInlineBox* FlowContent(bool first_box, float available_width, float right_spacing_width);

	/// Computes and sets the vertical position of this element, relative to its parent inline box (or block box,
	/// for an un-nested inline box).
	/// @param ascender[out] The maximum ascender of this inline box and all of its children.
	/// @param descender[out] The maximum descender of this inline box and all of its children.
	virtual void CalculateBaseline(float& ascender, float& descender);
	/// Offsets the baseline of this box, and all of its children, by the ascender of the parent line box.
	/// @param ascender[in] The ascender of the line box.
	virtual void OffsetBaseline(float ascender);

	/// Returns true if this box is capable of overflowing, or if it must be rendered on a single line.
	/// @return True if this box can overflow, false otherwise.
	virtual bool CanOverflow() const;
	/// Returns true if this box's element is the last child of its parent.
	/// @param True if this box is a last child.
	bool IsLastChild() const;

	/// Returns the inline box's offset from its line.
	/// @return The box's offset from its line.
	const Vector2f& GetPosition() const;

	/// Sets the inline box's horizontal offset from its parent's content area.
	/// @param position[in] The box's horizontal offset.
	void SetHorizontalPosition(float position);
	/// Sets the inline box's vertical offset from its parent's content area.
	/// @param position[in] The box's vertical offset.
	void SetVerticalPosition(float position);

	/// Positions the inline box's element.
	virtual void PositionElement();
	/// Sizes the inline box's element.
	/// @param split[in] True if this box is split, false otherwise.
	virtual void SizeElement(bool split);

	/// Returns the vertical align property of the box's element.
	/// @return the vertical align property, or -1 if it is set to a numerical value.
	int GetVerticalAlignProperty() const;

	/// Returns the inline box's element.
	/// @return The inline box's element.
	Element* GetElement();

	/// Returns the inline box's parent.
	/// @param The parent of this inline box. This will be NULL for a root-level inline box (ie, one that has a block element has a parent in the true hierarchy).
	LayoutInlineBox* GetParent();

	/// Returns the inline box's dimension box.
	/// @return The inline box's dimension box.
	const Box& GetBox() const;
	/// Returns the height of the inline box. This is separate from the the box, as different types of inline
	/// elements generate different line heights. The possible types are:
	///  * replaced elements (or inline-block elements), which use their entire box (including margins) as their
	///    height
	///  * non-replaced elements, which use the maximum line-height of their children
	///  * text elements, which use their line-height
	float GetHeight() const;
	/// Returns the baseline of the inline box.
	/// @return The box's baseline.
	float GetBaseline() const;

	void* operator new(size_t size);
	void operator delete(void* chunk);

protected:
	/// Returns our parent box's font face handle.
	/// @return The font face handle of our parent box.
	FontFaceHandle* GetParentFont() const;
	/// Returns our parent box's line height.
	/// @return The line height of our parent box.
	float GetParentLineHeight() const;

	// The box's element.
	Element* element;

	// The line box's offset relative to its parent block box.
	Vector2f position;
	// The element's inline box.
	Box box;
	// The inline box's width; note that this is stored separately from the box dimensions. It is only used by
	// nesting inline boxes, such as HTML spans containing text.
	float width;
	// The inline box's height; note that this is stored separately from the box dimensions, as inline elements
	// don't necessarily have identical relationships between their box dimensions and line height.
	float height;

	// The value of this box's element's vertical-align property.
	int vertical_align_property;
	// The baseline of the inline element.
	float baseline;

	// The inline box's parent; this will be NULL if we're not a nested inline element.
	LayoutInlineBox* parent;
	// This inline box's line.
	LayoutLineBox* line;

	std::vector< LayoutInlineBox* > children;

	// The next link in our element's chain of inline boxes.
	LayoutInlineBox* chain;
	// True if we're a link in a chain of inline boxes flowing from previous lines.
	bool chained;
};

}
}

#endif
