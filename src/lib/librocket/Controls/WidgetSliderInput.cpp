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

#include "WidgetSliderInput.h"
#include <Rocket/Core/Math.h>
#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Controls {

WidgetSliderInput::WidgetSliderInput(ElementFormControl* parent) : WidgetSlider(parent)
{
	value = 0;
	min_value = 0;
	max_value = 100;
	step = 1;
}

WidgetSliderInput::~WidgetSliderInput()
{
}

void WidgetSliderInput::SetValue(float value)
{
	float num_steps = (value - min_value) / step;
	float new_value = min_value + Rocket::Core::Math::Round(num_steps) * step;

	SetBarPosition(SetValueInternal(new_value));
}

float WidgetSliderInput::GetValue()
{
	return value;
}

// Sets the minimum value of the slider.
void WidgetSliderInput::SetMinValue(float _min_value)
{
	min_value = _min_value;
}

// Sets the maximum value of the slider.
void WidgetSliderInput::SetMaxValue(float _max_value)
{
	max_value = _max_value;
}

// Sets the slider's value increment.
void WidgetSliderInput::SetStep(float _step)
{
	// Can't have a zero step!
	if (_step == 0)
		return;

	step = _step;
}

// Formats the slider's elements.
void WidgetSliderInput::FormatElements()
{
	Rocket::Core::Vector2f box = GetParent()->GetBox().GetSize();
	WidgetSlider::FormatElements(box, GetOrientation() == VERTICAL ? box.y : box.x);
}

// Called when the slider's bar position is set or dragged.
float WidgetSliderInput::OnBarChange(float bar_position)
{
	float new_value = min_value + bar_position * (max_value - min_value);
	int num_steps = Rocket::Core::Math::Round((new_value - value) / step);

	return SetValueInternal(value + num_steps * step);
}

// Called when the slider is incremented by one 'line'.
float WidgetSliderInput::OnLineIncrement()
{
	return SetValueInternal(value + step);
}

// Called when the slider is decremented by one 'line'.
float WidgetSliderInput::OnLineDecrement()
{
	return SetValueInternal(value - step);
}

// Called when the slider is incremented by one 'page'.
float WidgetSliderInput::OnPageIncrement(float click_position)
{
	return OnBarChange(click_position);
}

// Called when the slider is incremented by one 'page'.
float WidgetSliderInput::OnPageDecrement(float click_position)
{
	return OnBarChange(click_position);
}

// Clamps the new value, sets it on the slider.
float WidgetSliderInput::SetValueInternal(float new_value)
{
	if (min_value < max_value)
	{
		value = Rocket::Core::Math::Clamp(new_value, min_value, max_value);
	}
	else if (min_value > max_value)
	{
		value = Rocket::Core::Math::Clamp(new_value, max_value, min_value);
	}
	else
	{
		value = min_value;
		return 0;
	}

	return (value - min_value) / (max_value - min_value);
}

}
}
