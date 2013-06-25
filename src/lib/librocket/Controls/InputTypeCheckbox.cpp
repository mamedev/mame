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

#include "InputTypeCheckbox.h"
#include <Rocket/Controls/ElementFormControlInput.h>

namespace Rocket {
namespace Controls {

InputTypeCheckbox::InputTypeCheckbox(ElementFormControlInput* element) : InputType(element)
{
}

InputTypeCheckbox::~InputTypeCheckbox()
{
}

// Returns if this value should be submitted with the form.
bool InputTypeCheckbox::IsSubmitted()
{
	return element->HasAttribute("checked");
}

// Checks for necessary functional changes in the control as a result of changed attributes.
bool InputTypeCheckbox::OnAttributeChange(const Core::AttributeNameList& changed_attributes)
{
	// Check if maxlength has been defined.
	if (changed_attributes.find("checked") != changed_attributes.end())
	{
		bool checked = element->HasAttribute("checked");
		element->SetPseudoClass("checked", checked);

		Rocket::Core::Dictionary parameters;
		parameters.Set("value", Rocket::Core::String(checked ? GetValue() : ""));
		element->DispatchEvent("change", parameters);
	}

	return true;
}

// Checks for necessary functional changes in the control as a result of the event.
void InputTypeCheckbox::ProcessEvent(Core::Event& event)
{
	if (event == "click" &&
		!element->IsDisabled())
	{
		if (element->HasAttribute("checked"))
			element->RemoveAttribute("checked");
		else
			element->SetAttribute("checked", "");
	}
}

// Sizes the dimensions to the element's inherent size.
bool InputTypeCheckbox::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	dimensions.x = 16;
	dimensions.y = 16;

	return true;
}

}
}
