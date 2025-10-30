/*
 * fxpt_atan2.c
 *
 * Copyright (C) 2012, Xo Wang
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

// #include <stdint.h>

/**
 * Negative absolute value. Used to avoid undefined behavior for most negative
 * integer (see C99 standard 7.20.6.1.2 and footnote 265 for the description of
 * abs/labs/llabs behavior).
 *
 * @param i 16-bit signed integer
 * @return negative absolute value of i; defined for all values of i
 */
static inline short s16_nabs(const short j) {
//#if (((short)-1) >> 1) == ((short)-1)
//  // signed right shift sign-extends (arithmetic)
//  const short negSign =
//      ~(j >> 15);  // splat sign bit into all 16 and complement
//  // if j is positive (negSign is -1), xor will invert j and sub will add 1
//  // otherwise j is unchanged
//  return (j ^ negSign) - negSign;
//#else
  return (j < 0 ? j : -j);
//#endif
}

/**
 * Q15 (1.0.15 fixed point) multiplication. Various common rounding modes are in
 * the function definition for reference (and preference).
 *
 * @param j 16-bit signed integer representing -1 to (1 - (2**-15)), inclusive
 * @param k same format as j
 * @return product of j and k, in same format
 */
static inline short q15_mul(const short j, const short k) {
  const int intermediate = j * (int)k;
#if 0
  // don't round
  return intermediate >> 15;
#elif 0
  // biased rounding
  return (intermediate + 0x4000) >> 15;
#else
  // unbiased rounding
  return (intermediate + ((intermediate & 0x7FFF) == 0x4000 ? 0 : 0x4000)) >>
         15;
#endif
}

/**
 * Q15 (1.0.15 fixed point) division (non-saturating). Be careful when using
 * this function, as it does not behave well when the result is out-of-range.
 *
 * Value is not defined if numerator is greater than or equal to denominator.
 *
 * @param numer 16-bit signed integer representing -1 to (1 - (2**-15))
 * @param denom same format as numer; must be greater than numerator
 * @return numer / denom in same format as numer and denom
 */
static inline short q15_div(const short numer, const short denom) {
  return ((int)numer << 15) / denom;
}

/**
 * 16-bit fixed point four-quadrant arctangent. Given some Cartesian vector
 * (x, y), find the angle subtended by the vector and the positive x-axis.
 *
 * The value returned is in units of 1/65536ths of one turn. This allows the use
 * of the full 16-bit unsigned range to represent a turn. e.g. 0x0000 is 0
 * radians, 0x8000 is pi radians, and 0xFFFF is (65535 / 32768) * pi radians.
 *
 * Because the magnitude of the input vector does not change the angle it
 * represents, the inputs can be in any signed 16-bit fixed-point format.
 *
 * @param y y-coordinate in signed 16-bit
 * @param x x-coordinate in signed 16-bit
 * @return angle in (val / 32768) * pi radian increments from 0x0000 to 0xFFFF
 */
unsigned short fxpt_atan2(const short y, const short x) {
  if (x == y) {   // x/y or y/x would return -1 since 1 isn't representable
    if (y > 0) {  // 1/8
      return 8192;
    } else if (y < 0) {  // 5/8
      return 40960;
    } else {  // x = y = 0
      return 0;
    }
  }
  const short nabs_y = s16_nabs(y), nabs_x = s16_nabs(x);
  if (nabs_x < nabs_y) {  // octants 1, 4, 5, 8
    const short y_over_x = q15_div(y, x);
    const short correction = q15_mul(2847, s16_nabs(y_over_x));
    const short unrotated = q15_mul(11039 + correction, y_over_x);
    if (x > 0) {  // octants 1, 8
      return unrotated;
    } else {  // octants 4, 5
      return 32768 + unrotated;
    }
  } else {  // octants 2, 3, 6, 7
    const short x_over_y = q15_div(x, y);
    const short correction = q15_mul(2847, s16_nabs(x_over_y));
    const short unrotated = q15_mul(11039 + correction, x_over_y);
    if (y > 0) {  // octants 2, 3
      return 16384 - unrotated;
    } else {  // octants 6, 7
      return 49152 - unrotated;
    }
  }
}
