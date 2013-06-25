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
#include <Rocket/Core/Math.h>
#include <time.h>
#include <math.h>

namespace Rocket {
namespace Core {
namespace Math {

const float PI = 3.141592653f;
const float PI_BY_TWO = PI * 0.5f;
const float TWO_PI_BY_THREE = PI * 1.5f;
const float TWO_PI = PI * 2;

static const float FZERO = 0.0001f;

// Evaluates if a number is, or close to, zero.
ROCKETCORE_API bool IsZero(float value)
{
	return AbsoluteValue(value) < FZERO;
}

// Evaluates if two floating-point numbers are equal, or so similar that they could be considered
// so.
ROCKETCORE_API bool AreEqual(float value_0, float value_1)
{
	return IsZero(value_1 - value_0);
}

// Calculates the absolute value of a number.
ROCKETCORE_API float AbsoluteValue(float value)
{
	return fabsf(value);
}

// Calculates the cosine of an angle.
ROCKETCORE_API float Cos(float angle)
{
	return cosf(angle);
}

// Calculates the arc-cosine of an value.
ROCKETCORE_API float ACos(float value)
{
	return acos(value);
}

// Calculates the sine of an angle.
ROCKETCORE_API float Sin(float angle)
{
	return sin(angle);
}

// Calculates the arc-sine of an value.
ROCKETCORE_API float ASin(float angle)
{
	return asinf(angle);
}

// Calculates the tangent of an angle.
ROCKETCORE_API float Tan(float angle)
{
	return tanf(angle);
}

// Calculates the angle of a two-dimensional line.
ROCKETCORE_API float ATan2(float y, float x)
{
	return atan2f(y, x);
}

// Converts an angle from radians to degrees.
ROCKETCORE_API float RadiansToDegrees(float angle)
{
	return angle * (180.0f / PI);
}

// Converts an angle from degrees to radians.
ROCKETCORE_API float DegreesToRadians(float angle)
{
	return angle * (PI / 180.0f);
}

// Normalises and angle in radians
ROCKETCORE_API float NormaliseAngle(float angle)
{
	return fmodf(angle, TWO_PI);
}

// Calculates the square root of a value.
ROCKETCORE_API float SquareRoot(float value)
{
	return sqrtf(value);
}

// Rounds a floating-point value to the nearest integer.
ROCKETCORE_API int Round(float value)
{
	if (value > 0.0f)
		return RealToInteger(value + 0.5f);

	return RealToInteger(value - 0.5f);
}

// Rounds a floating-point value up to the nearest integer.
ROCKETCORE_API int RoundUp(float value)
{
	return RealToInteger(ceilf(value));
}

// Rounds a floating-point value down to the nearest integer.
ROCKETCORE_API int RoundDown(float value)
{
	return RealToInteger(floorf(value));
}

// Efficiently truncates a floating-point value into an integer.
ROCKETCORE_API int RealToInteger(float value)
{
#if defined(ROCKET_PLATFORM_WIN32) && !defined(__MINGW32__)
	int i;
	_asm
	{
		mov  eax, value;		// loaded mem to acc
		rcl  eax, 1;			// left shift acc to remove the sign
		mov  ebx, eax;			// save the acc
		mov  edx, 4278190080;	// clear reg edx;
		and  eax, edx;			// and acc to retrieve the exponent
		shr  eax, 24;
		sub  eax, 7fh;			// subtract 7fh(127) to get the actual power 
		mov  edx, eax;			// save acc val power
		mov  eax, ebx;			// retrieve from ebx
		rcl  eax, 8;			// trim the left 8 bits that contain the power
		mov  ebx, eax;			// store
		mov  ecx, 1fh;			// subtract 17 h
		sub  ecx, edx;
		mov  edx, 00000000h;
		cmp  ecx, 0;
		je   loop2;
		shr  eax, 1;
		or   eax, 80000000h;        
	loop1:
		shr  eax, 1;			// shift (total bits - power bits);
		sub  ecx, 1;
		add  edx, 1;
		cmp  ecx, 0;
		ja   loop1;
	loop2:
		mov  i, eax;

		// check sign +/- 
		mov  eax, value;
		and  eax, 80000000h;
		cmp  eax, 80000000h;
		je   putsign;
	}

	return i;

putsign:
	return -i;
#else
	return (int) value;
#endif
}

// Converts the given number to a power of two, rounding up if necessary.
ROCKETCORE_API int ToPowerOfTwo(int number)
{
	// Check if the number is already a power of two.
	if ((number & (number - 1)) == 0)
		return number;

	// Assuming 31 useful bits in an int here ... !
	for (int i = 31; i >= 0; i--)
	{
		if (number & (1 << i))
		{
			if (i == 31)
				return 1 << 31;
			else
				return 1 << (i + 1);
		}
	}

	return 0;
}

// Converts from a hexadecimal digit to decimal.
ROCKETCORE_API int HexToDecimal(char hex_digit)
{
	if (hex_digit >= '0' && hex_digit <= '9')
		return hex_digit - '0';
	else if (hex_digit >= 'a' && hex_digit <= 'f')
		return 10 + (hex_digit - 'a');
	else if (hex_digit >= 'A' && hex_digit <= 'F')
		return 10 + (hex_digit - 'A');

	return -1;
}

// Generates a random floating-point value between 0 and a user-specified value.
ROCKETCORE_API float RandomReal(float max_value)
{
	return (rand() / (float) RAND_MAX) * max_value;
}

// Generates a random integer value between 0 and a user-specified value.
ROCKETCORE_API int RandomInteger(int max_value)
{
	return (rand() % max_value);
}

// Generates a random boolean value, with equal chance of true or false.
ROCKETCORE_API bool RandomBool()
{
	return RandomInteger(2) == 1;
}

}
}
}
