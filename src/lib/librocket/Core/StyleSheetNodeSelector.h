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

#ifndef ROCKETCORESTYLESHEETNODESELECTOR_H
#define ROCKETCORESTYLESHEETNODESELECTOR_H

namespace Rocket {
namespace Core {

class Element;

/**
	The ABC for any complex node selector, such as structural selectors.

	@author Peter Curry
 */

class StyleSheetNodeSelector
{
public:
	StyleSheetNodeSelector();
	virtual ~StyleSheetNodeSelector();

	/// Returns true if the the node this selector is discriminating for is applicable to a given element.
	/// @param element[in] The element to determine node applicability for.
	/// @param a[in] For counting selectors, this is the 'a' variable of an + b.
	/// @param b[in] For counting selectors, this is the 'b' variable of an + b.
	virtual bool IsApplicable(const Element* element, int a, int b) = 0;

protected:
	/// Returns true if a positive integer can be found for n in the equation an + b = count.
	bool IsNth(int a, int b, int count);
};

}
}

#endif
