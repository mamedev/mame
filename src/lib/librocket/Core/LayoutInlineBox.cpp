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
#include "LayoutInlineBox.h"
#include "FontFaceHandle.h"
#include "LayoutBlockBox.h"
#include "LayoutEngine.h"
#include <Rocket/Core/ElementText.h>
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/Property.h>
#include <Rocket/Core/StyleSheetKeywords.h>

namespace Rocket {
namespace Core {

// Constructs a new inline box for an element.
LayoutInlineBox::LayoutInlineBox(Element* _element, const Box& _box) : position(0, 0), box(_box)
{
	line = NULL;

	parent = NULL;
	element = _element;

	width = 0;

	// If this box has intrinsic dimensions, then we set our height to the total height of the element; otherwise, it
	// is zero height.
	if (box.GetSize().y > 0)
	{
		height = box.GetSize(Box::MARGIN).y;
		baseline = element->GetBaseline() + box.GetCumulativeEdge(Box::CONTENT, Box::BOTTOM);
	}
	else
	{
		FontFaceHandle* font_face = element->GetFontFaceHandle();
		if (font_face != NULL)
		{
			height = (float) ElementUtilities::GetLineHeight(element);
			baseline = (height - font_face->GetLineHeight()) * 0.5f + font_face->GetBaseline();
		}
		else
		{
			height = 0;
			baseline = 0;
		}
	}

	const Property* property = element->GetProperty(VERTICAL_ALIGN);
	if (property->unit == Property::KEYWORD)
		vertical_align_property = property->value.Get< int >();
	else
		vertical_align_property = -1;

	chained = false;
	chain = NULL;
}

// Constructs a new inline box for a split box.
LayoutInlineBox::LayoutInlineBox(LayoutInlineBox* _chain) : position(0, 0), box(_chain->GetBox())
{
	line = NULL;

	parent = NULL;
	element = _chain->element;

	width = 0;
	height = _chain->height;
	baseline = _chain->baseline;
	vertical_align_property = _chain->vertical_align_property;

	_chain->chain = this;
	chain = NULL;
	chained = true;

	// As we're a split box, our left side is cleared and content set back to (-1, -1).
	box.SetEdge(Box::PADDING, Box::LEFT, 0);
	box.SetEdge(Box::BORDER, Box::LEFT, 0);
	box.SetEdge(Box::MARGIN, Box::LEFT, 0);
	box.SetContent(Vector2f(-1, -1));
}

LayoutInlineBox::~LayoutInlineBox()
{
}

// Sets the inline box's line.
void LayoutInlineBox::SetLine(LayoutLineBox* _line)
{
	line = _line;
}

// Sets the inline box's parent.
void LayoutInlineBox::SetParent(LayoutInlineBox* _parent)
{
	parent = _parent;
	if (parent != NULL)
		parent->children.push_back(this);
}

// Closes the box.
void LayoutInlineBox::Close()
{
	if (chain)
		chain->Close();
	else
	{
		ROCKET_ASSERT(line != NULL);
		line->CloseInlineBox(this);
	}
}

// Returns true if this box needs to close on its line.
bool LayoutInlineBox::CanOverflow() const
{
	return box.GetSize().x < 0;
}

// Returns true if this box's element is the last child of its parent.
bool LayoutInlineBox::IsLastChild() const
{
	Element* parent = element->GetParentNode();
	if (parent == NULL)
		return true;

	return parent->GetLastChild() == element;
}

// Flows the inline box's content into its parent line.
LayoutInlineBox* LayoutInlineBox::FlowContent(bool ROCKET_UNUSED(first_box), float ROCKET_UNUSED(available_width), float ROCKET_UNUSED(right_spacing_width))
{
	// If we're representing a sized element, then add our element's width onto our parent's.
	if (parent != NULL &&
		box.GetSize().x > 0)
		parent->width += box.GetSize(Box::MARGIN).x;

	// Nothing else to do here; static elements will automatically be 'flowed' into their lines when they are placed.
	return NULL;
}

// Computes and sets the vertical position of this element, relative to its parent box.
void LayoutInlineBox::CalculateBaseline(float& ascender, float& descender)
{
	// We're vertically-aligned with one of the standard types.
	switch (vertical_align_property)
	{
		// Aligned with our parent box's baseline, our relative vertical position is set to 0.
		case VERTICAL_ALIGN_BASELINE:
		{
			SetVerticalPosition(0);
		}
		break;

		// The middle of this box is aligned with the baseline of its parent's plus half an ex.
		case VERTICAL_ALIGN_MIDDLE:
		{
			FontFaceHandle* parent_font = GetParentFont();
			int x_height = 0;
			if (parent_font != NULL)
				x_height = parent_font->GetXHeight() / -2;

			SetVerticalPosition(x_height + (height / 2 - baseline));
		}
		break;

		// This box's baseline is offset from its parent's so it is appropriate for rendering subscript.
		case VERTICAL_ALIGN_SUB:
		{
			FontFaceHandle* parent_font = GetParentFont();
			if (parent_font == NULL)
				SetVerticalPosition(0);
			else
				SetVerticalPosition(LayoutEngine::Round(parent_font->GetLineHeight() * 0.2f));
		}
		break;

		// This box's baseline is offset from its parent's so it is appropriate for rendering superscript.
		case VERTICAL_ALIGN_SUPER:
		{
			FontFaceHandle* parent_font = GetParentFont();
			if (parent_font == NULL)
				SetVerticalPosition(0);
			else
				SetVerticalPosition(-1 * LayoutEngine::Round(parent_font->GetLineHeight() * 0.4f));
		}
		break;

		// The top of this box is aligned to the top of its parent's font.
		case VERTICAL_ALIGN_TEXT_TOP:
		{
			FontFaceHandle* parent_font = GetParentFont();
			if (parent_font == NULL)
				SetVerticalPosition(0);
			else
				SetVerticalPosition((height - baseline) - (parent_font->GetLineHeight() - parent_font->GetBaseline()));
		}
		break;

		// The bottom of this box is aligned to the bottom of its parent's font (not the baseline).
		case VERTICAL_ALIGN_TEXT_BOTTOM:
		{
			FontFaceHandle* parent_font = GetParentFont();
			if (parent_font == NULL)
				SetVerticalPosition(0);
			else
				SetVerticalPosition(parent_font->GetBaseline() - baseline);
		}
		break;

		// This box is aligned with the line box, not an inline box, so we can't position it yet.
		case VERTICAL_ALIGN_TOP:
		case VERTICAL_ALIGN_BOTTOM:
			break;

		// The baseline of this box is offset by a fixed amount from its parent's baseline.
		default:
		{
			SetVerticalPosition(-1 * element->ResolveProperty(VERTICAL_ALIGN, GetParentLineHeight()));
		}
		break;
	}

	// Set the ascender and descender relative to this element. If we're an unsized element (span, em, etc) then we
	// have no dimensions ourselves.
	if (box.GetSize() == Vector2f(-1, -1))
	{
		ascender = 0;
		descender = 0;
	}
	else
	{
		ascender = height - baseline;
		descender = height - ascender;
	}

	for (size_t i = 0; i < children.size(); ++i)
	{
		// Don't include any of our children that are aligned relative to the line box; the line box treats them
		// separately.
		if (children[i]->GetVerticalAlignProperty() != VERTICAL_ALIGN_TOP &&
			children[i]->GetVerticalAlignProperty() != VERTICAL_ALIGN_BOTTOM)
		{
			float child_ascender, child_descender;
			children[i]->CalculateBaseline(child_ascender, child_descender);

			ascender = Math::Max(ascender, child_ascender - children[i]->GetPosition().y);
			descender = Math::Max(descender, child_descender + children[i]->GetPosition().y);
		}
	}
}

// Offsets the baseline of this box, and all of its children, by the ascender of the parent line box.
void LayoutInlineBox::OffsetBaseline(float ascender)
{
	for (size_t i = 0; i < children.size(); ++i)
	{
		// Don't offset any of our children that are aligned relative to the line box; the line box will take care of
		// them separately.
		if (children[i]->GetVerticalAlignProperty() != VERTICAL_ALIGN_TOP &&
			children[i]->GetVerticalAlignProperty() != VERTICAL_ALIGN_BOTTOM)
			children[i]->OffsetBaseline(ascender + position.y);
	}

	position.y += (ascender - (height - baseline));
}

// Returns the inline box's offset from its parent's content area.
const Vector2f& LayoutInlineBox::GetPosition() const
{
	return position;
}

// Sets the inline box's relative horizontal offset from its parent's content area.
void LayoutInlineBox::SetHorizontalPosition(float _position)
{
	position.x = _position;
}

// Sets the inline box's relative vertical offset from its parent's content area.
void LayoutInlineBox::SetVerticalPosition(float _position)
{
	position.y = _position;
}

void LayoutInlineBox::PositionElement()
{
	if (box.GetSize() == Vector2f(-1, -1))
	{
		// If this unsised element has any top margins, border or padding, then shift the position up so the borders
		// and background will render in the right place.
		position.y -= box.GetCumulativeEdge(Box::CONTENT, Box::TOP);
	}
	// Otherwise; we're a sized element (replaced or inline-block), so we need to offset our element's vertical
	// position by our top margin (as the origin of an element is the top-left of the border, not the margin).
	else
		position.y += box.GetEdge(Box::MARGIN, Box::TOP);

	if (!chained)
		element->SetOffset(line->GetRelativePosition() + position, line->GetBlockBox()->GetOffsetParent()->GetElement());
}

// Sizes the inline box's element.
void LayoutInlineBox::SizeElement(bool split)
{
	// Resize the box for an unsized inline element.
	if (box.GetSize() == Vector2f(-1, -1))
	{
		box.SetContent(Vector2f(width, (float) ElementUtilities::GetLineHeight(element)));
		if (parent != NULL)
			parent->width += width;
	}

	Box element_box = box;
	if (split)
	{
		element_box.SetEdge(Box::MARGIN, Box::RIGHT, 0);
		element_box.SetEdge(Box::BORDER, Box::RIGHT, 0);
		element_box.SetEdge(Box::PADDING, Box::RIGHT, 0);
	}

	// The elements of a chained box have already had their positions set by the first link.
	if (chained)
	{
		element_box.SetOffset((line->GetPosition() + position) - element->GetRelativeOffset(Box::BORDER));
		element->AddBox(element_box);

		if (chain != NULL)
			element->OnLayout();
	}
	else
	{
		element->SetBox(element_box);
		element->OnLayout();
	}
}

// Returns the vertical align property of the box's element.
int LayoutInlineBox::GetVerticalAlignProperty() const
{
	return vertical_align_property;
}

// Returns the inline box's element.
Element* LayoutInlineBox::GetElement()
{
	return element;
}

// Returns the inline box's parent.
LayoutInlineBox* LayoutInlineBox::GetParent()
{
	return parent;
}

// Returns the inline box's dimension box.
const Box& LayoutInlineBox::GetBox() const
{
	return box;
}

// Returns the height of the inline box.
float LayoutInlineBox::GetHeight() const
{
	return height;
}

// Returns the baseline of the inline box.
float LayoutInlineBox::GetBaseline() const
{
	return baseline;
}

void* LayoutInlineBox::operator new(size_t size)
{
	return LayoutEngine::AllocateLayoutChunk(size);
}

void LayoutInlineBox::operator delete(void* chunk)
{
	LayoutEngine::DeallocateLayoutChunk(chunk);
}

// Returns our parent box's font face handle.
FontFaceHandle* LayoutInlineBox::GetParentFont() const
{
	if (parent == NULL)
		return line->GetBlockBox()->GetParent()->GetElement()->GetFontFaceHandle();
	else
		return parent->GetElement()->GetFontFaceHandle();
}

// Returns our parent box's line height.
float LayoutInlineBox::GetParentLineHeight() const
{
	if (parent == NULL)
		return (float) ElementUtilities::GetLineHeight(line->GetBlockBox()->GetParent()->GetElement());
	else
		return (float) ElementUtilities::GetLineHeight(parent->GetElement());
}

}
}
