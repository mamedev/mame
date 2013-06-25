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

#include "WidgetSlider.h"
#include <Rocket/Core.h>
#include <Rocket/Controls/ElementFormControl.h>

namespace Rocket {
namespace Controls {

const float DEFAULT_REPEAT_DELAY = 0.5f;
const float DEFAULT_REPEAT_PERIOD = 0.1f;

WidgetSlider::WidgetSlider(ElementFormControl* _parent)
{
	parent = _parent;

	orientation = HORIZONTAL;

	track = NULL;
	bar = NULL;
	arrows[0] = NULL;
	arrows[1] = NULL;

	bar_position = 0;

	arrow_timers[0] = -1;
	arrow_timers[1] = -1;
	last_update_time = 0;
}

WidgetSlider::~WidgetSlider()
{
	if (bar != NULL)
	{
		bar->RemoveEventListener("drag", this);
		bar->RemoveEventListener("dragstart", this);
		parent->RemoveChild(bar);
	}

	parent->RemoveEventListener("blur", this);
	parent->RemoveEventListener("focus", this);
	parent->RemoveEventListener("keydown", this, true);

	if (track != NULL)
	{
		track->RemoveEventListener("click", this);
		parent->RemoveChild(track);
	}

	for (int i = 0; i < 2; i++)
	{
		if (arrows[i] != NULL)
		{
			arrows[i]->RemoveEventListener("mousedown", this);
			arrows[i]->RemoveEventListener("mouseup", this);
			arrows[i]->RemoveEventListener("mouseout", this);
			parent->RemoveChild(arrows[i]);
		}
	}
}

// Initialises the slider to a given orientation.
bool WidgetSlider::Initialise()
{
	// Create all of our child elements as standard elements, and abort if we can't create them.
	track = Core::Factory::InstanceElement(parent, "*", "slidertrack", Rocket::Core::XMLAttributes());

	bar = Core::Factory::InstanceElement(parent, "*", "sliderbar", Rocket::Core::XMLAttributes());
	bar->SetProperty("drag", "drag");

	arrows[0] = Core::Factory::InstanceElement(parent, "*", "sliderarrowdec", Rocket::Core::XMLAttributes());
	arrows[1] = Core::Factory::InstanceElement(parent, "*", "sliderarrowinc", Rocket::Core::XMLAttributes());

	if (track == NULL ||
		bar == NULL ||
		arrows[0] == NULL ||
		arrows[1] == NULL)
	{
		if (track != NULL)
			track->RemoveReference();

		if (bar != NULL)
			bar->RemoveReference();

		if (arrows[0] != NULL)
			arrows[0]->RemoveReference();

		if (arrows[1] != NULL)
			arrows[1]->RemoveReference();

		return false;
	}

	// Add them as non-DOM elements.
	parent->AppendChild(track, false);
	parent->AppendChild(bar, false);
	parent->AppendChild(arrows[0], false);
	parent->AppendChild(arrows[1], false);

	// Remove the initial references on the elements.
	track->RemoveReference();
	bar->RemoveReference();
	arrows[0]->RemoveReference();
	arrows[1]->RemoveReference();

	// Attach the listeners as appropriate.
	bar->AddEventListener("drag", this);
	bar->AddEventListener("dragstart", this);

	parent->AddEventListener("blur", this);
	parent->AddEventListener("focus", this);
	parent->AddEventListener("keydown", this, true);
	track->AddEventListener("click", this);

	for (int i = 0; i < 2; i++)
	{
		arrows[i]->AddEventListener("mousedown", this);
		arrows[i]->AddEventListener("mouseup", this);
		arrows[i]->AddEventListener("mouseout", this);
	}

	return true;
}

// Updates the key repeats for the increment / decrement arrows.
void WidgetSlider::Update()
{
	for (int i = 0; i < 2; i++)
	{
		bool updated_time = false;
		float delta_time = 0;

		if (arrow_timers[i] > 0)
		{
			if (!updated_time)
			{
				float current_time = Core::GetSystemInterface()->GetElapsedTime();
				delta_time = current_time - last_update_time;
				last_update_time = current_time;
			}

			arrow_timers[i] -= delta_time;
			while (arrow_timers[i] <= 0)
			{
				arrow_timers[i] += DEFAULT_REPEAT_PERIOD;
				SetBarPosition(i == 0 ? OnLineDecrement() : OnLineIncrement());
			}
		}
	}
}

// Sets the position of the bar.
void WidgetSlider::SetBarPosition(float _bar_position)
{
	bar_position = Rocket::Core::Math::Clamp(_bar_position, 0.0f, 1.0f);
	PositionBar();

	Rocket::Core::Dictionary parameters;
	parameters.Set("value", bar_position);
	parent->DispatchEvent("change", parameters);
}

// Returns the current position of the bar.
float WidgetSlider::GetBarPosition()
{
	return bar_position;
}

// Sets the orientation of the slider.
void WidgetSlider::SetOrientation(Orientation _orientation)
{
	orientation = _orientation;
}

// Returns the slider's orientation.
WidgetSlider::Orientation WidgetSlider::GetOrientation() const
{
	return orientation;
}

// Sets the dimensions to the size of the slider.
void WidgetSlider::GetDimensions(Rocket::Core::Vector2f& dimensions) const
{
	switch (orientation)
	{
		case VERTICAL:		dimensions.x = 16; dimensions.y = 256; break;
		case HORIZONTAL:	dimensions.x = 256; dimensions.y = 16; break;
	}
}

// Lays out and resizes the internal elements.
void WidgetSlider::FormatElements(const Rocket::Core::Vector2f& containing_block, float slider_length, float bar_length)
{
	int length_axis = orientation == VERTICAL ? 1 : 0;

	// Build the box for the containing slider element. As the containing block is not guaranteed to have a defined
	// height, we must use the width for both axes.
	Core::Box parent_box;
	Core::ElementUtilities::BuildBox(parent_box, Rocket::Core::Vector2f(containing_block.x, containing_block.x), parent);

	// Set the length of the slider.
	Rocket::Core::Vector2f content = parent_box.GetSize();
	content[length_axis] = slider_length;
	parent_box.SetContent(content);

	// Generate the initial dimensions for the track. It'll need to be cut down to fit the arrows.
	Core::Box track_box;
	Core::ElementUtilities::BuildBox(track_box, parent_box.GetSize(), track);
	content = track_box.GetSize();
	content[length_axis] = slider_length -= orientation == VERTICAL ? (track_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::TOP) + track_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::BOTTOM)) :
																	  (track_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::LEFT) + track_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::RIGHT));
	// If no height has been explicitly specified for the track, it'll be initialised to -1 as per normal block
	// elements. We'll fix that up here.
	if (orientation == HORIZONTAL &&
		content.y < 0)
		content.y = parent_box.GetSize().y;

	// Now we size the arrows.
	for (int i = 0; i < 2; i++)
	{
		Core::Box arrow_box;
		Core::ElementUtilities::BuildBox(arrow_box, parent_box.GetSize(), arrows[i]);

		// Clamp the size to (0, 0).
		Rocket::Core::Vector2f arrow_size = arrow_box.GetSize();
		if (arrow_size.x < 0 ||
			arrow_size.y < 0)
			arrow_box.SetContent(Rocket::Core::Vector2f(0, 0));

		arrows[i]->SetBox(arrow_box);

		// Shrink the track length by the arrow size.
		content[length_axis] -= arrow_box.GetSize(Core::Box::MARGIN)[length_axis];
	}

	// Now the track has been sized, we can fix everything into position.
	track_box.SetContent(content);
	track->SetBox(track_box);

	if (orientation == VERTICAL)
	{
		Rocket::Core::Vector2f offset(arrows[0]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT), arrows[0]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP));
		arrows[0]->SetOffset(offset, parent);

		offset.x = track->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT);
		offset.y += arrows[0]->GetBox().GetSize(Core::Box::BORDER).y + arrows[0]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::BOTTOM) + track->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP);
		track->SetOffset(offset, parent);

		offset.x = arrows[1]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT);
		offset.y += track->GetBox().GetSize(Core::Box::BORDER).y + track->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::BOTTOM) + arrows[1]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP);
		arrows[1]->SetOffset(offset, parent);
	}
	else
	{
		Rocket::Core::Vector2f offset(arrows[0]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT), arrows[0]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP));
		arrows[0]->SetOffset(offset, parent);

		offset.x += arrows[0]->GetBox().GetSize(Core::Box::BORDER).x + arrows[0]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::RIGHT) + track->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT);
		offset.y = track->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP);
		track->SetOffset(offset, parent);

		offset.x += track->GetBox().GetSize(Core::Box::BORDER).x + track->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::RIGHT) + arrows[1]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT);
		offset.y = arrows[1]->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP);
		arrows[1]->SetOffset(offset, parent);
	}

	FormatBar(bar_length);

	if (parent->IsDisabled())
	{
	    // Propagate disabled state to child elements
	    bar->SetPseudoClass("disabled", true);
	    track->SetPseudoClass("disabled", true);
	    arrows[0]->SetPseudoClass("disabled", true);
	    arrows[1]->SetPseudoClass("disabled", true);
	}
}

