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

#ifndef ROCKETCOREELEMENTUTILITIES_H
#define ROCKETCOREELEMENTUTILITIES_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Box.h>
#include <Rocket/Core/WString.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

class Context;
class FontFaceHandle;
class RenderInterface;

/**
	Utility functions for dealing with elements.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API ElementUtilities
{
public:
	enum PositionAnchor
	{
		TOP = 1 << 0,
		BOTTOM = 1 << 1,
		LEFT = 1 << 2,
		RIGHT = 1 << 3,

		TOP_LEFT = TOP | LEFT,
		TOP_RIGHT = TOP | RIGHT,
		BOTTOM_LEFT = BOTTOM | LEFT,
		BOTTOM_RIGHT = BOTTOM | RIGHT
	};

	/// Get the element with the given id.
	/// @param[in] root_element First element to check.
	/// @param[in] id ID of the element to look for.
	static Element* GetElementById(Element* root_element, const String& id);
	/// Get all elements with the given tag.
	/// @param[out] elements Resulting elements.
	/// @param[in] root_element First element to check.
	/// @param[in] tag Tag to search for.
	static void GetElementsByTagName(ElementList& elements, Element* root_element, const String& tag);
	/// Get all elements with the given class set on them.
	/// @param[out] elements Resulting elements.
	/// @param[in] root_element First element to check.
	/// @param[in] tag Class name to search for.
	static void GetElementsByClassName(ElementList& elements, Element* root_element, const String& class_name);

	/// Returns an element's font face.
	/// @param[in] element The element to determine the font face for.
	/// @return The element's font face. This will be NULL if no valid RCSS font styles have been set up for this element.
	static FontFaceHandle* GetFontFaceHandle(Element* element);
	/// Returns an element's font size, if it has a font defined.
	/// @param[in] element The element to determine the font size for.
	/// @return The font size as determined by the element's font, or 0 if it has no font specified.
	static int GetFontSize(Element* element);
	/// Returns an element's line height, if it has a font defined.
	/// @param[in] element The element to determine the line height for.
	/// @return The line height as specified by the element's font and line height styles.
	static int GetLineHeight(Element* element);
	/// Returns the width of a string rendered within the context of the given element.
	/// @param[in] element The element to measure the string from.
	/// @param[in] string The string to measure.
	/// @return The string width, in pixels.
	static int GetStringWidth(Element* element, const WString& string);

	/// Bind and instance all event attributes on the given element onto the element
	/// @param element Element to bind events on
	static void BindEventAttributes(Element* element);

	/// Generates the clipping region for an element.
	/// @param[out] clip_origin The origin, in context coordinates, of the origin of the element's clipping window.
	/// @param[out] clip_dimensions The size, in context coordinates, of the element's clipping window.
	/// @param[in] element The element to generate the clipping region for.
	/// @return True if a clipping region exists for the element and clip_origin and clip_window were set, false if not.
	static bool GetClippingRegion(Vector2i& clip_origin, Vector2i& clip_dimensions, Element* element);
	/// Sets the clipping region from an element and its ancestors.
	/// @param[in] element The element to generate the clipping region from.
	/// @param[in] context The context of the element; if this is not supplied, it will be derived from the element.
	/// @return The visibility of the given element within its clipping region.
	static bool SetClippingRegion(Element* element, Context* context = NULL);
	/// Applies the clip region from the render interface to the renderer
	/// @param[in] context The context to read the clip region from
	/// @param[in] render_interface The render interface to update.
	static void ApplyActiveClipRegion(Context* context, RenderInterface* render_interface);

	/// Formats the contents of an element. This does not need to be called for ordinary elements, but can be useful
	/// for non-DOM elements of custom elements.
	/// @param[in] element The element to lay out.
	/// @param[in] containing_block The size of the element's containing block.
	static bool FormatElement(Element* element, const Vector2f& containing_block);

	/// Generates the box for an element.
	/// @param[out] box The box to be built.
	/// @param[in] containing_block The dimensions of the content area of the block containing the element.
	/// @param[in] element The element to build the box for.
	/// @param[in] inline_element True if the element is placed in an inline context, false if not.
	static void BuildBox(Box& box, const Vector2f& containing_block, Element* element, bool inline_element = false);

	/// Sizes and positions an element within its parent. Any relative values will be evaluated against the size of the
	/// element parent's content area.
	/// @param element[in] The element to size and position.
	/// @param offset[in] The offset of the element inside its parent's content area.
	static bool PositionElement(Element* element, const Vector2f& offset);
	/// Sizes an element, and positions it within its parent offset from the borders of its content area. Any relative
	/// values will be evaluated against the size of the element parent's content area.
	/// @param element[in] The element to size and position.
	/// @param offset[in] The offset from the parent's borders.
	/// @param anchor[in] Defines which corner or edge the border is to be positioned relative to.
	static bool PositionElement(Element* element, const Vector2f& offset, PositionAnchor anchor);
};

}
}

#endif
