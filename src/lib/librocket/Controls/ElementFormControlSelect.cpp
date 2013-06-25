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

#include <Rocket/Controls/ElementFormControlSelect.h>
#include <Rocket/Core/ElementText.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/ElementUtilities.h>
#include "WidgetDropDown.h"

namespace Rocket {
namespace Controls {

// Constructs a new ElementFormControlSelect.
ElementFormControlSelect::ElementFormControlSelect(const Rocket::Core::String& tag) : ElementFormControl(tag)
{
	widget = new WidgetDropDown(this);
}

ElementFormControlSelect::~ElementFormControlSelect()
{
	delete widget;
}

// Returns a string representation of the current value of the form control.
Rocket::Core::String ElementFormControlSelect::GetValue() const
{
	ROCKET_ASSERT(widget != NULL);
	return widget->GetValue();
}

// Sets the current value of the form control.
void ElementFormControlSelect::SetValue(const Rocket::Core::String& value)
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	widget->SetValue(value);
}

// Sets the index of the selection. If the new index lies outside of the bounds, it will be clamped.
void ElementFormControlSelect::SetSelection(int selection)
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	widget->SetSelection(selection);
}

// Returns the index of the currently selected item.
int ElementFormControlSelect::GetSelection() const
{
	ROCKET_ASSERT(widget != NULL);
	return widget->GetSelection();
}

// Returns one of the select control's option elements.
SelectOption* ElementFormControlSelect::GetOption(int index)
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	return widget->GetOption(index);
}

// Returns the number of options in the select control.
int ElementFormControlSelect::GetNumOptions()
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	return widget->GetNumOptions();
}

// Adds a new option to the select control.
int ElementFormControlSelect::Add(const Rocket::Core::String& rml, const Rocket::Core::String& value, int before, bool selectable)
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	return widget->AddOption(rml, value, before, false, selectable);
}

// Removes an option from the select control.
void ElementFormControlSelect::Remove(int index)
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	widget->RemoveOption(index);
}

// Removes all options from the select control.
void ElementFormControlSelect::RemoveAll()
{
	OnUpdate();

	ROCKET_ASSERT(widget != NULL);
	widget->ClearOptions();
}

// Moves all children to be under control of the widget.
void ElementFormControlSelect::OnUpdate()
{
	ElementFormControl::OnUpdate();

	// Move any child elements into the widget (except for the three functional elements).
	while (HasChildNodes())
	{
		Core::Element* child = GetFirstChild();

		// Check for a value attribute.
		Rocket::Core::String attribute_value = child->GetAttribute<Rocket::Core::String>("value", "");

		// Pull the inner RML and add the option.
		Rocket::Core::String rml;
		child->GetInnerRML(rml);
		widget->AddOption(rml, attribute_value, -1, child->GetAttribute("selected") != NULL, child->GetAttribute("unselectable") == NULL);

		RemoveChild(child);
	}
}

// Updates the layout of the widget's elements.
void ElementFormControlSelect::OnRender()
{
	ElementFormControl::OnRender();

	widget->OnRender();
}

// Forces an internal layout.
void ElementFormControlSelect::OnLayout()
{
	widget->OnLayout();
}

// Returns true to mark this element as replaced.
bool ElementFormControlSelect::GetIntrinsicDimensions(Rocket::Core::Vector2f& intrinsic_dimensions)
{
	intrinsic_dimensions.x = 128;
	intrinsic_dimensions.y = 16;
	return true;
}

}
}
