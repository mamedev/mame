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

#ifndef ROCKETCOREFONTFACELAYER_H
#define ROCKETCOREFONTFACELAYER_H

#include <Rocket/Core/FontGlyph.h>
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core/GeometryUtilities.h>
#include <Rocket/Core/String.h>
#include <Rocket/Core/Texture.h>
#include "TextureLayout.h"

namespace Rocket {
namespace Core {

class FontEffect;
class FontFaceHandle;

/**
	A textured layer stored as part of a font face handle. Each handle will have at least a base
	layer for the standard font. Further layers can be added to allow to rendering of text effects.

	@author Peter Curry
 */

class FontFaceLayer
{
public:
	FontFaceLayer();
	~FontFaceLayer();

	/// Generates the character and texture data for the layer.
	/// @param[in] handle The handle generating this layer.
	/// @param[in] effect The effect to initialise the layer with.
	/// @param[in] clone The layer to optionally clone geometry and texture data from.
	/// @param[in] deep_clone If true, the clones geometry will be completely cloned and the effect will have no option to affect even the glyph origins.
	/// @return True if the layer was generated successfully, false if not.
	bool Initialise(const FontFaceHandle* handle, FontEffect* effect = NULL, const FontFaceLayer* clone = NULL, bool deep_clone = false);

	/// Generates the texture data for a layer (for the texture database).
	/// @param[out] texture_data The pointer to be set to the generated texture data.
	/// @param[out] texture_dimensions The dimensions of the texture.
	/// @param[in] glyphs The glyphs required by the font face handle.
	/// @param[in] texture_id The index of the texture within the layer to generate.
	bool GenerateTexture(const byte*& texture_data, Vector2i& texture_dimensions, int texture_id);
	/// Generates the geometry required to render a single character.
	/// @param[out] geometry An array of geometries this layer will write to. It must be at least as big as the number of textures in this layer.
	/// @param[in] character_code The character to generate geometry for.
	/// @param[in] position The position of the baseline.
	/// @param[in] colour The colour of the string.
	inline void GenerateGeometry(Geometry* geometry, const word character_code, const Vector2f& position, const Colourb& colour) const
	{
		CharacterMap::const_iterator iterator = characters.find(character_code);
		if (iterator == characters.end())
			return;

		const Character& character = (*iterator).second;

		// Generate the geometry for the character.
		std::vector< Vertex >& character_vertices = geometry[character.texture_index].GetVertices();
		std::vector< int >& character_indices = geometry[character.texture_index].GetIndices();

		character_vertices.resize(character_vertices.size() + 4);
		character_indices.resize(character_indices.size() + 6);
		GeometryUtilities::GenerateQuad(&character_vertices[0] + (character_vertices.size() - 4), &character_indices[0] + (character_indices.size() - 6), Vector2f(position.x + character.origin.x, position.y + character.origin.y), character.dimensions, colour, character.texcoords[0], character.texcoords[1], character_vertices.size() - 4);
	}

	/// Returns the effect used to generate the layer.
	/// @return The layer's effect.
	const FontEffect* GetFontEffect() const;

	/// Returns on the layer's textures.
	/// @param[in] index The index of the desired texture.
	/// @return The requested texture.
	const Texture* GetTexture(int index);
	/// Returns the number of textures employed by this layer.
	/// @return The number of used textures.
	int GetNumTextures() const;

	/// Returns the layer's colour.
	/// @return The layer's colour.
	const Colourb& GetColour() const;

private:
	struct Character
	{
		// The offset, in pixels, of the baseline from the start of this character's geometry.
		Vector2f origin;
		// The width and height, in pixels, of this character's geometry.
		Vector2f dimensions;
		// The texture coordinates for the character's geometry.
		Vector2f texcoords[2];

		// The texture this character renders from.
		int texture_index;
	};

	typedef std::map< word, Character > CharacterMap;
	typedef std::vector< Texture > TextureList;

	const FontFaceHandle* handle;
	FontEffect* effect;

	TextureLayout texture_layout;

	CharacterMap characters;
	TextureList textures;
	Colourb colour;
};

}
}

#endif
