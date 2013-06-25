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

#ifndef ROCKETCOREFONTEFFECT_H
#define ROCKETCOREFONTEFFECT_H

#include <Rocket/Core/FontGlyph.h>

namespace Rocket {
namespace Core {

class FontEffectInstancer;

/**
	@author Peter Curry
 */

class FontEffect : public ReferenceCountable
{
public:
	FontEffect();
	virtual ~FontEffect();

	/// Returns the name of the effect; this is the type that instanced the effect.
	/// @return The effect's name.
	const String& GetName() const;

	/// Asks the font effect if it requires, and will generate, its own unique texture. If it does
	/// not, it will share the font's base layer's textures instead.
	/// @return True if the effect generates its own textures, false if not. The default implementation returns false.
	virtual bool HasUniqueTexture() const;

	/// Requests the effect for a size and position of a single glyph's bitmap.
	/// @param[out] origin The desired origin of the effect's glyph bitmap, as a pixel offset from its original origin. This defaults to (0, 0).
	/// @param[out] dimensions The desired dimensions of the effect's glyph bitmap, in pixels. This defaults to the dimensions of the glyph's original bitmap. If the font effect is not generating a unique texture, this will be ignored.
	/// @param[in] glyph The glyph the effect is being asked to size.
	/// @return False if the effect is not providing support for the glyph, true otherwise.
	virtual bool GetGlyphMetrics(Vector2i& origin, Vector2i& dimensions, const FontGlyph& glyph) const;

	/// Requests the effect to generate the texture data for a single glyph's bitmap. The default implementation does
	/// nothing.
	/// @param[out] destination_data The top-left corner of the glyph's 32-bit, RGBA-ordered, destination texture. Note that they glyph shares its texture with other glyphs.
	/// @param[in] destination_dimensions The dimensions of the glyph's area on its texture.
	/// @param[in] destination_stride The stride of the glyph's texture.
	/// @param[in] glyph The glyph the effect is being asked to generate an effect texture for.
	virtual void GenerateGlyphTexture(byte* destination_data, const Vector2i& destination_dimensions, int destination_stride, const FontGlyph& glyph) const;

	/// Sets the colour of the effect's geometry.
	/// @param[in] colour The effect's colour.
	void SetColour(const Colourb& colour);
	/// Returns the effect's colour.
	/// @return The colour of the effect.
	const Colourb& GetColour() const;

	/// Sets the z-index of the font effect. An effect with a higher z-index will be rendered after
	/// an effect with a lower z-index. By default, all effects have a z-index of 0.
	/// @param[in] z-index The new z-index of the effect.
	void SetZIndex(float z_index);
	/// Returns the font effect's z-index.
	/// @return The z-index of the effect.
	float GetZIndex() const;

	/// Sets the specificity of the font effect.
	/// @param[in] specificity The specificity of the effect.
	void SetSpecificity(int specificity);
	/// Returns the specificity of the font effect. This is used when multiple pseudo-classes are
	/// active on an element, each with similarly-named font effects.
	/// @return The specificity of the effect.
	int GetSpecificity() const;

	/// Returns the font effect's generation key.
	/// @return A hash of the effect's properties used to generate the geometry and texture data.
	const String& GetGenerationKey() const;

protected:
	/// Releases the effect through its instancer.
	virtual void OnReferenceDeactivate();

private:
	FontEffectInstancer* instancer;

	// The name of the effect.
	String name;

	// The colour of the effect's geometry.
	Colourb colour;

	// The z-index of this font effect, used to resolve render order when multiple font effects are rendered.
	float z_index;
	// The maximum specificity of the properties used to define the font effect.
	int specificity;

	// A string identifying the properties that affected the generation of the effect's geometry and texture data.
	String generation_key;

	friend class Factory;
};

typedef std::vector< FontEffect* > FontEffectList;
typedef std::map< String, FontEffect* > FontEffectMap;

}
}

#endif
