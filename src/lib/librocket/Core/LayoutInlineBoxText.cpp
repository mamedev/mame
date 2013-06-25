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
#include "LayoutInlineBoxText.h"
#include "FontFaceHandle.h"
#include "LayoutEngine.h"
#include "LayoutLineBox.h"
#include <Rocket/Core/ElementText.h>
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/Log.h>
#include <Rocket/Core/Property.h>

namespace Rocket {
namespace Core {

LayoutInlineBoxText::LayoutInlineBoxText(Element* element, int _line_begin) : LayoutInlineBox(element, Box())
{
	line_begin = _line_begin;

	// Build the box to represent the dimensions of the first word.
	BuildWordBox();
}

LayoutInlineBoxText::~LayoutInlineBoxText()
{
}

// Returns true if this box is capable of overflowing, or if it must be rendered on a single line.
bool LayoutInlineBoxText::CanOverflow() const
{
	return line_segmented;
}

// Flows the inline box's content into its parent line.
LayoutInlineBox* LayoutInlineBoxText::FlowContent(bool first_box, float available_width, float right_spacing_width)
{
	ElementText* text_element = GetTextElement();
	ROCKET_ASSERT(text_element != NULL);

	int line_length;
	float line_width;
	bool overflow = !text_element->GenerateLine(line_contents, line_length, line_width, line_begin, available_width, right_spacing_width, first_box);

	Vector2f content_area;
	content_area.x = line_width;
	content_area.y = box.GetSize().y;
	box.SetContent(content_area);

	// Call the base-class's FlowContent() to increment the width of our parent's box.
	LayoutInlineBox::FlowContent(first_box, available_width, right_spacing_width);

	if (overflow)
		return new LayoutInlineBoxText(element, line_begin + line_length);

	return NULL;
}

// Computes and sets the vertical position of this element, relative to its parent inline box (or block box, for an un-nested inline box).
void LayoutInlineBoxText::CalculateBaseline(float& ascender, float& descender)
{
	ascender = height - baseline;
	descender = height - ascender;
}

// Offsets the baseline of this box, and all of its children, by the ascender of the parent line box.
void LayoutInlineBoxText::OffsetBaseline(float ascender)
{
	// Offset by the ascender.
	position.y += (ascender - (height - baseline));

	// Calculate the leading (the difference between font height and line height).
	float leading = 0;

	FontFaceHandle* font_face_handle = element->GetFontFaceHandle();
	if (font_face_handle != NULL)
		leading = height - font_face_handle->GetLineHeight();

	// Offset by the half-leading.
	position.y += leading * 0.5f;
	position.y = LayoutEngine::Round(position.y);
}

// Positions the inline box's element.
void LayoutInlineBoxText::PositionElement()
{
	if (line_begin == 0)
	{
		LayoutInlineBox::PositionElement();

		GetTextElement()->ClearLines();
		GetTextElement()->AddLine(Vector2f(0, 0), line_contents);
	}
	else
	{
		GetTextElement()->AddLine(line->GetRelativePosition() + position - element->GetRelativeOffset(Box::BORDER), line_contents);
	}
}

// Sizes the inline box's element.
void LayoutInlineBoxText::SizeElement(bool ROCKET_UNUSED(split))
{
}

void* LayoutInlineBoxText::operator new(size_t size)
{
	return LayoutEngine::AllocateLayoutChunk(size);
}

void LayoutInlineBoxText::operator delete(void* chunk)
{
	LayoutEngine::DeallocateLayoutChunk(chunk);
}

// Returns the box's element as a text element.
ElementText* LayoutInlineBoxText::GetTextElement()
{
	return dynamic_cast< ElementText* >(element);
}

// Builds a box for the first word of the element.
void LayoutInlineBoxText::BuildWordBox()
{
	ElementText* text_element = GetTextElement();
	ROCKET_ASSERT(text_element != NULL);

	FontFaceHandle* font_face_handle = text_element->GetFontFaceHandle();
	if (font_face_handle == NULL)
	{
		height = 0;
		baseline = 0;

		Log::Message(Log::LT_WARNING, "No font face defined on element %s. Please specify a font-family in your RCSS.", text_element->GetAddress().CString());
		return;
	}

	Vector2f content_area;
	line_segmented = !text_element->GenerateToken(content_area.x, line_begin);
	content_area.y = (float) ElementUtilities::GetLineHeight(element);
	box.SetContent(content_area);
}

}
}
