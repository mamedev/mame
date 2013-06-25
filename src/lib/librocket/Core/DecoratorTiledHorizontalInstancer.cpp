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
#include "DecoratorTiledHorizontalInstancer.h"
#include "DecoratorTiledHorizontal.h"

namespace Rocket {
namespace Core {

DecoratorTiledHorizontalInstancer::DecoratorTiledHorizontalInstancer()
{
	RegisterTileProperty("left-image", false);
	RegisterTileProperty("right-image", false);
	RegisterTileProperty("center-image", true);
}

DecoratorTiledHorizontalInstancer::~DecoratorTiledHorizontalInstancer()
{
}

// Instances a box decorator.
Decorator* DecoratorTiledHorizontalInstancer::InstanceDecorator(const String& ROCKET_UNUSED(name), const PropertyDictionary& properties)
{
	DecoratorTiled::Tile tiles[3];
	String texture_names[3];
	String rcss_paths[3];

	GetTileProperties(tiles[0], texture_names[0], rcss_paths[0], properties, "left-image");
	GetTileProperties(tiles[1], texture_names[1], rcss_paths[1], properties, "right-image");
	GetTileProperties(tiles[2], texture_names[2], rcss_paths[2], properties, "center-image");

	DecoratorTiledHorizontal* decorator = new DecoratorTiledHorizontal();
	if (decorator->Initialise(tiles, texture_names, rcss_paths))
		return decorator;

	decorator->RemoveReference();
	ReleaseDecorator(decorator);
	return NULL;
}

// Releases the given decorator.
void DecoratorTiledHorizontalInstancer::ReleaseDecorator(Decorator* decorator)
{
	delete decorator;
}

// Releases the instancer.
void DecoratorTiledHorizontalInstancer::Release()
{
	delete this;
}

}
}
