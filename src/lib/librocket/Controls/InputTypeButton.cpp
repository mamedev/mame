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

#include "InputTypeButton.h"
#include <Rocket/Controls/ElementForm.h>
#include <Rocket/Controls/ElementFormControlInput.h>

namespace Rocket {
namespace Controls {

InputTypeButton::InputTypeButton(ElementFormControlInput* element) : InputType(element)
{
	document = NULL;

	// Call OnChildAdd() immediately; if the input element is already part of a document, this will
	// attach our listeners to the document so the events can be intercepted.
	OnChildAdd();
}

InputTypeButton::~InputTypeButton()
{
	// Call OnChildRemove(); in case our element is still attached to a document, this will detach
	// our listeners.
	OnChildRemove();
}

// Buttons are never submitted.
bool InputTypeButton::IsSubmitted()
{
	return false;
}

// Checks for necessary functional changes in the control as a result of the event.
void InputTypeButton::ProcessEvent(Core::Event& event)
{
	// Stop a click event from proceeding any further if this button is disabled.
	if (event.GetTargetElement() == element &&
		element->IsDisabled() &&
		(event == "click" || event == "dblclick"))
	{
		event.StopPropagation();
	}
}

// Sizes the dimensions to the element's inherent size.
bool InputTypeButton::GetIntrinsicDimensions(Rocket::Core::Vector2f& ROCKET_UNUSED(dimensions))
{
	return false;
}

// Called when the element is added into a hierarchy.
void InputTypeButton::OnChildAdd()
{
	document = element->GetOwnerDocument();
	if (document == NULL)
		return;

	document->AddEventListener("click", this, true);
	document->AddEventListener("dblclick", this, true);
}

// Called when the element is removed from a hierarchy.
void InputTypeButton::OnChildRemove()
{
	if (document != NULL)
	{
		document->RemoveEventListener("click", this, true);
		document->RemoveEventListener("dblclick", this, true);
		document = NULL;
	}
}

}
}
