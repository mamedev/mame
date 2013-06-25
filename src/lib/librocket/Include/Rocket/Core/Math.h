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

#ifndef ROCKETCOREMATH_H
#define ROCKETCOREMATH_H

#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {
namespace Math {

// The constant PI / 2.
extern ROCKETCORE_API const float PI_BY_TWO;
// The constant PI.
extern ROCKETCORE_API const float PI;
// The constant 2 * PI / 3.
extern ROCKETCORE_API const float TWO_PI_BY_THREE;
// The constant 2 * PI.
extern ROCKETCORE_API const float TWO_PI;


template < typename Type >
Type Max(Type a, Type b)
{
	return (a > b) ? a : b;
}

template< typename Type >
Type Min(Type a, Type b)
{
	return (a < b) ? a : b;
}

template < typename Type >
Type ClampLower(Type value, Type min)
{
	return (value < min) ? min : value;
}

template < typename Type >
Type ClampUpper(Type value, Type max)
{
	return (value > max) ? max: value;
}

template< typename Type >
Type Clamp(Type value, Type min, Type max)
{
	return (value < min) ? min : (value > max) ? max : value;
}

/// Evaluates if a number is, or close to, zero.
/// @param[in] value The number to compare to zero.
/// @return True if the number if zero or close to it, false otherwise.
ROCKETCORE_API bool IsZero(float value);
/// Evaluates if two floating-point numbers are equal, or so similar that they could be considered
/// so.
/// @param[in] value_0 The first number to compare.
/// @param[in] value_1 The second number to compare.
/// @return True if the numbers are similar or equal.
ROCKETCORE_API bool AreEqual(float value_0, float value_1);

/// Calculates the absolute value of a number.
/// @param[in] value The number of get the absolute value of.
/// @return The absolute value of the number.
ROCKETCORE_API float AbsoluteValue(float value);

/// Calculates the cosine of an angle.
/// @param[in] angle The angle to calculate the cosine of, in radians.
/// @return The cosine of the angle.
ROCKETCORE_API float Cos(float angle);
/// Calculates the arc-cosine of an value.
/// @param[in] angle The value to calculate the arc-cosine of.
/// @return The angle, in radians.
ROCKETCORE_API float ACos(float value);
/// Calculates the sine of an angle.
/// @param[in] angle The angle to calculate the sine of, in radians.
/// @return The sine of the angle.
ROCKETCORE_API float Sin(float angle);
/// Calculates the arc-sine of an value.
/// @param[in] angle The value to calculate the arc-sine of.
/// @return The angle, in radians.
ROCKETCORE_API float ASin(float angle);
/// Calculates the tangent of an angle.
/// @param[in] angle The angle to calculate the tangent of, in radians.
/// @return The tanget of the angle.
ROCKETCORE_API float Tan(float angle);
/// Calculates the angle of a two-dimensional line.
/// @param[in] y The y-component of the line.
/// @param[in] x The x-component of the line.
/// @return The angle of the line in radians.
ROCKETCORE_API float ATan2(float y, float x);

/// Converts an angle from radians to degrees.
/// @param[in] The angle, in radians.
/// @return The angle converted to degrees.
ROCKETCORE_API float RadiansToDegrees(float angle);
/// Converts an angle from degrees to radians.
/// @param[in] The angle, in degrees.
/// @return The angle converted to radians.
ROCKETCORE_API float DegreesToRadians(float angle);
/// Normalises and angle in radians
/// @param[in] The angle, in randians
/// @return The normalised angle
ROCKETCORE_API float NormaliseAngle(float angle);

/// Calculates the square root of a value.
/// @param[in] value The value to calculate the square root of. This must be above zero.
/// @return The square root of the value.
ROCKETCORE_API float SquareRoot(float value);

/// Rounds a floating-point value to the nearest integer.
/// @param[in] value The value to round.
/// @return The rounded integer.
ROCKETCORE_API int Round(float value);
/// Rounds a floating-point value up to the nearest integer.
/// @param[in] value The value to round.
/// @return The rounded integer.
ROCKETCORE_API int RoundUp(float value);
/// Rounds a floating-point value down to the nearest integer.
/// @param[in] value The value to round.
/// @return The rounded integer.
ROCKETCORE_API int RoundDown(float value);

/// Efficiently truncates a floating-point value into an integer.
/// @param[in] value The value to truncate.
/// @return The truncated value as a signed integer.
ROCKETCORE_API int RealToInteger(float value);

/// Converts a number to the nearest power of two, rounding up if necessary.
/// @param[in] value The value to convert to a power-of-two.
/// @return The smallest power of two that is as least as big as the input value.
ROCKETCORE_API int ToPowerOfTwo(int value);

/// Converts from the ASCII-representation of a hexadecimal digit to decimal.
/// @param[in] hex_digit The hexadecimal digit to convert to decimal.
/// @return The digit in decimal.
ROCKETCORE_API int HexToDecimal(char hex_digit);

/// Generates a random floating-point value between 0 and a user-specified value.
/// @param[in] max_value The limit to random value. The generated value will be guaranteed to be below this limit.
/// @return The random value.
ROCKETCORE_API float RandomReal(float max_value);
/// Generates a random integer value between 0 and a user-specified value.
/// @param[in] max_value The limit to random value. The generated value will be guaranteed to be below this limit.
/// @return The random value.
ROCKETCORE_API int RandomInteger(int max_value);
/// Generates a random boolean value, with equal chance of true or false.
/// @return The random value.
ROCKETCORE_API bool RandomBool();

}
}
}

#endif
