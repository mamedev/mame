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

#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Core/Event.h>
#include "InputTypeButton.h"
#include "InputTypeCheckbox.h"
#include "InputTypeRadio.h"
#include "InputTypeRange.h"
#include "InputTypeSubmit.h"
#include "InputTypeText.h"

namespace Rocket {
namespace Controls {

// Constructs a new ElementFormControlInput.
ElementFormControlInput::ElementFormControlInput(const Rocket::Core::String& tag) : ElementFormControl(tag)
{
	type = NULL;
	type = new InputTypeText(this);
	type_name = "text";
	SetClass(type_name, true);
}

ElementFormControlInput::~ElementFormControlInput()
{
	delete type;
}

// Returns a string representation of the current value of the form control.
Rocket::Core::String ElementFormControlInput::GetValue() const
{
	return type->GetValue();
}

// Sets the current value of the form control.
void ElementFormControlInput::SetValue(const Rocket::Core::String& value)
{
	SetAttribute("value", value);
}

// Returns if this value should be submitted with the form.
bool ElementFormControlInput::IsSubmitted()
{
	return type->IsSubmitted();
}

// Updates the element's underlying type.
void ElementFormControlInput::OnUpdate()
{
	type->OnUpdate();
}

// Renders the element's underlying type.
void ElementFormControlInput::OnRender()
{
	type->OnRender();
}

// Checks for necessary functional changes in the control as a result of changed attributes.
void ElementFormControlInput::OnAttributeChange(const Core::AttributeNameList& changed_attributes)
{
	ElementFormControl::OnAttributeChange(changed_attributes);

	if (changed_attributes.find("type") != changed_attributes.end())
	{
		Rocket::Core::String new_type_name = GetAttribute< Rocket::Core::String >("type", "text");
		if (new_type_name != type_name)
		{
			InputType* new_type = NULL;

			if (new_type_name == "password")
				new_type = new InputTypeText(this, Rocket::Controls::InputTypeText::OBSCURED);
			else if (new_type_name == "radio")
				new_type = new InputTypeRadio(this);
			else if (new_type_name == "checkbox")
				new_type = new InputTypeCheckbox(this);
			else if (new_type_name == "range")
				new_type = new InputTypeRange(this);
			else if (new_type_name == "submit")
				new_type = new InputTypeSubmit(this);
			else if (new_type_name == "button")
				new_type = new InputTypeButton(this);
			else if (type_name == "text")
				new_type = new InputTypeText(this);

			if (new_type != NULL)
			{
				delete type;
				type = new_type;

				SetClass(type_name, false);
				SetClass(new_type_name, true);
				type_name = new_type_name;

				DirtyLayout();
			}
		}
	}

	if (!type->OnAttributeChange(changed_attributes))
		DirtyLayout();
}

// Called when properties on the element are changed.
void ElementFormControlInput::OnPropertyChange(const Core::PropertyNameList& changed_properties)
{
	ElementFormControl::OnPropertyChange(changed_properties);

	if (type != NULL)
		type->OnPropertyChange(changed_properties);
}

// If we are the added element, this will pass the call onto our type handler.
void ElementFormControlInput::OnChildAdd(Rocket::Core::Element* child)
{
	if (child == this)
		type->OnChildAdd();
}

// If we are the removed element, this will pass the call onto our type handler.
void ElementFormControlInput::OnChildRemove(Rocket::Core::Element* child)
{
	if (child == this)
		type->OnChildRemove();
}

// Handles the "click" event to toggle the control's checked status.
void ElementFormControlInput::ProcessEvent(Core::Event& event)
{
	ElementFormControl::ProcessEvent(event);
	type->ProcessEvent(event);
}

bool ElementFormControlInput::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	return type->GetIntrinsicDimensions(dimensions);
}

}
}
