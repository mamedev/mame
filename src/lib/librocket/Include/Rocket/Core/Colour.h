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

#ifndef ROCKETCORECOLOUR_H
#define ROCKETCORECOLOUR_H

#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

/**	
	Templated class for a four-component RGBA colour.

	@author Peter Curry
 */

template < typename ColourType, int AlphaDefault >
class Colour
{
public:
	/// Lightweight, non-initialising constructor.
	inline Colour();
	/// Initialising constructor.
	/// @param[in] red Initial red value of the colour.
	/// @param[in] green Initial green value of the colour.
	/// @param[in] blue Initial blue value of the colour.
	/// @param[in] alpha Initial alpha value of the colour.
	inline Colour(ColourType red, ColourType green, ColourType blue, ColourType alpha = AlphaDefault);

	/// Returns the sum of this colour and another. This does not saturate the channels.
	/// @param[in] rhs The colour to add this to.
	/// @return The sum of the two colours.
	inline Colour operator+(const Colour& rhs) const;
	/// Returns the result of subtracting another colour from this colour.
	/// @param[in] rhs The colour to subtract from this colour.
	/// @return The result of the subtraction.
	inline Colour operator-(const Colour& rhs) const;
	/// Returns the result of multiplying this colour by another.
	/// @param[in] rhs The colour to multiply by.
	/// @return The result of the multiplication.
	Colour operator*(const Colour& rhs) const;
	/// Returns the result of multiplying this colour component-wise by a scalar.
	/// @param[in] rhs The scalar value to multiply by.
	/// @return The result of the scale.
	inline Colour operator*(float rhs) const;
	/// Returns the result of dividing this colour component-wise by a scalar.
	/// @param[in] rhs The scalar value to divide by.
	/// @return The result of the scale.
	inline Colour operator/(float rhs) const;

	/// Adds another colour to this in-place. This does not saturate the channels.
	/// @param[in] rhs The colour to add.
	inline void operator+=(const Colour& rhs);
	/// Subtracts another colour from this in-place.
	/// @param[in] rhs The colour to subtract.
	inline void operator-=(const Colour& rhs);
	/// Multiplies this colour component-wise with another in-place.
	/// @param[in] rhs The colour to multiply by.
	/// @return This colour, post-operation.
	void operator*=(const Colour& rhs);
	/// Scales this colour component-wise in-place.
	/// @param[in] rhs The value to scale this colours's components by.
	inline void operator*=(float rhs);
	/// Scales this colour component-wise in-place by the inverse of a value.
	/// @param[in] rhs The value to divide this colour's components by.
	inline void operator/=(float rhs);

	/// Equality operator.
	/// @param[in] rhs The colour to compare this against.
	/// @return True if the two colours are equal, false otherwise.
	inline bool operator==(const Colour& rhs)	{ return red == rhs.red && green == rhs.green && blue == rhs.blue && alpha == rhs.alpha; }
	/// Inequality operator.
	/// @param[in] rhs The colour to compare this against.
	/// @return True if the two colours are not equal, false otherwise.
	inline bool operator!=(const Colour& rhs)	{ return red != rhs.red || green != rhs.green || blue != rhs.blue || alpha != rhs.alpha; }

	/// Auto-cast operator.
	/// @return A pointer to the first value.
	inline operator const ColourType*() const { return &red; }
	/// Constant auto-cast operator.
	/// @return A constant pointer to the first value.
	inline operator ColourType*() { return &red; }

	ColourType red, green, blue, alpha;
};

}
}

//#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

#include "Colour.inl"

}
}

#endif
