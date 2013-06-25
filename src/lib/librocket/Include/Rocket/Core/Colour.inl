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

// Lightweight, non-initialising constructor.
template < typename ColourType, int AlphaDefault >
Colour< ColourType, AlphaDefault >::Colour()
{
}

// Initialising constructor.
template < typename ColourType, int AlphaDefault >
Colour< ColourType, AlphaDefault >::Colour(ColourType _red, ColourType _green, ColourType _blue, ColourType _alpha)
{
	red = _red;
	green = _green;
	blue = _blue;
	alpha = _alpha;
}

// Returns the sum of this colour and another. This does not saturate the channels.
template < typename ColourType, int AlphaDefault >
Colour< ColourType, AlphaDefault > Colour< ColourType, AlphaDefault >::operator+(const Colour< ColourType, AlphaDefault >& rhs) const
{
	return Colour< ColourType, AlphaDefault >(red + rhs.red, green + rhs.green, blue + rhs.blue, alpha + rhs.alpha);
}

// Returns the result of subtracting another colour from this colour.
template < typename ColourType, int AlphaDefault >
Colour< ColourType, AlphaDefault > Colour< ColourType, AlphaDefault >::operator-(const Colour< ColourType, AlphaDefault >& rhs) const
{
	return Colour< ColourType, AlphaDefault >(red - rhs.red, green - rhs.green, blue - rhs.blue, alpha - rhs.alpha);
}

// Returns the result of multiplying this colour component-wise by a scalar.
template < typename ColourType, int AlphaDefault >
Colour< ColourType, AlphaDefault > Colour< ColourType, AlphaDefault >::operator*(float rhs) const
{
	return Colour((ColourType) (red * rhs), (ColourType) (green * rhs), (ColourType) (blue * rhs), (ColourType) (alpha * rhs));
}

// Returns the result of dividing this colour component-wise by a scalar.
template < typename ColourType, int AlphaDefault >
Colour< ColourType, AlphaDefault > Colour< ColourType, AlphaDefault >::operator/(float rhs) const
{
	return Colour((ColourType) (red / rhs), (ColourType) (green / rhs), (ColourType) (blue / rhs), (ColourType) (alpha / rhs));
}

// Adds another colour to this in-place. This does not saturate the channels.
template < typename ColourType, int AlphaDefault >
void Colour< ColourType, AlphaDefault >::operator+=(const Colour& rhs)
{
	red += rhs.red;
	green += rhs.green;
	blue += rhs.blue;
	alpha += rhs.alpha;
}

// Subtracts another colour from this in-place.
template < typename ColourType, int AlphaDefault >
void Colour< ColourType, AlphaDefault >::operator-=(const Colour& rhs)
{
	red -= rhs.red;
	green -= rhs.green;
	blue -= rhs.blue;
	alpha -= rhs.alpha;
}

// Scales this colour component-wise in-place.
template < typename ColourType, int AlphaDefault >
void Colour< ColourType, AlphaDefault >::operator*=(float rhs)
{
	red = (ColourType)(red * rhs);
	green = (ColourType)(green * rhs);
	blue = (ColourType)(blue * rhs);
	alpha = (ColourType)(alpha * rhs);
}

// Scales this colour component-wise in-place by the inverse of a value.
template < typename ColourType, int AlphaDefault >
void Colour< ColourType, AlphaDefault >::operator/=(float rhs)
{
	*this *= (1.0f / rhs);
}

template < >
Colour< float, 1 > ROCKETCORE_API Colour< float, 1 >::operator*(const Colour< float, 1 >& rhs) const;

template < >
Colour< byte, 255 > ROCKETCORE_API Colour< byte, 255 >::operator*(const Colour< byte, 255 >& rhs) const;

template < >
void ROCKETCORE_API Colour< float, 1 >::operator*=(const Colour& rhs);

template < >
void ROCKETCORE_API Colour< byte, 255 >::operator*=(const Colour& rhs);
