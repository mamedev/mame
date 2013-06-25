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

#ifndef ROCKETCONTROLSINPUTTYPECHECKBOX_H
#define ROCKETCONTROLSINPUTTYPECHECKBOX_H

#include "InputType.h"

namespace Rocket {
namespace Controls {

/**
	A checkbox input type handler.

	@author Peter Curry
 */

class InputTypeCheckbox : public InputType
{
public:
	InputTypeCheckbox(ElementFormControlInput* element);
	virtual ~InputTypeCheckbox();

	/// Returns if this value should be submitted with the form.
	/// @return True if the form control is to be submitted, false otherwise.
	virtual bool IsSubmitted();

	/// Checks for necessary functional changes in the control as a result of changed attributes.
	/// @param[in] changed_attributes The list of changed attributes.
	/// @return True if no layout is required, false if the layout needs to be dirtied.
	virtual bool OnAttributeChange(const Core::AttributeNameList& changed_attributes);

	/// Checks for necessary functional changes in the control as a result of the event.
	/// @param[in] event The event to process.
	virtual void ProcessEvent(Core::Event& event);

	/// Sizes the dimensions to the element's inherent size.
	/// @return True.
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);
};

}
}

#endif
