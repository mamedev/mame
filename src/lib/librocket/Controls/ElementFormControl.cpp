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

#include <Rocket/Controls/ElementFormControl.h>

namespace Rocket {
namespace Controls {

ElementFormControl::ElementFormControl(const Rocket::Core::String& tag) : Core::Element(tag)
{
	SetProperty("tab-index", "auto");
}

ElementFormControl::~ElementFormControl()
{
}

// Returns the name of the form control.
Rocket::Core::String ElementFormControl::GetName() const
{
	return GetAttribute<Rocket::Core::String>("name", "");	
}

// Sets the name of the form control.
void ElementFormControl::SetName(const Rocket::Core::String& name)
{
	SetAttribute("name", name);
}

// Returns if this value should be submitted with the form
bool ElementFormControl::IsSubmitted()
{
	return true;
}

// Returns the disabled status of the form control.
bool ElementFormControl::IsDisabled() const
{
	return GetAttribute("disabled") != NULL;
}

// Sets the disabled status of the form control.
void ElementFormControl::SetDisabled(bool disable)
{
	if (disable)
	{
		SetAttribute("disabled", "");
		Blur();
	}
	else
		RemoveAttribute("disabled");
}

// Checks for changes to the 'disabled' attribute.
void ElementFormControl::OnAttributeChange(const Core::AttributeNameList& changed_attributes)
{
	Core::Element::OnAttributeChange(changed_attributes);

	if (changed_attributes.find("disabled") != changed_attributes.end())
		SetPseudoClass("disabled", IsDisabled());
}

}
}
