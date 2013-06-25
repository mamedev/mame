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

#ifndef ROCKETCONTROLSELEMENTFORMCONTROLINPUT_H
#define ROCKETCONTROLSELEMENTFORMCONTROLINPUT_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Controls/ElementFormControl.h>

namespace Rocket {
namespace Controls {

class InputType;

/**
	A form control for the generic input element. All functionality is handled through an input type interface.

	@author Peter Curry
 */

class ROCKETCONTROLS_API ElementFormControlInput : public ElementFormControl
{
public:
	/// Constructs a new ElementFormControlInput. This should not be called directly; use the
	/// Factory instead.
	/// @param[in] tag The tag the element was declared as in RML.
	ElementFormControlInput(const Rocket::Core::String& tag);
	virtual ~ElementFormControlInput();

	/// Returns a string representation of the current value of the form control.
	/// @return The value of the form control.
	virtual Rocket::Core::String GetValue() const;
	/// Sets the current value of the form control.
	/// @param value[in] The new value of the form control.
	virtual void SetValue(const Rocket::Core::String& value);
	/// Returns if this value's type should be submitted with the form.
	/// @return True if the form control is to be submitted, false otherwise.
	virtual bool IsSubmitted();

protected:
	/// Updates the element's underlying type.
	virtual void OnUpdate();
	/// Renders the element's underlying type.
	virtual void OnRender();

	/// Checks for necessary functional changes in the control as a result of changed attributes.
	/// @param[in] changed_attributes The list of changed attributes.
	virtual void OnAttributeChange(const Core::AttributeNameList& changed_attributes);
	/// Called when properties on the control are changed.
	/// @param[in] changed_properties The properties changed on the element.
	virtual void OnPropertyChange(const Core::PropertyNameList& changed_properties);

	/// If we are the added element, this will pass the call onto our type handler.
	/// @param[in] child The new member of the hierarchy.
	virtual void OnChildAdd(Rocket::Core::Element* child);
	/// If we are the removed element, this will pass the call onto our type handler.
	/// @param[in] child The member of the hierarchy that was just removed.
	virtual void OnChildRemove(Rocket::Core::Element* child);

	/// Checks for necessary functional changes in the control as a result of the event.
	/// @param[in] event The event to process.
	virtual void ProcessEvent(Core::Event& event);

	/// Sizes the dimensions to the element's inherent size.
	/// @return True.
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);

private:
	InputType* type;
	Rocket::Core::String type_name;
};

}
}

#endif
