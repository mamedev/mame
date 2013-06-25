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

#ifndef ROCKETCONTROLSELEMENTTEXTSELECTION_H
#define ROCKETCONTROLSELEMENTTEXTSELECTION_H

#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Controls {

class WidgetTextInput;

/**
	A stub element used by the WidgetTextInput to query the RCSS-specified text colour and
	background colour for selected text.

	@author Peter Curry
 */

class ElementTextSelection : public Core::Element
{
public:
	ElementTextSelection(const Rocket::Core::String& tag);
	virtual ~ElementTextSelection();

	/// Set the widget that this selection element was created for. This is the widget that will be
	/// notified when this element's properties are altered.
	void SetWidget(WidgetTextInput* widget);

protected:
	/// Processes 'color' and 'background-color' property changes.
	virtual void OnPropertyChange(const Rocket::Core::PropertyNameList& changed_properties);

private:
	WidgetTextInput* widget;
};

}
}

#endif
