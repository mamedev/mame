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
#include "DecoratorTiled.h"
#include <Rocket/Core.h>

namespace Rocket {
namespace Core {

DecoratorTiled::DecoratorTiled()
{
}

DecoratorTiled::~DecoratorTiled()
{
}

static Vector2f oriented_texcoords[6][2] = {{Vector2f(0, 0), Vector2f(1, 1)},
													   {Vector2f(0, 1), Vector2f(1, 0)},
													   {Vector2f(1, 1), Vector2f(0, 0)},
													   {Vector2f(1, 0), Vector2f(0, 1)},
													   {Vector2f(1, 0), Vector2f(0, 1)},
													   {Vector2f(0, 1), Vector2f(1, 0)}};

DecoratorTiled::Tile::Tile()
{
	texture_index = -1;
	repeat_mode = STRETCH;
	orientation = ROTATE_0_CW;

	texcoords[0].x = 0;
	texcoords[0].y = 0;
	texcoords[1].x = 1;
	texcoords[1].y = 1;

	texcoords_absolute[0][0] = false;
	texcoords_absolute[0][1] = false;
	texcoords_absolute[1][0] = false;
	texcoords_absolute[1][1] = false;
}

// Calculates the tile's dimensions from the texture and texture coordinates.
void DecoratorTiled::Tile::CalculateDimensions(Element* element, const Texture& texture)
{
	RenderInterface* render_interface = element->GetRenderInterface();
	TileDataMap::iterator data_iterator = data.find(render_interface);
	if (data_iterator == data.end())
	{
		TileData new_data;
		Vector2i texture_dimensions = texture.GetDimensions(render_interface);

		for (int i = 0; i < 2; i++)
		{
			new_data.texcoords[i] = texcoords[i];

			if (texcoords_absolute[i][0] &&
				texture_dimensions.x > 0)
				new_data.texcoords[i].x /= texture_dimensions.x;
			if (texcoords_absolute[i][1] &&
				texture_dimensions.y > 0)
				new_data.texcoords[i].y /= texture_dimensions.y;
		}

		new_data.dimensions.x = Math::AbsoluteValue((new_data.texcoords[1].x * texture_dimensions.x) - (new_data.texcoords[0].x * texture_dimensions.x));
		new_data.dimensions.y = Math::AbsoluteValue((new_data.texcoords[1].y * texture_dimensions.y) - (new_data.texcoords[0].y * texture_dimensions.y));

		data[render_interface] = new_data;
	}
}

// Get this tile's dimensions.
Vector2f DecoratorTiled::Tile::GetDimensions(Element* element)
{
	RenderInterface* render_interface = element->GetRenderInterface();
	TileDataMap::iterator data_iterator = data.find(render_interface);
	if (data_iterator == data.end())
		return Vector2f(0, 0);

	return data_iterator->second.dimensions;
}

// Generates geometry to render this tile across a surface.
void DecoratorTiled::Tile::GenerateGeometry(std::vector< Vertex >& vertices, std::vector< int >& indices, Element* element, const Vector2f& surface_origin, const Vector2f& surface_dimensions, const Vector2f& tile_dimensions) const
{
	RenderInterface* render_interface = element->GetRenderInterface();
	TileDataMap::iterator data_iterator = data.find(render_interface);
	if (data_iterator == data.end())
		return;

	const TileData& data = data_iterator->second;

	int num_tiles[2];
	num_tiles[0] = 0;
	num_tiles[1] = 0;
	Vector2f final_tile_dimensions;

	// Generate the oriented texture coordinates for the tiles.
	Vector2f scaled_texcoords[3];
	for (int i = 0; i < 2; i++)
	{
		scaled_texcoords[i].x = data.texcoords[0].x + oriented_texcoords[orientation][i].x * (data.texcoords[1].x - data.texcoords[0].x);
		scaled_texcoords[i].y = data.texcoords[0].y + oriented_texcoords[orientation][i].y * (data.texcoords[1].y - data.texcoords[0].y);
	}
	scaled_texcoords[2] = scaled_texcoords[1];

	// Resize the dimensions (if necessary) to fit this tile's repeat mode.
	for (int i = 0; i < 2; i++)
	{
		if (surface_dimensions[i] <= 0)
			num_tiles[i] = 0;
		else
		{
			switch (repeat_mode)
			{
				// If the tile is stretched, we only need one quad.
				case STRETCH:
				{
					num_tiles[i] = 1;
					final_tile_dimensions[i] = surface_dimensions[i];
				}
				break;

				// If the tile is clamped, we only need one quad if the surface is smaller than the tile, or two if it's
				// larger (to take the last stretched pixel).
				case CLAMP_STRETCH:
				case CLAMP_TRUNCATE:
				{
					num_tiles[i] = surface_dimensions[i] > tile_dimensions[i] ? 2 : 1;
					if (num_tiles[i] == 1)
					{
						final_tile_dimensions[i] = surface_dimensions[i];
						if (repeat_mode == CLAMP_TRUNCATE)
							scaled_texcoords[1][i] -= (scaled_texcoords[1][i] - scaled_texcoords[0][i]) * (1.0f - (final_tile_dimensions[i] / tile_dimensions[i]));
					}
					else
						final_tile_dimensions[i] = surface_dimensions[i] - tile_dimensions[i];
				}
				break;

				case REPEAT_STRETCH:
				case REPEAT_TRUNCATE:
				{
					num_tiles[i] = Math::RealToInteger((surface_dimensions[i] + (tile_dimensions[i] - 1)) / tile_dimensions[i]);
					num_tiles[i] = Math::Max(0, num_tiles[i]);

					final_tile_dimensions[i] = surface_dimensions[i] - (num_tiles[i] - 1) * tile_dimensions[i];
					if (final_tile_dimensions[i] <= 0)
						final_tile_dimensions[i] = tile_dimensions[i];

					if (repeat_mode == REPEAT_TRUNCATE)
						scaled_texcoords[2][i] -= (scaled_texcoords[1][i] - scaled_texcoords[0][i]) * (1.0f - (final_tile_dimensions[i] / tile_dimensions[i]));
				}
				break;
			}
		}
	}

	// If any of the axes are zero or below, then we have a zero surface area and nothing to render.
	if (num_tiles[0] <= 0 || num_tiles[1] <= 0)
		return;

	// Resize the vertex and index arrays to fit the new geometry.
	int index_offset = (int) vertices.size();
	vertices.resize(vertices.size() + num_tiles[0] * num_tiles[1] * 4);
	Vertex* new_vertices = &vertices[0] + index_offset;

	size_t num_indices = indices.size();
	indices.resize(indices.size() + num_tiles[0] * num_tiles[1] * 6);
	int* new_indices = &indices[0] + num_indices;

	// Generate the vertices for the tiled surface.
	for (int y = 0; y < num_tiles[1]; y++)
	{
		Vector2f tile_position;
		tile_position.y = surface_origin.y + (float) tile_dimensions.y * y;

		Vector2f tile_size;
		tile_size.y = (float) (y < num_tiles[1] - 1 ? data.dimensions.y : final_tile_dimensions.y);

		// Squish the texture coordinates in the y if we're clamping and this is the last in a double-tile.
		Vector2f tile_texcoords[2];
		if (num_tiles[1] == 2 &&
			y == 1 &&
			(repeat_mode == CLAMP_STRETCH ||
			 repeat_mode == CLAMP_TRUNCATE))
		{
			tile_texcoords[0].y = scaled_texcoords[1].y;
			tile_texcoords[1].y = scaled_texcoords[1].y;
		}
		else
		{
			tile_texcoords[0].y = scaled_texcoords[0].y;
			// The last tile might have truncated texture coords
			if (y == num_tiles[1] - 1)
				tile_texcoords[1].y = scaled_texcoords[2].y;
			else
				tile_texcoords[1].y = scaled_texcoords[1].y;
		}

		for (int x = 0; x < num_tiles[0]; x++)
		{
			// Squish the texture coordinates in the x if we're clamping and this is the last in a double-tile.
			if (num_tiles[0] == 2 &&
				x == 1 &&
				(repeat_mode == CLAMP_STRETCH ||
				 repeat_mode == CLAMP_TRUNCATE))
			{
				tile_texcoords[0].x = scaled_texcoords[1].x;
				tile_texcoords[1].x = scaled_texcoords[1].x;
			}
			else
			{
				tile_texcoords[0].x = scaled_texcoords[0].x;
				// The last tile might have truncated texture coords
				if (x == num_tiles[0] - 1)
					tile_texcoords[1].x = scaled_texcoords[2].x;
				else
					tile_texcoords[1].x = scaled_texcoords[1].x;
			}

			tile_position.x = surface_origin.x + (float) tile_dimensions.x * x;
			tile_size.x = (float) (x < num_tiles[0] - 1 ? tile_dimensions.x : final_tile_dimensions.x);

			GeometryUtilities::GenerateQuad(new_vertices, new_indices, tile_position, tile_size, Colourb(255, 255, 255), tile_texcoords[0], tile_texcoords[1], index_offset);
			new_vertices += 4;
			new_indices += 6;
			index_offset += 4;
		}
	}
}

// Scales a tile dimensions by a fixed value along one axis.
void DecoratorTiled::ScaleTileDimensions(Vector2f& tile_dimensions, float axis_value, int axis)
{
	if (tile_dimensions[axis] != axis_value)
	{
		tile_dimensions[1 - axis] = tile_dimensions[1 - axis] * (axis_value / tile_dimensions[axis]);
		tile_dimensions[axis] = axis_value;
	}
}

}
}
