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
#include "DecoratorTiledBox.h"
#include "TextureResource.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/Geometry.h>

namespace Rocket {
namespace Core {

struct DecoratorTiledBoxData
{
	DecoratorTiledBoxData(Element* host_element)
	{
		for (int i = 0; i < 9; ++i)
			geometry[i] = new Geometry(host_element);
	}

	~DecoratorTiledBoxData()
	{
		for (int i = 0; i < 9; ++i)
			delete geometry[i];
	}

	Geometry* geometry[9];
};

DecoratorTiledBox::DecoratorTiledBox()
{
}

DecoratorTiledBox::~DecoratorTiledBox()
{
}

// Initialises the tiles for the decorator.
bool DecoratorTiledBox::Initialise(const Tile* _tiles, const String* _texture_names, const String* _rcss_paths)
{
	// Load the textures.
	for (int i = 0; i < 9; i++)
	{
		if (!_texture_names[i].Empty())
		{
			tiles[i] = _tiles[i];
			tiles[i].texture_index = LoadTexture(_texture_names[i], _rcss_paths[i]);
			if (tiles[i].texture_index < 0)
				return false;
		}
	}

	// If only one side of the left / right edges have been configured, then mirror the tile for the other side.
	if (tiles[LEFT_EDGE].texture_index == -1 && tiles[RIGHT_EDGE].texture_index > -1)
	{
		tiles[LEFT_EDGE] = tiles[RIGHT_EDGE];
		tiles[LEFT_EDGE].orientation = FLIP_HORIZONTAL;
	}
	else if (tiles[RIGHT_EDGE].texture_index == -1 && tiles[LEFT_EDGE].texture_index > -1)
	{
		tiles[RIGHT_EDGE] = tiles[LEFT_EDGE];
		tiles[RIGHT_EDGE].orientation = FLIP_HORIZONTAL;
	}
	else if (tiles[LEFT_EDGE].texture_index == -1 && tiles[RIGHT_EDGE].texture_index == -1)
		return false;

	// If only one side of the top / bottom edges have been configured, then mirror the tile for the other side.
	if (tiles[TOP_EDGE].texture_index == -1 && tiles[BOTTOM_EDGE].texture_index > -1)
	{
		tiles[TOP_EDGE] = tiles[BOTTOM_EDGE];
		tiles[TOP_EDGE].orientation = FLIP_VERTICAL;
	}
	else if (tiles[BOTTOM_EDGE].texture_index == -1 && tiles[TOP_EDGE].texture_index > -1)
	{
		tiles[BOTTOM_EDGE] = tiles[TOP_EDGE];
		tiles[BOTTOM_EDGE].orientation = FLIP_VERTICAL;
	}
	else if (tiles[TOP_EDGE].texture_index == -1 && tiles[BOTTOM_EDGE].texture_index == -1)
		return false;

	// Check that the centre tile has been specified.
	if (tiles[CENTRE].texture_index < 0)
		return false;

	return true;
}

// Called on a decorator to generate any required per-element data for a newly decorated element.
DecoratorDataHandle DecoratorTiledBox::GenerateElementData(Element* element)
{
	// Initialise the tiles for this element.
	for (int i = 0; i < 9; i++)
	{
		if (tiles[i].texture_index >= 0)
			tiles[i].CalculateDimensions(element, *GetTexture(tiles[i].texture_index));
	}

	Vector2f padded_size = element->GetBox().GetSize(Box::PADDING);

	// Calculate the size for the top row of tiles.
	Vector2f top_left_dimensions = tiles[TOP_LEFT_CORNER].GetDimensions(element);
	Vector2f top_dimensions = tiles[TOP_EDGE].GetDimensions(element);
	Vector2f top_right_dimensions = tiles[TOP_RIGHT_CORNER].GetDimensions(element);

	// Calculate the size for the bottom row of tiles.
	Vector2f bottom_left_dimensions = tiles[BOTTOM_LEFT_CORNER].GetDimensions(element);
	Vector2f bottom_dimensions = tiles[BOTTOM_EDGE].GetDimensions(element);
	Vector2f bottom_right_dimensions = tiles[BOTTOM_RIGHT_CORNER].GetDimensions(element);

	// The size of the left and right tiles.
	Vector2f left_dimensions = tiles[LEFT_EDGE].GetDimensions(element);
	Vector2f right_dimensions = tiles[RIGHT_EDGE].GetDimensions(element);

	// Scale the top corners down if appropriate. If they are scaled, then the left and right edges are also scaled
	// if they shared a width with their corner. Best solution? Don't know.
	if (padded_size.x < top_left_dimensions.x + top_right_dimensions.x)
	{
		float minimum_width = top_left_dimensions.x + top_right_dimensions.x;

		top_left_dimensions.x = padded_size.x * (top_left_dimensions.x / minimum_width);
		if (tiles[TOP_LEFT_CORNER].GetDimensions(element).x == tiles[LEFT_EDGE].GetDimensions(element).x)
			left_dimensions.x = top_left_dimensions.x;

		top_right_dimensions.x = padded_size.x * (top_right_dimensions.x / minimum_width);
		if (tiles[TOP_RIGHT_CORNER].GetDimensions(element).x == tiles[RIGHT_EDGE].GetDimensions(element).x)
			right_dimensions.x = top_right_dimensions.x;
	}

	// Scale the bottom corners down if appropriate. If they are scaled, then the left and right edges are also scaled
	// if they shared a width with their corner. Best solution? Don't know.
	if (padded_size.x < bottom_left_dimensions.x + bottom_right_dimensions.x)
	{
		float minimum_width = bottom_left_dimensions.x + bottom_right_dimensions.x;

		bottom_left_dimensions.x = padded_size.x * (bottom_left_dimensions.x / minimum_width);
		if (tiles[BOTTOM_LEFT_CORNER].GetDimensions(element).x == tiles[LEFT_EDGE].GetDimensions(element).x)
			left_dimensions.x = bottom_left_dimensions.x;

		bottom_right_dimensions.x = padded_size.x * (bottom_right_dimensions.x / minimum_width);
		if (tiles[BOTTOM_RIGHT_CORNER].GetDimensions(element).x == tiles[RIGHT_EDGE].GetDimensions(element).x)
			right_dimensions.x = bottom_right_dimensions.x;
	}

	// Scale the left corners down if appropriate. If they are scaled, then the top and bottom edges are also scaled
	// if they shared a width with their corner. Best solution? Don't know.
	if (padded_size.y < top_left_dimensions.y + bottom_left_dimensions.y)
	{
		float minimum_height = top_left_dimensions.y + bottom_left_dimensions.y;

		top_left_dimensions.y = padded_size.y * (top_left_dimensions.y / minimum_height);
		if (tiles[TOP_LEFT_CORNER].GetDimensions(element).y == tiles[TOP_EDGE].GetDimensions(element).y)
			top_dimensions.y = top_left_dimensions.y;

		bottom_left_dimensions.y = padded_size.y * (bottom_left_dimensions.y / minimum_height);
		if (tiles[BOTTOM_LEFT_CORNER].GetDimensions(element).y == tiles[BOTTOM_EDGE].GetDimensions(element).y)
			bottom_dimensions.y = bottom_left_dimensions.y;
	}

	// Scale the right corners down if appropriate. If they are scaled, then the top and bottom edges are also scaled
	// if they shared a width with their corner. Best solution? Don't know.
	if (padded_size.y < top_right_dimensions.y + bottom_right_dimensions.y)
	{
		float minimum_height = top_right_dimensions.y + bottom_right_dimensions.y;

		top_right_dimensions.y = padded_size.y * (top_right_dimensions.y / minimum_height);
		if (tiles[TOP_RIGHT_CORNER].GetDimensions(element).y == tiles[TOP_EDGE].GetDimensions(element).y)
			top_dimensions.y = top_right_dimensions.y;

		bottom_right_dimensions.y = padded_size.y * (bottom_right_dimensions.y / minimum_height);
		if (tiles[BOTTOM_RIGHT_CORNER].GetDimensions(element).y == tiles[BOTTOM_EDGE].GetDimensions(element).y)
			bottom_dimensions.y = bottom_right_dimensions.y;
	}

	DecoratorTiledBoxData* data = new DecoratorTiledBoxData(element);

	// Generate the geometry for the top-left tile.
	tiles[TOP_LEFT_CORNER].GenerateGeometry(data->geometry[tiles[TOP_LEFT_CORNER].texture_index]->GetVertices(),
											data->geometry[tiles[TOP_LEFT_CORNER].texture_index]->GetIndices(),
											element,
											Vector2f(0, 0),
											top_left_dimensions,
											top_left_dimensions);
	// Generate the geometry for the top edge tiles.
	tiles[TOP_EDGE].GenerateGeometry(data->geometry[tiles[TOP_EDGE].texture_index]->GetVertices(),
									 data->geometry[tiles[TOP_EDGE].texture_index]->GetIndices(),
									 element,
									 Vector2f(top_left_dimensions.x, 0),
									 Vector2f(padded_size.x - (top_left_dimensions.x + top_right_dimensions.x), top_dimensions.y),
									 top_dimensions);
	// Generate the geometry for the top-right tile.
	tiles[TOP_RIGHT_CORNER].GenerateGeometry(data->geometry[tiles[TOP_RIGHT_CORNER].texture_index]->GetVertices(),
											 data->geometry[tiles[TOP_RIGHT_CORNER].texture_index]->GetIndices(),
											 element,
											 Vector2f(padded_size.x - top_right_dimensions.x, 0),
											 top_right_dimensions,
											 top_right_dimensions);

	// Generate the geometry for the left side.
	tiles[LEFT_EDGE].GenerateGeometry(data->geometry[tiles[LEFT_EDGE].texture_index]->GetVertices(),
									  data->geometry[tiles[LEFT_EDGE].texture_index]->GetIndices(),
									  element,
									  Vector2f(0, top_left_dimensions.y),
									  Vector2f(left_dimensions.x, padded_size.y - (top_left_dimensions.y + bottom_left_dimensions.y)),
									  left_dimensions);

	// Generate the geometry for the right side.
	tiles[RIGHT_EDGE].GenerateGeometry(data->geometry[tiles[RIGHT_EDGE].texture_index]->GetVertices(),
									   data->geometry[tiles[RIGHT_EDGE].texture_index]->GetIndices(),
									   element,
									   Vector2f((padded_size.x - right_dimensions.x), top_right_dimensions.y),
									   Vector2f(right_dimensions.x, padded_size.y - (top_right_dimensions.y + bottom_right_dimensions.y)),
									   right_dimensions);

	// Generate the geometry for the bottom-left tile.
	tiles[BOTTOM_LEFT_CORNER].GenerateGeometry(data->geometry[tiles[BOTTOM_LEFT_CORNER].texture_index]->GetVertices(),
											   data->geometry[tiles[BOTTOM_LEFT_CORNER].texture_index]->GetIndices(),
											   element,
											   Vector2f(0, padded_size.y - bottom_left_dimensions.y),
											   bottom_left_dimensions,
											   bottom_left_dimensions);
	// Generate the geometry for the bottom edge tiles.
	tiles[BOTTOM_EDGE].GenerateGeometry(data->geometry[tiles[BOTTOM_EDGE].texture_index]->GetVertices(),
										data->geometry[tiles[BOTTOM_EDGE].texture_index]->GetIndices(),
										element,
										Vector2f(bottom_left_dimensions.x, padded_size.y - bottom_dimensions.y),
										Vector2f(padded_size.x - (bottom_left_dimensions.x + bottom_right_dimensions.x), bottom_dimensions.y),
										bottom_dimensions);
	// Generate the geometry for the bottom-right tile.
	tiles[BOTTOM_RIGHT_CORNER].GenerateGeometry(data->geometry[tiles[BOTTOM_RIGHT_CORNER].texture_index]->GetVertices(),
												data->geometry[tiles[BOTTOM_RIGHT_CORNER].texture_index]->GetIndices(),
												element,
												Vector2f(padded_size.x - bottom_right_dimensions.x, padded_size.y - bottom_right_dimensions.y),
												bottom_right_dimensions,
												bottom_right_dimensions);

	// Generate the centre geometry.
	if (tiles[CENTRE].texture_index >= 0)
	{
		Vector2f centre_dimensions = tiles[CENTRE].GetDimensions(element);
		Vector2f centre_surface_dimensions(padded_size.x - (left_dimensions.x + right_dimensions.x),
											  padded_size.y - (top_dimensions.y + bottom_dimensions.y));

		tiles[CENTRE].GenerateGeometry(data->geometry[tiles[CENTRE].texture_index]->GetVertices(),
									   data->geometry[tiles[CENTRE].texture_index]->GetIndices(),
									   element,
									   Vector2f(left_dimensions.x, top_dimensions.y),
									   centre_surface_dimensions,
									   centre_dimensions);
	}

	// Set the textures on the geometry.
	const Texture* texture = NULL;
	int texture_index = 0;
	while ((texture = GetTexture(texture_index)) != NULL)
		data->geometry[texture_index++]->SetTexture(texture);

	return reinterpret_cast<DecoratorDataHandle>(data);
}

// Called to release element data generated by this decorator.
void DecoratorTiledBox::ReleaseElementData(DecoratorDataHandle element_data)
{
	delete reinterpret_cast< DecoratorTiledBoxData* >(element_data);
}

// Called to render the decorator on an element.
void DecoratorTiledBox::RenderElement(Element* element, DecoratorDataHandle element_data)
{
	Vector2f translation = element->GetAbsoluteOffset(Box::PADDING);
	DecoratorTiledBoxData* data = reinterpret_cast< DecoratorTiledBoxData* >(element_data);

	for (int i = 0; i < 9; i++)
		data->geometry[i]->Render(translation);
}

}
}
