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

#ifndef ROCKETCOREFONTEFFECTSHADOW_H
#define ROCKETCOREFONTEFFECTSHADOW_H

#include <Rocket/Core/FontEffect.h>

namespace Rocket {
namespace Core {

/**
	A concrete font effect for rendering text shadows.

	@author Peter Curry
 */

class FontEffectShadow : public FontEffect
{
public:
	FontEffectShadow();
	virtual ~FontEffectShadow();

	/// Initialise the shadow effect.
	/// @param[in] offset The offset, in pixels, of the shadow from the original text.
	/// @return True if the effect initialised successfully, false if not.
	bool Initialise(const Vector2i& offset);

	/// Returns false.
	/// @return False.
	virtual bool HasUniqueTexture() const;

	/// Repositions the glyph by the offset.
	/// @param[out] origin The desired origin of the effect's glyph bitmap, as a pixel offset from its original origin. This defaults to (0, 0).
	/// @param[out] dimensions The desired dimensions of the effect's glyph bitmap, in pixels. This defaults to the dimensions of the glyph's original bitmap.
	/// @param[in] glyph The glyph the effect is being asked to size.
	/// @return False if the effect is not providing support for the glyph, true otherwise.
	virtual bool GetGlyphMetrics(Vector2i& origin, Vector2i& dimensions, const FontGlyph& glyph) const;

private:
	Vector2i offset;
};

}
}

#endif
