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
#include <Rocket/Core/ElementScroll.h>
#include "LayoutEngine.h"
#include "WidgetSliderScroll.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/Factory.h>

namespace Rocket {
namespace Core {

ElementScroll::ElementScroll(Element* _element)
{
	element = _element;
	corner = NULL;
}

ElementScroll::~ElementScroll()
{
	for (int i = 0; i < 2; i++)
	{
		if (scrollbars[i].element != NULL)
			scrollbars[i].element->RemoveEventListener("scrollchange", this);
	}
}

// Updates the increment / decrement arrows.
void ElementScroll::Update()
{
	for (int i = 0; i < 2; i++)
	{
		if (scrollbars[i].widget != NULL)
			scrollbars[i].widget->Update();
	}
}

// Enables and sizes one of the scrollbars.
void ElementScroll::EnableScrollbar(Orientation orientation, float element_width)
{
	if (!scrollbars[orientation].enabled)
	{
		CreateScrollbar(orientation);
		scrollbars[orientation].element->SetProperty(VISIBILITY, "visible");
		scrollbars[orientation].enabled = true;
	}

	// Determine the size of the scrollbar.
	Box box;
	LayoutEngine::BuildBox(box, Vector2f(element_width, element_width), scrollbars[orientation].element);

	if (orientation == VERTICAL)
		scrollbars[orientation].size = box.GetSize(Box::MARGIN).x;
	if (orientation == HORIZONTAL)
	{
		if (box.GetSize().y < 0)
			scrollbars[orientation].size = box.GetCumulativeEdge(Box::CONTENT, Box::LEFT) +
										   box.GetCumulativeEdge(Box::CONTENT, Box::RIGHT) +
										   scrollbars[orientation].element->ResolveProperty(HEIGHT, element_width);
		else
			scrollbars[orientation].size = box.GetSize(Box::MARGIN).y;
	}
}

// Disables and hides one of the scrollbars.
void ElementScroll::DisableScrollbar(Orientation orientation)
{
	if (scrollbars[orientation].enabled)
	{
		scrollbars[orientation].element->SetProperty(VISIBILITY, "hidden");
		scrollbars[orientation].enabled = false;
	}
}

// Updates the position of the scrollbar.
void ElementScroll::UpdateScrollbar(Orientation orientation)
{
	float bar_position;
	float traversable_track;
	if (orientation == VERTICAL)
	{
		bar_position = element->GetScrollTop();
		traversable_track = element->GetScrollHeight() - element->GetClientHeight();
	}
	else
	{
		bar_position = element->GetScrollLeft();
		traversable_track = element->GetScrollWidth() - element->GetClientWidth();
	}

	if (traversable_track <= 0)
		bar_position = 0;
	else
		bar_position /= traversable_track;

	if (scrollbars[orientation].widget != NULL)
	{
		bar_position = Math::Clamp(bar_position, 0.0f, 1.0f);

		if (scrollbars[orientation].widget->GetBarPosition() != bar_position)
			scrollbars[orientation].widget->SetBarPosition(bar_position);
	}
}

// Returns one of the scrollbar elements.
Element* ElementScroll::GetScrollbar(Orientation orientation)
{
	return scrollbars[orientation].element;
}

// Returns the size, in pixels, of one of the scrollbars; for a vertical scrollbar, this is width, for a horizontal scrollbar, this is height.
float ElementScroll::GetScrollbarSize(Orientation orientation)
{
	if (!scrollbars[orientation].enabled)
		return 0;

	return scrollbars[orientation].size;
}

// Formats the enabled scrollbars based on the current size of the host element.
void ElementScroll::FormatScrollbars()
{
	Vector2f containing_block = element->GetBox().GetSize(Box::PADDING);

	for (int i = 0; i < 2; i++)
	{
		if (!scrollbars[i].enabled)
			continue;

		if (i == VERTICAL)
		{
			scrollbars[i].widget->SetBarLength(element->GetClientHeight());
			scrollbars[i].widget->SetTrackLength(element->GetScrollHeight());

			float traversable_track = element->GetScrollHeight() - element->GetClientHeight();
			if (traversable_track > 0)
				scrollbars[i].widget->SetBarPosition(element->GetScrollTop() / traversable_track);
			else
				scrollbars[i].widget->SetBarPosition(0);
		}
		else
		{
			scrollbars[i].widget->SetBarLength(element->GetClientWidth());
			scrollbars[i].widget->SetTrackLength(element->GetScrollWidth());

			float traversable_track = element->GetScrollWidth() - element->GetClientWidth();
			if (traversable_track > 0)
				scrollbars[i].widget->SetBarPosition(element->GetScrollLeft() / traversable_track);
			else
				scrollbars[i].widget->SetBarPosition(0);
		}

		float slider_length = containing_block[1 - i];
		float user_scrollbar_margin = scrollbars[i].element->ResolveProperty(SCROLLBAR_MARGIN, slider_length);
		float min_scrollbar_margin = GetScrollbarSize(i == VERTICAL ? HORIZONTAL : VERTICAL);
		slider_length -= Math::Max(user_scrollbar_margin, min_scrollbar_margin);

		scrollbars[i].widget->FormatElements(containing_block, slider_length);
		scrollbars[i].widget->SetLineHeight((float) ElementUtilities::GetLineHeight(element));

		int variable_axis = i == VERTICAL ? 0 : 1;
		Vector2f offset = element->GetBox().GetPosition(Box::PADDING);
		offset[variable_axis] += containing_block[variable_axis] - (scrollbars[i].element->GetBox().GetSize(Box::BORDER)[variable_axis] + scrollbars[i].element->GetBox().GetEdge(Box::MARGIN, i == VERTICAL ? Box::RIGHT : Box::BOTTOM));
		// Add the top or left margin (as appropriate) onto the scrollbar's position.
		offset[1 - variable_axis] += scrollbars[i].element->GetBox().GetEdge(Box::MARGIN, i == VERTICAL ? Box::TOP : Box::LEFT);
		scrollbars[i].element->SetOffset(offset, element, true);
	}

	// Format the corner, if it is necessary.
	if (scrollbars[0].enabled &&
		scrollbars[1].enabled)
	{
		CreateCorner();

		Box corner_box;
		corner_box.SetContent(Vector2f(scrollbars[VERTICAL].size, scrollbars[HORIZONTAL].size));
		corner->SetBox(corner_box);
		corner->SetOffset(containing_block - Vector2f(scrollbars[VERTICAL].size, scrollbars[HORIZONTAL].size), element, true);

		corner->SetProperty(VISIBILITY, "visible");
	}
	else
	{
		if (corner != NULL)
			corner->SetProperty(VISIBILITY, "hidden");
	}
}

// Handles the 'onchange' events for the scrollbars.
void ElementScroll::ProcessEvent(Event& event)
{
	if (event == "scrollchange")
	{
		float value = event.GetParameter< float >("value", 0);

		if (event.GetTargetElement() == scrollbars[VERTICAL].element)
			element->SetScrollTop(value * (element->GetScrollHeight() - element->GetClientHeight()));
		else
			element->SetScrollLeft(value * (element->GetScrollWidth() - element->GetClientWidth()));
	}
}

// Creates one of the scroll component's scrollbar.
bool ElementScroll::CreateScrollbar(Orientation orientation)
{
	if (scrollbars[orientation].element != NULL &&
		scrollbars[orientation].widget != NULL)
		return true;

	scrollbars[orientation].element = Factory::InstanceElement(element, "*", orientation == VERTICAL ? "scrollbarvertical" : "scrollbarhorizontal", XMLAttributes());
	scrollbars[orientation].element->AddEventListener("scrollchange", this);
	scrollbars[orientation].element->SetProperty(CLIP, "1");

	scrollbars[orientation].widget = new WidgetSliderScroll(scrollbars[orientation].element);
	scrollbars[orientation].widget->Initialise(orientation == VERTICAL ? WidgetSlider::VERTICAL : WidgetSlider::HORIZONTAL);

	element->AppendChild(scrollbars[orientation].element, false);
	scrollbars[orientation].element->RemoveReference();

	return true;
}

// Creates the scrollbar corner.
bool ElementScroll::CreateCorner()
{
	if (corner != NULL)
		return true;

	corner = Factory::InstanceElement(element, "*", "scrollbarcorner", XMLAttributes());
	element->AppendChild(corner, false);
	corner->RemoveReference();

	return true;
}

ElementScroll::Scrollbar::Scrollbar()
{
	element = NULL;
	widget = NULL;
	enabled = false;
	size = 0;
}

ElementScroll::Scrollbar::~Scrollbar()
{
	if (widget != NULL)
		delete widget;

	if (element != NULL)
	{
		if (element->GetParentNode() != NULL)
			element->GetParentNode()->RemoveChild(element);
	}
}

}
}
