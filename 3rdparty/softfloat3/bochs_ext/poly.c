/*============================================================================
This source file is an extension to the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2b, written for Bochs (x86 achitecture simulator)
floating point emulation.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.
=============================================================================*/

/*============================================================================
 * Written for Bochs (x86 achitecture simulator) by
 *            Stanislav Shwartsman [sshwarts at sourceforge net]
 * ==========================================================================*/

#include <assert.h>

#include "../source/include/softfloat.h"

//                            2         3         4               n
// f(x) ~ C + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//         0    1         2         3         4               n
//
//          --       2k                --        2k+1
//   p(x) = >  C  * x           q(x) = >  C   * x
//          --  2k                     --  2k+1
//
//   f(x) ~ [ p(x) + x * q(x) ]
//

float128_t EvalPoly(float128_t x, const float128_t *arr, int n)
{
	float128_t r = arr[--n];

	do {
		r = f128_mulAdd(r, x, arr[--n]);
//      r = f128_mul(r, x);
//      r = f128_add(r, arr[--n]);

	} while (n > 0);

	return r;
}

//                  2         4         6         8               2n
// f(x) ~ C + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//         0    1         2         3         4               n
//
//          --       4k                --        4k+2
//   p(x) = >  C  * x           q(x) = >  C   * x
//          --  2k                     --  2k+1
//
//                    2
//   f(x) ~ [ p(x) + x * q(x) ]
//

float128_t EvenPoly(float128_t x, const float128_t *arr, int n)
{
	 return EvalPoly(f128_mul(x, x), arr, n);
}

//                        3         5         7         9               2n+1
// f(x) ~ (C * x) + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//          0         1         2         3         4               n
//                        2         4         6         8               2n
//      = x * [ C + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//               0    1         2         3         4               n
//
//          --       4k                --        4k+2
//   p(x) = >  C  * x           q(x) = >  C   * x
//          --  2k                     --  2k+1
//
//                        2
//   f(x) ~ x * [ p(x) + x * q(x) ]
//

float128_t OddPoly(float128_t x, const float128_t *arr, int n)
{
	 return f128_mul(x, EvenPoly(x, arr, n));
}

