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

#ifndef ROCKETCORELAYOUTENGINE_H
#define ROCKETCORELAYOUTENGINE_H

#include "LayoutBlockBox.h"

namespace Rocket {
namespace Core {

class Box;

/**
	@author Robert Curry
 */

class LayoutEngine
{
public:
	/// Constructs a new layout engine.
	LayoutEngine();
	~LayoutEngine();

	/// Formats the contents for a root-level element (usually a document, floating or replaced element).
	/// @param element[in] The element to lay out.
	/// @param containing_block[in] The size of the containing block.
	bool FormatElement(Element* element, const Vector2f& containing_block);

	/// Generates the box for an element.
	/// @param[out] box The box to be built.
	/// @param[in] containing_block The dimensions of the content area of the block containing the element.
	/// @param[in] element The element to build the box for.
	/// @param[in] inline_element True if the element is placed in an inline context, false if not.
	static void BuildBox(Box& box, const Vector2f& containing_block, Element* element, bool inline_element = false);
	/// Generates the box for an element placed in a block box.
	/// @param[out] box The box to be built.
	/// @param[out] min_height The minimum height of the element's box.
	/// @param[out] max_height The maximum height of the element's box.
	/// @param[in] containing_box The block box containing the element.
	/// @param[in] element The element to build the box for.
	/// @param[in] inline_element True if the element is placed in an inline context, false if not.
	static void BuildBox(Box& box, float& min_height, float& max_height, LayoutBlockBox* containing_box, Element* element, bool inline_element = false);

	/// Clamps the width of an element based from its min-width and max-width properties.
	/// @param[in] width The width to clamp.
	/// @param[in] element The element to read the properties from.
	/// @param[in] containing_block_width The width of the element's containing block.
	/// @return The clamped width.
	static float ClampWidth(float width, Element* element, float containing_block_width);
	/// Clamps the height of an element based from its min-height and max-height properties.
	/// @param[in] height The height to clamp.
	/// @param[in] element The element to read the properties from.
	/// @param[in] containing_block_height The height of the element's containing block.
	/// @return The clamped height.
	static float ClampHeight(float height, Element* element, float containing_block_height);

	/// Rounds a vector of two floating-point values to integral values.
	/// @param[inout] value The vector to round.
	/// @return The rounded vector.
	static Vector2f& Round(Vector2f& value);
	/// Rounds a floating-point value to an integral value.
	/// @param[in] value The value to round.
	/// @return The rounded value.
	static float Round(float value);

	static void* AllocateLayoutChunk(size_t size);
	static void DeallocateLayoutChunk(void* chunk);

private:
	/// Positions a single element and its children within this layout.
	/// @param[in] element The element to lay out.
	bool FormatElement(Element* element);

	/// Formats and positions an element as a block element.
	/// @param[in] element The block element.
	bool FormatElementBlock(Element* element);
	/// Formats and positions an element as an inline element.
	/// @param[in] element The inline element.
	bool FormatElementInline(Element* element);
	/// Positions an element as a sized inline element, formatting its internal hierarchy as a block element.
	/// @param[in] element The replaced element.
	void FormatElementReplaced(Element* element);
	/// Executes any special formatting for special elements.
	/// @param[in] element The element to parse.
	/// @return True if the element was parsed as a special element, false otherwise.
	bool FormatElementSpecial(Element* element);

	/// Returns the fully-resolved, fixed-width and -height containing block from a block box.
	/// @param[in] containing_box The leaf box.
	/// @return The dimensions of the content area, using the latest fixed dimensions for width and height in the hierarchy.
	static Vector2f GetContainingBlock(const LayoutBlockBox* containing_box);

	/// Builds the block-specific width and horizontal margins of a Box.
	/// @param[in,out] box The box to generate. The padding and borders must be set on the box already. If the content area is sized, then it will be used instead of the width property.
	/// @param[in] element The element the box is being generated for.
	/// @param[in] containing_block_width The width of the containing block.
	static void BuildBoxWidth(Box& box, Element* element, float containing_block_width);
	/// Builds the block-specific height and vertical margins of a Box.
	/// @param[in,out] box The box to generate. The padding and borders must be set on the box already. If the content area is sized, then it will be used instead of the height property.
	/// @param[in] element The element the box is being generated for.
	/// @param[in] containing_block_height The height of the containing block.
	static void BuildBoxHeight(Box& box, Element* element, float containing_block_height);

	// The root block box, representing the document.
	LayoutBlockBox* block_box;

	// The open block box containing displaying in a block-context.
	LayoutBlockBox* block_context_box;
};

}
}

#endif
