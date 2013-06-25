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

#ifndef ROCKETCORETYPES_H
#define ROCKETCORETYPES_H

// Define NULL as zero.
#if !defined NULL
#define NULL 0
#endif

#include <float.h>
#include <limits.h>
#include <string>
#include <map>
#include <set>
#include <vector>

#include <Rocket/Core/Platform.h>
#include <Rocket/Core/Debug.h>

namespace Rocket {
namespace Core {

// Define commonly used basic types.
typedef unsigned char byte;
typedef unsigned short word;
typedef double Time;
typedef float TimeDelta;
typedef unsigned int Hash;
typedef void* ScriptObject;

}
}

#ifdef ROCKET_PLATFORM_WIN32
typedef unsigned __int64 uint64_t;
#else
#include <inttypes.h>
#endif

#include <Rocket/Core/Colour.h>
#include <Rocket/Core/Vector2.h>
#include <Rocket/Core/String.h>

namespace Rocket {
namespace Core {

// Default colour types.
typedef Colour< float, 1 > Colourf;
typedef Colour< byte, 255 > Colourb;
typedef Vector2< float > Vector2f;
typedef Vector2< int > Vector2i;
	

class Element;
class Dictionary;

// Types for external interfaces.
typedef uintptr_t FileHandle;
typedef uintptr_t TextureHandle;
typedef uintptr_t CompiledGeometryHandle;
typedef uintptr_t DecoratorDataHandle;

// List of elements.
typedef std::vector< Element* > ElementList;
typedef std::set< String > PseudoClassList;
typedef std::set< String > PropertyNameList;
typedef std::set< String > AttributeNameList;
typedef Dictionary ElementAttributes;

}
}

#endif
