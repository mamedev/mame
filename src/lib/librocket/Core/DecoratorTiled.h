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

#ifndef ROCKETCOREDECORATORTILED_H
#define ROCKETCOREDECORATORTILED_H

#include <Rocket/Core/Decorator.h>
#include <Rocket/Core/Vertex.h>

namespace Rocket {
namespace Core {

struct Texture;

/**
	Base class for tiled decorators.

	@author Peter Curry
 */

class DecoratorTiled : public Decorator
{
public:
	DecoratorTiled();
	virtual ~DecoratorTiled();

	/**
		Stores the repetition mode for a tile, for when it is rendered on a surface that is a
		different size than itself.
	 */
	enum TileRepeatMode
	{
		STRETCH = 0,			// Stretches a single tile across the required surface.
		CLAMP_STRETCH = 1,		// Clamps the tile to the upper left, stretching the tile inwards to fit into the element if it is too small.
		CLAMP_TRUNCATE = 2,		// Clamps the tile to the upper left, truncating the tile to fit into the element if it is too small.
		REPEAT_STRETCH = 3,		// Repeats the tile, stretching the final tile inwards.
		REPEAT_TRUNCATE = 4,	// Repeats the tile, truncating the final tile.
	};

	/**
		Stores the orientation of a tile.
	 */
	enum TileOrientation
	{
		ROTATE_0_CW = 0,		// Rotated zero degrees clockwise.
		ROTATE_90_CW = 1,		// Rotated 90 degrees clockwise.
		ROTATE_180_CW = 2,		// Rotated 180 degrees clockwise.
		ROTATE_270_CW = 3,		// Rotated 270 degrees clockwise.
		FLIP_HORIZONTAL = 4,	// Flipped horizontally.
		FLIP_VERTICAL = 5		// Flipped vertically.
	};

	/**
		Structure for storing the different tiles the tiled decorator uses internally over its
		surface.

		@author Peter Curry
	 */
	struct Tile
	{
		/// Constructs the tile with safe default values.
		Tile();

		/// Calculates the tile's dimensions from the texture and texture coordinates.
		void CalculateDimensions(Element* element, const Texture& texture);
		/// Get this tile's dimensions.
		Vector2f GetDimensions(Element* element);

		/// Generates geometry to render this tile across a surface.
		/// @param[out] vertices The array to store the generated vertex data.
		/// @param[out] indices The array to store the generated index data.
		/// @param[in] element The element hosting the decorator.
		/// @param[in] surface_origin The starting point of the first tile to generate.
		/// @param[in] surface_dimensions The dimensions of the surface to be tiled.
		/// @param[in] tile_dimensions The dimensions to render this tile at.
		void GenerateGeometry(std::vector< Vertex >& vertices, std::vector< int >& indices, Element* element, const Vector2f& surface_origin, const Vector2f& surface_dimensions, const Vector2f& tile_dimensions) const;

		struct TileData
		{
			Vector2f dimensions;
			Vector2f texcoords[2];
		};

		typedef std::map< RenderInterface*, TileData > TileDataMap;

		int texture_index;
		Vector2f texcoords[2];
		bool texcoords_absolute[2][2];

		mutable TileDataMap data;

		TileRepeatMode repeat_mode;
		TileOrientation orientation;
	};

protected:
	/// Scales a tile dimensions by a fixed value along one axis.
	/// @param tile_dimensions[in, out] The tile dimensions to scale.
	/// @param axis_value[in] The fixed value to scale against.
	/// @param axis[in] The axis to scale against; either 0 (for x) or 1 (for y).
	void ScaleTileDimensions(Vector2f& tile_dimensions, float axis_value, int axis);
};

}
}

#endif
