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

#include "InputTypeRange.h"
#include "WidgetSliderInput.h"
#include <Rocket/Controls/ElementFormControlInput.h>

namespace Rocket {
namespace Controls {

InputTypeRange::InputTypeRange(ElementFormControlInput* element) : InputType(element)
{
	widget = new WidgetSliderInput(element);
	widget->Initialise();
}

InputTypeRange::~InputTypeRange()
{
	delete widget;
}

// Returns a string representation of the current value of the form control.
Rocket::Core::String InputTypeRange::GetValue() const
{
	return Rocket::Core::String(32, "%f", widget->GetValue());
}

// Called every update from the host element.
void InputTypeRange::OnUpdate()
{
	widget->Update();
}

// Checks for necessary functional changes in the control as a result of changed attributes.
bool InputTypeRange::OnAttributeChange(const Core::AttributeNameList& changed_attributes)
{
	bool dirty_layout = false;

	// Check if maxlength has been defined.
	if (changed_attributes.find("orientation") != changed_attributes.end())
	{
		widget->SetOrientation(element->GetAttribute< Rocket::Core::String >("orientation", "horizontal") == "horizontal" ? WidgetSliderInput::HORIZONTAL : WidgetSliderInput::VERTICAL);
		dirty_layout = true;
	}

	// Check if size has been defined.
	if (changed_attributes.find("step") != changed_attributes.end())
		widget->SetStep(element->GetAttribute< float >("step", 1.0f));

	// Check if min has been defined.
	if (changed_attributes.find("min") != changed_attributes.end())
		widget->SetMinValue(element->GetAttribute< float >("min", 0.0f));

	// Check if max has been defined.
	if (changed_attributes.find("max") != changed_attributes.end())
		widget->SetMaxValue(element->GetAttribute< float >("max", 100.0f));

	// Check if the value has been changed.
	if (changed_attributes.find("value") != changed_attributes.end())
		widget->SetValue(element->GetAttribute< float >("value", 0.0f));

	return !dirty_layout;
}

// Checks for necessary functional changes in the control as a result of the event.
void InputTypeRange::ProcessEvent(Core::Event& event)
{
	if (event == "resize" &&
		event.GetTargetElement() == element)
	{
		widget->FormatElements();
	}
}

// Sizes the dimensions to the element's inherent size.
bool InputTypeRange::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	widget->GetDimensions(dimensions);
	return true;
}

}
}
