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

#ifndef ROCKETDEBUGGERGEOMETRY_H
#define ROCKETDEBUGGERGEOMETRY_H

#include <Rocket/Core/Types.h>

namespace Rocket {

namespace Core {
	class Context;
}

namespace Debugger {

/**
	Helper class for generating geometry for the debugger.

	@author Peter Curry
 */

class Geometry
{
public:
	// Set the context to render through.
	static void SetContext(Core::Context* context);

	// Renders a one-pixel rectangular outline.
	static void RenderOutline(const Core::Vector2f& origin, const Core::Vector2f& dimensions, const Core::Colourb& colour, float width);
	// Renders a box.
	static void RenderBox(const Core::Vector2f& origin, const Core::Vector2f& dimensions, const Core::Colourb& colour);
	// Renders a box with a hole in the middle.
	static void RenderBox(const Core::Vector2f& origin, const Core::Vector2f& dimensions, const Core::Vector2f& hole_origin, const Core::Vector2f& hole_dimensions, const Core::Colourb& colour);

private:
	Geometry();
};

}
}

#endif
