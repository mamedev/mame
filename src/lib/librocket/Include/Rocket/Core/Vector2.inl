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

// Default constructor.
template < typename Type >
Vector2< Type >::Vector2()
{
}

// Initialising constructor.
template < typename Type >
Vector2< Type >::Vector2(Type _x, Type _y)
{
	x = _x;
	y = _y;
}

// Returns the magnitude of the vector.
template < typename Type >
float Vector2< Type >::Magnitude() const
{
	float squared_magnitude = (float) SquaredMagnitude();
	if (Math::IsZero(squared_magnitude))
		return 0;

	return Math::SquareRoot(squared_magnitude);
}

// Returns the squared magnitude of the vector.
template < typename Type >
Type Vector2< Type >::SquaredMagnitude() const
{
	return x * x +
		   y * y;
}

// Generates a normalised vector from this vector.
template < typename Type >
Vector2< Type > Vector2< Type >::Normalise() const
{
	ROCKET_STATIC_ASSERT(sizeof(Type) == 0, Invalid_Operation);
	return *this;
}

template <>
ROCKETCORE_API Vector2< float > Vector2< float >::Normalise() const;

// Computes the dot-product between this vector and another.
template < typename Type >
Type Vector2< Type >::DotProduct(const Vector2< Type >& rhs) const
{
	return x * rhs.x +
		   y * rhs.y;
}

// Returns this vector rotated around the origin.
template < typename Type >
Vector2< Type > Vector2< Type >::Rotate(float theta) const
{
	ROCKET_STATIC_ASSERT(sizeof(Type) == 0, Invalid_Operation);
	return *this;
}

template <>
ROCKETCORE_API Vector2< float > Vector2< float >::Rotate(float) const;

// Returns the negation of this vector.
template < typename Type >
Vector2< Type > Vector2< Type >::operator-() const
{
	return Vector2(-x, -y);
}

// Returns the sum of this vector and another.
template < typename Type >
Vector2< Type > Vector2< Type >::operator+(const Vector2< Type >& rhs) const
{
	return Vector2< Type >(x + rhs.x, y + rhs.y);
}

// Returns the result of subtracting another vector from this vector.
template < typename Type >
Vector2< Type > Vector2< Type >::operator-(const Vector2< Type >& rhs) const
{
	return Vector2(x - rhs.x, y - rhs.y);
}

// Returns the result of multiplying this vector by a scalar.
template < typename Type >
Vector2< Type > Vector2< Type >::operator*(Type rhs) const
{
	return Vector2(x * rhs, y * rhs);
}

// Returns the result of dividing this vector by a scalar.
template < typename Type >
Vector2< Type > Vector2< Type >::operator/(Type rhs) const
{
	return Vector2(x / rhs, y / rhs);
}

// Adds another vector to this in-place.
template < typename Type >
Vector2< Type >& Vector2< Type >::operator+=(const Vector2& rhs)
{
	x += rhs.x;
	y += rhs.y;

	return *this;
}

// Subtracts another vector from this in-place.
template < typename Type >
Vector2< Type >& Vector2< Type >::operator-=(const Vector2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;

	return *this;
}

// Scales this vector in-place.
template < typename Type >
Vector2< Type >& Vector2< Type >::operator*=(const Type& rhs)
{
	x *= rhs;
	y *= rhs;

	return *this;
}

// Scales this vector in-place by the inverse of a value.
template < typename Type >
Vector2< Type >& Vector2< Type >::operator/=(const Type& rhs)
{
	x /= rhs;
	y /= rhs;

	return *this;
}

// Equality operator.
template < typename Type >
bool Vector2< Type >::operator==(const Vector2& rhs) const
{
	return (x == rhs.x && y == rhs.y);
}

// Inequality operator.
template < typename Type >
bool Vector2< Type >::operator!=(const Vector2& rhs) const
{
	return (x != rhs.x || y != rhs.y);
}

// Auto-cast operator.
template < typename Type >
Vector2< Type >::operator const Type*() const
{
	return &x;
}

// Constant auto-cast operator.
template < typename Type >
Vector2< Type >::operator Type*()
{
	return &x;
}
