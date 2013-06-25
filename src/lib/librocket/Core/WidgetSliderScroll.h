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

#ifndef ROCKETCOREWIDGETSLIDERSCROLL_H
#define ROCKETCOREWIDGETSLIDERSCROLL_H

#include "WidgetSlider.h"

namespace Rocket {
namespace Core {

/**
	@author Peter Curry
 */

class WidgetSliderScroll : public WidgetSlider
{
public:
	WidgetSliderScroll(Element* parent);
	virtual ~WidgetSliderScroll();

	/// Sets the length of the entire track in scrollable units (usually lines or characters). This affects the
	/// length of the bar element and the speed of scrolling using the mouse-wheel or arrows.
	/// @param[in] track_length The length of the track.
	/// @param[in] force_resize True to resize the bar immediately, false to wait until the next format.
	void SetTrackLength(float track_length, bool force_resize = true);
	/// Sets the length the bar represents in scrollable units (usually lines or characters), relative to the track
	/// length. For example, for a scroll bar, this would represent how big each visible 'page' is compared to the
	/// whole document (which would be set as the track length).
	/// @param[in] bar_length The length of the slider's bar.
	/// @param[in] force_resize True to resize the bar immediately, false to wait until the next format.
	void SetBarLength(float bar_length, bool force_resize = true);

	/// Sets the line height of the parent element; this is used for scrolling speeds.
	/// @param[in] line_height The line height of the parent element.
	void SetLineHeight(float line_height);

	/// Lays out and resizes the internal elements.
	/// @param[in] containing_block The padded box containing the slider. This is used to resolve relative properties.
	/// @param[in] length The total length, in pixels, of the slider widget.
	void FormatElements(const Vector2f& containing_block, float slider_length);

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
	// Returns the bar position after scrolling for a number of pixels.
	float Scroll(float distance);

	float track_length;
	float bar_length;
	float line_height;
};

}
}

#endif