// Lays out and positions the bar element.
void WidgetSlider::FormatBar(float bar_length)
{
	Core::Box bar_box;
	Core::ElementUtilities::BuildBox(bar_box, parent->GetBox().GetSize(), bar);

	Rocket::Core::Vector2f bar_box_content = bar_box.GetSize();
	if (orientation == HORIZONTAL)
	{
		if (bar->GetLocalProperty("height") == NULL)
			bar_box_content.y = parent->GetBox().GetSize().y;
	}

	if (bar_length >= 0)
	{
		Rocket::Core::Vector2f track_size = track->GetBox().GetSize();

		if (orientation == VERTICAL)
		{
			float track_length = track_size.y - (bar_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::TOP) + bar_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::BOTTOM));

			if (bar->GetLocalProperty("height") == NULL)
			{
				bar_box_content.y = track_length * bar_length;

				// Check for 'min-height' restrictions.
				float min_track_length = bar->ResolveProperty("min-height", track_length);
				bar_box_content.y = Rocket::Core::Math::Max(min_track_length, bar_box_content.y);

				// Check for 'max-height' restrictions.
				float max_track_length = bar->ResolveProperty("max-height", track_length);
				if (max_track_length > 0)
					bar_box_content.y = Rocket::Core::Math::Min(max_track_length, bar_box_content.y);
			}

			// Make sure we haven't gone further than we're allowed to (min-height may have made us too big).
			bar_box_content.y = Rocket::Core::Math::Min(bar_box_content.y, track_length);
		}
		else
		{
			float track_length = track_size.x - (bar_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::LEFT) + bar_box.GetCumulativeEdge(Core::Box::CONTENT, Core::Box::RIGHT));

			if (bar->GetLocalProperty("width") == NULL)
			{
				bar_box_content.x = track_length * bar_length;

				// Check for 'min-width' restrictions.
				float min_track_length = bar->ResolveProperty("min-width", track_length);
				bar_box_content.x = Rocket::Core::Math::Max(min_track_length, bar_box_content.x);

				// Check for 'max-width' restrictions.
				float max_track_length = bar->ResolveProperty("max-width", track_length);
				if (max_track_length > 0)
					bar_box_content.x = Rocket::Core::Math::Min(max_track_length, bar_box_content.x);
			}

			// Make sure we haven't gone further than we're allowed to (min-width may have made us too big).
			bar_box_content.x = Rocket::Core::Math::Min(bar_box_content.x, track_length);
		}
	}

	// Set the new dimensions on the bar to re-decorate it.
	bar_box.SetContent(bar_box_content);
	bar->SetBox(bar_box);

	// Now that it's been resized, re-position it.
	PositionBar();
}

