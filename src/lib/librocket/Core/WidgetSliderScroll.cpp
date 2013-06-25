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

#include "precompiled.h"
#include "WidgetSliderScroll.h"

namespace Rocket {
namespace Core {

WidgetSliderScroll::WidgetSliderScroll(Element* parent) : WidgetSlider(parent)
{
	track_length = 0;
	bar_length = 0;
	line_height = 12;
}

WidgetSliderScroll::~WidgetSliderScroll()
{
}

// Sets the length of the entire track in some arbitrary unit.
void WidgetSliderScroll::SetTrackLength(float _track_length, bool ROCKET_UNUSED(force_resize))
{
	if (track_length != _track_length)
	{
		track_length = _track_length;
//		GenerateBar();
	}
}

// Sets the length the bar represents in some arbitrary unit, relative to the track length.
void WidgetSliderScroll::SetBarLength(float _bar_length, bool ROCKET_UNUSED(force_resize))
{
	if (bar_length != _bar_length)
	{
		bar_length = _bar_length;
//		GenerateBar();
	}
}

// Sets the line height of the parent element; this is used for scrolling speeds.
void WidgetSliderScroll::SetLineHeight(float _line_height)
{
	line_height = _line_height;
}

// Lays out and resizes the internal elements.
void WidgetSliderScroll::FormatElements(const Vector2f& containing_block, float slider_length)
{
	float relative_bar_length;

	if (track_length <= 0)
		relative_bar_length = 1;
	else if (bar_length <= 0)
		relative_bar_length = 0;
	else
		relative_bar_length = bar_length / track_length;

	WidgetSlider::FormatElements(containing_block, true, slider_length, relative_bar_length);
}

// Called when the slider's bar position is set or dragged.
float WidgetSliderScroll::OnBarChange(float bar_position)
{
	return bar_position;
}

// Called when the slider is incremented by one 'line', either by the down / right key or a mouse-click on the
// increment arrow.
float WidgetSliderScroll::OnLineIncrement()
{
	return Scroll(line_height);
}

// Called when the slider is decremented by one 'line', either by the up / left key or a mouse-click on the decrement
// arrow.
float WidgetSliderScroll::OnLineDecrement()
{
	return Scroll(-line_height);
}

// Called when the slider is incremented by one 'page', either by the page-up key or a mouse-click on the track
// below / right of the bar.
float WidgetSliderScroll::OnPageIncrement(float ROCKET_UNUSED(click_position))
{
	return Scroll(bar_length);
}

// Called when the slider is incremented by one 'page', either by the page-down key or a mouse-click on the track
// above / left of the bar.
float WidgetSliderScroll::OnPageDecrement(float ROCKET_UNUSED(click_position))
{
	return Scroll(-bar_length);
}

// Returns the bar position after scrolling for a number of pixels.
float WidgetSliderScroll::Scroll(float distance)
{
	float traversable_track_length = (track_length - bar_length);
	if (traversable_track_length <= 0)
		return GetBarPosition();

	return (GetBarPosition() * traversable_track_length + distance) / traversable_track_length;
}

}
}
