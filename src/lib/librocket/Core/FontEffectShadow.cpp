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
#include "FontEffectShadow.h"

namespace Rocket {
namespace Core {

FontEffectShadow::FontEffectShadow() : offset(0, 0)
{
	// Default the z-index of a shadow effect to be behind the main layer.
	SetZIndex(-1);
}

FontEffectShadow::~FontEffectShadow()
{
}

// Initialise the shadow effect.
bool FontEffectShadow::Initialise(const Vector2i& _offset)
{
	offset = _offset;
	return true;
}

// Returns true.
bool FontEffectShadow::HasUniqueTexture() const
{
	return false;
}

// Resizes and repositions the glyph to fit the outline.
bool FontEffectShadow::GetGlyphMetrics(Vector2i& origin, Vector2i& ROCKET_UNUSED(dimensions), const FontGlyph& ROCKET_UNUSED(glyph)) const
{
	origin += offset;
	return true;
}

}
}
