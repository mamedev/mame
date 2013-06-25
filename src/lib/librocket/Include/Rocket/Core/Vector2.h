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

#ifndef ROCKETCOREVECTOR2_H
#define ROCKETCOREVECTOR2_H

#include <Rocket/Core/Debug.h>
#include <Rocket/Core/Math.h>

namespace Rocket {
namespace Core {

/**
	Templated class for a generic two-component vector.
	@author Peter Curry
 */

template < typename Type >
class Vector2
{
	public:
		/// Lightweight, non-initialising constructor.
		inline Vector2();
		/// Initialising constructor.
		/// @param[in] x Initial x-value of the vector.
		/// @param[in] y Initial y-value of the vector.
		inline Vector2(Type x, Type y);

		/// Returns the magnitude of the vector.
		/// @return The computed magnitude.
		inline float Magnitude() const;
		/// Returns the squared magnitude of the vector.
		/// @return The computed squared magnitude.
		inline Type SquaredMagnitude() const;
		/// Generates a normalised vector from this vector.
		/// @return The normalised vector.
		inline Vector2 Normalise() const;

		/// Computes the dot-product between this vector and another.
		/// @param[in] rhs The other vector to use in the dot-product.
		/// @return The computed dot-product between the two vectors.
		inline Type DotProduct(const Vector2& rhs) const;

		/// Returns this vector rotated around the origin.
		/// @param[in] theta The angle to rotate by, in radians.
		/// @return The rotated vector.
		inline Vector2 Rotate(float theta) const;

		/// Returns the negation of this vector.
		/// @return The negation of this vector.
		inline Vector2 operator-() const;

		/// Returns the sum of this vector and another.
		/// @param[in] rhs The vector to add this to.
		/// @return The sum of the two vectors.
		inline Vector2 operator+(const Vector2& rhs) const;
		/// Returns the result of subtracting another vector from this vector.
		/// @param[in] rhs The vector to subtract from this vector.
		/// @return The result of the subtraction.
		inline Vector2 operator-(const Vector2& rhs) const;
		/// Returns the result of multiplying this vector by a scalar.
		/// @param[in] rhs The scalar value to multiply by.
		/// @return The result of the scale.
		inline Vector2 operator*(Type rhs) const;
		/// Returns the result of dividing this vector by a scalar.
		/// @param[in] rhs The scalar value to divide by.
		/// @return The result of the scale.
		inline Vector2 operator/(Type rhs) const;

		/// Adds another vector to this in-place.
		/// @param[in] rhs The vector to add.
		/// @return This vector, post-operation.
		inline Vector2& operator+=(const Vector2& rhs);
		/// Subtracts another vector from this in-place.
		/// @param[in] rhs The vector to subtract.
		/// @return This vector, post-operation.
		inline Vector2& operator-=(const Vector2& rhs);
		/// Scales this vector in-place.
		/// @param[in] rhs The value to scale this vector's components by.
		/// @return This vector, post-operation.
		inline Vector2& operator*=(const Type& rhs);
		/// Scales this vector in-place by the inverse of a value.
		/// @param[in] rhs The value to divide this vector's components by.
		/// @return This vector, post-operation.
		inline Vector2& operator/=(const Type& rhs);

		/// Equality operator.
		/// @param[in] rhs The vector to compare this against.
		/// @return True if the two vectors are equal, false otherwise.
		inline bool operator==(const Vector2& rhs) const;
		/// Inequality operator.
		/// @param[in] rhs The vector to compare this against.
		/// @return True if the two vectors are not equal, false otherwise.
		inline bool operator!=(const Vector2& rhs) const;

		/// Auto-cast operator.
		/// @return A pointer to the first value.
		inline operator const Type*() const;
		/// Constant auto-cast operator.
		/// @return A constant pointer to the first value.
		inline operator Type*();

		// The components of the vector.
		Type x;
		Type y;
};

#include <Rocket/Core/Vector2.inl>

}
}

#endif
