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

#ifndef ROCKETCONTROLSSELECTOPTION_H
#define ROCKETCONTROLSSELECTOPTION_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Core/String.h>

namespace Rocket {
namespace Core {

class Element;

}

namespace Controls {

/**
	Represents individual options within a select control.

	@author Peter Curry
 */

class ROCKETCONTROLS_API SelectOption
{
public:
	SelectOption(Core::Element* element, const Rocket::Core::String& value, bool selectable);
	~SelectOption();

	/// Returns the element that represents the option visually.
	/// @return The option's element.
	Core::Element* GetElement();
	/// Returns the value of the option.
	/// @return The option's value.
	const Rocket::Core::String& GetValue() const;

	/// Returns true if the item is selectable.
	/// @return True if the item is selectable.
	bool IsSelectable() { return selectable; }

private:
	Core::Element* element;
	Rocket::Core::String value;
	bool selectable;
};

}
}

#endif
