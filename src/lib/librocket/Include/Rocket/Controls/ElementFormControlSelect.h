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

#ifndef ROCKETCONTROLSELEMENTFORMCONTROLSELECT_H
#define ROCKETCONTROLSELEMENTFORMCONTROLSELECT_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Controls/ElementFormControl.h>
#include <Rocket/Controls/SelectOption.h>

namespace Rocket {
namespace Controls {

class WidgetDropDown;

/**
	A drop-down select form control.

	@author Peter Curry
 */

class ROCKETCONTROLS_API ElementFormControlSelect : public ElementFormControl
{
public:
	/// Constructs a new ElementFormControlSelect. This should not be called directly; use the
	/// Factory instead.
	/// @param[in] tag The tag the element was declared as in RML.
	ElementFormControlSelect(const Rocket::Core::String& tag);
	virtual ~ElementFormControlSelect();

	/// Returns a string representation of the current value of the form control.
	/// @return The value of the form control.
	virtual Rocket::Core::String GetValue() const;
	/// Sets the current value of the form control.
	/// @param[in] value The new value of the form control.
	virtual void SetValue(const Rocket::Core::String& value);

	/// Sets the index of the selection. If the new index lies outside of the bounds, it will be clamped.
	/// @param[in] selection The new selection index.
	void SetSelection(int selection);
	/// Returns the index of the currently selected item.
	/// @return The index of the currently selected item.
	int GetSelection() const;

	/// Returns one of the select control's option elements.
	/// @param[in] The index of the desired option element.
	/// @return The option element at the given index. This will be NULL if the index is out of bounds.
	SelectOption* GetOption(int index);
	/// Returns the number of options in the select control.
	/// @return The number of options.
	int GetNumOptions();

	/// Adds a new option to the select control.
	/// @param[in] rml The RML content used to represent the option. This is usually a simple string, but can include RML tags.
	/// @param[in] value The value of the option. This is used to identify the option, but does not necessarily need to be unique.
	/// @param[in] before The index of the element to insert the new option before. If out of bounds of the control's option list (the default) the new option will be added at the end of the list.
	/// @param[in] selectable If true this option can be selected. If false, this option is not selectable.
	/// @return The index of the new option.
	int Add(const Rocket::Core::String& rml, const Rocket::Core::String& value, int before = -1, bool selectable = true);
	/// Removes an option from the select control.
	/// @param[in] index The index of the option to remove. If this is outside of the bounds of the control's option list, no option will be removed.
	void Remove(int index);

	/// Removes all options from the select control.
	void RemoveAll();

protected:
	/// Moves all children to be under control of the widget.
	virtual void OnUpdate();
	/// Updates the layout of the widget's elements.
	virtual void OnRender();

	/// Forces an internal layout.
	virtual void OnLayout();

	/// Returns true to mark this element as replaced.
	/// @param[out] intrinsic_dimensions Set to the arbitrary dimensions of 128 x 16 just to give this element a size. Resize with the 'width' and 'height' properties.
	/// @return True.
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& intrinsic_dimensions);

	WidgetDropDown* widget;
};

}
}

#endif
