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

#ifndef ROCKETCOREMATHTYPES_H
#define ROCKETCOREMATHTYPES_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Vector2.h>
#include <Rocket/Core/Vector3.h>
#include <Rocket/Core/Vector4.h>

namespace Rocket {
namespace Core {

// Define common Vector2 types.
typedef Vector2< int > Vector2i;
typedef Vector2< float > Vector2f;
ROCKETCORE_API Vector2i operator*(int lhs, const Vector2i& rhs);
ROCKETCORE_API Vector2f operator*(float lhs, const Vector2f& rhs);

// Define common Vector3 types.
typedef Vector3< int > Vector3i;
typedef Vector3< float > Vector3f;
ROCKETCORE_API Vector3i operator*(int lhs, const Vector3i& rhs);
ROCKETCORE_API Vector3f operator*(float lhs, const Vector3f& rhs);

// Define common Vector4 types.
typedef Vector4< int > Vector4i;
typedef Vector4< float > Vector4f;
ROCKETCORE_API Vector4i operator*(int lhs, const Vector4i& rhs);
ROCKETCORE_API Vector4f operator*(float lhs, const Vector4f& rhs);

}
}

#endif
