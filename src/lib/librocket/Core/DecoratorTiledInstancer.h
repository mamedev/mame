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

#ifndef ROCKETCOREDECORATORTILEDINSTANCER_H
#define ROCKETCOREDECORATORTILEDINSTANCER_H

#include <Rocket/Core/DecoratorInstancer.h>
#include "DecoratorTiled.h"

namespace Rocket {
namespace Core {

/**
	@author Peter Curry
 */

class DecoratorTiledInstancer : public DecoratorInstancer
{
public:
	virtual ~DecoratorTiledInstancer();

protected:
	/// Adds the property declarations for a tile.
	/// @param[in] name The name of the tile property.
	/// @param[in] register_repeat_modes If true, the tile will have the repeat modes registered.
	void RegisterTileProperty(const String& name, bool register_repeat_modes);
	/// Retrieves all the properties for a tile from the property dictionary.
	/// @param[out] tile The tile structure for storing the tile properties.
	/// @param[out] texture_name Holds the name of the texture declared for the tile (if one exists).
	/// @param[out] rcss_path The path of the RCSS file that generated the texture path.
	/// @param[in] properties The user-defined list of parameters for the decorator.
	/// @param[in] name The name of the tile to fetch the properties for.
	void GetTileProperties(DecoratorTiled::Tile& tile, String& texture_name, String& rcss_path, const PropertyDictionary& properties, const String& name);

private:
	// Loads a single texture coordinate value from the properties.
	void LoadTexCoord(const PropertyDictionary& properties, const String& name, float& tex_coord, bool& tex_coord_absolute);
};

}
}

#endif
