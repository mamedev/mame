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
#include "LayoutEngine.h"
#include <Rocket/Core/Math.h>
#include "Pool.h"
#include "LayoutBlockBoxSpace.h"
#include "LayoutInlineBoxText.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementScroll.h>
#include <Rocket/Core/ElementText.h>
#include <Rocket/Core/Property.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/StyleSheetKeywords.h>
#include <math.h>

namespace Rocket {
namespace Core {

#define MAX(a, b) (a > b ? a : b)

struct LayoutChunk
{
	LayoutChunk()
	{
	}

	static const unsigned int size = MAX(sizeof(LayoutBlockBox), MAX(sizeof(LayoutInlineBox), MAX(sizeof(LayoutInlineBoxText), MAX(sizeof(LayoutLineBox), sizeof(LayoutBlockBoxSpace)))));
	char buffer[size];
};

static Pool< LayoutChunk > layout_chunk_pool(200, true);

LayoutEngine::LayoutEngine()
{
	block_box = NULL;
	block_context_box = NULL;
}

LayoutEngine::~LayoutEngine()
{
}

// Formats the contents for a root-level element (usually a document or floating element).
bool LayoutEngine::FormatElement(Element* element, const Vector2f& containing_block)
{
	block_box = new LayoutBlockBox(this, NULL, NULL);
	block_box->GetBox().SetContent(containing_block);

	block_context_box = block_box->AddBlockElement(element);

	for (int i = 0; i < element->GetNumChildren(); i++)
	{
		if (!FormatElement(element->GetChild(i)))
			i = -1;
	}

	block_context_box->Close();
	block_context_box->CloseAbsoluteElements();

	element->OnLayout();

	delete block_box;
	return true;
}

// Generates the box for an element.
void LayoutEngine::BuildBox(Box& box, const Vector2f& containing_block, Element* element, bool inline_element)
{
	if (element == NULL)
	{
		box.SetContent(containing_block);
		return;
	}

	// Calculate the padding area.
	float padding = element->ResolveProperty(PADDING_TOP, containing_block.x);
	box.SetEdge(Box::PADDING, Box::TOP, Math::Max(0.0f, padding));
	padding = element->ResolveProperty(PADDING_RIGHT, containing_block.x);
	box.SetEdge(Box::PADDING, Box::RIGHT, Math::Max(0.0f, padding));
	padding = element->ResolveProperty(PADDING_BOTTOM, containing_block.x);
	box.SetEdge(Box::PADDING, Box::BOTTOM, Math::Max(0.0f, padding));
	padding = element->ResolveProperty(PADDING_LEFT, containing_block.x);
	box.SetEdge(Box::PADDING, Box::LEFT, Math::Max(0.0f, padding));

	// Calculate the border area.
	float border = element->ResolveProperty(BORDER_TOP_WIDTH, containing_block.x);
	box.SetEdge(Box::BORDER, Box::TOP, Math::Max(0.0f, border));
	border = element->ResolveProperty(BORDER_RIGHT_WIDTH, containing_block.x);
	box.SetEdge(Box::BORDER, Box::RIGHT, Math::Max(0.0f, border));
	border = element->ResolveProperty(BORDER_BOTTOM_WIDTH, containing_block.x);
	box.SetEdge(Box::BORDER, Box::BOTTOM, Math::Max(0.0f, border));
	border = element->ResolveProperty(BORDER_LEFT_WIDTH, containing_block.x);
	box.SetEdge(Box::BORDER, Box::LEFT, Math::Max(0.0f, border));

	// Calculate the size of the content area.
	Vector2f content_area(-1, -1);
	bool replaced_element = false;

	// If the element has intrinsic dimensions, then we use those as the basis for the content area and only adjust
	// them if a non-auto style has been applied to them.
	if (element->GetIntrinsicDimensions(content_area))
	{
		replaced_element = true;

		Vector2f original_content_area = content_area;

		// The element has resized itself, so we only resize it if a RCSS width or height was set explicitly. A value of
		// 'auto' (or 'auto-fit', ie, both keywords) means keep (or adjust) the intrinsic dimensions.
		bool auto_width = false, auto_height = false;
		const Property* width_property = element->GetProperty(WIDTH);
		if (width_property->unit != Property::KEYWORD)
			content_area.x = element->ResolveProperty(WIDTH, containing_block.x);
		else
			auto_width = true;

		const Property* height_property = element->GetProperty(HEIGHT);
		if (height_property->unit != Property::KEYWORD)
			content_area.y = element->ResolveProperty(HEIGHT, containing_block.y);
		else
			auto_height = true;

		// If one of the dimensions is 'auto' then we need to scale it such that the original ratio is preserved.
		if (auto_width && !auto_height)
			content_area.x = (content_area.y / original_content_area.y) * original_content_area.x;
		else if (auto_height && !auto_width)
			content_area.y = (content_area.x / original_content_area.x) * original_content_area.y;

		// Reduce the width and height to make up for borders and padding.
		content_area.x -= (box.GetEdge(Box::BORDER, Box::LEFT) +
						   box.GetEdge(Box::PADDING, Box::LEFT) +
						   box.GetEdge(Box::BORDER, Box::RIGHT) +
						   box.GetEdge(Box::PADDING, Box::RIGHT));
		content_area.y -= (box.GetEdge(Box::BORDER, Box::TOP) +
						   box.GetEdge(Box::PADDING, Box::TOP) +
						   box.GetEdge(Box::BORDER, Box::BOTTOM) +
						   box.GetEdge(Box::PADDING, Box::BOTTOM));

		content_area.x = Math::Max(content_area.x, 0.0f);
		content_area.y = Math::Max(content_area.y, 0.0f);
	}

	// If the element is inline, then its calculations are much more straightforward (no worrying about auto margins
	// and dimensions, etc). All we do is calculate the margins, set the content area and bail.
	if (inline_element)
	{
		if (replaced_element)
		{
			content_area.x = ClampWidth(content_area.x, element, containing_block.x);
			content_area.y = ClampWidth(content_area.y, element, containing_block.y);
		}

		// If the element was not replaced, then we leave its dimension as unsized (-1, -1) and ignore the width and
		// height properties.
		box.SetContent(content_area);

		// Evaluate the margins. Any declared as 'auto' will resolve to 0.
		box.SetEdge(Box::MARGIN, Box::TOP, element->ResolveProperty(MARGIN_TOP, containing_block.x));
		box.SetEdge(Box::MARGIN, Box::RIGHT, element->ResolveProperty(MARGIN_RIGHT, containing_block.x));
		box.SetEdge(Box::MARGIN, Box::BOTTOM, element->ResolveProperty(MARGIN_BOTTOM, containing_block.x));
		box.SetEdge(Box::MARGIN, Box::LEFT, element->ResolveProperty(MARGIN_LEFT, containing_block.x));
	}

	// The element is block, so we need to run the box through the ringer to potentially evaluate auto margins and
	// dimensions.
	else
	{
		box.SetContent(content_area);
		BuildBoxWidth(box, element, containing_block.x);
		BuildBoxHeight(box, element, containing_block.y);
	}
}

// Generates the box for an element placed in a block box.
void LayoutEngine::BuildBox(Box& box, float& min_height, float& max_height, LayoutBlockBox* containing_box, Element* element, bool inline_element)
{
	Vector2f containing_block = GetContainingBlock(containing_box);
	BuildBox(box, GetContainingBlock(containing_box), element, inline_element);

	float box_height = box.GetSize().y;
	if (box_height < 0)
	{
		if (element->GetLocalProperty(MIN_HEIGHT) != NULL)
			min_height = element->ResolveProperty(MIN_HEIGHT, containing_block.y);
		else
			min_height = 0;

		if (element->GetLocalProperty(MAX_HEIGHT) != NULL)
			max_height = element->ResolveProperty(MAX_HEIGHT, containing_block.y);
		else
			max_height = FLT_MAX;
	}
	else
	{
		min_height = box_height;
		max_height = box_height;
	}
}

// Clamps the width of an element based from its min-width and max-width properties.
float LayoutEngine::ClampWidth(float width, Element* element, float containing_block_width)
{
	float min_width, max_width;

	if (element->GetLocalProperty(MIN_WIDTH) != NULL)
		min_width = element->ResolveProperty(MIN_WIDTH, containing_block_width);
	else
		min_width = 0;

	if (element->GetLocalProperty(MAX_WIDTH) != NULL)
		max_width = element->ResolveProperty(MAX_WIDTH, containing_block_width);
	else
		max_width = FLT_MAX;

	return Math::Clamp(width, min_width, max_width);
}

// Clamps the height of an element based from its min-height and max-height properties.
float LayoutEngine::ClampHeight(float height, Element* element, float containing_block_height)
{
	float min_height, max_height;

	if (element->GetLocalProperty(MIN_HEIGHT) != NULL)
		min_height = element->ResolveProperty(MIN_HEIGHT, containing_block_height);
	else
		min_height = 0;

	if (element->GetLocalProperty(MAX_HEIGHT) != NULL)
		max_height = element->ResolveProperty(MAX_HEIGHT, containing_block_height);
	else
		max_height = FLT_MAX;

	return Math::Clamp(height, min_height, max_height);
}

// Rounds a vector of two floating-point values to integral values.
Vector2f& LayoutEngine::Round(Vector2f& value)
{
	value.x = Round(value.x);
	value.y = Round(value.y);

	return value;
}

// Rounds a floating-point value to an integral value.
float LayoutEngine::Round(float value)
{
	return ceilf(value);
}

void* LayoutEngine::AllocateLayoutChunk(size_t size)
{
	(void)(size);
	ROCKET_ASSERT(size <= LayoutChunk::size);

	return layout_chunk_pool.AllocateObject();
}

void LayoutEngine::DeallocateLayoutChunk(void* chunk)
{
	layout_chunk_pool.DeallocateObject((LayoutChunk*) chunk);
}

// Positions a single element and its children within this layout.
bool LayoutEngine::FormatElement(Element* element)
{
	// Check if we have to do any special formatting for any elements that don't fit into the standard layout scheme.
	if (FormatElementSpecial(element))
		return true;

	// Fetch the display property, and don't lay this element out if it is set to a display type of none.
	int display_property = element->GetProperty< int >(DISPLAY);
	if (display_property == DISPLAY_NONE)
		return true;

	// Check for an absolute position; if this has been set, then we remove it from the flow and add it to the current
	// block box to be laid out and positioned once the block has been closed and sized.
	int position_property = element->GetProperty< int >(POSITION);
	if (position_property == POSITION_ABSOLUTE ||
		position_property == POSITION_FIXED)
	{
		// Display the element as a block element.
		block_context_box->AddAbsoluteElement(element);
		return true;
	}

	// If the element is floating, we remove it from the flow.
	int float_property = element->GetProperty< int >(FLOAT);
	if (float_property != FLOAT_NONE)
	{
		// Format the element as a block element.
		LayoutEngine layout_engine;
		layout_engine.FormatElement(element, GetContainingBlock(block_context_box));

		return block_context_box->AddFloatElement(element);
	}

	// The element is nothing exceptional, so we treat it as a normal block, inline or replaced element.
	switch (display_property)
	{
		case DISPLAY_BLOCK:			return FormatElementBlock(element); break;
		case DISPLAY_INLINE:		return FormatElementInline(element); break;
		case DISPLAY_INLINE_BLOCK:	FormatElementReplaced(element); break;
		default:					ROCKET_ERROR;
	}

	return true;
}

// Formats and positions an element as a block element.
bool LayoutEngine::FormatElementBlock(Element* element)
{
	LayoutBlockBox* new_block_context_box = block_context_box->AddBlockElement(element);
	if (new_block_context_box == NULL)
		return false;

	block_context_box = new_block_context_box;

	// Format the element's children.
	for (int i = 0; i < element->GetNumChildren(); i++)
	{
		if (!FormatElement(element->GetChild(i)))
			i = -1;
	}

	// Close the block box, and check the return code; we may have overflowed either this element or our parent.
	new_block_context_box = block_context_box->GetParent();
	switch (block_context_box->Close())
	{
		// We need to reformat ourself; format all of our children again and close the box. No need to check for error
		// codes, as we already have our vertical slider bar.
		case LayoutBlockBox::LAYOUT_SELF:
		{
			for (int i = 0; i < element->GetNumChildren(); i++)
				FormatElement(element->GetChild(i));

			if (block_context_box->Close() == LayoutBlockBox::OK)
			{
				element->OnLayout();
				break;
			}
		}

		// We caused our parent to add a vertical scrollbar; bail out!
		case LayoutBlockBox::LAYOUT_PARENT:
		{
			block_context_box = new_block_context_box;
			return false;
		}
		break;

		default:
			element->OnLayout();
	}

	block_context_box = new_block_context_box;
	return true;
}

// Formats and positions an element as an inline element.
bool LayoutEngine::FormatElementInline(Element* element)
{
	Box box;
	float min_height, max_height;
	BuildBox(box, min_height, max_height, block_context_box, element, true);
	LayoutInlineBox* inline_box = block_context_box->AddInlineElement(element, box);

	// Format the element's children.
	for (int i = 0; i < element->GetNumChildren(); i++)
	{
		if (!FormatElement(element->GetChild(i)))
			return false;
	}

	inline_box->Close();
//	element->OnLayout();

	return true;
}

// Positions an element as a sized inline element, formatting its internal hierarchy as a block element.
void LayoutEngine::FormatElementReplaced(Element* element)
{
	// Format the element separately as a block element, then position it inside our own layout as an inline element.
	LayoutEngine layout_engine;
	layout_engine.FormatElement(element, GetContainingBlock(block_context_box));

	block_context_box->AddInlineElement(element, element->GetBox())->Close();
}

// Executes any special formatting for special elements.
bool LayoutEngine::FormatElementSpecial(Element* element)
{
	static String br("br");
	
	// Check for a <br> tag.
	if (element->GetTagName() == br)
	{
		block_context_box->AddBreak();
		element->OnLayout();
		return true;
	}

	return false;
}

// Returns the fully-resolved, fixed-width and -height containing block from a block box.
Vector2f LayoutEngine::GetContainingBlock(const LayoutBlockBox* containing_box)
{
	Vector2f containing_block;

	containing_block.x = containing_box->GetBox().GetSize(Box::CONTENT).x;
	if (containing_box->GetElement() != NULL)
		containing_block.x -= containing_box->GetElement()->GetElementScroll()->GetScrollbarSize(ElementScroll::VERTICAL);

	while ((containing_block.y = containing_box->GetBox().GetSize(Box::CONTENT).y) < 0)
	{
		containing_box = containing_box->GetParent();
		if (containing_box == NULL)
		{
			ROCKET_ERROR;
			containing_block.y = 0;
		}
	}
	if (containing_box != NULL &&
		containing_box->GetElement() != NULL)
		containing_block.y -= containing_box->GetElement()->GetElementScroll()->GetScrollbarSize(ElementScroll::HORIZONTAL);

	containing_block.x = Math::Max(0.0f, containing_block.x);
	containing_block.y = Math::Max(0.0f, containing_block.y);

	return containing_block;
}

// Builds the block-specific width and horizontal margins of a Box.
void LayoutEngine::BuildBoxWidth(Box& box, Element* element, float containing_block_width)
{
	Vector2f content_area = box.GetSize();

	// Determine if the element has an automatic width, and if not calculate it.
	bool width_auto;
	if (content_area.x >= 0)
		width_auto = false;
	else
	{
		const Property* width_property = element->GetProperty(WIDTH);
		if (width_property->unit == Property::KEYWORD)
		{
			width_auto = true;
		}
		else
		{
			width_auto = false;
			content_area.x = element->ResolveProperty(WIDTH, containing_block_width);
		}
	}

	// Determine if the element has automatic margins.
	bool margins_auto[2];
	int num_auto_margins = 0;
	for (int i = 0; i < 2; ++i)
	{
		const String& property_name = i == 0 ? MARGIN_LEFT : MARGIN_RIGHT;
		const Property* margin_property = element->GetLocalProperty(property_name);
		if (margin_property != NULL &&
			margin_property->unit == Property::KEYWORD)
		{
			margins_auto[i] = true;
			num_auto_margins++;
		}
		else
		{
			margins_auto[i] = false;
			box.SetEdge(Box::MARGIN, i == 0 ? Box::LEFT : Box::RIGHT, element->ResolveProperty(property_name, containing_block_width));
		}
	}

	// If the width is set to auto, then any margins also set to auto are resolved to 0 and the width is set to the
	// whatever if left of the containing block.
	if (width_auto)
	{
		if (margins_auto[0])
			box.SetEdge(Box::MARGIN, Box::LEFT, 0);
		if (margins_auto[1])
			box.SetEdge(Box::MARGIN, Box::RIGHT, 0);

		content_area.x = containing_block_width - (box.GetCumulativeEdge(Box::CONTENT, Box::LEFT) +
												   box.GetCumulativeEdge(Box::CONTENT, Box::RIGHT));
		content_area.x = Math::Max(0.0f, content_area.x);
	}
	// Otherwise, the margins that are set to auto will pick up the remaining width of the containing block.
	else if (num_auto_margins > 0)
	{
		float margin = (containing_block_width - (box.GetCumulativeEdge(Box::CONTENT, Box::LEFT) +
												  box.GetCumulativeEdge(Box::CONTENT, Box::RIGHT) +
												  content_area.x)) / num_auto_margins;

		if (margins_auto[0])
			box.SetEdge(Box::MARGIN, Box::LEFT, margin);
		if (margins_auto[1])
			box.SetEdge(Box::MARGIN, Box::RIGHT, margin);
	}

	// Clamp the calculated width; if the width is changed by the clamp, then the margins need to be recalculated if
	// they were set to auto.
	float clamped_width = ClampWidth(content_area.x, element, containing_block_width);
	if (clamped_width != content_area.x)
	{
		content_area.x = clamped_width;
		box.SetContent(content_area);

		if (num_auto_margins > 0)
		{
			// Reset the automatic margins.
			if (margins_auto[0])
				box.SetEdge(Box::MARGIN, Box::LEFT, 0);
			if (margins_auto[1])
				box.SetEdge(Box::MARGIN, Box::RIGHT, 0);

			BuildBoxWidth(box, element, containing_block_width);
		}
	}
	else
		box.SetContent(content_area);
}

// Builds the block-specific height and vertical margins of a Box.
void LayoutEngine::BuildBoxHeight(Box& box, Element* element, float containing_block_height)
{
	Vector2f content_area = box.GetSize();

	// Determine if the element has an automatic height, and if not calculate it.
	bool height_auto;
	if (content_area.y >= 0)
		height_auto = false;
	else
	{
		const Property* height_property = element->GetProperty(HEIGHT);
		if (height_property->unit == Property::KEYWORD)
		{
			height_auto = true;
		}
		else
		{
			height_auto = false;
			if (height_property != NULL)
				content_area.y = element->ResolveProperty(HEIGHT, containing_block_height);
		}
	}

	// Determine if the element has automatic margins.
	bool margins_auto[2];
	int num_auto_margins = 0;
	for (int i = 0; i < 2; ++i)
	{
		const String& property_name = i == 0 ? MARGIN_TOP : MARGIN_BOTTOM;
		const Property* margin_property = element->GetLocalProperty(property_name);
		if (margin_property != NULL &&
			margin_property->unit == Property::KEYWORD)
		{
			margins_auto[i] = true;
			num_auto_margins++;
		}
		else
		{
			margins_auto[i] = false;
			box.SetEdge(Box::MARGIN, i == 0 ? Box::TOP : Box::BOTTOM, element->ResolveProperty(property_name, containing_block_height));
		}
	}

	// If the height is set to auto, then any margins also set to auto are resolved to 0 and the height is set to -1.
	if (height_auto)
	{
		if (margins_auto[0])
			box.SetEdge(Box::MARGIN, Box::TOP, 0);
		if (margins_auto[1])
			box.SetEdge(Box::MARGIN, Box::BOTTOM, 0);

		content_area.y = -1;
	}
	// Otherwise, the margins that are set to auto will pick up the remaining width of the containing block.
	else if (num_auto_margins > 0)
	{
		float margin;
		if (content_area.y >= 0)
		{
			margin = (containing_block_height - (box.GetCumulativeEdge(Box::CONTENT, Box::TOP) +
												 box.GetCumulativeEdge(Box::CONTENT, Box::BOTTOM) +
												 content_area.y)) / num_auto_margins;
		}
		else
			margin = 0;

		if (margins_auto[0])
			box.SetEdge(Box::MARGIN, Box::TOP, margin);
		if (margins_auto[1])
			box.SetEdge(Box::MARGIN, Box::BOTTOM, margin);
	}

	if (content_area.y >= 0)
	{
		// Clamp the calculated height; if the height is changed by the clamp, then the margins need to be recalculated if
		// they were set to auto.
		float clamped_height = ClampHeight(content_area.y, element, containing_block_height);
		if (clamped_height != content_area.y)
		{
			content_area.y = clamped_height;
			box.SetContent(content_area);

			if (num_auto_margins > 0)
			{
				// Reset the automatic margins.
				if (margins_auto[0])
					box.SetEdge(Box::MARGIN, Box::TOP, 0);
				if (margins_auto[1])
					box.SetEdge(Box::MARGIN, Box::BOTTOM, 0);

				BuildBoxHeight(box, element, containing_block_height);
			}

			return;
		}
	}

	box.SetContent(content_area);
}

}
}