// Returns the widget's parent element.
Core::Element* WidgetSlider::GetParent() const
{
	return parent;
}

// Handles events coming through from the slider's components.
void WidgetSlider::ProcessEvent(Core::Event& event)
{
	if (parent->IsDisabled())
		return;

	if (event.GetTargetElement() == bar)
	{
		if (event == "drag")
		{
			if (orientation == HORIZONTAL)
			{
				float traversable_track_length = track->GetBox().GetSize(Core::Box::CONTENT).x - bar->GetBox().GetSize(Core::Box::CONTENT).x;
				if (traversable_track_length > 0)
				{
					float traversable_track_origin = track->GetAbsoluteOffset().x + bar_drag_anchor;
					float new_bar_position = (event.GetParameter< float >("mouse_x", 0) - traversable_track_origin) / traversable_track_length;
					new_bar_position = Rocket::Core::Math::Clamp(new_bar_position, 0.0f, 1.0f);

					SetBarPosition(OnBarChange(new_bar_position));
				}
			}
			else
			{
				float traversable_track_length = track->GetBox().GetSize(Core::Box::CONTENT).y - bar->GetBox().GetSize(Core::Box::CONTENT).y;
				if (traversable_track_length > 0)
				{
					float traversable_track_origin = track->GetAbsoluteOffset().y + bar_drag_anchor;
					float new_bar_position = (event.GetParameter< float >("mouse_y", 0) - traversable_track_origin) / traversable_track_length;
					new_bar_position = Rocket::Core::Math::Clamp(new_bar_position, 0.0f, 1.0f);

					SetBarPosition(OnBarChange(new_bar_position));
				}
			}
		}
		else if (event == "dragstart")
		{
			if (orientation == HORIZONTAL)
				bar_drag_anchor = event.GetParameter< int >("mouse_x", 0) - Rocket::Core::Math::RealToInteger(bar->GetAbsoluteOffset().x);
			else
				bar_drag_anchor = event.GetParameter< int >("mouse_y", 0) - Rocket::Core::Math::RealToInteger(bar->GetAbsoluteOffset().y);
		}
	}
	else if (event.GetTargetElement() == track)
	{
		if (event == "click")
		{
			if (orientation == HORIZONTAL)
			{
				float mouse_position = event.GetParameter< float >("mouse_x", 0);
				float click_position = (mouse_position - track->GetAbsoluteOffset().x) / track->GetBox().GetSize().x;

				SetBarPosition(click_position <= bar_position ? OnPageDecrement(click_position) : OnPageIncrement(click_position));
			}
			else
			{
				float mouse_position = event.GetParameter< float >("mouse_y", 0);
				float click_position = (mouse_position - track->GetAbsoluteOffset().y) / track->GetBox().GetSize().y;

				SetBarPosition(click_position <= bar_position ? OnPageDecrement(click_position) : OnPageIncrement(click_position));
			}
		}
	}

	if (event == "mousedown")
	{
		if (event.GetTargetElement() == arrows[0])
		{
			arrow_timers[0] = DEFAULT_REPEAT_DELAY;
			last_update_time = Core::GetSystemInterface()->GetElapsedTime();
			SetBarPosition(OnLineDecrement());
		}
		else if (event.GetTargetElement() == arrows[1])
		{
			arrow_timers[1] = DEFAULT_REPEAT_DELAY;
			last_update_time = Core::GetSystemInterface()->GetElapsedTime();
			SetBarPosition(OnLineIncrement());
		}
	}
	else if (event == "mouseup" ||
			 event == "mouseout")
	{
		if (event.GetTargetElement() == arrows[0])
			arrow_timers[0] = -1;
		else if (event.GetTargetElement() == arrows[1])
			arrow_timers[1] = -1;
	}
	else if (event == "keydown")
	{
		Core::Input::KeyIdentifier key_identifier = (Core::Input::KeyIdentifier) event.GetParameter< int >("key_identifier", 0);

		switch (key_identifier)
		{
			case Core::Input::KI_LEFT:
				if (orientation == HORIZONTAL) SetBarPosition(OnLineDecrement());
				break;
			case Core::Input::KI_UP:
				if (orientation == VERTICAL) SetBarPosition(OnLineDecrement());
				break;
			case Core::Input::KI_RIGHT:
				if (orientation == HORIZONTAL) SetBarPosition(OnLineIncrement());
				break;
			case Core::Input::KI_DOWN:		
				if (orientation == VERTICAL) SetBarPosition(OnLineIncrement());
				break;
			default:
				break;
		}
	}


	if (event.GetTargetElement() == parent)
	{
		if (event == "focus")
		{
			bar->SetPseudoClass("focus", true);
		}
		else if (event == "blur")
		{
			bar->SetPseudoClass("focus", false);
		}
	}
}

void WidgetSlider::PositionBar()
{
	const Rocket::Core::Vector2f& track_dimensions = track->GetBox().GetSize();
	const Rocket::Core::Vector2f& bar_dimensions = bar->GetBox().GetSize(Core::Box::BORDER);

	if (orientation == VERTICAL)
	{
		float traversable_track_length = track_dimensions.y - bar_dimensions.y;
		bar->SetOffset(Rocket::Core::Vector2f(bar->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::LEFT), track->GetRelativeOffset().y + traversable_track_length * bar_position), parent);
	}
	else
	{
		float traversable_track_length = track_dimensions.x - bar_dimensions.x;
		bar->SetOffset(Rocket::Core::Vector2f(track->GetRelativeOffset().x + traversable_track_length * bar_position, bar->GetBox().GetEdge(Core::Box::MARGIN, Core::Box::TOP)), parent);
	}
}

}
}
