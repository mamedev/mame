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
#include "DecoratorTiledVertical.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core/GeometryUtilities.h>
#include <Rocket/Core/Texture.h>

namespace Rocket {
namespace Core {

struct DecoratorTiledVerticalData
{
	DecoratorTiledVerticalData(Element* host_element)
	{
		for (int i = 0; i < 3; ++i)
			geometry[i] = new Geometry(host_element);
	}

	~DecoratorTiledVerticalData()
	{
		for (int i = 0; i < 3; ++i)
			delete geometry[i];
	}

	Geometry* geometry[3];
};

DecoratorTiledVertical::DecoratorTiledVertical()
{
}

DecoratorTiledVertical::~DecoratorTiledVertical()
{
}

// Initialises the tiles for the decorator.
bool DecoratorTiledVertical::Initialise(const Tile* _tiles, const String* _texture_names, const String* _rcss_paths)
{
	// Load the textures.
	for (int i = 0; i < 3; i++)
	{
		if (!_texture_names[i].Empty())
		{
			tiles[i] = _tiles[i];
			tiles[i].texture_index = LoadTexture(_texture_names[i], _rcss_paths[i]);
			if (tiles[i].texture_index < 0)
				return false;
		}
		else
			tiles[i].texture_index = -1;
	}

	// If only one side of the decorator has been configured, then mirror the texture for the other side.
	if (tiles[TOP].texture_index == -1 && tiles[BOTTOM].texture_index > -1)
	{
		tiles[TOP] = tiles[BOTTOM];
		tiles[TOP].orientation = FLIP_HORIZONTAL;
	}
	else if (tiles[BOTTOM].texture_index == -1 && tiles[TOP].texture_index > -1)
	{
		tiles[BOTTOM] = tiles[TOP];
		tiles[BOTTOM].orientation = FLIP_HORIZONTAL;
	}
	else if (tiles[TOP].texture_index == -1 && tiles[BOTTOM].texture_index == -1)
		return false;

	if (tiles[CENTRE].texture_index == -1)
		return false;

	return true;
}

// Called on a decorator to generate any required per-element data for a newly decorated element.
DecoratorDataHandle DecoratorTiledVertical::GenerateElementData(Element* element)
{
	// Initialise the tile for this element.
	for (int i = 0; i < 3; i++)
		tiles[i].CalculateDimensions(element, *GetTexture(tiles[i].texture_index));

	DecoratorTiledVerticalData* data = new DecoratorTiledVerticalData(element);

	Vector2f padded_size = element->GetBox().GetSize(Box::PADDING);

	Vector2f top_dimensions = tiles[TOP].GetDimensions(element);
	Vector2f bottom_dimensions = tiles[BOTTOM].GetDimensions(element);
	Vector2f centre_dimensions = tiles[CENTRE].GetDimensions(element);

	// Scale the tile sizes by the width scale.
	ScaleTileDimensions(top_dimensions, padded_size.x, 0);
	ScaleTileDimensions(bottom_dimensions, padded_size.x, 0);
	ScaleTileDimensions(centre_dimensions, padded_size.x, 0);

	// Shrink the y-sizes on the left and right tiles if necessary.
	if (padded_size.y < top_dimensions.y + bottom_dimensions.y)
	{
		float minimum_height = top_dimensions.y + bottom_dimensions.y;
		top_dimensions.y = padded_size.y * (top_dimensions.y / minimum_height);
		bottom_dimensions.y = padded_size.y * (bottom_dimensions.y / minimum_height);
	}

	// Generate the geometry for the left tile.
	tiles[TOP].GenerateGeometry(data->geometry[tiles[TOP].texture_index]->GetVertices(), data->geometry[tiles[TOP].texture_index]->GetIndices(), element, Vector2f(0, 0), top_dimensions, top_dimensions);
	// Generate the geometry for the centre tiles.
	tiles[CENTRE].GenerateGeometry(data->geometry[tiles[CENTRE].texture_index]->GetVertices(), data->geometry[tiles[CENTRE].texture_index]->GetIndices(), element, Vector2f(0, top_dimensions.y), Vector2f(centre_dimensions.x, padded_size.y - (top_dimensions.y + bottom_dimensions.y)), centre_dimensions);
	// Generate the geometry for the right tile.
	tiles[BOTTOM].GenerateGeometry(data->geometry[tiles[BOTTOM].texture_index]->GetVertices(), data->geometry[tiles[BOTTOM].texture_index]->GetIndices(), element, Vector2f(0, padded_size.y - bottom_dimensions.y), bottom_dimensions, bottom_dimensions);

	// Set the textures on the geometry.
	const Texture* texture = NULL;
	int texture_index = 0;
	while ((texture = GetTexture(texture_index)) != NULL)
		data->geometry[texture_index++]->SetTexture(texture);

	return reinterpret_cast<DecoratorDataHandle>(data);
}

// Called to release element data generated by this decorator.
void DecoratorTiledVertical::ReleaseElementData(DecoratorDataHandle element_data)
{
	delete reinterpret_cast< DecoratorTiledVerticalData* >(element_data);
}

// Called to render the decorator on an element.
void DecoratorTiledVertical::RenderElement(Element* element, DecoratorDataHandle element_data)
{
	Vector2f translation = element->GetAbsoluteOffset(Box::PADDING);
	DecoratorTiledVerticalData* data = reinterpret_cast< DecoratorTiledVerticalData* >(element_data);

	for (int i = 0; i < 3; i++)
		data->geometry[i]->Render(translation);
}

}
}
