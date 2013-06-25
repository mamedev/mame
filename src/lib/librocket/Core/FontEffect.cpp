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
#include <Rocket/Core/FontEffect.h>
#include <Rocket/Core/FontDatabase.h>
#include <Rocket/Core/FontEffectInstancer.h>

namespace Rocket {
namespace Core {

FontEffect::FontEffect() : colour(255, 255, 255)
{
	instancer = NULL;
	z_index = 0;
	specificity = -1;
}

FontEffect::~FontEffect()
{
}

// Returns the name of the effect; this is the type that instanced the effect.
const String& FontEffect::GetName() const
{
	return name;
}

// Asks the font effect if it requires, and will generate, its own unique texture.
bool FontEffect::HasUniqueTexture() const
{
	return false;
}

// Gets the effect to resize and reposition a glyph's bitmap.
bool FontEffect::GetGlyphMetrics(Vector2i& ROCKET_UNUSED(origin), Vector2i& ROCKET_UNUSED(dimensions), const FontGlyph& ROCKET_UNUSED(glyph)) const
{
	return false;
}

// Requests the effect to generate the texture data for a single glyph's bitmap.
void FontEffect::GenerateGlyphTexture(byte* ROCKET_UNUSED(destination_data), const Vector2i& ROCKET_UNUSED(destination_dimensions), int ROCKET_UNUSED(destination_stride), const FontGlyph& ROCKET_UNUSED(glyph)) const
{
}

// Sets the colour of the effect's geometry.
void FontEffect::SetColour(const Colourb& _colour)
{
	colour = _colour;
}

// Returns the effect's colour.
const Colourb& FontEffect::GetColour() const
{
	return colour;
}

// Sets the z-index of the font effect.
void FontEffect::SetZIndex(float _z_index)
{
	z_index = _z_index;
}

// Returns the font effect's z-index.
float FontEffect::GetZIndex() const
{
	return z_index;
}

// Sets the specificity of the font effect.
void FontEffect::SetSpecificity(int _specificity)
{
	specificity = _specificity;
}

// Returns the specificity of the font effect.
int FontEffect::GetSpecificity() const
{
	return specificity;
}

// Returns the font effect's geometry / texture generation key.
const String& FontEffect::GetGenerationKey() const
{
	return generation_key;
}

// Releases the decorator through its instancer.
void FontEffect::OnReferenceDeactivate()
{
	FontDatabase::ReleaseFontEffect(this);

	if (instancer != NULL)
		instancer->ReleaseFontEffect(this);
}

}
}
