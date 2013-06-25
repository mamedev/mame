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

#include "InputTypeText.h"
#include <Rocket/Core/ElementUtilities.h>
#include "WidgetTextInputSingleLine.h"
#include "WidgetTextInputSingleLinePassword.h"
#include <Rocket/Controls/ElementFormControlInput.h>

namespace Rocket {
namespace Controls {

InputTypeText::InputTypeText(ElementFormControlInput* element, Visibility visibility) : InputType(element)
{
	if (visibility == VISIBLE)
		widget = new WidgetTextInputSingleLine(element);
	else
		widget = new WidgetTextInputSingleLinePassword(element);

	widget->SetMaxLength(element->GetAttribute< int >("maxlength", -1));
	widget->SetValue(element->GetAttribute< Rocket::Core::String >("value", ""));

	size = element->GetAttribute< int >("size", 20);
}

InputTypeText::~InputTypeText()
{
	delete widget;
}

// Called every update from the host element.
void InputTypeText::OnUpdate()
{
	widget->OnUpdate();
}

// Called every render from the host element.
void InputTypeText::OnRender()
{
	widget->OnRender();
}

// Checks for necessary functional changes in the control as a result of changed attributes.
bool InputTypeText::OnAttributeChange(const Core::AttributeNameList& changed_attributes)
{
	bool dirty_layout = false;

	// Check if maxlength has been defined.
	if (changed_attributes.find("maxlength") != changed_attributes.end())
		widget->SetMaxLength(element->GetAttribute< int >("maxlength", -1));

	// Check if size has been defined.
	if (changed_attributes.find("size") != changed_attributes.end())
	{
		size = element->GetAttribute< int >("size", 20);
		dirty_layout = true;
	}

	// Check if the value has been changed.
	if (changed_attributes.find("value") != changed_attributes.end())
		widget->SetValue(element->GetAttribute< Rocket::Core::String >("value", ""));

	return !dirty_layout;
}

// Called when properties on the control are changed.
void InputTypeText::OnPropertyChange(const Core::PropertyNameList& changed_properties)
{
	if (changed_properties.find("color") != changed_properties.end() ||
		changed_properties.find("background-color") != changed_properties.end())
		widget->UpdateSelectionColours();
}

// Checks for necessary functional changes in the control as a result of the event.
void InputTypeText::ProcessEvent(Core::Event& ROCKET_UNUSED(event))
{
}

// Sizes the dimensions to the element's inherent size.
bool InputTypeText::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	dimensions.x = (float) (size * Core::ElementUtilities::GetStringWidth(element, "m"));
	dimensions.y = (float) Core::ElementUtilities::GetLineHeight(element) + 2;

	return true;
}

}
}
