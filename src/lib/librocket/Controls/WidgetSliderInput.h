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

#ifndef ROCKETCONTROLSWIDGETSLIDERINPUT_H
#define ROCKETCONTROLSWIDGETSLIDERINPUT_H

#include "WidgetSlider.h"

namespace Rocket {
namespace Controls {

/**
	A specialisation of the slider widget for input sliders.

	@author Peter Curry
 */

class WidgetSliderInput : public WidgetSlider
{
public:
	WidgetSliderInput(ElementFormControl* parent);
	virtual ~WidgetSliderInput();

	/// Sets a new value on the slider.
	/// @param[in] value The new value for the slider. This will be clamped between the min and max values, and set to the nearest increment.
	void SetValue(float value);
	/// Returns the current value of the slider.
	/// @return The current value of the slider.
	float GetValue();

	/// Sets the minimum value of the slider.
	/// @param[in] min_value The new minimum value of the slider.
	void SetMinValue(float min_value);
	/// Sets the maximum value of the slider.
	/// @param[in] max_value The new minimum value of the slider.
	void SetMaxValue(float max_value);
	/// Sets the slider's value increment.
	/// @param[in] step The new increment for the slider's value.
	void SetStep(float step);

	/// Formats the slider's elements.
	void FormatElements();

protected:
	/// Called when the slider's bar position is set or dragged.
	/// @param bar_position[in] The new position of the bar (0 representing the start of the track, 1 representing the end).
	/// @return The new position of the bar.
	virtual float OnBarChange(float bar_position);
	/// Called when the slider is incremented by one 'line', either by the down / right key or a mouse-click on the
	/// increment arrow.
	/// @return The new position of the bar.
	virtual float OnLineIncrement();
	/// Called when the slider is decremented by one 'line', either by the up / left key or a mouse-click on the
	/// decrement arrow.
	/// @return The new position of the bar.
	virtual float OnLineDecrement();
	/// Called when the slider is incremented by one 'page', either by the page-up key or a mouse-click on the
	/// track below / right of the bar.
	/// @return The new position of the bar.
	virtual float OnPageIncrement(float click_position);
	/// Called when the slider is incremented by one 'page', either by the page-down key or a mouse-click on the
	/// track above / left of the bar.
	/// @return The new position of the bar.
	virtual float OnPageDecrement(float click_position);

private:
	/// Clamps the new value, sets it on the slider and returns it as a number from 0 to 1, 0 being the minimum
	/// value and 1 the maximum.
	/// @param[in] new_value The new value to set on the slider.
	/// @return The new parametric value of the slider.
	float SetValueInternal(float new_value);

	float value;
	float min_value;
	float max_value;

	float step;
};

}
}

#endif
