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

#include "WidgetDropDown.h"
#include <Rocket/Core/Math.h>
#include <Rocket/Core/Factory.h>
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/Input.h>
#include <Rocket/Core/Property.h>
#include <Rocket/Core/StyleSheetKeywords.h>
#include <Rocket/Controls/ElementFormControl.h>

namespace Rocket {
namespace Controls {

WidgetDropDown::WidgetDropDown(ElementFormControl* element)
{
	parent_element = element;

	box_layout_dirty = false;
	value_layout_dirty = false;

	selected_option = -1;

	// Create the button and selection elements.
	button_element = Core::Factory::InstanceElement(parent_element, "*", "selectarrow", Rocket::Core::XMLAttributes());
	value_element = Core::Factory::InstanceElement(element, "*", "selectvalue", Rocket::Core::XMLAttributes());
	selection_element = Core::Factory::InstanceElement(parent_element, "*", "selectbox", Rocket::Core::XMLAttributes());

	value_element->SetProperty("overflow", "hidden");

	selection_element->SetProperty("visibility", "hidden");
	selection_element->SetProperty("z-index", Core::Property(1.0f, Core::Property::NUMBER));
	selection_element->SetProperty("clip", "none");

	parent_element->AddEventListener("click", this, true);
	parent_element->AddEventListener("blur", this);
	parent_element->AddEventListener("focus", this);
	parent_element->AddEventListener("keydown", this, true);

	// Add the elements to our parent element.
	parent_element->AppendChild(button_element, false);
	parent_element->AppendChild(selection_element, false);
	parent_element->AppendChild(value_element, false);
}

WidgetDropDown::~WidgetDropDown()
{
	ClearOptions();

	parent_element->RemoveEventListener("click", this, true);
	parent_element->RemoveEventListener("blur", this);
	parent_element->RemoveEventListener("focus", this);
	parent_element->RemoveEventListener("keydown", this, true);

	button_element->RemoveReference();
	selection_element->RemoveReference();
	value_element->RemoveReference();
}

// Updates the selection box layout if necessary.
void WidgetDropDown::OnRender()
{
	if (box_layout_dirty)
	{
		Core::Box box;
		Core::ElementUtilities::BuildBox(box, parent_element->GetBox().GetSize(), selection_element);

		// Layout the selection box.
		Core::ElementUtilities::FormatElement(selection_element, parent_element->GetBox().GetSize(Core::Box::BORDER));
		selection_element->SetOffset(Rocket::Core::Vector2f(box.GetEdge(Core::Box::MARGIN, Core::Box::LEFT), parent_element->GetBox().GetSize(Core::Box::BORDER).y + box.GetEdge(Core::Box::MARGIN, Core::Box::TOP)), parent_element);

		box_layout_dirty = false;
	}

	if (value_layout_dirty)
	{
		Core::ElementUtilities::FormatElement(value_element, parent_element->GetBox().GetSize(Core::Box::BORDER));
		value_element->SetOffset(parent_element->GetBox().GetPosition(Core::Box::CONTENT), parent_element);

		value_layout_dirty = false;
	}
}

void WidgetDropDown::OnLayout()
{
	if(parent_element->IsDisabled())
	{
		// Propagate disabled state to selectvalue and selectarrow
		value_element->SetPseudoClass("disabled", true);
		button_element->SetPseudoClass("disabled", true);
	}

	// Layout the button and selection boxes.
	Core::Box parent_box = parent_element->GetBox();

	Core::ElementUtilities::PositionElement(button_element, Rocket::Core::Vector2f(0, 0), Core::ElementUtilities::TOP_RIGHT);
	Core::ElementUtilities::PositionElement(selection_element, Rocket::Core::Vector2f(0, 0), Core::ElementUtilities::TOP_LEFT);

	// Calculate the value element position and size.
	Rocket::Core::Vector2f size;
	size.x = parent_element->GetBox().GetSize(Core::Box::CONTENT).x - button_element->GetBox().GetSize(Core::Box::MARGIN).x;
	size.y = parent_element->GetBox().GetSize(Core::Box::CONTENT).y;

	value_element->SetOffset(parent_element->GetBox().GetPosition(Core::Box::CONTENT), parent_element);
	value_element->SetBox(Core::Box(size));

	box_layout_dirty = true;
	value_layout_dirty = true;
}

// Sets the value of the widget.
void WidgetDropDown::SetValue(const Rocket::Core::String& _value)
{
	for (size_t i = 0; i < options.size(); ++i)
	{
		if (options[i].GetValue() == _value)
		{
			SetSelection((int) i);
			return;
		}
	}

	value = _value;
	value_element->SetInnerRML(value);
	value_layout_dirty = true;

	selected_option = -1;
}

// Returns the current value of the widget.
const Rocket::Core::String& WidgetDropDown::GetValue() const
{
	return value;
}

// Sets the index of the selection. If the new index lies outside of the bounds, it will be clamped.
void WidgetDropDown::SetSelection(int selection, bool force)
{
	Rocket::Core::String new_value;

	if (selection < 0 ||
		selection >= (int) options.size())
	{
		selection = -1;
	}
	else
	{
		new_value = options[selection].GetValue();
	}

	if (force ||
		selection != selected_option ||
		value != new_value)
	{
		selected_option = selection;
		value = new_value;

		Rocket::Core::String value_rml;
		if (selected_option >= 0)
			options[selected_option].GetElement()->GetInnerRML(value_rml);

		value_element->SetInnerRML(value_rml);
		value_layout_dirty = true;

		Rocket::Core::Dictionary parameters;
		parameters.Set("value", value);
		parent_element->DispatchEvent("change", parameters);
	}
}

// Returns the index of the currently selected item.
int WidgetDropDown::GetSelection() const
{
	return selected_option;
}

// Adds a new option to the select control.
int WidgetDropDown::AddOption(const Rocket::Core::String& rml, const Rocket::Core::String& value, int before, bool select, bool selectable)
{
	// Instance a new element for the option.
	Core::Element* element = Core::Factory::InstanceElement(selection_element, "*", "option", Rocket::Core::XMLAttributes());

	// Force to block display and inject the RML. Register a click handler so we can be notified of selection.
	element->SetProperty("display", "block");
	element->SetProperty("clip", "auto");
	element->SetInnerRML(rml);
	element->AddEventListener("click", this);

	int option_index;
	if (before < 0 ||
		before >= (int) options.size())
	{
		selection_element->AppendChild(element);
		options.push_back(SelectOption(element, value, selectable));
		option_index = (int) options.size() - 1;
	}
	else
	{
		selection_element->InsertBefore(element, selection_element->GetChild(before));
		options.insert(options.begin() + before, SelectOption(element, value, selectable));
		option_index = before;
	}

	element->RemoveReference();

	// Select the option if appropriate.
	if (select)
		SetSelection(option_index);

	box_layout_dirty = true;
	return option_index;
}

// Removes an option from the select control.
void WidgetDropDown::RemoveOption(int index)
{
	if (index < 0 ||
		index >= (int) options.size())
		return;

	// Remove the listener and delete the option element.
	options[index].GetElement()->RemoveEventListener("click", this);
	selection_element->RemoveChild(options[index].GetElement());
	options.erase(options.begin() + index);

	box_layout_dirty = true;
}

// Removes all options from the list.
void WidgetDropDown::ClearOptions()
{
	while (!options.empty())
		RemoveOption((int) options.size() - 1);
}

// Returns on of the widget's options.
SelectOption* WidgetDropDown::GetOption(int index)
{
	if (index < 0 ||
		index >= GetNumOptions())
		return NULL;

	return &options[index];
}

// Returns the number of options in the widget.
int WidgetDropDown::GetNumOptions() const
{
	return (int) options.size();
}

void WidgetDropDown::ProcessEvent(Core::Event& event)
{
	if (parent_element->IsDisabled()) 
		return;

	// Process the button onclick
	if (event == "click")
	{

		if (event.GetCurrentElement()->GetParentNode() == selection_element)
		{
			// Find the element in the options and fire the selection event
			for (size_t i = 0; i < options.size(); i++)
			{
				if (options[i].GetElement() == event.GetCurrentElement())
				{
					if (options[i].IsSelectable())
					{
						SetSelection(i);
						event.StopPropagation();

						ShowSelectBox(false);
						parent_element->Focus();
					}
				}
			}
		}
		else
		{
			// We have to check that this event isn't targeted to an element
			// inside the selection box as we'll get all events coming from our
			// root level select element as well as the ones coming from options (which
			// get caught in the above if)
			Core::Element* element = event.GetTargetElement();
			while (element && element != parent_element)
			{
				if (element == selection_element)
					return;
				element = element->GetParentNode();
			}

			if (selection_element->GetProperty< int >("visibility") == Core::VISIBILITY_HIDDEN)
				ShowSelectBox(true);
			else
				ShowSelectBox(false);
		}		
	}
	else if (event == "blur" && event.GetTargetElement() == parent_element)
	{
		ShowSelectBox(false);
	}
	else if (event == "keydown")
	{
		Core::Input::KeyIdentifier key_identifier = (Core::Input::KeyIdentifier) event.GetParameter< int >("key_identifier", 0);

		switch (key_identifier)
		{
			case Core::Input::KI_UP:
				SetSelection( (selected_option - 1 + options.size()) % options.size() );
				break;
			case Core::Input::KI_DOWN:		
				SetSelection( (selected_option + 1) % options.size() );
				break;
			default:
				break;
		}
	}

	if (event.GetTargetElement() == parent_element)
	{
		if (event == "focus")
		{
			value_element->SetPseudoClass("focus", true);
			button_element->SetPseudoClass("focus", true);
		}
		else if (event == "blur")
		{
			value_element->SetPseudoClass("focus", false);
			button_element->SetPseudoClass("focus", false);
		}
	}

}

// Shows or hides the selection box.
void WidgetDropDown::ShowSelectBox(bool show)
{
	if (show)
	{
		selection_element->SetProperty("visibility", "visible");
		value_element->SetPseudoClass("checked", true);
		button_element->SetPseudoClass("checked", true);
	}
	else
	{
		selection_element->SetProperty("visibility", "hidden");
		value_element->SetPseudoClass("checked", false);
		button_element->SetPseudoClass("checked", false);
	}
}

}
}
